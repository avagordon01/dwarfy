#include "dwarfy.hh"
#include <cstring>

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
        a.data = read_form(ir, a.form);
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
    return compilation_unit_header::iterator{this};
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
        //std::cout << "reading dae at offset " << offset << std::endl;
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

struct target_address {
    uint64_t segment = 0;
    uint64_t address = 0;
};
void read(span_reader &r, target_address& addr) {
    auto s = r.read_bytes(r.machine_segment_size);
    std::memcpy(&addr.segment, s.data(), r.machine_segment_size);
    auto a = r.read_bytes(r.machine_address_size);
    std::memcpy(&addr.address, a.data(), r.machine_address_size);
}

struct arange_unit_header {
    initial_length unit_length;
    uint16_t version;
    file_offset_size debug_info_offset;
    uint8_t machine_address_size;
    uint8_t machine_segment_size;
};
void read(span_reader &r, arange_unit_header& au) {
    r & au.unit_length & au.version & au.debug_info_offset & au.machine_address_size & au.machine_segment_size;
    if (au.machine_segment_size > 8) {
        throw std::runtime_error("error, dwarfy doesn't support segment sizes over 8 bytes");
    }
    if (au.machine_address_size > 8) {
        throw std::runtime_error("error, dwarfy doesn't support address sizes over 8 bytes");
    }
    r.machine_segment_size = au.machine_segment_size;
    r.machine_address_size = au.machine_address_size;
}

struct arange_descriptor {
    target_address address;
    machine_address_size length;
    bool is_last() {
        return address.segment == 0 && address.address == 0 && length == 0;
    }
};
void read(span_reader &r, arange_descriptor& ad) {
    r & ad.address & ad.length;
}

void dwarf::address_to_cu_arange() {
    target_address addr;

    span_reader debug_aranges_reader {debug_aranges};
    while (!debug_aranges_reader.data.empty()) {
        arange_unit_header au;
        debug_aranges_reader & au;
        std::cout << "read arange unit header" << std::endl;
        size_t mod = debug_aranges_reader.machine_segment_size + 2 * debug_aranges_reader.machine_address_size;
        size_t offset = debug_aranges_reader.data.data() - debug_aranges.data();
        if (offset % mod != 0) {
            offset = mod - (offset % mod);
        }
        debug_aranges_reader.read_bytes(offset);
        while (true) {
            arange_descriptor ad;
            debug_aranges_reader & ad;
            if (ad.is_last()) {
                break;
            }
            std::cout << "read arange descriptor" << std::endl;
        }
    }
}

void dwarf::read_debug_info() {
    span_reader debug_abbrev_reader {debug_abbrev};
    debug_abbrev_reader.file_endianness = initial_endianness;

    auto cu_it = cu_iter();
    for (compilation_unit_header cu: cu_it) {
        std::cout << "cu:" << std::endl;

        auto die_it = cu_it.die_iter();
        for (; die_it != std::end(die_it); die_it++) {
            debugging_information_entry die = *die_it;
            std::cout << "die:" << std::endl;

            size_t offset = find_abbrev(die.abbrev_code, cu);
            debug_abbrev_reader.reset(debug_abbrev.subspan(offset));
            debug_abbrev_entry dae;
            debug_abbrev_reader & dae;

            std::cout << to_string(dae.tag) << std::endl;

            span_reader debug_info_reader = die_it.debug_info_reader;
            while (true) {
                attribute attr;
                read(debug_info_reader, debug_abbrev_reader, attr);
                if (!attr.is_last()) {
                    std::cout << to_string(attr) << std::endl;
                } else {
                    break;
                }
            }
            die_it.debug_info_reader = debug_info_reader;

            std::cout << std::endl;
        }
    }
}

}
