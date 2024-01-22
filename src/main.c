#include "clock.h"
#include "i2c.h"
#include "io.h"
#include "menus.h"
#include "morse.h"
#include "rotary_encoder.h"
#include "settings.h"
#include "ssd1306.h"
#include "state.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

// Current pin connections:
//                      | P1.0      P2.6 |
//                      | P1.1      P2.7 |
//           Buzzer PWM | P1.2      TEST |
//                      | P1.3      #RST |
//  Rotary encoder butn | P1.4      P1.7 | I2C SDA
//                      | P1.5      P1.6 | I2C SCL
//                      | P2.0      P2.5 | Morse button
// Rotary encoder "DT"  | P2.1      P2.4 |
// Rotary encoder "CLK" | P2.2      P2.3 |

void setup_io(void) {
  // Rotary encoder inputs
    P1DIR &= ~BIT4;
    P1REN |= BIT4;
    P1OUT |= BIT4;
    P1IES |= BIT4;
    P1IFG = 0;
  P2DIR &= ~(BIT1 | BIT2);
  P2REN |= BIT1 | BIT2; // Pull-up
  P2OUT |= BIT1 | BIT2;
  P2IES &= ~BIT1; // Interrupt on rotary encoder rotation (rising edge in bit 1,
                  // falling edge in bit 2 & button)
  P2IES |= BIT2;
  P2IFG = 0; // Clear interrupt flags

  // Morse button
  P2DIR &= ~(BIT5);
  P2REN |= BIT5; // Pull-up
  P2OUT |= BIT5;
  P2IES |= BIT5; // Interrupt on morse button press (falling edge)

  // Buzzer PWM, use timer 0
  P2DIR |= BIT6;
  P2SEL |= BIT6;
  P2SEL &= ~BIT7;
  P2SEL2 &= ~(BIT6 | BIT7);

  P1DIR &= BIT2;  // Disable aux output, but configure it
  P1SEL |= BIT2;
  P1SEL2 &= ~BIT2;

  P1OUT &= ~BIT2;
  TA0CCTL0 = 0;
  TA0CCTL1 = OUTMOD_7;      // PWM reset/set
  TA0CCR0 = 0;              // Turn it off
  TA0CTL = TASSEL_1 | MC_1; // ACLK (12kHz), do not divide, up to CCR0

  // Use timer 1 for morse dit/dah classification & knowing when to start new
  // letter
  TA1CTL = TASSEL_1 | ID_3 | MC_1; // ACLK (12kHz), divide by 8, up to CCR0
  TA1CCR0 = 0;                     // Halt timer
  TA1CCTL0 = CCIE;                 // Interrupt when timer reaches value at CCR0

  // Initialize I2C to use with OLED display (Pins 1.6, 1.7)
  i2c_init();

  // Initialize SSD1306 OLED
  ssd1306_init();
}

int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
  Set_Clk(16);
  setup_io();

  __bis_SR_register(GIE);

  State state = {0};

  redraw_morse_transmission_screen(&state);

  for (;;) {
    P2IE |= BIT1 | BIT2 | BIT5;
    P1IE |= BIT4;
    LPM0;
    // Process the IO actions sent via interruptions
    const IoActions actions_to_process = io_actions;
    // Disable button interruptions to decrease I2C comms errors
    P2IE = 0;
    P1IE = 0;
    if (state.settings_menu_open) {
      process_settings_menu(&state, &actions_to_process);
    } else {
      process_morse_tx_menu(&state, &actions_to_process);
    }

    // Clear the actions since they have been processed
    io_actions = (IoActions){0};
  }
}
