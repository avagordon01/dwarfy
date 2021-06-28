#pragma once

#include <cstdint>
#include <cstdlib>
#include <span>
#include <optional>
#include <iostream>

#include "elfy.hh"
#include "leb128.hh"
#include "serialise.hh"

namespace dwarfy {

using std::to_string;

enum class dw_tag {
    array_type = 0x01,
    class_type = 0x02,
    entry_point = 0x03,
    enumeration_type = 0x04,
    formal_parameter = 0x05,
    imported_declaration = 0x08,
    label = 0x0a,
    lexical_block = 0x0b,
    member = 0x0d,
    pointer_type = 0x0f,
    reference_type = 0x10,
    compile_unit = 0x11,
    string_type = 0x12,
    structure_type = 0x13,
    subroutine_type = 0x15,
    typedef_ = 0x16,
    union_type = 0x17,
    unspecified_parameters = 0x18,
    variant = 0x19,
    common_block = 0x1a,
    common_inclusion = 0x1b,
    inheritance = 0x1c,
    inlined_subroutine = 0x1d,
    module = 0x1e,
    ptr_to_member_type = 0x1f,
    set_type = 0x20,
    subrange_type = 0x21,
    with_stmt = 0x22,
    access_declaration = 0x23,
    base_type = 0x24,
    catch_block = 0x25,
    const_type = 0x26,
    constant = 0x27,
    enumerator = 0x28,
    file_type = 0x29,
    friend_ = 0x2a,
    namelist = 0x2b,
    namelist_item = 0x2c,
    packed_type = 0x2d,
    subprogram = 0x2e,
    template_type_parameter = 0x2f,
    template_value_parameter = 0x30,
    thrown_type = 0x31,
    try_block = 0x32,
    variant_part = 0x33,
    variable = 0x34,
    volatile_type = 0x35,
    dwarf_procedure = 0x36,
    restrict_type = 0x37,
    interface_type = 0x38,
    namespace_ = 0x39,
    imported_module = 0x3a,
    unspecified_type = 0x3b,
    partial_unit = 0x3c,
    imported_unit = 0x3d,
    condition = 0x3f,
    shared_type = 0x40,
    type_unit = 0x41,
    rvalue_reference_type = 0x42,
    template_alias = 0x43,
    lo_user = 0x4080,
    hi_user = 0xffff,
};
std::unordered_map<dw_tag, std::string> map_tag_to_string = {
    {dw_tag::array_type, "array_type"},
    {dw_tag::class_type, "class_type"},
    {dw_tag::entry_point, "entry_point"},
    {dw_tag::enumeration_type, "enumeration_type"},
    {dw_tag::formal_parameter, "formal_parameter"},
    {dw_tag::imported_declaration, "imported_declaration"},
    {dw_tag::label, "label"},
    {dw_tag::lexical_block, "lexical_block"},
    {dw_tag::member, "member"},
    {dw_tag::pointer_type, "pointer_type"},
    {dw_tag::reference_type, "reference_type"},
    {dw_tag::compile_unit, "compile_unit"},
    {dw_tag::string_type, "string_type"},
    {dw_tag::structure_type, "structure_type"},
    {dw_tag::subroutine_type, "subroutine_type"},
    {dw_tag::typedef_, "typedef_"},
    {dw_tag::union_type, "union_type"},
    {dw_tag::unspecified_parameters, "unspecified_parameters"},
    {dw_tag::variant, "variant"},
    {dw_tag::common_block, "common_block"},
    {dw_tag::common_inclusion, "common_inclusion"},
    {dw_tag::inheritance, "inheritance"},
    {dw_tag::inlined_subroutine, "inlined_subroutine"},
    {dw_tag::module, "module"},
    {dw_tag::ptr_to_member_type, "ptr_to_member_type"},
    {dw_tag::set_type, "set_type"},
    {dw_tag::subrange_type, "subrange_type"},
    {dw_tag::with_stmt, "with_stmt"},
    {dw_tag::access_declaration, "access_declaration"},
    {dw_tag::base_type, "base_type"},
    {dw_tag::catch_block, "catch_block"},
    {dw_tag::const_type, "const_type"},
    {dw_tag::constant, "constant"},
    {dw_tag::enumerator, "enumerator"},
    {dw_tag::file_type, "file_type"},
    {dw_tag::friend_, "friend_"},
    {dw_tag::namelist, "namelist"},
    {dw_tag::namelist_item, "namelist_item"},
    {dw_tag::packed_type, "packed_type"},
    {dw_tag::subprogram, "subprogram"},
    {dw_tag::template_type_parameter, "template_type_parameter"},
    {dw_tag::template_value_parameter, "template_value_parameter"},
    {dw_tag::thrown_type, "thrown_type"},
    {dw_tag::try_block, "try_block"},
    {dw_tag::variant_part, "variant_part"},
    {dw_tag::variable, "variable"},
    {dw_tag::volatile_type, "volatile_type"},
    {dw_tag::dwarf_procedure, "dwarf_procedure"},
    {dw_tag::restrict_type, "restrict_type"},
    {dw_tag::interface_type, "interface_type"},
    {dw_tag::namespace_, "namespace_"},
    {dw_tag::imported_module, "imported_module"},
    {dw_tag::unspecified_type, "unspecified_type"},
    {dw_tag::partial_unit, "partial_unit"},
    {dw_tag::imported_unit, "imported_unit"},
    {dw_tag::condition, "condition"},
    {dw_tag::shared_type, "shared_type"},
    {dw_tag::type_unit, "type_unit"},
    {dw_tag::rvalue_reference_type, "rvalue_reference_type"},
    {dw_tag::template_alias, "template_alias"},
    //TODO handle hi_user lo_user
};
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
        return "unknown dw_tag: " + to_string(static_cast<uint64_t>(tag));
    }
};

