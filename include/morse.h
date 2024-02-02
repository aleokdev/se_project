#ifndef MORSE_H
#define MORSE_H

#include <stdint.h>

typedef struct {
  uint8_t length;
  uint8_t morse;
} MorseCharacter;

static const MorseCharacter morse_backspace = {.length = 8, .morse = 0};

// Returns the given morse code as an ASCII character, or 0 if it does not match
// any
char translate_morse(MorseCharacter);

// Returns the translation of the ASCII character given, if it exists
// Otherwise returns a MorseTranslation with length & morse = 0
MorseCharacter get_morse_translation(char);

#endif
