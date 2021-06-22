#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <span>
#include <optional>
#include <iostream>

#include "serialise.hh"

namespace elfy {

struct elf {

#include "elf-structs.hh"

    using T = uint64_t;

    std::span<std::byte> data;
    span_reader reader;
    elf_ident ident;
    elf_header header;
    std::span<program_header<T>> program_headers;
    std::span<section_header<T>> section_headers;
    std::span<std::byte> section_names;
    elf(std::span<std::byte> data_):
        data(data_),
        reader(data)
    {
        read(reader, ident);
        read(reader, header);

        assert(header.phentsize == sizeof(program_header<T>));
        program_headers = span_from_bytes<program_header<T>>(data.subspan(header.phoff.data), header.phnum);

        assert(header.shentsize == sizeof(section_header<T>));
        section_headers = span_from_bytes<section_header<T>>(data.subspan(header.shoff.data), header.shnum);

        section_names = section_headers[header.shstrndx].data(*this);
    }
    template<typename T>
    section_header<T>* get_section_by_name(const std::string_view& key) {
        for (auto& sh: section_headers) {
            if (sh.get_name(*this) == key) {
                return &sh;
            }
        }
        return nullptr;
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
