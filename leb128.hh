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
    std::span<uint8_t> data;
    operator uint64_t() const {
        uint64_t result = 0;
        size_t shift = 0;
        size_t i = 0;
        uint8_t byte;
        do {
            byte = data[i++];
            result |= (low_bits(byte) << shift);
            shift += 7;
        } while (high_bit(byte) != 0);
        return result;
    };
};
struct sleb128 {
    std::span<uint8_t> data;
    operator int64_t() const {
        int64_t result = 0;
        size_t shift = 0;
        size_t size = std::numeric_limits<decltype(result)>::digits;
        size_t i = 0;
        uint8_t byte;
        do {
            byte = data[i++];
            result |= (low_bits(byte) << shift);
            shift += 7;
        } while (high_bit(byte) != 0);
        if (shift < size && sign_bit(byte)) {
            result |= -(1 << shift);
        }
        return result;
    };
};

void uleb_test() {
    {
        std::array<uint8_t, 3> c {0xe5, 0x8e, 0x26};
        uint64_t x = uleb128{{c}};
        assert(x == 624485);
    }

    {
        std::array<uint8_t, 3> c {0xc0, 0xbb, 0x78};
        int64_t x = sleb128{{c}};
        assert(x == -123456);
    }
}
