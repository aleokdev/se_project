#pragma once
#include "state.h"
#include "io.h"

void process_settings_menu(State* state, const IoActions* actions);
void process_morse_tx_menu(State* state, const IoActions* actions);

void redraw_morse_transmission_screen(const State*);
void redraw_settings_screen(const State*);
