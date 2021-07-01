#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <iostream>
#include <span>
#include <string_view>
#include <array>
#include <bit>
#include <algorithm>

#include "serialise.hh"

using namespace std::literals;

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
    std::size_t bitwidth() {
        if (bitwidth_ == 1) {
            return sizeof(uint32_t);
        } else if (bitwidth_ == 2) {
            return sizeof(uint64_t);
        } else {
            throw std::runtime_error("bad ELF bitwidth field, expected 1 or 2 (indicating 32 bit or 64 bit), got: " + std::to_string(bitwidth_));
        }
    }
public:
    std::endian endianness() {
        if (endianness_ == 1) {
            return std::endian::little;
        } else if (endianness_ == 2) {
            return std::endian::big;
        } else {
            throw std::runtime_error("bad ELF endian field, expected 1 or 2 (indicating little or big endian), got: " + std::to_string(endianness_));
        }
    }
};

template<typename R>
void read(R& r, elf_ident& i) {
    std::span<std::byte> magic = r.read_bytes(sizeof(i.magic));
    r & i.bitwidth_ & i.endianness_ & i.version & i.abi & i.abiversion;
    r.read_bytes(sizeof(i.padding));

    if (!(
        magic[0] == std::byte{0x7f} &&
        magic[1] == std::byte{'E'} &&
        magic[2] == std::byte{'L'} &&
        magic[3] == std::byte{'F'}
    )) {
        throw std::invalid_argument("bad ELF magic number, this is likely not an elf file!");
    }

    r.file_offset_size = i.bitwidth();
    r.file_endianness = i.endianness();

    if (i.version != 1) {
        throw std::runtime_error("bad ELF header version field, expected 1, got: " + std::to_string(i.version));
    }
}

class elf_header {
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    file_offset_size entry;
    file_offset_size phoff;
    file_offset_size shoff;
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
};

template<typename R>
void read(R& r, elf_header& h) {
    r & h.type & h.machine & h.version & h.entry & h.phoff & h.shoff & h.flags & h.ehsize & h.phentsize & h.phnum & h.shentsize & h.shnum & h.shstrndx;
    if (h.version != 1) {
        throw std::runtime_error("bad ELF header version field, expected 1, got: " + std::to_string(h.version));
    }
}

class program_header {
    uint32_t type;
    uint32_t flags;
    file_offset_size offset;
    file_offset_size vaddr;
    file_offset_size paddr;
    file_offset_size filesz;
    file_offset_size memsz;
    file_offset_size align;

    template<typename R>
    friend void read(R& r, program_header& h);
};

template<typename R>
void read(R& r, program_header& h) {
    r & h.type;
    if (r.file_offset_size == sizeof(uint64_t)) {
        r & h.flags;
    }
    r & h.offset & h.vaddr & h.paddr & h.filesz & h.memsz;
    if (r.file_offset_size == sizeof(uint32_t)) {
        r & h.flags;
    }
    r & h.align;
}

class elf;

class section_header {
    uint32_t name_;
    uint32_t type;
    file_offset_size flags;
    file_offset_size addr;
    file_offset_size offset;
    file_offset_size size;
    uint32_t link;
    uint32_t info;
    file_offset_size addralign;
    file_offset_size entsize;

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
    elf_header header;
public:
    elf_ident ident;
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
    std::span<std::byte> get_section_data_by_name(const std::string_view& key) {
        auto o = get_section_by_name(key);
        if (o) {
            return o.value().data(*this);
        } else {
            return {};
        }
    }
    section_header get_section_by_name_ex(const std::string_view& key) {
        auto o = get_section_by_name(key);
        if (o) {
            return o.value();
        } else {
            throw std::runtime_error("no elf section: "s + std::string(key));
        }
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
