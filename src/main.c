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
//                      | P1.4      P1.7 | I2C SDA
//         Morse button | P1.5      P1.6 | I2C SCL
//  Rotary encoder butn | P2.0      P2.5 |
// Rotary encoder "DT"  | P2.1      P2.4 |
// Rotary encoder "CLK" | P2.2      P2.3 |

void setup_io(void) {
  // Rotary encoder inputs
  P2DIR &= ~(BIT1 | BIT2 | BIT0);
  P2REN |= BIT1 | BIT2 | BIT0; // Pull-up
  P2OUT |= BIT1 | BIT2 | BIT0;
  P2IES &= ~BIT1; // Interrupt on rotary encoder rotation (rising edge in bit 1,
                  // falling edge in bit 2 & button)
  P2IES |= BIT2 | BIT0;
  P2IFG = 0; // Clear interrupt flags

  // Morse button
  P1DIR &= ~(BIT5);
  P1REN |= BIT5; // Pull-up
  P1OUT |= BIT5;
  P1IES |= BIT5; // Interrupt on morse button press (falling edge)
  P1IFG = 0;     // Clear interrupt flags

  // Buzzer PWM, use timer 0
  P1DIR |= BIT2;
  P1SEL |= BIT2;
  P1SEL2 &= ~BIT2;
  TA0CCTL0 = 0;
  TA0CCTL1 = OUTMOD_7;      // PWM reset/set
  TA0CTL = TASSEL_1 | MC_1; // ACLK (12kHz), do not divide, up to CCR0
  TA0CCR0 = 19;             // Frequency: 12000 / 20 = 600 Hz
  TA0CCR1 = 0;              // Initial duty cycle 0% (off)

  // Use timer 1 for morse dit/dah classification & knowing when to start new
  // letter
  TA1CTL = TASSEL_1 | ID_3 | MC_1; // ACLK (12kHz), divide by 8, up to CCR0
  TA1CCR0 = 0;                     // halt timer

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
    P2IE |= BIT1 | BIT2 | BIT0;
    P1IE |= BIT5;
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
