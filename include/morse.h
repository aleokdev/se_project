#ifndef MORSE_H
#define MORSE_H

#include <stdint.h>

extern const uint16_t morse_characters[26];

typedef struct {
  uint8_t length;
  uint8_t morse;
} MorseCharacter;

// Returns the given morse code as an ASCII character, or 0 if it does not match
// any
char translate_morse(MorseCharacter);

// Returns the translation of the ASCII character given, if it exists
// Otherwise returns a MorseTranslation with length & morse = 0
MorseCharacter get_morse_translation(char);

#endif
