#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <span>
#include <variant>
#include <optional>
#include <string>
#include <iostream>

struct elf_ident {
    uint8_t magic[4];
    uint8_t bitwidth;
    uint8_t endianness;
    uint8_t version;
    uint8_t abi;
    uint8_t abiversion;
    uint8_t padding[7];
};
template<typename T>
struct elf_header {
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
    } type;
    uint16_t machine;
    uint32_t version;
    T entry;
    T phoff;
    T shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
};
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

struct elfy {
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
    std::string_view get_name(elfy& e) {
        std::span<std::byte> section_names = e.section_headers[e.header.shstrndx].data(e);
        return std::string_view{reinterpret_cast<char*>(section_names.subspan(name).data())};
    }
    std::span<std::byte> data(elfy& e) {
        return e.data.subspan(offset, size);
    }
};
    template<typename Y>
    Y& from_bytes(std::span<std::byte> bytes) {
        return *reinterpret_cast<Y*>(bytes.data());
    }
    template<typename Y>
    std::span<Y> span_from_bytes(std::span<std::byte> bytes, size_t num = 1) {
        return std::span{
            reinterpret_cast<Y*>(bytes.data()),
            num
        };
    }

    using T = uint64_t;

    std::span<std::byte> data;
    elf_ident& ident;
    elf_header<T>& header;
    std::span<program_header<T>> program_headers;
    std::span<section_header<T>> section_headers;
    std::span<std::byte> section_names;
    elfy(std::span<std::byte> data_):
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
