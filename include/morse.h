#ifndef MORSE_H
#define MORSE_H

#include <stdint.h>

extern const uint16_t morse_characters[26];

// Returns the given morse code as an ASCII character, or 0 if it does not match
// any
char translate_morse(uint8_t morse_count, uint8_t morse);

#endif
