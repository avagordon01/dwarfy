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
    if (true) {
    size_t i = 0;
    std::optional<elfy::section_header> sh;
    while ((sh = e.get_section_by_id(i++))) {
        std::cout << sh.value().name(e) << ", ";
    }
    std::cout << std::endl;
    }

    dwarfy::dwarf d{e};
    //d.read_debug_info();
    d.address_to_cu_arange();
    std::cout << "all good" << std::endl;
}

struct mmap_file {
    std::span<std::byte> data;
    std::string filename;
    int fd;
    mmap_file(std::string filename_):
        filename(filename_)
    {
        fd = open(filename.c_str(), O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error(filename + ": " + strerror(errno));
        }
        struct stat st;
        int err = fstat(fd, &st);
        if (err < 0) {
            throw std::runtime_error(filename + ": " + strerror(errno));
        }
        size_t len = st.st_size;
        void* addr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            throw std::runtime_error(filename + ": " + strerror(errno));
        }

        data = {static_cast<std::byte*>(addr), len};
    }

    ~mmap_file() {
        int err = munmap(data.data(), data.size());
        if (err < 0) {
            throw std::runtime_error(filename + ": " + strerror(errno));
        }
        err = close(fd);
        if (fd < 0) {
            throw std::runtime_error(filename + ": " + strerror(errno));
        }
    }
};

int main(int argc, char *argv[]) {
    for (argv++, argc--; argc > 0; argv++, argc--) {
        char* filename = *argv;
        mmap_file mf{filename};

        try {
            printf("processing file '%s':\n", filename);
            do_stuff(mf.data);
        } catch (std::runtime_error &e) {
            fprintf(stderr, "error processing file '%s': %s\n", filename, e.what());
        } catch (std::invalid_argument &e) {
            fprintf(stderr, "error processing file '%s': %s\n", filename, e.what());
        }
    }
    return 0;
}
