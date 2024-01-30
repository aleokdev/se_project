#pragma once

#include "rotary_encoder.h"
#include "settings.h"

#include <msp430.h>

#include <stdbool.h>
#include <stdint.h>

// SMCLK (2MHz) divided by 2: 1MHz
#define AUDIO_TIMER_FREQUENCY (1000000ul)

typedef struct {
  bool rotated_encoder : 1;
  ReDirection encoder_direction : 1;
  bool pressed_encoder : 1;
  bool released_encoder : 1;

  bool adc10_conv_finished : 1;
  bool pressed_morse_button : 1;
  bool released_morse_button : 1;
  bool timer1_finished : 1;
  bool low_power_mode_requested : 1;
} IoActions;
extern volatile IoActions io_actions;

void setup_clocks(void);
void setup_io(void);

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

// Change the tone output used for the one given, disabling the other one
void config_morse_output(MorseOutput);
void play_tone(uint16_t tone_time);
void silence_tone(void);

bool is_encoder_pressed(void);

// LPM (Low Power Mode) functions

// Resets the time counter to enable LPM
void lpm_reset_time(void);
// Sets the amount of time needed to trigger LPM, given in number of 366ms periods
void lpm_set_interval(uint8_t time);

// Starts an ADC conversion on A0 (P1.0). Used for generating pseudorandom numbers.
void start_adc_conv(void);
// Finishes the previously started ADC conversion by turning off the ADC and returns the value read.
// Call after ADC10IFG is set.
uint16_t finish_adc_conv(void);
