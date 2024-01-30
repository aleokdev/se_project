#include "morse.h"

const uint16_t morse_characters[26] = {
    (2 << 8) | 0b10,   // A
    (4 << 8) | 0b0001, // B
    (4 << 8) | 0b0101, // C
    (3 << 8) | 0b001,  // D
    (1 << 8) | 0b0,    // E
    (4 << 8) | 0b0100, // F
    (3 << 8) | 0b011,  // G
    (4 << 8) | 0b0000, // H
    (2 << 8) | 0b00,   // I
    (4 << 8) | 0b1110, // J
    (3 << 8) | 0b101,  // K
    (4 << 8) | 0b0010, // L
    (2 << 8) | 0b11,   // M
    (2 << 8) | 0b01,   // N
    (3 << 8) | 0b111,  // O
    (4 << 8) | 0b0110, // P
    (4 << 8) | 0b1011, // Q
    (3 << 8) | 0b010,  // R
    (3 << 8) | 0b000,  // S
    (1 << 8) | 0b1,    // T
    (3 << 8) | 0b100,  // U
    (4 << 8) | 0b1000, // V
    (3 << 8) | 0b110,  // W
    (4 << 8) | 0b1001, // X
    (4 << 8) | 0b1101, // Y
    (4 << 8) | 0b0011, // Z
};

char translate_morse(MorseCharacter ch) {
  for (uint8_t i = sizeof(morse_characters) / 2; i > 0; i--) {
    uint16_t data = morse_characters[i - 1u];
    uint8_t char_len = (data >> 8) & 0xFF;
    uint8_t char_morse = data & 0xFF;
    if (char_len == ch.length && char_morse == ch.morse) {
      return i - 1 + 'A';
    }
  }

  return 0;
}

MorseCharacter get_morse_translation(char ch) {
  if(ch >= 'A' && ch <= 'Z') {
    const uint16_t data = morse_characters[ch - 'A'];
    const uint8_t char_len = (data >> 8) & 0xFF;
    const uint8_t char_morse = data & 0xFF;

    return (MorseCharacter) { .length = char_len, .morse = char_morse };
  } else {
    return (MorseCharacter) { 0 };
  }
}
