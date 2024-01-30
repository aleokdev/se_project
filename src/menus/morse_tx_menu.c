#include "menus.h"

#include "morse.h"
#include "ssd1306.h"
#include "settings.h"
#include "io.h"
#include "morse_input.h"

#include <stdint.h>

typedef struct {
  // A buffer for the ASCII message sent until now.
  char msg_buffer[MESSAGE_BUFFER_MAX_LENGTH];
  // Character index being sent right now on the morse buffer
  uint8_t current_msg_char;
  MorseInputData input_data;
} TxState;

TxState tx_state;

void redraw_message_buffer(void) {
    for(uint8_t i = 0; i < tx_state.current_msg_char; i++) {
        ssd1306_printChar2x((i << 3) + (i << 2), 2, tx_state.msg_buffer[i], false);
    }
    for(uint8_t i = tx_state.current_msg_char; i < MESSAGE_BUFFER_MAX_LENGTH; i++) {
        ssd1306_printChar2x((i << 3) + (i << 2), 2, ' ', false);
    }
}

void skip_to_next_msg_char(void) {
  if(tx_state.current_msg_char >= MESSAGE_BUFFER_MAX_LENGTH - 1) {
      // Shift all existing chars to the left to accomodate one more
      for(uint8_t i = 0; i < MESSAGE_BUFFER_MAX_LENGTH - 1u; i++) {
          tx_state.msg_buffer[i] = tx_state.msg_buffer[i + 1u];
      }
      tx_state.msg_buffer[MESSAGE_BUFFER_MAX_LENGTH - 1u] = 0;
      redraw_message_buffer();
  } else {
      tx_state.current_msg_char++;
  }
  ssd1306_printText(127-5*6, 0, "     ", true);
}

void redraw_morse_transmission_screen(const State* _state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Transmisor", true);
  redraw_message_buffer();
}

void process_morse_tx_menu(State* state, const IoActions* actions) {
  if (actions->pressed_encoder) {
    silence_tone();
    open_selection_menu(state);
    return;
  }

  const MorseCharacter input_char = process_morse_input(&tx_state.input_data, actions);
  MorseCharacter char_to_draw;

  if(input_char.length > 0) {
    char_to_draw = input_char;
  } else {
    // Draw character being currently input
    char_to_draw = (MorseCharacter) { .length = tx_state.input_data.current_morse_element, .morse = tx_state.input_data.morse_buffer };
  }

  char translated_char = translate_morse(char_to_draw);
  if (translated_char) {
    ssd1306_printChar2x((tx_state.current_msg_char << 3) + (tx_state.current_msg_char << 2), 2,
                        translated_char, false);
    tx_state.msg_buffer[tx_state.current_msg_char] = translated_char;
  } else {
    tx_state.msg_buffer[tx_state.current_msg_char] = '?';
  }

  if(input_char.length > 0) {
    skip_to_next_msg_char();
  }
}

void open_morse_tx_menu(State* state) {
  state->menu_open = Menu_MorseTx;
  tx_state = (TxState) { 0 }; // Reset menu state
  redraw_morse_transmission_screen(state);
}
