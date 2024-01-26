#include "menus.h"

#include "settings.h"
#include "ssd1306.h"

#include <stdbool.h>
#include <stdint.h>

void redraw_settings_screen(const State* state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Preferencias", true);

  for (uint8_t i = SETTINGS_COUNT; i > 0; i--) {
    const bool is_setting_hovered = state->setting_hovered == (i - 1u);
    setting_params[i - 1u].redraw_fn(is_setting_hovered,
                                     is_setting_hovered && state->setting_is_selected);
  }
}

void process_settings_menu(State* state, const IoActions* actions) {
  // Close the settings menu if the rotary encoder button is pressed
    if (actions->pressed_morse_button) {
      open_selection_menu(state);
      redraw_selection_menu(state);
      silence_tone();
      return;
    }

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

  if (actions->pressed_encoder) {
    state->setting_is_selected = !state->setting_is_selected;
    setting_params[state->setting_hovered].redraw_fn(true, state->setting_is_selected);
  }
}
