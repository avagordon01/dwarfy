#pragma once

#include <span>
#include <cstddef>
#include <bit>

struct span_reader {
    std::span<std::byte> data;
    size_t input_size_t;
    size_t input_address_size_t;
    std::endian input_endianness;

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
T fix_endianness(T value, std::endian input_endianness = std::endian::little) {
    if constexpr (!std::is_scalar_v<T>) {
        return value;
    }
    if constexpr (sizeof(T) == 1) {
        return value;
    }
    if (input_endianness == std::endian::native) {
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
    v = fix_endianness(v, r.input_endianness);
}

struct input_address_size_t {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};

template<typename R>
void read(R& r, input_address_size_t& x) {
    if (r.input_address_size_t == 4) {
        uint32_t l;
        r & l;
        x.data = l;
    } else if (r.input_address_size_t == 8) {
        uint64_t l;
        r & l;
        x.data = l;
    } else {
        throw std::runtime_error("unsupported (not 32 or 64 bit) bitwidth in file!");
    }
}

struct input_size_t {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};

template<typename R>
void read(R& r, input_size_t& x) {
    if (r.input_size_t == 4) {
        uint32_t l;
        r & l;
        x.data = l;
    } else if (r.input_size_t == 8) {
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
