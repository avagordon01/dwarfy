#include "dwarfy.hh"

namespace dwarfy {

bool compilation_unit_header::iterator::operator==(compilation_unit_header::sentinel) {
    return debug_info_reader.data.empty();
}
compilation_unit_header::sentinel compilation_unit_header::iterator::end() const {
    return compilation_unit_header::sentinel{};
}
compilation_unit_header::iterator::iterator():
    debug_info_reader({})
{}
compilation_unit_header::iterator::iterator(std::span<std::byte> debug_info, std::endian initial_endianness):
    debug_info_reader(debug_info)
{
    debug_info_reader.file_endianness = initial_endianness;
    debug_info_reader & cu;
    next_cu = debug_info.subspan(cu.unit_length + cu.unit_length.size());
}
const compilation_unit_header compilation_unit_header::iterator::operator*() const {
    return cu;
}
compilation_unit_header::iterator& compilation_unit_header::iterator::operator++() {
    debug_info_reader.data = next_cu;
    if (*this != end()) {
        debug_info_reader & cu;
        next_cu = next_cu.subspan(cu.unit_length + cu.unit_length.size());
    }
    return *this;
}
compilation_unit_header::iterator compilation_unit_header::iterator::operator++(int) {
    compilation_unit_header::iterator ret = *this;
    this->operator++();
    return ret;
}
span_reader compilation_unit_header::iterator::die_reader() {
    return debug_info_reader;
}
debugging_information_entry::iterator compilation_unit_header::iterator::die_iter() {
    std::span<std::byte> debug_info;
    return debugging_information_entry::iterator{
        debug_info,
        debug_info,
        debug_info_reader.file_endianness
    };
}

compilation_unit_header::iterator compilation_unit_header::iterator::begin() const {
    return *this;
}
static_assert(std::input_iterator<compilation_unit_header::iterator>);

}
