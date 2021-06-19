#include <cstdint>
#include <cstddef>
#include <span>
#include <string_view>

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

    std::string_view get_name(elf& e) const {
        return std::string_view{reinterpret_cast<char*>(e.section_names.subspan(name).data())};
    }
    std::span<std::byte> data(elf& e) const {
        return e.data.subspan(offset, size);
    }
};
