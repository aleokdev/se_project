#pragma once

#include "rotary_encoder.h"

#include <msp430.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool rotated_encoder : 1;
  ReDirection encoder_direction : 1;
  bool pressed_encoder : 1;
  bool released_encoder : 1;

  bool pressed_morse_button : 1;
  bool released_morse_button : 1;
  bool timer1_finished : 1;
  bool low_power_mode_requested : 1;
} IoActions;
extern volatile IoActions io_actions;

inline void reset_timer(void) {
    TA1CCR0 = 0;
    TA1CTL |= TACLR;
}

// Time is given as a value in 1/1500ths of a second
inline void setup_timer(uint16_t time) {
    reset_timer();
    TA1CCR0 = time;
    TA1CTL &= ~TACLR;
}

inline bool is_timer_setup(void) {
    return TA1CCR0;
}

void output_tone(uint16_t tone_time);
void silence_tone(void);

bool is_encoder_pressed(void);

// LPM (Low Power Mode) functions

// Resets the time counter to enable LPM
void lpm_reset_time(void);

// Sets the amount of time needed to trigger LPM, given in number of 366ms periods
void lpm_set_interval(uint8_t time);
