#include "menus.h"

#include "morse.h"
#include "ssd1306.h"
#include "settings.h"
#include "io.h"

#include <stdint.h>

void skip_to_next_msg_char(State* state);

void redraw_message_buffer(const State* state) {
    ssd1306_clearPage(2, false);
    ssd1306_clearPage(3, false);
    for(uint8_t i = 0; i < state->current_msg_char; i++) {
        ssd1306_printChar2x((i << 3) + (i << 2), 2, state->msg_buffer[i], false);
    }
}

void redraw_morse_transmission_screen(const State* state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_clearPage(7, true);
  ssd1306_printText(2, 0, "Transmisor", true);
  redraw_message_buffer(state);
}

void process_morse_tx_menu(State* state, const IoActions* actions) {
  if (actions->pressed_encoder) {
    state->settings_menu_open = true;
    redraw_settings_screen(state);
    return;
  }

  if (actions->pressed_morse_button) {
      // Turn on buzzer
      TA0CCR0 = tone_value;
      // Set up timer for 'dah' detection
      setup_timer(400);

      ssd1306_printChar(state->current_morse_element << 3, 7, '.', true);
  }
  if (actions->timer1_finished) {
      if(TA0CCR0) {
          // Buzzer is active, so we're sending an element: Must be a 'dah'
          state->morse_buffer |= 1 << state->current_morse_element;
          ssd1306_printChar(state->current_morse_element << 3, 7, '-', true);
      } else {
          // Buzzer is not active, so we're skipping to the next message character
          skip_to_next_msg_char(state);
      }
  }
  if (actions->released_morse_button && TA0CCR0) {
      // Turn off buzzer
      TA0CCR0 = 0;
      // Setup next character timer
      setup_timer(400);
      state->current_morse_element++;

      if (state->current_morse_element > 4) {
          // No consonant or vowel is longer than 4 morse elements, so go to next character directly
          skip_to_next_msg_char(state);
      } else {
          char translated_char =
              translate_morse(state->current_morse_element, state->morse_buffer);
          if (translated_char) {
            ssd1306_printChar2x((state->current_msg_char << 3) + (state->current_msg_char << 2), 2,
                                translated_char, false);
            state->msg_buffer[state->current_msg_char] = translated_char;
          } else {
              state->msg_buffer[state->current_msg_char] = '?';
          }
      }
  }

  /*

  // Draw previous character
  if (state->current_morse_element > 0) {
    const uint8_t element_idx = state->current_morse_element - 1;
    ssd1306_printChar(
        element_idx << 3, 7,
        (state->morse_buffer[state->current_msg_char] & (1 << element_idx)) ? '-' : '.', true);
  } else {
    // Clear elements
    for (uint8_t i = 132 / 6; i > 0; i -= 6) { ssd1306_printChar(i - 6, 7, ' ', true); }
  }*/
}

void skip_to_next_msg_char(State* state) {
    if(state->current_msg_char >= MESSAGE_BUFFER_MAX_LENGTH - 1) {
        // Shift all existing chars to the left to accomodate one more
        for(uint8_t i = 0; i < MESSAGE_BUFFER_MAX_LENGTH - 1; i++) {
            state->msg_buffer[i] = state->msg_buffer[i + 1u];
        }
        state->msg_buffer[MESSAGE_BUFFER_MAX_LENGTH - 1u] = 0;
        redraw_message_buffer(state);
    } else {
        state->current_msg_char++;
    }
    state->current_morse_element = 0;
    state->morse_buffer = 0;
    ssd1306_clearPage(7, true);
}
