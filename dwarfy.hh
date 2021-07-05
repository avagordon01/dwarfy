#pragma once

#include <cstdint>
#include <cstdlib>
#include <span>
#include <optional>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "elfy.hh"
#include "leb128.hh"
#include "serialise.hh"

namespace dwarfy {

using std::to_string;

#include "dwarf-enums.hh"

template<typename R>
void read(R &r, enum dw_tag& tag) {
    uleb128 v;
    r & v;
    tag = static_cast<dw_tag>(static_cast<uint64_t>(v));
}
std::string to_string(enum dw_tag tag) {
    auto it = map_tag_to_string.find(tag);
    if (it != map_tag_to_string.end()) {
        return it->second;
    } else {
        if (static_cast<size_t>(tag) >= static_cast<size_t>(dw_tag::lo_user) &&
            static_cast<size_t>(tag) <= static_cast<size_t>(dw_tag::hi_user)) {
            return "vendor specific dw_tag: " + to_string(static_cast<uint64_t>(tag));
        } else {
            return "unknown dw_tag: " + to_string(static_cast<uint64_t>(tag));
        }
    }
};

template<typename R>
void read(R &r, enum dw_at& attr) {
    uleb128 v;
    r & v;
    attr = static_cast<dw_at>(static_cast<uint64_t>(v));
}
std::string to_string(enum dw_at attr) {
    auto it = map_at_to_string.find(attr);
    if (it != map_at_to_string.end()) {
        return it->second;
    } else {
        if (static_cast<size_t>(attr) >= static_cast<size_t>(dw_at::lo_user) &&
            static_cast<size_t>(attr) <= static_cast<size_t>(dw_at::hi_user)) {
            return "vendor specific dw_at: " + to_string(static_cast<uint64_t>(attr));
        } else {
            return "unknown dw_at: " + to_string(static_cast<uint64_t>(attr));
        }
    }
};

template<typename R>
void read(R &r, enum dw_form& form) {
    uleb128 v;
    r & v;
    form = static_cast<dw_form>(static_cast<uint64_t>(v));
}
std::string to_string(enum dw_form form) {
    auto it = map_form_to_string.find(form);
    if (it != map_form_to_string.end()) {
        return it->second;
    } else {
        return "unknown dw_form: " + to_string(static_cast<uint64_t>(form));
    }
};

std::string to_string(std::span<std::byte> bytes) {
    std::stringstream stream;
    stream << "<";
    if (bytes.data() == nullptr) {
        stream << "nullptr";
    } else {
        stream << bytes.size() << " bytes";
        if (bytes.size() > 0) {
            stream << " ";
        }
        stream << std::hex;
        stream << std::setfill('0');
        for (size_t i = 0; i < bytes.size(); i++) {
            stream << std::setw(2);
            stream << static_cast<uint16_t>(bytes[i]);
        }
    }
    stream << ">";
    return stream.str();
};

enum class dw_children : uint8_t {
    no = 0x00,
    yes = 0x01,
};

struct initial_length {
    uint64_t length;
    size_t read_bytes;
    operator uint64_t() {
        return length;
    }
    size_t size() {
        return read_bytes;
    }
};

template<typename R>
void read(R& r, initial_length& i) {
    uint32_t l;
    r & l;
    if (l == 0xffffffffUL) {
        r.file_offset_size = 8;
        r & i.length;
        i.read_bytes = 12;
    } else if (l < 0xfffffff0UL) {
        r.file_offset_size = 4;
        i.length = l;
        i.read_bytes = 4;
    } else {
        throw std::runtime_error("bad DWARF initial length field, expected ==0xffffffff or <0xfffffff0, got: " + to_string(l));
    }
    //FIXME what about mixed bitwidths in the same file?
};

struct attribute {
    dw_at name;
    dw_form form;
    std::span<std::byte> data;
    bool is_last() {
        return static_cast<uint64_t>(name) == 0 && static_cast<uint64_t>(form) == 0;
    }
};
std::span<std::byte> read_form(span_reader &ir, span_reader &ar, dw_form form);
void read(span_reader &ir, span_reader &ar, attribute& a) {
    ar & a.name & a.form;
    if (!a.is_last()) {
        a.data = read_form(ir, ar, a.form);
    }
}
std::span<std::byte> read_form(span_reader &ir, span_reader &ar, dw_form form) {
    switch (form) {
        case dw_form::addr:
            {
                machine_address_size offset;
                ir & offset;
                return {};//TODO ???
            }
        case dw_form::block2:
            {
                uint16_t l;
                ir & l;
                return ir.read_bytes(l);
            }
        case dw_form::block4:
            {
                uint32_t l;
                ir & l;
                return ir.read_bytes(l);
            }
        case dw_form::data2:
            {
                return ir.read_bytes(2);
            }
        case dw_form::data4:
            {
                return ir.read_bytes(4);
            }
        case dw_form::data8:
            {
                return ir.read_bytes(8);
            }
        case dw_form::string:
            {
                size_t i;
                for (i = 0; i < ir.data.size(); i++) {
                    if (ir.data[i] == std::byte{0}) {
                        break;
                    }
                }
                return ir.read_bytes(i + 1);
            }
        case dw_form::block:
            {
                uleb128 l;
                ir & l;
                return ir.read_bytes(l);
            }
        case dw_form::block1:
            {
                uint8_t l;
                ir & l;
                return ir.read_bytes(l);
            }
        case dw_form::data1:
            {
                return ir.read_bytes(1);
            }
        case dw_form::flag:
            {
                uint8_t flag;
                ir & flag;
                return {};
            }
        case dw_form::sdata:
            {
                sleb128 l;
                ir & l;
                return {};
            }
        case dw_form::strp:
            {
                file_offset_size offset;
                ir & offset;
                return {};

                //TODO
                std::span<std::byte> string = {};//debug_str;
                string = string.subspan(offset);
                size_t i;
                for (i = 0; i < string.size(); i++) {
                    if (string[i] == std::byte{0}) {
                        break;
                    }
                }
                string = string.first(i);
                return string;
            }
        case dw_form::udata:
            {
                uleb128 l;
                ir & l;
                return {};
            }
        case dw_form::ref_addr:
            {
                file_offset_size offset;
                ir & offset;
                return {};//TODO {debug_info.offset() + offset};
            }
        case dw_form::ref1:
            {
                uint8_t l;
                ir & l;
                return {};//TODO {cu.offset() + l};
            }
        case dw_form::ref2:
            {
                uint16_t l;
                ir & l;
                return {};//TODO {cu.offset() + l};
            }
        case dw_form::ref4:
            {
                uint32_t l;
                ir & l;
                return {};//TODO {cu.offset() + l};
            }
        case dw_form::ref8:
            {
                uint64_t l;
                ir & l;
                return {};//TODO {cu.offset() + l};
            }
        case dw_form::ref_udata:
            {
                uleb128 l;
                ir & l;
                return {};//TODO {cu.offset() + l};
            }
        case dw_form::indirect:
            {
                uleb128 v;
                ir & v;
                form = static_cast<dw_form>(static_cast<uint64_t>(v));
                return read_form(ir, ar, form);
            }
        case dw_form::sec_offset:
            {
                file_offset_size offset;
                ir & offset;
                return {};//TODO {section.offset() + offset};
            }
        case dw_form::exprloc:
            {
                uleb128 l;
                ir & l;
                return ir.read_bytes(l);
            }
        case dw_form::flag_present:
            {
                return {};
            }
        case dw_form::ref_sig8:
            {
                uint64_t sig;
                ir & sig;
                return {};//TODO debug_types.get_type_by_signature(sig);
            }
    }
}
std::string to_string(attribute attr) {
    return to_string(attr.name) + ":" + to_string(attr.form) + " " + to_string(attr.data);
}

struct debugging_information_entry {
    uleb128 abbrev_code;
    bool is_last() {
        return abbrev_code == 0;
    }
};
template<typename R>
void read(R &r, debugging_information_entry& die) {
    r & die.abbrev_code;
}

struct type_unit_header {
    initial_length unit_length;
    uint16_t version;
    file_offset_size debug_abbrev_offset;
    uint8_t address_size;
    uint64_t type_signature;
    file_offset_size type_offset;
};

template<typename R>
void read(R &r, type_unit_header& tu) {
    r & tu.unit_length & tu.version & tu.debug_abbrev_offset & tu.address_size & tu.type_signature & tu.type_offset;
    r.machine_address_size = tu.address_size;
    if (tu.version < 2 || tu.version > 5) {
        throw std::runtime_error("unsupported DWARF version, expected 2 <= version <= 5, got: " + to_string(tu.version));
    }
}

struct compilation_unit_header {
    initial_length unit_length;
    uint16_t version;
    file_offset_size debug_abbrev_offset;
    uint8_t address_size;

