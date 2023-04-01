#include <range/v3/iterator/basic_iterator.hpp>
#include <span>
#include <cstddef>
#include <iterator>

template<typename T>
struct byte_span_to_type_random_view {
private:
    struct cursor {
        std::byte* _ptr;
        size_t _stride;

        struct mixin: ranges::basic_mixin<cursor> {
            using ranges::basic_mixin<cursor>::basic_mixin;

            mixin(std::byte* _ptr, size_t _stride = sizeof(T)):
                mixin{cursor{_ptr, _stride}}
            {}
        };

        using value_type = T;
        T& read() const {
            //TODO make this generic
            return *reinterpret_cast<T*>(_ptr);
        }
        bool equal(const cursor& other) const {
            return _ptr == other._ptr;
        }
        void next() {
            _ptr += _stride;
        }
        void prev() {
            _ptr += _stride;
        }
        void advance(std::ptrdiff_t n) {
            _ptr += n * _stride;
        }
        std::ptrdiff_t distance_to(const cursor& other) const {
            return other._ptr - _ptr;
        }
    };

    using iterator = ranges::basic_iterator<cursor>;

    static_assert(std::random_access_iterator<iterator>);

public:
    std::span<std::byte> data;
    size_t stride;
    byte_span_to_type_random_view(std::span<std::byte> _data, size_t _count = -1, size_t _stride = sizeof(T)):
        data(_data),
        stride(_stride)
    {
        if (_count != -1) {
            assert(data.size() == _count * stride);
        }
    }
    auto begin() const { return iterator(data.data(), stride); }
    auto end() const { return iterator(data.data() + data.size(), stride); }
};

template<typename T>
struct byte_span_to_type_forward_view {
private:
    struct cursor {
        std::byte* _ptr;

        struct mixin: ranges::basic_mixin<cursor> {
            using ranges::basic_mixin<cursor>::basic_mixin;

            mixin(std::byte* _ptr):
                mixin{cursor{_ptr}}
            {}
        };

        using value_type = T;
        T& read() const {
            //TODO make this generic
            return *reinterpret_cast<T*>(_ptr);
        }
        bool equal(const cursor& other) const {
            return _ptr == other._ptr;
        }
        void next() {
            //TODO make this generic
            //_ptr = read().next_address;
            _ptr += 1;
        }
    };

    using iterator = ranges::basic_iterator<cursor>;

    struct sentinel {};
    friend bool operator!=(iterator i, sentinel) {
        //TODO make this generic
        //return read().is_last();
        return *i != 0;
    }

    static_assert(std::forward_iterator<iterator>);

public:
    std::span<std::byte> data;
    byte_span_to_type_forward_view(std::span<std::byte> _data):
        data(_data)
    {}
    auto begin() const { return iterator(data.data()); }
    auto end() const { return sentinel{}; }
};
