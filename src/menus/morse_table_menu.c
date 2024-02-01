#include "menus.h"

#include "morse.h"
#include "ssd1306.h"

void process_morse_table_menu(Menu* menu_open, const IoActions* actions) {
    if (actions->pressed_encoder) {
      open_selection_menu(menu_open);
    }
}

void redraw_morse_table_menu(void) {
    ssd1306_clearDisplay();
    ssd1306_clearPage(0, true);
    ssd1306_printText(2, 0, "Tabla de Morse", true);
    uint8_t i = 'A';
    for(uint8_t x = 2;; x += 31) {
        for(uint8_t y = 1; y < 8; y++) {
            MorseCharacter ch = get_morse_translation(i);

            ssd1306_printChar(x, y, i, false);
            for(uint8_t m = 0; m < ch.length; m++) {
                ssd1306_printChar(x + (m + 1) * 6, y, ch.morse & (1 << m) ? '-' : '.', false);
            }
            if(++i >= 'Z') {
                return;
            }
        }
    }
}

void open_morse_table_menu(Menu* menu_open) {
  *menu_open = Menu_MorseTable;
  redraw_morse_table_menu();
}