    class sentinel {};
    class iterator;
};
class compilation_unit_header::iterator {
    span_reader debug_info_reader;
    std::span<std::byte> next_cu;
    compilation_unit_header cu;
public:
    using iterator_concept  = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = compilation_unit_header;
    using pointer           = value_type*;
    using reference         = value_type&;
    bool operator==(sentinel) {
        return debug_info_reader.data.empty();
    }
    sentinel end() const {
        return sentinel{};
    }
    iterator():
        debug_info_reader({})
    {}
    iterator(std::span<std::byte> debug_info, std::endian initial_endianness):
        debug_info_reader(debug_info)
    {
        debug_info_reader.file_endianness = initial_endianness;
        debug_info_reader & cu;
        next_cu = debug_info.subspan(cu.unit_length + cu.unit_length.size());
    }
    const compilation_unit_header operator*() const {
        return cu;
    }
    iterator& operator++() {
        debug_info_reader.data = next_cu;
        if (*this != end()) {
            debug_info_reader & cu;
            next_cu = next_cu.subspan(cu.unit_length + cu.unit_length.size());
        }
        return *this;
    }
    iterator operator++(int) {
        iterator ret = *this;
        this->operator++();
        return ret;
    }
    span_reader die_reader() {
        return debug_info_reader;
    }

