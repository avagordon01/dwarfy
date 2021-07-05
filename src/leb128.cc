#include "leb128.hh"

uint8_t high_bit(uint8_t x) {
    return x & 0x80;
}
uint8_t low_bits(uint8_t x) {
    return x & 0x7f;
}
uint8_t sign_bit(uint8_t x) {
    return x & 0x40;
}
