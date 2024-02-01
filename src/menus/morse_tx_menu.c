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

void redraw_message_buffer(void) {
  for (uint8_t i = 0; i < tx_state.current_msg_char; i++) {
    ssd1306_printChar2x((i << 3) + (i << 2), 2, tx_state.msg_buffer[i], false);
  }
  for (uint8_t i = tx_state.current_msg_char; i < MESSAGE_BUFFER_MAX_LENGTH; i++) {
    ssd1306_printChar2x((i << 3) + (i << 2), 2, ' ', false);
  }
}

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
  clear_morse_display();
}

void redraw_morse_transmission_menu(void) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Transmisor", true);
  redraw_message_buffer();
}

void process_morse_tx_menu(Menu* menu_open, const IoActions* actions) {
  if (actions->pressed_encoder) {
    silence_tone();
    open_selection_menu(menu_open);
    return;
  }

  const MorseCharacter input_char = process_morse_input(&tx_state.input_data, actions);

  if (input_char.length > 0) {
    char translated_char = translate_morse(input_char);
    if (translated_char) {
      tx_state.msg_buffer[tx_state.current_msg_char] = translated_char;
    } else {
      tx_state.msg_buffer[tx_state.current_msg_char] = '?';
    }
    ssd1306_printChar2x((tx_state.current_msg_char << 3) + (tx_state.current_msg_char << 2), 2,
                        tx_state.msg_buffer[tx_state.current_msg_char], false);

    if (input_char.length == morse_backspace.length && input_char.morse == morse_backspace.morse) {
      // Backspace: Delete last character shown as well as the current one
      ssd1306_printChar2x((tx_state.current_msg_char << 3) + (tx_state.current_msg_char << 2), 2,
                          ' ', false);
      if (tx_state.current_msg_char > 0) {
        tx_state.current_msg_char--;
        ssd1306_printChar2x((tx_state.current_msg_char << 3) + (tx_state.current_msg_char << 2), 2,
                            ' ', false);
      }
      clear_morse_display();
    } else {
      skip_to_next_msg_char();
    }
  }
}

void open_morse_tx_menu(Menu* menu_open) {
  *menu_open = Menu_MorseTx;
  tx_state = (TxState){0}; // Reset menu state
  redraw_morse_transmission_menu();
}
