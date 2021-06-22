#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h> 
#include <cstdio>

#include "elfy.hh"
#include "dwarfy.hh"

void do_stuff(std::span<std::byte> data) {
    elfy::elf e{data};
    e.print_section_names<uint64_t>();
    if (e.get_section_by_name<uint64_t>(".debug_ranges")) {
        printf("debug ranges!\n");
    } else {
        printf("no debug ranges!\n");
    }
    printf("%lu program headers\n", e.program_headers.size());
    printf("%lu section headers\n", e.section_headers.size());

    //dwarfy::dwarf d{e};
}

int main(int argc, char *argv[]) {
    for (argv++, argc--; argc > 0; argv++, argc--) {
        char* filename = *argv;
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "%s: %s\n", filename, strerror(errno));
            return 1;
        }
        struct stat st;
        int err = fstat(fd, &st);
        if (err < 0) {
            fprintf(stderr, "%s: %s\n", filename, strerror(errno));
            return 1;
        }
        size_t len = st.st_size;
        void* addr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            fprintf(stderr, "%s: %s\n", filename, strerror(errno));
            return 1;
        }

        do_stuff(std::span<std::byte>{static_cast<std::byte*>(addr), len});

        err = munmap(addr, len);
        if (err < 0) {
            fprintf(stderr, "%s: %s\n", filename, strerror(errno));
            return 1;
        }
        err = close(fd);
        if (fd < 0) {
            fprintf(stderr, "%s: %s\n", filename, strerror(errno));
            return 1;
        }
    }
    return 0;
}