enum class dw_at {
    sibling = 0x01,
    location = 0x02,
    name = 0x03,
    ordering = 0x09,
    byte_size = 0x0b,
    bit_offset = 0x0c,
    bit_size = 0x0d,
    stmt_list = 0x10,
    low_pc = 0x11,
    high_pc = 0x12,
    language = 0x13,
    discr = 0x15,
    discr_value = 0x16,
    visibility = 0x17,
    import = 0x18,
    string_length = 0x19,
    common_reference = 0x1a,
    comp_dir = 0x1b,
    const_value = 0x1c,
    containing_type = 0x1d,
    default_value = 0x1e,
    inline_ = 0x20,
    is_optional = 0x21,
    lower_bound = 0x22,
    producer = 0x25,
    prototyped = 0x27,
    return_addr = 0x2a,
    start_scope = 0x2c,
    bit_stride = 0x2e,
    upper_bound = 0x2f,
    abstract_origin = 0x31,
    accessibility = 0x32,
    address_class = 0x33,
    artificial = 0x34,
    base_types = 0x35,
    calling_convention = 0x36,
    count = 0x37,
    data_member_location = 0x38,
    decl_column = 0x39,
    decl_file = 0x3a,
    decl_line = 0x3b,
    declaration = 0x3c,
    discr_list = 0x3d,
    encoding = 0x3e,
    external = 0x3f,
    frame_base = 0x40,
    friend_ = 0x41,
    identifier_case = 0x42,
    macro_info = 0x43,
    namelist_item = 0x44,
    priority = 0x45,
    segment = 0x46,
    specification = 0x47,
    static_link = 0x48,
    type = 0x49,
    use_location = 0x4a,
    variable_parameter = 0x4b,
    virtuality = 0x4c,
    vtable_elem_location = 0x4d,
    allocated = 0x4e,
    associated = 0x4f,
    data_location = 0x50,
    byte_stride = 0x51,
    entry_pc = 0x52,
    use_UTF8 = 0x53,
    extension = 0x54,
    ranges = 0x55,
    trampoline = 0x56,
    call_column = 0x57,
    call_file = 0x58,
    call_line = 0x59,
    description = 0x5a,
    binary_scale = 0x5b,
    decimal_scale = 0x5c,
    small = 0x5d,
    decimal_sign = 0x5e,
    digit_count = 0x5f,
    picture_string = 0x60,
    mutable_ = 0x61,
    threads_scaled = 0x62,
    explicit_ = 0x63,
    object_pointer = 0x64,
    endianity = 0x65,
    elemental = 0x66,
    pure = 0x67,
    recursive = 0x68,
    signature = 0x69,
    main_subprogram = 0x6a,
    data_bit_offset = 0x6b,
    const_expr = 0x6c,
    enum_class = 0x6d,
    linkage_name = 0x6e,
    lo_user = 0x2000,
    hi_user = 0x3fff,
};
std::unordered_map<dw_at, std::string> map_at_to_string = {
    {dw_at::sibling, "sibling"},
    {dw_at::location, "location"},
    {dw_at::name, "name"},
    {dw_at::ordering, "ordering"},
    {dw_at::byte_size, "byte_size"},
    {dw_at::bit_offset, "bit_offset"},
    {dw_at::bit_size, "bit_size"},
    {dw_at::stmt_list, "stmt_list"},
    {dw_at::low_pc, "low_pc"},
    {dw_at::high_pc, "high_pc"},
    {dw_at::language, "language"},
    {dw_at::discr, "discr"},
    {dw_at::discr_value, "discr_value"},
    {dw_at::visibility, "visibility"},
    {dw_at::import, "import"},
    {dw_at::string_length, "string_length"},
    {dw_at::common_reference, "common_reference"},
    {dw_at::comp_dir, "comp_dir"},
    {dw_at::const_value, "const_value"},
    {dw_at::containing_type, "containing_type"},
    {dw_at::default_value, "default_value"},
    {dw_at::inline_, "inline_"},
    {dw_at::is_optional, "is_optional"},
    {dw_at::lower_bound, "lower_bound"},
    {dw_at::producer, "producer"},
    {dw_at::prototyped, "prototyped"},
    {dw_at::return_addr, "return_addr"},
    {dw_at::start_scope, "start_scope"},
    {dw_at::bit_stride, "bit_stride"},
    {dw_at::upper_bound, "upper_bound"},
    {dw_at::abstract_origin, "abstract_origin"},
    {dw_at::accessibility, "accessibility"},
    {dw_at::address_class, "address_class"},
    {dw_at::artificial, "artificial"},
    {dw_at::base_types, "base_types"},
    {dw_at::calling_convention, "calling_convention"},
    {dw_at::count, "count"},
    {dw_at::data_member_location, "data_member_location"},
    {dw_at::decl_column, "decl_column"},
    {dw_at::decl_file, "decl_file"},
    {dw_at::decl_line, "decl_line"},
    {dw_at::declaration, "declaration"},
    {dw_at::discr_list, "discr_list"},
    {dw_at::encoding, "encoding"},
    {dw_at::external, "external"},
    {dw_at::frame_base, "frame_base"},
    {dw_at::friend_, "friend_"},
    {dw_at::identifier_case, "identifier_case"},
    {dw_at::macro_info, "macro_info"},
    {dw_at::namelist_item, "namelist_item"},
    {dw_at::priority, "priority"},
    {dw_at::segment, "segment"},
    {dw_at::specification, "specification"},
    {dw_at::static_link, "static_link"},
    {dw_at::type, "type"},
    {dw_at::use_location, "use_location"},
    {dw_at::variable_parameter, "variable_parameter"},
    {dw_at::virtuality, "virtuality"},
    {dw_at::vtable_elem_location, "vtable_elem_location"},
    {dw_at::allocated, "allocated"},
    {dw_at::associated, "associated"},
    {dw_at::data_location, "data_location"},
    {dw_at::byte_stride, "byte_stride"},
    {dw_at::entry_pc, "entry_pc"},
    {dw_at::use_UTF8, "use_UTF8"},
    {dw_at::extension, "extension"},
    {dw_at::ranges, "ranges"},
    {dw_at::trampoline, "trampoline"},
    {dw_at::call_column, "call_column"},
    {dw_at::call_file, "call_file"},
    {dw_at::call_line, "call_line"},
    {dw_at::description, "description"},
    {dw_at::binary_scale, "binary_scale"},
    {dw_at::decimal_scale, "decimal_scale"},
    {dw_at::small, "small"},
    {dw_at::decimal_sign, "decimal_sign"},
    {dw_at::digit_count, "digit_count"},
    {dw_at::picture_string, "picture_string"},
    {dw_at::mutable_, "mutable_"},
    {dw_at::threads_scaled, "threads_scaled"},
    {dw_at::explicit_, "explicit_"},
    {dw_at::object_pointer, "object_pointer"},
    {dw_at::endianity, "endianity"},
    {dw_at::elemental, "elemental"},
    {dw_at::pure, "pure"},
    {dw_at::recursive, "recursive"},
    {dw_at::signature, "signature"},
    {dw_at::main_subprogram, "main_subprogram"},
    {dw_at::data_bit_offset, "data_bit_offset"},
    {dw_at::const_expr, "const_expr"},
    {dw_at::enum_class, "enum_class"},
    {dw_at::linkage_name, "linkage_name"},
    //TODO handle hi_user lo_user
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
        return "unknown dw_at: " + to_string(static_cast<uint64_t>(attr));
    }
};

