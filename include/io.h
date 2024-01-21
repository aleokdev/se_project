#pragma once

#include "rotary_encoder.h"

#include <msp430.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool rotated_encoder : 1;
    ReDirection encoder_direction : 1;
    bool pressed_encoder : 1;

    bool pressed_morse_button : 1;
    bool timer1_finished : 1;
} IoActions;
extern volatile IoActions io_actions;
