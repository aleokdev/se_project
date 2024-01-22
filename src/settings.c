#include "settings.h"
#include "ssd1306.h"
#include <msp430.h>
#include <stdio.h>

// Default tone frequency: 12000 / 20 = 600 Hz
uint16_t tone_value = 19;
// Default dah time: 200 / 1500 * 1000 = 130ms
uint16_t dah_time = 199;
// Default tone volume: About half-way
uint16_t tone_volume = 3;

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
    if(P2DIR & BIT6) {
        ssd1306_printText(6 * 10, 1, "Buzzer", selected);
    } else {
        ssd1306_printText(6 * 10, 1, "3.5mm Aux", selected);
    }
}

void redraw_volume(bool hovered, bool selected) {
  ssd1306_printText(0, 2, "  Volumen", hovered);
  ssd1306_printText(6 * 10, 2, "-------", selected);
  ssd1306_printChar(6 * 10 + tone_volume * 6, 2, '*', selected);
}

void redraw_tone(bool hovered, bool selected) {
  ssd1306_printText(0, 3, "     Tono", hovered);
  const uint16_t tone = 12000 / (tone_value + 1);
  char buffer[16];
  snprintf(buffer, 16, "%d Hz", tone);
  ssd1306_printText(6 * 10, 3, buffer, selected);
}

void redraw_dah_time(bool hovered, bool selected) {
  ssd1306_printText(0, 4, "Velocidad", hovered);
  const uint16_t speed = (dah_time + 1) * 10 / 15;
  char buffer[16];
  snprintf(buffer, 16, "%d ms/dah", speed);
  ssd1306_printText(6 * 10, 4, buffer, selected);
}

void changed_output_sel(ReDirection dir) {
    if (dir == Cw) {
        // Buzzer
        P2DIR |= BIT6;
        P1DIR &= ~BIT2;
    } else {
        // Aux
        P1DIR |= BIT2;
        P2DIR &= ~BIT6;
    }

}

void changed_volume(ReDirection dir) {
    if (dir == Cw) {
      if (tone_volume < 7) {
          tone_volume++;
      }
    } else {
      if (tone_volume > 0) {
          tone_volume--;
      }
    }
}

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
