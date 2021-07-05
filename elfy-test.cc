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
#include "leb128.hh"

void do_stuff(std::span<std::byte> data) {
    elfy::elf e{data};
    if (false) {
    size_t i = 0;
    std::optional<elfy::section_header> sh;
    while ((sh = e.get_section_by_id(i++))) {
        std::cout << sh.value().name(e) << ", ";
    }
    std::cout << std::endl;
    }

    dwarfy::dwarf d{e};
    d.read_debug_info();
    std::cout << "all good" << std::endl;
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

        try {
            printf("processing file '%s':\n", filename);
            do_stuff(std::span<std::byte>{static_cast<std::byte*>(addr), len});
        } catch (std::runtime_error &e) {
            fprintf(stderr, "error processing file '%s': %s\n", filename, e.what());
        } catch (std::invalid_argument &e) {
            fprintf(stderr, "error processing file '%s': %s\n", filename, e.what());
        }

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
