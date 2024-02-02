#pragma once

#include "io.h"
#include "morse.h"

typedef struct {
  // The index of the next morse element to write
  uint8_t current_morse_element;
  // The morse written until now for this character
  uint8_t morse_buffer;
  // Defines why the Timer1 was last set up.
  enum { TimerReason_Dah, TimerReason_NextChar } last_timer_reason;
} MorseInputData;

// Processes actions given as morse input. Does everything from playing tones
// to drawing the input on the screen. Returns a valid MorseCharacter (not zero)
// when a character finishes being input.
MorseCharacter process_morse_input(MorseInputData* data, const IoActions* actions);