    iterator begin() const {
        return *this;
    }
};
static_assert(std::input_iterator<compilation_unit_header::iterator>);
template<typename R>
void read(R &r, compilation_unit_header& cu) {
    r & cu.unit_length & cu.version & cu.debug_abbrev_offset & cu.address_size;
    r.machine_address_size = cu.address_size;
    if (cu.version < 2 || cu.version > 5) {
        throw std::runtime_error("unsupported DWARF version, expected 2 <= version <= 5, got: " + to_string(cu.version));
    }
}

struct debug_abbrev_entry {
    uleb128 abbrev_code;
    dw_tag tag;
    dw_children debug_info_sibling;
    bool is_last() {
        return abbrev_code == 0;
    }
};
template<typename R>
void read(R &r, debug_abbrev_entry& dae) {
    r & dae.abbrev_code;
    if (!dae.is_last()) {
        r & dae.tag & dae.debug_info_sibling;
    }
}

struct dwarf {

    elfy::elf elf;

    std::span<std::byte> debug_abbrev;
    std::span<std::byte> debug_addr;
    std::span<std::byte> debug_aranges;
    std::span<std::byte> debug_frame;
    std::span<std::byte> debug_info;
    std::span<std::byte> debug_line;
    std::span<std::byte> debug_line_str;
    std::span<std::byte> debug_loc;
    std::span<std::byte> debug_loclists;
    std::span<std::byte> debug_macinfo;
    std::span<std::byte> debug_macro;
    std::span<std::byte> debug_names;
    std::span<std::byte> debug_pubnames;
    std::span<std::byte> debug_pubtypes;
    std::span<std::byte> debug_ranges;
    std::span<std::byte> debug_rnglists;
    std::span<std::byte> debug_str;
    std::span<std::byte> debug_str_offsets;
    std::span<std::byte> debug_sup;
    std::span<std::byte> debug_types;
    std::span<std::byte> debug_abbrev_dwo;
    std::span<std::byte> debug_info_dwo;
    std::span<std::byte> debug_line_dwo;
    std::span<std::byte> debug_loclists_dwo;
    std::span<std::byte> debug_macro_dwo;
    std::span<std::byte> debug_rnglists_dwo;
    std::span<std::byte> debug_str_dwo;
    std::span<std::byte> debug_str_offsets_dwo;
    std::span<std::byte> debug_framesection;
    std::span<std::byte> debug_cu_index;
    std::span<std::byte> debug_tu_index;

    std::endian initial_endianness;

    dwarf(elfy::elf& elf_):
        elf(elf_),

        debug_abbrev(elf.get_section_data_by_name(".debug_abbrev")),
        debug_addr(elf.get_section_data_by_name(".debug_addr")),
        debug_aranges(elf.get_section_data_by_name(".debug_aranges")),
        debug_frame(elf.get_section_data_by_name(".debug_frame")),
        debug_info(elf.get_section_data_by_name(".debug_info")),
        debug_line(elf.get_section_data_by_name(".debug_line")),
        debug_line_str(elf.get_section_data_by_name(".debug_line_str")),
        debug_loc(elf.get_section_data_by_name(".debug_loc")),
        debug_loclists(elf.get_section_data_by_name(".debug_loclists")),
        debug_macinfo(elf.get_section_data_by_name(".debug_macinfo")),
        debug_macro(elf.get_section_data_by_name(".debug_macro")),
        debug_names(elf.get_section_data_by_name(".debug_names")),
        debug_pubnames(elf.get_section_data_by_name(".debug_pubnames")),
        debug_pubtypes(elf.get_section_data_by_name(".debug_pubtypes")),
        debug_ranges(elf.get_section_data_by_name(".debug_ranges")),
        debug_rnglists(elf.get_section_data_by_name(".debug_rnglists")),
        debug_str(elf.get_section_data_by_name(".debug_str")),
        debug_str_offsets(elf.get_section_data_by_name(".debug_str_offsets")),
        debug_sup(elf.get_section_data_by_name(".debug_sup")),
        debug_types(elf.get_section_data_by_name(".debug_types")),
        debug_abbrev_dwo(elf.get_section_data_by_name(".debug_abbrev.dwo")),
        debug_info_dwo(elf.get_section_data_by_name(".debug_info.dwo")),
        debug_line_dwo(elf.get_section_data_by_name(".debug_line.dwo")),
        debug_loclists_dwo(elf.get_section_data_by_name(".debug_loclists.dwo")),
        debug_macro_dwo(elf.get_section_data_by_name(".debug_macro.dwo")),
        debug_rnglists_dwo(elf.get_section_data_by_name(".debug_rnglists.dwo")),
        debug_str_dwo(elf.get_section_data_by_name(".debug_str.dwo")),
        debug_str_offsets_dwo(elf.get_section_data_by_name(".debug_str_offsets.dwo")),
        debug_framesection(elf.get_section_data_by_name(".debug_framesection")),
        debug_cu_index(elf.get_section_data_by_name(".debug_cu_index")),
        debug_tu_index(elf.get_section_data_by_name(".debug_tu_index")),

