#include "menus.h"

#include "ssd1306.h"

Menu last_menu;

void draw_menu_tab(Menu menu, bool is_hovered) {
    const char* cancel_text = "Volver";
    const char* menu_names[Menu_REGULAR_MENU_COUNT] = {
                                 "Transmisor",
                                 "Tabla de Morse",
                                 "Preferencias",
                                 "Modo guia"
    };

    ssd1306_clearPage((uint8_t)menu, is_hovered);
    ssd1306_printText(2, (uint8_t)menu, (uint8_t)last_menu == (uint8_t)menu ? cancel_text : menu_names[(uint8_t)menu], is_hovered);
}

void process_selection_menu(State* state, const IoActions* actions) {
    if (actions->rotated_encoder) {
        uint8_t previous_tab_hovered = state->setting_hovered;
        if (actions->encoder_direction == Cw) {
          if (state->setting_hovered < (uint8_t)Menu_REGULAR_MENU_COUNT - 1) {
            state->setting_hovered++;
          }
        } else {
          if (state->setting_hovered > 0) {
            state->setting_hovered--;
          }
        }

        draw_menu_tab((Menu)previous_tab_hovered, false);
        draw_menu_tab((Menu)state->setting_hovered, true);
    }

    if (actions->pressed_encoder) {
      void(*open_functions[Menu_REGULAR_MENU_COUNT]) (State*) = {
                                                                open_morse_tx_menu,
                                                                open_morse_table_menu,
                                                                open_settings_menu,
                                                                open_guide_menu
      };
      const uint8_t menu_to_open = state->setting_hovered;
      state->setting_hovered = 0;
      open_functions[menu_to_open](state);
    }
}

void redraw_selection_menu(const State* state) {
    for(uint8_t menu = 0; menu < (uint8_t)Menu_REGULAR_MENU_COUNT; menu++) {
        draw_menu_tab((Menu)menu, state->setting_hovered == menu);
    }
    for(uint8_t page = Menu_REGULAR_MENU_COUNT; page < 8; page++) {
      ssd1306_clearPage(page, false);
    }
}

void open_selection_menu(State* state) {
    last_menu = state->menu_open;
    state->setting_hovered = (uint8_t)last_menu;
    state->menu_open = Menu_SelectMenu;
    redraw_selection_menu(state);
}
