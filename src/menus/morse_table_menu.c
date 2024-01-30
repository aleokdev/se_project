#include "menus.h"

#include "morse.h"
#include "ssd1306.h"

void process_morse_table_menu(State* state, const IoActions* actions) {
    if (actions->pressed_encoder) {
      open_selection_menu(state);
    }
}

void redraw_morse_table_screen(const State* _state) {
    ssd1306_clearDisplay();
    ssd1306_clearPage(0, true);
    ssd1306_printText(2, 0, "Tabla de Morse", true);
    uint8_t i = 0;
    for(uint8_t x = 2;; x += 31) {
        for(uint8_t y = 1; y < 8; y++) {
            uint16_t data = morse_characters[i];
            uint8_t char_len = (data >> 8) & 0xFF;
            uint8_t char_morse = data & 0xFF;

            ssd1306_printChar(x, y, 'A' + i, false);
            for(uint8_t m = 0; m < char_len; m++) {
                ssd1306_printChar(x + (m + 1) * 6, y, char_morse & (1 << m) ? '-' : '.', false);
            }
            i++;
            if(i >= sizeof(morse_characters) / 2) {
                return;
            }
        }
    }
}

void open_morse_table_menu(State* state) {
  state->menu_open = Menu_MorseTable;
  redraw_morse_table_screen(state);
}
