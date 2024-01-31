#pragma once

#include "morse.h"
#include "io.h"

#include <stdint.h>

typedef enum {
  TimerReason_Dah,
  TimerReason_NextChar
} TimerReason;

typedef struct {
  // The index of the next morse element to write
  uint8_t current_morse_element;
  // The morse written until now for this character
  uint8_t morse_buffer;
  TimerReason last_timer_reason;
} MorseInputData;

// Processes actions given as morse input. Does everything from playing tones to drawing the input on the screen.
// Returns a valid MorseCharacter (not zero) when a character finishes being input
MorseCharacter process_morse_input(MorseInputData* data, const IoActions* actions);

void clear_morse_display(void);
