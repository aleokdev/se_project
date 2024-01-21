#include "menus.h"

#include "morse.h"
#include "ssd1306.h"

#include <stdint.h>

void redraw_morse_transmission_screen(const State* _state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_clearPage(7, true);
  ssd1306_printText(2, 0, "Transmisor", true);
}

void process_morse_tx_menu(State* state, const IoActions* actions) {
    if (actions->pressed_encoder) {
        state->settings_menu_open = true;
        redraw_settings_screen(state);
        return;
    }

  char translated_char =
      translate_morse(state->current_morse_element,
                      state->morse_buffer[state->current_msg_char]);
  if (translated_char) {
    ssd1306_printChar2x((state->current_msg_char << 3) +
                            (state->current_msg_char << 2),
                        2, translated_char, false);
  }

  // Draw previous character
  if (state->current_morse_element > 0) {
    const uint8_t element_idx = state->current_morse_element - 1;
    ssd1306_printChar(
        element_idx << 3, 7,
        (state->morse_buffer[state->current_msg_char] & (1 << element_idx))
            ? '-'
            : '.',
        true);
  } else {
    // Clear elements
    for (uint8_t i = 132 / 6; i > 0; i -= 6) {
      ssd1306_printChar(i - 6, 7, ' ', true);
    }
  }
}
