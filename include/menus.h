#pragma once
#include "io.h"

typedef enum {
  Menu_MorseTx = 0,
  Menu_MorseTable = 1,
  Menu_Settings = 2,
  Menu_Guide = 3,

  Menu_REGULAR_MENU_COUNT,

  Menu_SelectMenu,
} Menu;

// These functions do a menu's processing.
// A menu's `open_*_menu()` function should be called before its process one.
void process_morse_tx_menu(Menu* menu_open, const IoActions* actions);
void process_morse_table_menu(Menu* menu_open, const IoActions* actions);
void process_settings_menu(Menu* menu_open, const IoActions* actions);
void process_guide_menu(Menu* menu_open, const IoActions* actions);
void process_selection_menu(Menu* menu_open, const IoActions* actions);

void open_morse_tx_menu(Menu* menu_open);
void open_morse_table_menu(Menu* menu_open);
void open_settings_menu(Menu* menu_open);
void open_guide_menu(Menu* menu_open);
void open_selection_menu(Menu* menu_open);
