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

class program_header {
    uint32_t type;
    uint32_t flags;
    input_size_t offset;
    input_size_t vaddr;
    input_size_t paddr;
    input_size_t filesz;
    input_size_t memsz;
    input_size_t align;

    template<typename R>
    friend void read(R& r, program_header& h);
};

template<typename R>
void read(R& r, program_header& h) {
    r & h.type;
    if (r.input_size_t == sizeof(uint64_t)) {
        r & h.flags;
    }
    r & h.offset & h.vaddr & h.paddr & h.filesz & h.memsz;
    if (r.input_size_t == sizeof(uint32_t)) {
        r & h.flags;
    }
    r & h.align;
}

class elf;

class section_header {
    uint32_t name_;
    uint32_t type;
    input_size_t flags;
    input_size_t addr;
    input_size_t offset;
    input_size_t size;
    uint32_t link;
    uint32_t info;
    input_size_t addralign;
    input_size_t entsize;

public:
    std::string_view name(elf& e) const;
    std::span<std::byte> data(elf& e) const;
    template<typename R>
    friend void read(R& r, section_header& h);
};

template<typename R>
void read(R& r, section_header& h) {
    r & h.name_ & h.type & h.flags & h.addr & h.offset & h.size & h.link & h.info & h.addralign & h.entsize;
}

class elf {
    std::span<std::byte> data;
    span_reader reader;
    elf_ident ident;
    elf_header header;
public:
    elf(std::span<std::byte> data_):
        data(data_),
        reader(data)
    {
        reader & ident & header;
    }
    std::optional<program_header> get_program_by_id(size_t id) {
        if (id >= header.phnum) {
            return std::nullopt;
        }
        program_header ph;
        reader.reset(data.subspan(header.phoff + header.phentsize * id));
        reader & ph;
        return ph;
    }
    std::optional<section_header> get_section_by_id(size_t id) {
        if (id >= header.shnum) {
            return std::nullopt;
        }
        section_header sh;
        reader.reset(data.subspan(header.shoff + header.shentsize * id));
        reader & sh;
        return sh;
    }
    std::optional<section_header> get_section_by_name(const std::string_view& key) {
        for (size_t i = 0; i < header.shnum; i++) {
            section_header sh = get_section_by_id(i).value();
            if (sh.name(*this) == key) {
                return sh;
            }
        }
        return std::nullopt;
    }

    friend class section_header;
};

std::string_view section_header::name(elf& e) const {
    std::span<std::byte> section_names = e.get_section_by_id(e.header.shstrndx).value().data(e);
    return std::string_view{reinterpret_cast<char*>(section_names.subspan(name_).data())};
}
std::span<std::byte> section_header::data(elf& e) const {
    return e.data.subspan(offset, size);
}

}
