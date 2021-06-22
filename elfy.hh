#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <optional>
#include <iostream>
#include <span>
#include <string_view>
#include <array>
#include <bit>
#include <algorithm>

#include "serialise.hh"

namespace elfy {

class elf_ident {
    uint8_t magic[4];
    uint8_t bitwidth_;
    uint8_t endianness_;
    uint8_t version;
    uint8_t abi;
    uint8_t abiversion;
    uint8_t padding[7];

    template<typename R>
    friend void read(R& r, elf_ident& i);
    void check() {
        assert(bitwidth_ == 1 || bitwidth_ == 2);
        assert(endianness_ == 1 || endianness_ == 2);
        assert(version == 1);
    }
    std::endian endianness() {
        if (endianness_ == 1) {
            return std::endian::little;
        } else if (endianness_ == 2) {
            return std::endian::big;
        } else {
            assert(false && "bad ELF endian field");
        }
    }
    std::size_t bitwidth() {
        if (bitwidth_ == 1) {
            return sizeof(uint32_t);
        } else if (bitwidth_ == 2) {
            return sizeof(uint64_t);
        } else {
            assert(false && "bad ELF bitwidth field");
        }
    }
};

template<typename R>
void read(R& r, elf_ident& i) {
    i = from_bytes<elf_ident>(r.read_bytes(sizeof(elf_ident)));

    if (!(
        i.magic[0] == 0x7f &&
        i.magic[1] == 'E' &&
        i.magic[2] == 'L' &&
        i.magic[3] == 'F'
    )) {
        fprintf(stderr, "not an elf file!\n");
        abort();
    }

    r.input_size_t = i.bitwidth();
    r.input_endianness = i.endianness();
}

class elf_header {
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    input_size_t entry;
    input_size_t phoff;
    input_size_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;

    friend class elf;
    template<typename T>
    friend class section_header;
    template<typename R>
    friend void read(R& r, elf_header& h);
    void check() {
        assert(version == 1);
    }
};

template<typename R>
void read(R& r, elf_header& h) {
    r & h.type & h.machine & h.version & h.entry & h.phoff & h.shoff & h.flags & h.ehsize & h.phentsize & h.phnum & h.shentsize & h.shnum & h.shstrndx;
}

template<typename T>
class program_header;
template<>
class program_header<uint32_t> {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
};

template<>
class program_header<uint64_t> {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
};

class elf;

template<typename T>
class section_header {
    uint32_t name_;
    uint32_t type;
    T flags;
    T addr;
    T offset;
    T size;
    uint32_t link;
    uint32_t info;
    T addralign;
    T entsize;

public:
    std::string_view name(elf& e) const;
    std::span<std::byte> data(elf& e) const;
};

class elf {

    using T = uint64_t;

    std::span<std::byte> data;
    span_reader reader;
    elf_ident ident;
    elf_header header;
    std::span<program_header<T>> program_headers;
public:
    std::span<section_header<T>> section_headers;
    elf(std::span<std::byte> data_):
        data(data_),
        reader(data)
    {
        reader & ident & header;

        assert(header.phentsize == sizeof(program_header<T>));
        program_headers = span_from_bytes<program_header<T>>(data.subspan(header.phoff), header.phnum);

        assert(header.shentsize == sizeof(section_header<T>));
        section_headers = span_from_bytes<section_header<T>>(data.subspan(header.shoff), header.shnum);

    }
    template<typename T>
    section_header<T>* get_section_by_name(const std::string_view& key) {
        for (auto& sh: section_headers) {
            if (sh.name(*this) == key) {
                return &sh;
            }
        }
        return nullptr;
    }
    template<typename T>
    void print_section_names() {
        for (auto sh: section_headers) {
            std::cout << sh.name(*this) << ", ";
        }
        std::cout << std::endl;
    }

    template<typename T>
    friend class section_header;
};

template<typename T>
std::string_view section_header<T>::name(elf& e) const {
    std::span<std::byte> section_names = e.section_headers[e.header.shstrndx].data(e);
    return std::string_view{reinterpret_cast<char*>(section_names.subspan(name_).data())};
}
template<typename T>
std::span<std::byte> section_header<T>::data(elf& e) const {
    return e.data.subspan(offset, size);
}

}
