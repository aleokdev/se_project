#pragma once
#include "io.h"

typedef enum {
    Menu_MorseTx,
    Menu_Settings,
    Menu_MorseTable,
} Menu;

#include "state.h"

void process_settings_menu(State* state, const IoActions* actions);
void process_morse_tx_menu(State* state, const IoActions* actions);
void process_morse_table_menu(State* state, const IoActions* actions);

void redraw_morse_transmission_screen(const State*);
void redraw_settings_screen(const State*);
void redraw_morse_table_screen(const State*);
