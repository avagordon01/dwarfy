#pragma once

#include <span>
#include <cstddef>
#include <bit>
#include <algorithm>
#include <stdexcept>

struct span_reader {
    std::span<std::byte> data;
    size_t file_offset_size;
    size_t machine_segment_size;
    size_t machine_address_size;
    //XXX ELF doesn't distinguish between file bitwidth and machine bitwidth, but DWARF does
    std::endian file_endianness;

    span_reader(std::span<std::byte> data_):
        data(data_)
    {}

    void reset(std::span<std::byte> data_) {
        data = data_;
    }

    std::span<std::byte> read_bytes(size_t size) {
        auto r = data.first(size);
        data = data.subspan(size);
        return r;
    }
};

template<typename T>
T fix_endianness(T value, std::endian file_endianness = std::endian::little) {
    if constexpr (!std::is_scalar_v<T>) {
        return value;
    }
    if constexpr (sizeof(T) == 1) {
        return value;
    }
    if (file_endianness == std::endian::native) {
        return value;
    }
    using A = std::array<std::byte, sizeof(T)>;
    A array = std::bit_cast<A>(value);
    std::ranges::reverse(array);
    return std::bit_cast<T>(array);
}

template<typename R, typename T>
requires std::is_scalar_v<T>
void read(R& r, T& v) {
    v = *std::bit_cast<T*>(r.read_bytes(sizeof(v)).data());
    v = fix_endianness(v, r.file_endianness);
}

struct machine_address_size {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};

template<typename R>
void read(R& r, machine_address_size& x) {
    if (r.machine_address_size == 4) {
        uint32_t l;
        r & l;
        x.data = l;
    } else if (r.machine_address_size == 8) {
        uint64_t l;
        r & l;
        x.data = l;
    } else {
        throw std::runtime_error("unsupported (not 32 or 64 bit) bitwidth in file!");
    }
}

struct file_offset_size {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};

template<typename R>
void read(R& r, file_offset_size& x) {
    if (r.file_offset_size == 4) {
        uint32_t l;
        r & l;
        x.data = l;
    } else if (r.file_offset_size == 8) {
        uint64_t l;
        r & l;
        x.data = l;
    } else {
        throw std::runtime_error("unsupported (not 32 or 64 bit) bitwidth in file!");
    }
}

template<typename T>
span_reader& operator&(span_reader &r, T& v) {
    read(r, v);
    return r;
}
