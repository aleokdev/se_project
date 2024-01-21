#include "io.h"

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
  // Change interrupt edge select to call interrupt when the button is
  // released/pressed next time
  io_actions.pressed_morse_button = !io_actions.pressed_morse_button;
  P1IES = ~P1IES;

  P1IFG = 0;
  LPM0_EXIT;
}

#pragma vector = PORT2_VECTOR
__interrupt void p2v() {
  if (P2IFG & BIT0) {
      io_actions.pressed_encoder = true;
  } else {
    // Bit 1 detects rising edges, bit 2 detects falling edges
    if (P2IFG & BIT1) {
      if (P2IN & BIT2) {
        // 1 and 2 are on, 2 turned on first
        io_actions.encoder_direction = Ccw;
      } else {
        // 1 is on, 1 turned on first
          io_actions.encoder_direction = Cw;
      }
    } else { // P2IFG & BIT2
      if (P2IN & BIT1) {
        // 2 is off, 2 turned off first
          io_actions.encoder_direction = Ccw;
      } else {
        // 1 and 2 are off, 1 turned off first
          io_actions.encoder_direction = Cw;
      }
    }
    io_actions.rotated_encoder = true;
  }
  P2IFG = 0;
  LPM0_EXIT;
}
