#include "menus.h"

#include "morse.h"
#include "ssd1306.h"
#include "settings.h"
#include "io.h"

#include <stdint.h>

void skip_to_next_msg_char(State* state);

void redraw_message_buffer(const State* state) {
    for(uint8_t i = 0; i < state->current_msg_char; i++) {
        ssd1306_printChar2x((i << 3) + (i << 2), 2, state->msg_buffer[i], false);
    }
    for(uint8_t i = state->current_msg_char; i < MESSAGE_BUFFER_MAX_LENGTH; i++) {
        ssd1306_printChar2x((i << 3) + (i << 2), 2, ' ', false);
    }
}

void redraw_morse_transmission_screen(const State* state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_clearPage(7, true);
  ssd1306_printText(2, 0, "Transmisor", true);
  redraw_message_buffer(state);
}

typedef enum {
    TimerReason_RemoveLastChar,
    TimerReason_Dah,
    TimerReason_NextChar
} TimerReason;

void process_morse_tx_menu(State* state, const IoActions* actions) {
  static TimerReason last_timer_reason;
  const TimerReason timer_reason = last_timer_reason;
  if (actions->rotated_encoder && is_encoder_pressed()) {
      if (actions->encoder_direction == Ccw) {
          state->menu_open = Menu_Settings;
          redraw_settings_screen(state);
      } else {
          state->menu_open = Menu_MorseTable;
          redraw_morse_table_screen(state);
      }
    silence_tone();
    return;
  }

  if(actions->pressed_encoder && !is_timer_setup()) {
      // Set up timer for half a second to remove last character
      last_timer_reason = TimerReason_RemoveLastChar;
      setup_timer(500);
  }
  if(actions->released_encoder && is_timer_setup() && timer_reason == TimerReason_RemoveLastChar) {
      reset_timer();
  }

  if (actions->pressed_morse_button) {
      output_tone(tone_value);
      ssd1306_printChar(state->current_morse_element << 3, 7, '.', true);

      // Set up timer for 'dah' detection
      last_timer_reason = TimerReason_Dah;
      setup_timer(dah_time);
  }
  if (actions->released_morse_button && TA0CCR0) {
      silence_tone();
      // Setup next character timer
      last_timer_reason = TimerReason_NextChar;
      setup_timer(dah_time);
      state->current_morse_element++;

      char translated_char =
          translate_morse(state->current_morse_element, state->morse_buffer);
      if (translated_char) {
        ssd1306_printChar2x((state->current_msg_char << 3) + (state->current_msg_char << 2), 2,
                            translated_char, false);
        state->msg_buffer[state->current_msg_char] = translated_char;
      } else {
          state->msg_buffer[state->current_msg_char] = '?';
      }

     if (state->current_morse_element >= 4) {
         // No consonant or vowel is longer than 4 morse elements, so go to next character directly
         skip_to_next_msg_char(state);
     }
  }

  if (actions->timer1_finished) {
      switch(timer_reason) {
      case TimerReason_RemoveLastChar:
          if(is_encoder_pressed() && state->current_msg_char > 0) {
              state->current_msg_char--;
              redraw_message_buffer(state);
          }
          break;

      case TimerReason_Dah: {
          state->morse_buffer |= 1 << state->current_morse_element;
          ssd1306_printChar(state->current_morse_element << 3, 7, '-', true);

          char translated_char =
              translate_morse(state->current_morse_element, state->morse_buffer);
          if (translated_char) {
            ssd1306_printChar2x((state->current_msg_char << 3) + (state->current_msg_char << 2), 2,
                                translated_char, false);
            state->msg_buffer[state->current_msg_char] = translated_char;
          } else {
              state->msg_buffer[state->current_msg_char] = '?';
          }
          break;
      }

      case TimerReason_NextChar:
          if(state->current_morse_element > 0) {
              skip_to_next_msg_char(state);
          }
          break;
      }
  }
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
