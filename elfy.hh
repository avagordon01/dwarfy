#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <span>
#include <optional>
#include <iostream>

#include "util.hh"

namespace elfy {

struct elf {

#include "elf-structs.hh"

    using T = uint64_t;

    std::span<std::byte> data;
    elf_ident& ident;
    elf_header<T>& header;
    std::span<program_header<T>> program_headers;
    std::span<section_header<T>> section_headers;
    std::span<std::byte> section_names;
    elf(std::span<std::byte> data_):
        data(data_),
        ident(from_bytes<elf_ident>(data)),
        header(from_bytes<elf_header<T>>(data.subspan(sizeof(elf_ident))))
    {
        if (!(
            ident.magic[0] == 0x7f &&
            ident.magic[1] == 'E' &&
            ident.magic[2] == 'L' &&
            ident.magic[3] == 'F'
        )) {
            fprintf(stderr, "not an elf file!\n");
            return;
        }

        //TODO support big endianness
        assert(ident.endianness == 1);

        //TODO handle 32 bit
        assert(ident.bitwidth == 2);

        assert(header.ehsize == sizeof(elf_ident) + sizeof(elf_header<T>));

        assert(header.phentsize == sizeof(program_header<T>));
        program_headers = span_from_bytes<program_header<T>>(data.subspan(header.phoff), header.phnum);

        assert(header.shentsize == sizeof(section_header<T>));
        section_headers = span_from_bytes<section_header<T>>(data.subspan(header.shoff), header.shnum);

        section_names = section_headers[header.shstrndx].data(*this);
    }
    template<typename T>
    std::optional<section_header<T>> get_section_by_name(const std::string_view& key) {
        for (auto sh: section_headers) {
            if (sh.get_name(*this) == key) {
                return sh;
            }
        }
        return std::nullopt;
    }
    template<typename T>
    void print_section_names() {
        for (auto sh: section_headers) {
            std::cout << sh.get_name(*this) << ", ";
        }
        std::cout << std::endl;
    }
};

}