        initial_endianness(elf.ident.endianness())
    {}

    void read_debug_info();
    size_t find_abbrev(uleb128 abbrev_code);
    size_t find_abbrev(uleb128 abbrev_code, compilation_unit_header& cu);

    compilation_unit_header::iterator cu_iter() {
        return compilation_unit_header::iterator{debug_info, initial_endianness};
    }
};

size_t dwarf::find_abbrev(uleb128 abbrev_code) {
    static std::vector<size_t> abbrev_vec;

    if (abbrev_vec.empty()) {
        span_reader debug_abbrev_reader {debug_abbrev};
        debug_abbrev_reader.file_endianness = initial_endianness;
        while (true) {
            debug_abbrev_entry dae;
            std::span<std::byte> start = debug_abbrev_reader.data;
            std::cout << "reading dae at offset " << start.data() - debug_abbrev.data() << std::endl;
            debug_abbrev_reader & dae;
            if (dae.is_last()) {
                continue;
            }

            while (true) {
                attribute a;
                debug_abbrev_reader & a.name & a.form;
                if (a.is_last()) {
                    break;
                }
            }

            size_t offset = start.data() - debug_abbrev.data();
            abbrev_vec.resize(std::max(abbrev_vec.size(), dae.abbrev_code + 1));
            abbrev_vec[dae.abbrev_code] = offset;

            //FIXME
            //seems to run off the end of real DAEs but still has data in the section
            if (debug_abbrev_reader.data.empty()) {
                break;
            }
        }
    }
    if (abbrev_code < abbrev_vec.size()) {
        return abbrev_vec[abbrev_code];
    } else {
        throw std::runtime_error("no abbrev code found for die");
    }
}

size_t dwarf::find_abbrev(uleb128 abbrev_code, compilation_unit_header& cu) {
    size_t offset = cu.debug_abbrev_offset;
    span_reader debug_abbrev_reader {debug_abbrev.subspan(offset)};
    debug_abbrev_reader.file_endianness = initial_endianness;
    debug_abbrev_entry dae;
    while (true) {
        offset = debug_abbrev_reader.data.data() - debug_abbrev.data();
        std::cout << "reading dae at offset " << offset << std::endl;
        debug_abbrev_reader & dae;
        if (dae.is_last()) {
            throw std::runtime_error("no abbrev code found for die");
        }
        if (dae.abbrev_code == abbrev_code) {
            return offset;
        }
        while (true) {
            attribute a;
            debug_abbrev_reader & a.name & a.form;
            if (a.is_last()) {
                break;
            }
        }
    }
}

void dwarf::read_debug_info() {
    std::cout << to_string(debug_info) << std::endl;
    span_reader debug_abbrev_reader {debug_abbrev};
    debug_abbrev_reader.file_endianness = initial_endianness;

    auto cu_it = cu_iter();
    for (compilation_unit_header cu: cu_it) {
        std::cout << "cu:" << std::endl;
        span_reader debug_info_reader = cu_it.die_reader();
        std::cout << "offset " << debug_info_reader.data.data() - debug_info.data() << std::endl;

        while (true) {
            debugging_information_entry die;
            debug_info_reader & die;
            if (die.is_last()) {
                break;
            }
            std::cout << "die:" << std::endl;

            size_t offset = find_abbrev(die.abbrev_code, cu);
            debug_abbrev_reader.reset(debug_abbrev.subspan(offset));
            debug_abbrev_entry dae;
            debug_abbrev_reader & dae;

            std::cout << to_string(dae.tag) << std::endl;

            while (true) {
                attribute attr;
                read(debug_info_reader, debug_abbrev_reader, attr);
                if (!attr.is_last()) {
                    std::cout << to_string(attr) << std::endl;
                } else {
                    break;
                }
            }
            std::cout << std::endl;
        }
    }
}

}
