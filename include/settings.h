#pragma once

#include "rotary_encoder.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MorseOutput_Buzzer,
    MorseOutput_Aux
} MorseOutput;

typedef struct {
    uint16_t tone_value;
    uint16_t dah_time;
    // Value goes from 0 to 7
    uint16_t tone_volume;
    MorseOutput output;
} Settings;

extern Settings settings;

// Reset setting values to their default.
void reset_settings(void);
// Load setting values from flash memory.
void load_settings(void);
// Save setting values to flash memory.
void save_settings(void);
