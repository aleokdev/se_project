#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MESSAGE_BUFFER_MAX_LENGTH 10

typedef struct {
  // A buffer for the ASCII message sent until now.
  char msg_buffer[MESSAGE_BUFFER_MAX_LENGTH];
  // The morse element buffer for the current character being sent. Each bit represents either a dit (0) or a dah (1).
  uint8_t morse_buffer;
  // Character index being sent right now on the morse buffer
  uint8_t current_msg_char;
  // Current element within the character being sent right now
  uint8_t current_morse_element;

  bool settings_menu_open;
  uint8_t setting_hovered;
  bool setting_is_selected;
} State;
