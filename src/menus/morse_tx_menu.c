#include "menus.h"

#include "io.h"
#include "morse.h"
#include "morse_input.h"
#include "settings.h"
#include "ssd1306.h"

#include <stdint.h>

#define MESSAGE_BUFFER_MAX_LENGTH 10

typedef struct {
  // A buffer for the ASCII message sent until now.
  char msg_buffer[MESSAGE_BUFFER_MAX_LENGTH];
  // Character index being sent right now on the morse buffer
  uint8_t current_msg_char;
  MorseInputData input_data;
} TxState;

TxState tx_state;

// Draw the character given as a message character with the index of `position`.
inline void draw_message_char(uint8_t position, char ch) {
  ssd1306_printChar2x((position << 3) + (position << 2), 2, ch, false);
}

// Redraw the message buffer (tx_state.msg_buffer) on screen.
void redraw_message_buffer(void) {
  // Characters present in the buffer
  for (uint8_t i = 0; i < tx_state.current_msg_char; i++) {
    draw_message_char(i, tx_state.msg_buffer[i]);
  }
  // Characters not present in the buffer (clear their position drawing a space)
  for (uint8_t i = tx_state.current_msg_char; i < MESSAGE_BUFFER_MAX_LENGTH; i++) {
    draw_message_char(i, ' ');
  }
}

// Go to the next message character, and shift message to the left if the buffer is full
void skip_to_next_msg_char(void) {
  if (tx_state.current_msg_char >= MESSAGE_BUFFER_MAX_LENGTH - 1) {
    // Shift all existing chars to the left to accomodate one more
    for (uint8_t i = 0; i < MESSAGE_BUFFER_MAX_LENGTH - 1u; i++) {
      tx_state.msg_buffer[i] = tx_state.msg_buffer[i + 1u];
    }
    tx_state.msg_buffer[MESSAGE_BUFFER_MAX_LENGTH - 1u] = 0;
    redraw_message_buffer();
  } else {
    tx_state.current_msg_char++;
  }
}

void redraw_morse_transmission_menu(void) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Transmisor", true);
  redraw_message_buffer();
}

void process_morse_tx_menu(Menu* menu_open, const IoActions* actions) {
  if (actions->pressed_encoder) {
    // Close menu
    silence_tone();
    open_selection_menu(menu_open);
    return;
  }

  const MorseCharacter input_char = process_morse_input(&tx_state.input_data, actions);

  if (input_char.length > 0) {
    if (is_morse_backspace(input_char)) {
      // Backspace: Delete last character shown as well as the current one
      draw_message_char(tx_state.current_msg_char, ' ');
      if (tx_state.current_msg_char > 0) {
        tx_state.current_msg_char--;
        draw_message_char(tx_state.current_msg_char, ' ');
      }
    } else {
      // Not a backspace, try to translate
      const char translated_char = translate_morse(input_char);
      if (translated_char) {
        // Translation successful, append character to message buffer
        tx_state.msg_buffer[tx_state.current_msg_char] = translated_char;
      } else {
        // Translation is empty => Unknown character: Use a '?' instead
        tx_state.msg_buffer[tx_state.current_msg_char] = '?';
      }
      // Draw character added to buffer
      draw_message_char(tx_state.current_msg_char, tx_state.msg_buffer[tx_state.current_msg_char]);

      skip_to_next_msg_char();
    }
  }
}

void open_morse_tx_menu(Menu* menu_open) {
  *menu_open = Menu_MorseTx;
  tx_state = (TxState){}; // Reset menu state
  redraw_morse_transmission_menu();
}
