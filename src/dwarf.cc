#include "dwarfy.hh"

namespace dwarfy {

using std::to_string;
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
}

std::string to_string(attribute attr) {
    return to_string(attr.name) + ":" + to_string(attr.form) + " " + to_string(attr.data);
}
void read(span_reader &ir, span_reader &ar, attribute& a) {
    ar & a.name & a.form;
    if (!a.is_last()) {
        a.data = read_form(ir, ar, a.form);
    }
}
void read(span_reader &r, debugging_information_entry& die) {
    r & die.abbrev_code;
}
void read(span_reader &r, type_unit_header& tu) {
    r & tu.unit_length & tu.version & tu.debug_abbrev_offset & tu.address_size & tu.type_signature & tu.type_offset;
    r.machine_address_size = tu.address_size;
    if (tu.version < 2 || tu.version > 5) {
        throw std::runtime_error("unsupported DWARF version, expected 2 <= version <= 5, got: " + to_string(tu.version));
    }
}
void read(span_reader &r, compilation_unit_header& cu) {
    r & cu.unit_length & cu.version & cu.debug_abbrev_offset & cu.address_size;
    r.machine_address_size = cu.address_size;
    if (cu.version < 2 || cu.version > 5) {
        throw std::runtime_error("unsupported DWARF version, expected 2 <= version <= 5, got: " + to_string(cu.version));
    }
}
void read(span_reader &r, debug_abbrev_entry& dae) {
    r & dae.abbrev_code;
    if (!dae.is_last()) {
        r & dae.tag & dae.debug_info_sibling;
    }
}

void read(span_reader& r, initial_length& i) {
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

compilation_unit_header::iterator dwarf::cu_iter() {
    return compilation_unit_header::iterator{debug_info, initial_endianness};
}

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
