#include "settings.h"
#include "ssd1306.h"
#include <stdio.h>
#include <msp430.h>

void redraw_volume(bool hovered, bool selected);
void redraw_tone(bool hovered, bool selected);
void changed_volume(ReDirection dir);
void changed_tone(ReDirection dir);


const SettingParams setting_params[SETTINGS_COUNT] = {
    {.redraw_fn = redraw_volume, .changed_fn = changed_volume},
    {.redraw_fn = redraw_tone, .changed_fn = changed_tone}};

void redraw_volume(bool hovered, bool selected) {
  ssd1306_printText(0, 1, "  Volumen", hovered);
  ssd1306_printText(6 * 10, 1, "------*--", selected);
}

void redraw_tone(bool hovered, bool selected) {
  ssd1306_printText(0, 2, "     Tono", hovered);
  const uint16_t tone = 12000 / (TA0CCR0 + 1);
  char buffer[16];
  snprintf(buffer, 16, "%d Hz", tone);
  ssd1306_printText(6 * 10, 2, buffer, selected);
}

void changed_volume(ReDirection dir) {

}

void changed_tone(ReDirection dir) {
    if(dir == Ccw) {
        if(TA0CCR0 < 100) {
            TA0CCR0++;
        }
    } else {
        if(TA0CCR0 > 15) {
            TA0CCR0--;
        }
    }
}
