#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <optional>
#include <iostream>
#include <span>
#include <string_view>

#include "serialise.hh"

namespace elfy {

struct elf_ident {
    uint8_t magic[4];
    uint8_t bitwidth;
    uint8_t endianness;
    uint8_t version;
    uint8_t abi;
    uint8_t abiversion;
    uint8_t padding[7];
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

    r.input_size_t = (i.bitwidth == 1 ? sizeof(uint32_t) : sizeof(uint64_t));
    r.input_endianness = (i.endianness == 1 ? std::endian::little : std::endian::big);
}

struct input_size_t {
    uint64_t data;
};

template<typename R>
void read(R& r, input_size_t& x) {
    x.data = from_bytes<uint64_t>(r.read_bytes(r.input_size_t));
}

enum class type : uint16_t {
    NONE = 0,
    REL = 1,
    EXEC = 2,
    DYN = 3,
    CORE = 4,
    LOOS = 0xFE00,
    HIOS = 0xFEFF,
    LOPROC = 0xFF00,
    HIPROC = 0xFFFF,
};

struct elf_header {
    enum type type;
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
};

template<typename R, typename T>
requires std::is_scalar_v<T>
void read(R& r, T& v) {
    v = from_bytes<T>(r.read_bytes(sizeof(v)));
}

template<typename T>
span_reader& operator&(span_reader &r, T& v) {
    read(r, v);
    return r;
}

template<typename R>
void read(R& r, elf_header& h) {
    r & h.type & h.machine & h.version & h.entry & h.phoff & h.shoff & h.flags & h.ehsize & h.phentsize & h.phnum & h.shentsize & h.shnum & h.shstrndx;
}

template<typename T>
struct program_header;
template<>
struct program_header<uint32_t> {
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
struct program_header<uint64_t> {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
};

struct elf;

template<typename T>
struct section_header {
    uint32_t name;
    uint32_t type;
    T flags;
    T addr;
    T offset;
    T size;
    uint32_t link;
    uint32_t info;
    T addralign;
    T entsize;

    std::string_view get_name(elf& e) const;
    std::span<std::byte> data(elf& e) const;
};

struct elf {

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

template<typename T>
std::string_view section_header<T>::get_name(elf& e) const {
    return std::string_view{reinterpret_cast<char*>(e.section_names.subspan(name).data())};
}
template<typename T>
std::span<std::byte> section_header<T>::data(elf& e) const {
    return e.data.subspan(offset, size);
}

}
