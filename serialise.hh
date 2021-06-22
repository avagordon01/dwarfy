#pragma once

#include <span>
#include <cstddef>
#include <bit>

template<typename Y>
Y& from_bytes(std::span<std::byte> bytes) {
    return *std::bit_cast<Y*>(bytes.data());
    //return *reinterpret_cast<Y*>(bytes.data());
}

template<typename Y>
std::span<Y> span_from_bytes(std::span<std::byte> bytes, size_t num = 1) {
    return std::span{
        std::bit_cast<Y*>(bytes.data()),
        //reinterpret_cast<Y*>(bytes.data()),
        num
    };
}

struct span_reader {
    std::span<std::byte> data;

    span_reader(std::span<std::byte> data_):
        data(data_)
    {}

    std::span<std::byte> read_bytes(size_t size) {
        auto r = data.first(size);
        data = data.subspan(size);
        return r;
    }
};
