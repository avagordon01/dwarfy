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

#include "enums.hh"

namespace dwarfy {

void read(span_reader &r, enum dw_tag& tag);
std::string to_string(enum dw_tag tag);

void read(span_reader &r, enum dw_at& attr);
std::string to_string(enum dw_at attr);

void read(span_reader &r, enum dw_form& form);
std::string to_string(enum dw_form form);

std::string to_string(std::span<std::byte> bytes);

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

void read(span_reader& r, initial_length& i);

struct attribute {
    dw_at name;
    dw_form form;
    std::span<std::byte> data;
    bool is_last() {
        return static_cast<uint64_t>(name) == 0 && static_cast<uint64_t>(form) == 0;
    }
};
std::span<std::byte> read_form(span_reader &ir, dw_form form);
void read(span_reader &ir, span_reader &ar, attribute& a);
std::string to_string(attribute attr);

struct dwarf;

struct debugging_information_entry {
    uleb128 abbrev_code;
    bool is_last();

    class sentinel {};
    class iterator;
};
class debugging_information_entry::iterator {
    dwarf* d;
public:
    span_reader debug_info_reader;
    span_reader debug_abbrev_reader;
    debugging_information_entry die;
    using iterator_concept  = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = debugging_information_entry;
    using pointer           = value_type*;
    using reference         = value_type&;
    bool operator==(sentinel);
    sentinel end() const;
    iterator();
    iterator(dwarf* d_, span_reader debug_info_reader_);
    const debugging_information_entry operator*() const;
    iterator& operator++();
    iterator operator++(int);
    iterator begin() const;
};
static_assert(std::input_iterator<debugging_information_entry::iterator>);
void read(span_reader &r, debugging_information_entry& die);

struct type_unit_header {
    initial_length unit_length;
    uint16_t version;
    file_offset_size debug_abbrev_offset;
    uint8_t address_size;
    uint64_t type_signature;
    file_offset_size type_offset;
};

void read(span_reader &r, type_unit_header& tu);

struct compilation_unit_header {
    initial_length unit_length;
    uint16_t version;
    file_offset_size debug_abbrev_offset;
    uint8_t address_size;
    dwarf* d;

    class sentinel {};
    class iterator;
};

class compilation_unit_header::iterator {
    dwarf* d;
    span_reader debug_info_reader;
    std::span<std::byte> next_cu;
    compilation_unit_header cu;
public:
    using iterator_concept  = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = compilation_unit_header;
    using pointer           = value_type*;
    using reference         = value_type&;
    bool operator==(sentinel);
    sentinel end() const;
    iterator();
    iterator(dwarf* d_);
    const compilation_unit_header operator*() const;
    iterator& operator++();
    iterator operator++(int);
    span_reader die_reader();
    debugging_information_entry::iterator die_iter();
    iterator begin() const;
};
static_assert(std::input_iterator<compilation_unit_header::iterator>);
void read(span_reader &r, compilation_unit_header& cu);

struct debug_abbrev_entry {
    uleb128 abbrev_code;
    dw_tag tag;
    dw_children debug_info_sibling;
    bool is_last() {
        return abbrev_code == 0;
    }
};
void read(span_reader &r, debug_abbrev_entry& dae);

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

    compilation_unit_header::iterator cu_iter();
};

}
