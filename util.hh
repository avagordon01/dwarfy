#include <span>
#include <cstddef>

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

