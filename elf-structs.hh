#include <cstdint>
#include <cstddef>
#include <span>
#include <string_view>

#include "serialise.hh"

struct elf_ident {
    uint8_t magic[4];
    uint8_t bitwidth;
    uint8_t endianness;
    uint8_t version;
    uint8_t abi;
    uint8_t abiversion;
    uint8_t padding[7];
};

struct input_size_t {
    uint64_t data;
    std::size_t size;

    input_size_t():
        data(0),
        size(sizeof(uint64_t))
    {}

    input_size_t(std::size_t size_):
        size(size_)
    {}

    size_t get_size() {
        return size;
    }
};

template<typename R>
void read(R& r, input_size_t& x) {
    x.data = from_bytes<uint64_t>(r.read_bytes(x.size));
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
template<typename R>
void read(R& r, enum type& v) {
    v = from_bytes<enum type>(r.read_bytes(sizeof(v)));
}

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

    elf_header() {}
};

template<typename R, typename T>
requires std::is_scalar_v<T>
void read(R& r, T& v) {
    v = from_bytes<T>(r.read_bytes(sizeof(v)));
}

template<typename R>
void read(R& r, elf_header& h) {
    read(r, h.type);
    read(r, h.machine);
    read(r, h.version);
    read(r, h.entry);
    read(r, h.phoff);
    read(r, h.shoff);
    read(r, h.flags);
    read(r, h.ehsize);
    read(r, h.phentsize);
    read(r, h.phnum);
    read(r, h.shentsize);
    read(r, h.shnum);
    read(r, h.shstrndx);
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
