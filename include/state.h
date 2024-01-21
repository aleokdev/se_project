#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MESSAGE_BUFFER_MAX_LENGTH 6

typedef struct {
  // Each byte here represents a morse character, and each bit represents either
  // a dit (0) or a dah (1).
  uint8_t morse_buffer[MESSAGE_BUFFER_MAX_LENGTH];
  // Character index being sent right now on the morse buffer
  uint8_t current_msg_char;
  // Current element within the character being sent right now
  uint8_t current_morse_element;

  bool settings_menu_open;
  uint8_t setting_hovered;
  bool setting_is_selected;
} State;
