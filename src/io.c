#include "io.h"

#include "settings.h"

#include <msp430.h>

volatile IoActions io_actions = {0};

#pragma vector = TIMER1_A0_VECTOR
__interrupt void int_T1_0(void) {
  io_actions.timer1_finished = true;
  TA1CCR0 = 0;
  LPM0_EXIT;
}

#pragma vector = PORT1_VECTOR
__interrupt void p1v() {
    io_actions.pressed_encoder = true;

    P1IFG = 0;
    LPM0_EXIT;
}

#pragma vector = PORT2_VECTOR
__interrupt void p2v() {
  if (P2IFG & BIT5) {
      static bool is_morse_button_pressed = false;
      // Change interrupt edge select to call interrupt when the button is
      // released/pressed next time
      is_morse_button_pressed = !is_morse_button_pressed;
      io_actions.pressed_morse_button = is_morse_button_pressed;
      io_actions.released_morse_button = !is_morse_button_pressed;
      P2IES ^= BIT5;
  } else {
    // Bit 1 detects rising edges, bit 2 detects falling edges
    if (P2IFG & BIT1) {
      if (P2IN & BIT2) {
        // 1 and 2 are on, 2 turned on first
        io_actions.encoder_direction = Cw;
      } else {
        // 1 is on, 1 turned on first
        io_actions.encoder_direction = Ccw;
      }
    } else { // P2IFG & BIT2
      if (P2IN & BIT1) {
        // 2 is off, 2 turned off first
        io_actions.encoder_direction = Cw;
      } else {
        // 1 and 2 are off, 1 turned off first
        io_actions.encoder_direction = Ccw;
      }
    }
    io_actions.rotated_encoder = true;
  }
  P2IFG = 0;
  LPM0_EXIT;
}


void output_tone(uint16_t tone_time) {
    TA0CCR0 = tone_value;
    TA0CCR1 = (tone_value >> 4) * tone_volume;
    // Re-enable the PWM output: Set pin when TAR==T0CCR1, reset when TAR==T0CCR0
    TA0CCTL1 = OUTMOD_7;
}

void silence_tone(void) {
    // Reset the tone timer
    TA0CCR0 = 0;
    // Avoid leaving the PWM output on to prevent noise (set the PWM pins to PxOUT, which are 0)
    TA0CCTL1 = OUTMOD_0;
}
