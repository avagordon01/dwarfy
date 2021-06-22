#include <array>
#include <cstddef>
#include <bit>
#include <algorithm>

template<typename T>
T fix_endianness(T value, std::endian input_endianness = std::endian::little) {
    if constexpr (!std::is_scalar_v<T>) {
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

void endianness_test() {
    uint64_t x = 1;
    printf("%016lx\n", x);
    printf("%016lx\n", fix_endianness(x, std::endian::big));
}
