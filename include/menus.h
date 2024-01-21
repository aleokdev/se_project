#pragma once
#include "state.h"
#include "io.h"

void redraw_morse_transmission_screen(void);
void redraw_settings_screen(const State*);

void process_settings_menu(State* state, const IoActions* actions) {
    if (actions->rotated_encoder) {
        if (!state->setting_is_selected) {
            const uint8_t last_setting_hovered = state->setting_hovered;

            if (actions->encoder_direction == Cw) {
              if (state->setting_hovered < SETTINGS_COUNT - 1) {
                state->setting_hovered++;
              }
            } else {
              if (state->setting_hovered > 0) {
                state->setting_hovered--;
              }
            }

            if (last_setting_hovered != state->setting_hovered) {
              setting_params[last_setting_hovered].redraw_fn(false, false);
              setting_params[state->setting_hovered].redraw_fn(true, false);
            }
        } else {
            setting_params[state->setting_hovered].changed_fn(actions->encoder_direction);
            setting_params[state->setting_hovered].redraw_fn(true, true);
        }
    }

    if(actions->pressed_encoder) {
        state->setting_is_selected = !state->setting_is_selected;
        setting_params[state->setting_hovered].redraw_fn(true, state->setting_is_selected);
    }

    if (actions->pressed_morse_button && !state->setting_is_selected) {
        state->settings_menu_open = false;
        redraw_morse_transmission_screen();
    }
}

void process_morse_tx_menu(State* state, const IoActions* actions) {
    if (actions->pressed_encoder) {
        state->settings_menu_open = true;
        redraw_settings_screen(state);
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



void redraw_morse_transmission_screen(void) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_clearPage(7, true);
  ssd1306_printText(0, 0, "Transmisor", true);
}

void redraw_settings_screen(const State* state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(0, 0, "Preferencias", true);

  for (uint8_t i = SETTINGS_COUNT; i > 0; i--) {
    const bool is_setting_hovered = state->setting_hovered == (i - 1u);
    setting_params[i - 1u].redraw_fn(is_setting_hovered, is_setting_hovered && state->setting_is_selected);
  }
}
