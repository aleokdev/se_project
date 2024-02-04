#include "menus.h"

#include "ssd1306.h"

typedef struct {
  // The index of the menu currently being hovered.
  uint8_t menu_idx_hovered;
  // Identifies the menu that was last open, used for changing its display name to a message akin to
  // "Back".
  Menu last_menu;
} SelectionMenuState;
SelectionMenuState sstate;

void draw_menu_tab(Menu menu, bool is_hovered) {
  const char* cancel_text = "Volver";
  const char* menu_names[Menu_REGULAR_MENU_COUNT] = {"Transmisor", "Tabla de Morse", "Preferencias",
                                                     "Modo guia"};

  ssd1306_clearPage((uint8_t)menu, is_hovered);
  ssd1306_printText(2, (uint8_t)menu,
                    (uint8_t)sstate.last_menu == (uint8_t)menu ? cancel_text
                                                               : menu_names[(uint8_t)menu],
                    is_hovered);
}

void process_selection_menu(Menu* menu_open, const IoActions* actions) {
  if (actions->rotated_encoder) {
    uint8_t previous_tab_hovered = sstate.menu_idx_hovered;
    if (actions->encoder_direction == Cw) {
      if (sstate.menu_idx_hovered < (uint8_t)Menu_REGULAR_MENU_COUNT - 1) {
        sstate.menu_idx_hovered++;
      }
    } else {
      if (sstate.menu_idx_hovered > 0) {
        sstate.menu_idx_hovered--;
      }
    }

    draw_menu_tab((Menu)previous_tab_hovered, false);
    draw_menu_tab((Menu)sstate.menu_idx_hovered, true);
  }

  if (actions->pressed_encoder) {
    void (*open_functions[Menu_REGULAR_MENU_COUNT])(Menu*) = {
        open_morse_tx_menu, open_morse_table_menu, open_settings_menu, open_guide_menu};
    const uint8_t menu_to_open = sstate.menu_idx_hovered;
    sstate.menu_idx_hovered = 0;
    open_functions[menu_to_open](menu_open);
  }
}

void redraw_selection_menu(void) {
  for (uint8_t menu = 0; menu < (uint8_t)Menu_REGULAR_MENU_COUNT; menu++) {
    draw_menu_tab((Menu)menu, sstate.menu_idx_hovered == menu);
  }
  for (uint8_t page = Menu_REGULAR_MENU_COUNT; page < 8; page++) { ssd1306_clearPage(page, false); }
}

void open_selection_menu(Menu* menu_open) {
  sstate.last_menu = *menu_open;
  *menu_open = Menu_SelectMenu;
  sstate.menu_idx_hovered = (uint8_t)sstate.last_menu;
  redraw_selection_menu();
}