enum class dw_form {
    addr = 0x01,
    block2 = 0x03,
    block4 = 0x04,
    data2 = 0x05,
    data4 = 0x06,
    data8 = 0x07,
    string = 0x08,
    block = 0x09,
    block1 = 0x0a,
    data1 = 0x0b,
    flag = 0x0c,
    sdata = 0x0d,
    strp = 0x0e,
    udata = 0x0f,
    ref_addr = 0x10,
    ref1 = 0x11,
    ref2 = 0x12,
    ref4 = 0x13,
    ref8 = 0x14,
    ref_udata = 0x15,
    indirect = 0x16,
    sec_offset = 0x17,
    exprloc = 0x18,
    flag_present = 0x19,
    ref_sig8 = 0x20,
};
template<typename R>
void read(R &r, enum dw_form& form) {
    uleb128 v;
    r & v;
    form = static_cast<dw_form>(static_cast<uint64_t>(v));
}

enum class dw_children : uint8_t {
    no = 0x00,
    yes = 0x01,
};

struct initial_length {
    uint64_t length;
};

template<typename R>
void read(R& r, initial_length& i) {
    uint32_t l;
    r & l;
    if (l == 0xffffffff) {
        r.input_size_t = 64;
        r & i.length;
        return;
    }
    if (l >= 0xfffffff0) {
        throw std::runtime_error("bad DWARF initial length field, expected ==0xffffffff or <0xfffffff0, got: " + to_string(l));
    }
    r.input_size_t = 32;
    i.length = l;
    //FIXME what about mixed bitwidths in the same file?
};

