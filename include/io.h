#pragma once

#include <msp430.h>

#include <stdbool.h>
#include <stdint.h>

// Rotary encoder direction
typedef enum { Cw, Ccw } ReDirection;

typedef union {
  struct {
    bool rotated_encoder : 1;
    // Only relevant if `rotated_encoder` is set.
    ReDirection encoder_direction : 1;
    bool pressed_encoder : 1;
    bool released_encoder : 1;

    bool pressed_morse_button : 1;
    bool released_morse_button : 1;

    bool adc10_conv_finished : 1;
    bool timer1_finished : 1;

    bool low_power_mode_requested : 1;
  };
  uint16_t u16;
} IoActions;
extern volatile IoActions io_actions;

// == Timer frequencies ==
// SMCLK (2MHz) divided by 2: 1MHz
#define AUDIO_TIMER_FREQUENCY (1000000ul)
// For usage with `play_tone`. Input a note's frequency to get its equivalent period expression
#define AUDIO_NOTE(freq) (AUDIO_TIMER_FREQUENCY / freq)
// ACLK (12kHz) divided by 8
#define TIMER1_FREQUENCY (12000ul / 8ul)

// == Setup functions ==
// Sets up MCLK to 16MHz, SMCLK to 2MHz and ACLK to 12kHz.
void setup_clocks(void);
// Configures all the peripherals used.
void setup_io(void);

// == Timer1 functions ==
// Starts timer1 in one-shot mode. It will interrupt and turn on `timer1_finished` after the time is
// over. Time is given as a value in 1/1500ths of a second.
void setup_timer(uint16_t time);
// Resets timer1 and stops any ongoing countdown.
void reset_timer(void);

// == Audio declarations ==
typedef enum { MorseOutput_Buzzer, MorseOutput_Aux } MorseOutput;

// Change the tone output used for the one given, disabling the other one.
void config_morse_output(MorseOutput);
// Play a tone with the period given. Period is given in microseconds.
void play_tone(uint16_t period);
// Silences the tone currently being played.
void silence_tone(void);

// == LPM (Low Power Mode) functions ==
// Resets the time counter to enable LPM.
void lpm_reset_time(void);
// Sets the amount of time needed to trigger LPM, given in number of 366ms periods.
void lpm_set_interval(uint8_t time);

// == ADC functions ==
// Starts an ADC conversion on A0 (P1.0). Used for generating pseudorandom numbers.
void start_adc_conv(void);
// Finishes the previously started ADC conversion by turning off the ADC and returns the value read.
// Call after `adc10_conv_finished` is set.
uint16_t finish_adc_conv(void);
