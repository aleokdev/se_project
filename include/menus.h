#pragma once
#include "io.h"

typedef enum {
    Menu_MorseTx = 0,
    Menu_MorseTable = 1,
    Menu_Settings = 2,

    Menu_REGULAR_MENU_COUNT,

    Menu_SelectMenu,
} Menu;

#include "state.h"

void process_morse_tx_menu(State* state, const IoActions* actions);
void process_selection_menu(State* state, const IoActions* actions);
void process_settings_menu(State* state, const IoActions* actions);
void process_morse_table_menu(State* state, const IoActions* actions);

void redraw_morse_transmission_screen(const State*);
void redraw_selection_menu(const State*);
void redraw_settings_screen(const State*);
void redraw_morse_table_screen(const State*);

void open_selection_menu(State*);