struct attribute {
    dw_at name;
    dw_form form;
    bool is_last() {
        return static_cast<uint64_t>(name) == 0 && static_cast<uint64_t>(form) == 0;
    }
};
template<typename R>
void read(R &r, attribute& a) {
    r & a.name & a.form;
}
std::string to_string(attribute attr) {
    return to_string(attr.name);
}

struct debugging_information_entry {
    uleb128 abbrev_code;
};
template<typename R>
void read(R &r, debugging_information_entry& die) {
    r & die.abbrev_code;
    if (die.abbrev_code == 0) {
        return;
    }
    //TODO read attribute values
}

struct type_unit_header {
    initial_length unit_length;
    uint16_t version;
    input_size_t debug_abbrev_offset;
    uint8_t address_size;
    uint64_t type_signature;
    input_size_t type_offset;
};

template<typename R>
void read(R &r, type_unit_header& tu) {
    r & tu.unit_length & tu.version & tu.debug_abbrev_offset & tu.address_size & tu.type_signature & tu.type_offset;
    if (tu.version < 2 || tu.version > 5) {
        throw std::runtime_error("unsupported DWARF version, expected 2 <= version <= 5, got: " + to_string(tu.version));
    }
}

struct compilation_unit_header {
    initial_length unit_length;
    uint16_t version;
    input_size_t debug_abbrev_offset;
    uint8_t address_size;
};
template<typename R>
void read(R &r, compilation_unit_header& cu) {
    r & cu.unit_length & cu.version & cu.debug_abbrev_offset & cu.address_size;
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
    r & dae.abbrev_code & dae.tag & dae.debug_info_sibling;
}

struct die_reader {
    span_reader debug_info_reader;
    span_reader debug_abbrev_reader;
};

struct dwarf {

    elfy::elf elf;
    std::span<std::byte> debug_info_section;
    std::span<std::byte> debug_abbrev_section;
    dwarf(elfy::elf& elf_, std::endian initial_endianness):
        elf(elf_),
        debug_info_section(elf.get_section_by_name_ex(".debug_info").data(elf)),
        debug_abbrev_section(elf.get_section_by_name_ex(".debug_abbrev").data(elf))
    {
        span_reader debug_info_reader {debug_info_section};
        debug_info_reader.input_endianness = initial_endianness;
        span_reader debug_abbrev_reader {debug_abbrev_section};
        debug_abbrev_reader.input_endianness = initial_endianness;

        compilation_unit_header cu;
        debug_info_reader & cu;

        debugging_information_entry die;
        debug_info_reader & die;

        debug_abbrev_entry dae;
        debug_abbrev_reader & dae;
        attribute attr;
        do {
            debug_abbrev_reader & attr;
        } while (!attr.is_last());

        return;
    }
};

}
