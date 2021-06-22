#pragma once

#include <cstddef>
#include <span>
#include <limits>

uint8_t high_bit(uint8_t x) {
    return x & 0x80;
}
uint8_t low_bits(uint8_t x) {
    return x & 0x7f;
}
uint8_t sign_bit(uint8_t x) {
    return x & 0x40;
}

struct uleb128 {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};
template<typename R>
void read(R& r, uleb128& v) {
    uint64_t result = 0;
    size_t shift = 0;
    size_t i = 0;
    uint8_t byte;
    do {
        byte = r.read_bytes(1).first();
        result |= (low_bits(byte) << shift);
        shift += 7;
    } while (high_bit(byte) != 0);
    v.data = result;
};
struct sleb128 {
    uint64_t data;
    operator uint64_t() const {
        return data;
    };
};
template<typename R>
void read(R& r, sleb128& v) {
    int64_t result = 0;
    size_t shift = 0;
    size_t size = std::numeric_limits<decltype(result)>::digits;
    size_t i = 0;
    uint8_t byte;
    do {
        byte = r.read_bytes(1).first();
        result |= (low_bits(byte) << shift);
        shift += 7;
    } while (high_bit(byte) != 0);
    if (shift < size && sign_bit(byte)) {
        result |= -(1 << shift);
    }
    v.data = result;
};
