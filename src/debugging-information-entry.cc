#include "dwarfy.hh"

namespace dwarfy {

bool debugging_information_entry::is_last() {
    return abbrev_code == 0;
}

bool debugging_information_entry::iterator::operator==(debugging_information_entry::sentinel) {
    return die.is_last();
}
debugging_information_entry::sentinel debugging_information_entry::iterator::end() const {
    return debugging_information_entry::sentinel{};
}
debugging_information_entry::iterator::iterator():
    debug_info_reader({}),
    debug_abbrev_reader({})
{}
debugging_information_entry::iterator::iterator(dwarf* d_):
    d(d_),
    debug_info_reader(d->debug_info),
    debug_abbrev_reader(d->debug_abbrev)
{
    debug_abbrev_reader.file_endianness = d->initial_endianness;
    debug_info_reader.file_endianness = d->initial_endianness;
    debug_info_reader & die;
}
const debugging_information_entry debugging_information_entry::iterator::operator*() const {
    return die;
}
debugging_information_entry::iterator& debugging_information_entry::iterator::operator++() {
    debug_info_reader & die;
    return *this;
}
debugging_information_entry::iterator debugging_information_entry::iterator::operator++(int) {
    debugging_information_entry::iterator ret = *this;
    this->operator++();
    return ret;
}

debugging_information_entry::iterator debugging_information_entry::iterator::begin() const {
    return *this;
}

}
