#include "settings.h"
#include "ssd1306.h"
#include <msp430.h>
#include <stdio.h>

// Default tone frequency: 12000 / 20 = 600 Hz
uint16_t tone_value = 19;
// Default dah time: 200 / 1500 * 1000 = 130ms
uint16_t dah_time = 199;

void redraw_volume(bool hovered, bool selected);
void redraw_tone(bool hovered, bool selected);
void redraw_dah_time(bool hovered, bool selected);
void changed_volume(ReDirection dir);
void changed_tone(ReDirection dir);
void changed_dah_time(ReDirection dir);

const SettingParams setting_params[SETTINGS_COUNT] = {
    {.redraw_fn = redraw_volume, .changed_fn = changed_volume},
    {.redraw_fn = redraw_tone, .changed_fn = changed_tone},
    {.redraw_fn = redraw_dah_time, .changed_fn = changed_dah_time}};

void redraw_volume(bool hovered, bool selected) {
  ssd1306_printText(0, 1, "  Volumen", hovered);
  ssd1306_printText(6 * 10, 1, "------*--", selected);
}

void redraw_tone(bool hovered, bool selected) {
  ssd1306_printText(0, 2, "     Tono", hovered);
  const uint16_t tone = 12000 / (tone_value + 1);
  char buffer[16];
  snprintf(buffer, 16, "%d Hz", tone);
  ssd1306_printText(6 * 10, 2, buffer, selected);
}

void redraw_dah_time(bool hovered, bool selected) {
  ssd1306_printText(0, 3, "Velocidad", hovered);
  const uint16_t speed = (dah_time + 1) * 10 / 15;
  char buffer[16];
  snprintf(buffer, 16, "%d ms/dah", speed);
  ssd1306_printText(6 * 10, 3, buffer, selected);
}

void changed_volume(ReDirection dir) {}

void changed_tone(ReDirection dir) {
  if (dir == Ccw) {
    if (tone_value < 100) {
        tone_value++;
    }
  } else {
    if (tone_value > 15) {
        tone_value--;
    }
  }
}

void changed_dah_time(ReDirection dir) {
    if (dir == Cw) {
      if (dah_time < 1000) {
          dah_time++;
      }
    } else {
      if (dah_time > 1) {
          dah_time--;
      }
    }
}
