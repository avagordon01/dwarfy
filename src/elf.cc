#include "elfy.hh"

namespace elfy {

std::string_view section_header::name(elf& e) const {
    std::span<std::byte> section_names = e.get_section_by_id(e.header.shstrndx).value().data(e);
    return std::string_view{reinterpret_cast<char*>(section_names.subspan(name_).data())};
}
std::span<std::byte> section_header::data(elf& e) const {
    return e.data.subspan(offset, size);
}

}
