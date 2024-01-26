#include "menus.h"

#include "rotary_encoder.h"
#include "settings.h"
#include "ssd1306.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  void (*redraw_fn)(bool /* hovered */, bool /* selected */);
  void (*changed_fn)(ReDirection /* direction the rotary encoder has been rotated towards */);
} SettingParams;

#define SETTINGS_COUNT 4

void redraw_output_sel(bool hovered, bool selected);
void redraw_volume(bool hovered, bool selected);
void redraw_tone(bool hovered, bool selected);
void redraw_dah_time(bool hovered, bool selected);
void changed_output_sel(ReDirection dir);
void changed_volume(ReDirection dir);
void changed_tone(ReDirection dir);
void changed_dah_time(ReDirection dir);

const SettingParams setting_params[SETTINGS_COUNT] = {
    {.redraw_fn = redraw_output_sel, .changed_fn = changed_output_sel},
    {.redraw_fn = redraw_volume, .changed_fn = changed_volume},
    {.redraw_fn = redraw_tone, .changed_fn = changed_tone},
    {.redraw_fn = redraw_dah_time, .changed_fn = changed_dah_time}};

void redraw_output_sel(bool hovered, bool selected) {
    ssd1306_clearPage(1, false);
    ssd1306_printText(0, 1, "   Salida", hovered);
    switch(settings.output) {
    case MorseOutput_Buzzer:
        ssd1306_printText(6 * 10, 1, "Buzzer", selected);
        break;
    case MorseOutput_Aux:
        ssd1306_printText(6 * 10, 1, "3.5mm Aux", selected);
        break;
    }
}

void redraw_volume(bool hovered, bool selected) {
  ssd1306_printText(0, 2, "  Volumen", hovered);
  ssd1306_printText(6 * 10, 2, "--------", selected);
  ssd1306_printChar(6 * 10 + settings.tone_volume * 6, 2, '*', selected);
  if(selected) {
    play_tone(settings.tone_value);
  } else {
    silence_tone();
  }
}

void redraw_tone(bool hovered, bool selected) {
  ssd1306_printText(0, 3, "     Tono", hovered);
  const uint16_t tone = AUDIO_TIMER_FREQUENCY / (settings.tone_value + 1);
  char buffer[16];
  snprintf(buffer, 16, "%d Hz", tone);
  ssd1306_printText(6 * 10, 3, buffer, selected);
  if(selected) {
    play_tone(settings.tone_value);
  } else {
    silence_tone();
  }
}

void redraw_dah_time(bool hovered, bool selected) {
  ssd1306_printText(0, 4, "Velocidad", hovered);
  const uint16_t speed = (settings.dah_time + 1) * 10 / 15;
  char buffer[16];
  snprintf(buffer, 16, "%d ms/dah", speed);
  ssd1306_printText(6 * 10, 4, buffer, selected);
}

void changed_output_sel(ReDirection dir) {
    if (dir == Cw) {
        settings.output = MorseOutput_Buzzer;
    } else {
        settings.output = MorseOutput_Aux;
    }
    config_morse_output(settings.output);
}

void changed_volume(ReDirection dir) {
    if (dir == Cw) {
      if (settings.tone_volume < 7) {
          settings.tone_volume++;
      }
    } else {
      if (settings.tone_volume > 0) {
          settings.tone_volume--;
      }
    }

    play_tone(settings.tone_value);
}

void changed_tone(ReDirection dir) {
  if (dir == Ccw) {
    if (settings.tone_value < 2000) {
        settings.tone_value+=10;
    }
  } else {
    if (settings.tone_value > 1000) {
        settings.tone_value-=10;
    }
  }

  play_tone(settings.tone_value);
}

void changed_dah_time(ReDirection dir) {
    if (dir == Cw) {
      if (settings.dah_time < 1000) {
          settings.dah_time++;
      }
    } else {
      if (settings.dah_time > 1) {
          settings.dah_time--;
      }
    }
}



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
      save_settings();
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
