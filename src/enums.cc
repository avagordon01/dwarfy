#include "enums.hh"
#include "serialise.hh"
#include "leb128.hh"
#include <unordered_map>

namespace dwarfy {

void read(span_reader &r, enum dw_tag& tag) {
    uleb128 v;
    r & v;
    tag = static_cast<dw_tag>(static_cast<uint64_t>(v));
}
void read(span_reader &r, enum dw_at& attr) {
    uleb128 v;
    r & v;
    attr = static_cast<dw_at>(static_cast<uint64_t>(v));
}
void read(span_reader &r, enum dw_form& form) {
    uleb128 v;
    r & v;
    form = static_cast<dw_form>(static_cast<uint64_t>(v));
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
    {dw_tag::typedef_, "typedef"},
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
    {dw_tag::friend_, "friend"},
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
    {dw_tag::namespace_, "namespace"},
    {dw_tag::imported_module, "imported_module"},
    {dw_tag::unspecified_type, "unspecified_type"},
    {dw_tag::partial_unit, "partial_unit"},
    {dw_tag::imported_unit, "imported_unit"},
    {dw_tag::condition, "condition"},
    {dw_tag::shared_type, "shared_type"},
    {dw_tag::type_unit, "type_unit"},
    {dw_tag::rvalue_reference_type, "rvalue_reference_type"},
    {dw_tag::template_alias, "template_alias"},
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
    {dw_at::inline_, "inline"},
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
    {dw_at::friend_, "friend"},
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
    {dw_at::mutable_, "mutable"},
    {dw_at::threads_scaled, "threads_scaled"},
    {dw_at::explicit_, "explicit"},
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
};
std::unordered_map<dw_form, std::string> map_form_to_string = {
    {dw_form::addr, "addr"},
    {dw_form::block2, "block2"},
    {dw_form::block4, "block4"},
    {dw_form::data2, "data2"},
    {dw_form::data4, "data4"},
    {dw_form::data8, "data8"},
    {dw_form::string, "string"},
    {dw_form::block, "block"},
    {dw_form::block1, "block1"},
    {dw_form::data1, "data1"},
    {dw_form::flag, "flag"},
    {dw_form::sdata, "sdata"},
    {dw_form::strp, "strp"},
    {dw_form::udata, "udata"},
    {dw_form::ref_addr, "ref_addr"},
    {dw_form::ref1, "ref1"},
    {dw_form::ref2, "ref2"},
    {dw_form::ref4, "ref4"},
    {dw_form::ref8, "ref8"},
    {dw_form::ref_udata, "ref_udata"},
    {dw_form::indirect, "indirect"},
    {dw_form::sec_offset, "sec_offset"},
    {dw_form::exprloc, "exprloc"},
    {dw_form::flag_present, "flag_present"},
    {dw_form::ref_sig8, "ref_sig8"},
};

using std::to_string;
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
}

std::string to_string(enum dw_form form) {
    auto it = map_form_to_string.find(form);
    if (it != map_form_to_string.end()) {
        return it->second;
    } else {
        return "unknown dw_form: " + to_string(static_cast<uint64_t>(form));
    }
}

}
