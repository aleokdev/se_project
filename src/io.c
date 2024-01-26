#include "io.h"

#include "settings.h"

#include "i2c.h"
#include "ssd1306.h"

#include <msp430.h>

volatile IoActions io_actions = {0};
uint8_t lpm_counter = 0;
// About 30s of inactive time
uint8_t lpm_trigger = 11;

void setup_clocks(void) {
  // Setup MCLK & SMCLK to 16MHz using the DCO
  BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;   // MCLK source: DCO, divide by 1, SMCLK source: DCO, divide by 1
  if (CALBC1_16MHZ != 0xFF) {
    DCOCTL = 0x00;        // Use lowest DCO and modulation settings, as indicated by the datasheet
    // Use flash info memory calibration data to setup DCO to 16MHz
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
  }

  // Setup ACLK to 12kHZ using the VLO
  BCSCTL1 |= XT2OFF | DIVA_0; // Disable XT2 oscillator, set ACLK divider to 1
  BCSCTL3 = LFXT1S_2;         // ACLK source: VLO
}

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
  P2SEL |= BIT6;
  P2SEL &= ~BIT7;
  P2SEL2 &= ~(BIT6 | BIT7);
  // Aux config
  P1SEL |= BIT2;
  P1SEL2 &= ~BIT2;
  // Select the appropiate output depending on the settings
  config_morse_output(settings.output);

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

  // Use watchdog timer to enter low-power mode after 1min
  WDTCTL = WDTPW | WDTTMSEL | WDTHOLD | WDTCNTCL ; // Interval mode, halt timer, clear counter
  WDTCTL = WDTPW | WDTTMSEL | WDTSSEL ; // Interval mode, ACLK (12kHz) / 32768
  IE1 |= WDTIE;                         // Enable interrupts

  // Initialize I2C to use with OLED display (Pins 1.6, 1.7)
  i2c_init();

  // Initialize SSD1306 OLED
  ssd1306_init();
}

#pragma vector = WDT_VECTOR
__interrupt void int_wdt(void) {
    if(++lpm_counter > lpm_trigger) {
        io_actions.low_power_mode_requested = true;
        LPM4_EXIT;
    }
}

void lpm_reset_time(void) {
    lpm_counter = 0;
    WDTCTL = WDTPW | WDTTMSEL | WDTSSEL | WDTCNTCL ; // Interval mode, ACLK (12kHz) / 32768, clear counter
    WDTCTL = WDTPW | WDTTMSEL | WDTSSEL ; // Interval mode, ACLK (12kHz) / 32768
}

void lpm_set_interval(uint8_t time) {
    lpm_trigger = time;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void int_T1_0(void) {
  io_actions.timer1_finished = true;
  TA1CCR0 = 0;
  LPM4_EXIT;
}

#pragma vector = PORT1_VECTOR
__interrupt void p1v() {
    static bool is_encoder_pressed = false;
    // Change interrupt edge select to call interrupt when the button is
    // released/pressed next time
    is_encoder_pressed = !is_encoder_pressed;
    io_actions.pressed_encoder = is_encoder_pressed;
    io_actions.released_encoder = !is_encoder_pressed;
    P1IES ^= BIT4;

    P1IFG = 0;
    LPM4_EXIT;
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
  LPM4_EXIT;
}

void config_morse_output(MorseOutput output) {
    switch(output) {
    case MorseOutput_Aux:
        P2DIR &= BIT6;  // Disable buzzer output
        P1DIR |= BIT2;  // Enable aux output
        break;
    case MorseOutput_Buzzer:
        P2DIR |= BIT6;  // Enable buzzer output
        P1DIR &= BIT2;  // Disable aux output
        break;
    }
}

void silence_tone(void) {
    // Reset the tone timer
    TA0CCR0 = 0;
    // Avoid leaving the PWM output on to prevent noise (set the PWM pins to PxOUT, which are 0)
    TA0CCTL1 = OUTMOD_0;
}

void play_tone(uint16_t tone_time) {
    TA0CCR0 = tone_time;
    TA0CCR1 = (tone_time >> 4) * settings.tone_volume;
    // Re-enable the PWM output: Set pin when TAR==T0CCR1, reset when TAR==T0CCR0
    TA0CCTL1 = OUTMOD_7;
}

bool is_encoder_pressed(void) {
    return !(P1IN & BIT4);
}
