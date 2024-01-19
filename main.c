#include "i2c.h"
#include "ssd1306.h"
#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void Set_Clk(char VEL) {
  BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
  switch (VEL) {
  case 1:
    if (CALBC1_1MHZ != 0xFF) {
      DCOCTL = 0x00;
      BCSCTL1 = CALBC1_1MHZ; /* Set DCO to 1MHz */
      DCOCTL = CALDCO_1MHZ;
    }
    break;
  case 8:

    if (CALBC1_8MHZ != 0xFF) {
      __delay_cycles(100000);
      DCOCTL = 0x00;
      BCSCTL1 = CALBC1_8MHZ; /* Set DCO to 8MHz */
      DCOCTL = CALDCO_8MHZ;
    }
    break;
  case 12:
    if (CALBC1_12MHZ != 0xFF) {
      __delay_cycles(100000);
      DCOCTL = 0x00;
      BCSCTL1 = CALBC1_12MHZ; /* Set DCO to 12MHz */
      DCOCTL = CALDCO_12MHZ;
    }
    break;
  case 16:
    if (CALBC1_16MHZ != 0xFF) {
      __delay_cycles(100000);
      DCOCTL = 0x00;
      BCSCTL1 = CALBC1_16MHZ; /* Set DCO to 16MHz */
      DCOCTL = CALDCO_16MHZ;
    }
    break;
  default:
    if (CALBC1_1MHZ != 0xFF) {
      DCOCTL = 0x00;
      BCSCTL1 = CALBC1_1MHZ; /* Set DCO to 1MHz */
      DCOCTL = CALDCO_1MHZ;
    }
    break;
  }
  BCSCTL1 |= XT2OFF | DIVA_0;
  BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;
}

#define MESSAGE_BUFFER_MAX_LENGTH 6

// Current pin connections:
//                      | P1.0      P2.6 |
//                      | P1.1      P2.7 |
//           Buzzer PWM | P1.2      TEST |
//                      | P1.3      #RST |
//                      | P1.4      P1.7 | I2C SDA
//         Morse button | P1.5      P1.6 | I2C SCL
//                      | P2.0      P2.5 |
// Rotary encoder "DT"  | P2.1      P2.4 |
// Rotary encoder "CLK" | P2.2      P2.3 |

// Rotary encoder direction
typedef enum { Cw, Ccw } ReDirection;

volatile ReDirection direction = Cw;
volatile bool rotated_rotary_encoder = false;
volatile bool morse_button = false;

typedef struct {
    // Each byte here represents a morse character, and each bit represents either a dit (0) or a dah (1).
    uint8_t morse_buffer[MESSAGE_BUFFER_MAX_LENGTH];
    // Character index being sent right now on the morse buffer
    uint8_t current_msg_char;
    // Current element within the character being sent right now
    uint8_t current_morse_element;
} State;
volatile State state = {0};

const static uint16_t morse_characters[26] = {
    (2 << 8) | 0b10, // A
    (4 << 8) | 0b0001, // B
    (4 << 8) | 0b0101, // C
    (3 << 8) | 0b001, // D
    (1 << 8) | 0b0, // E
    (4 << 8) | 0b0100, // F
    (3 << 8) | 0b011, // G
    (4 << 8) | 0b0000, // H
    (2 << 8) | 0b00, // I
    (4 << 8) | 0b1110, // J
    (3 << 8) | 0b101, // K
    (4 << 8) | 0b0010, // L
    (2 << 8) | 0b11, // M
    (2 << 8) | 0b01, // N
    (3 << 8) | 0b111, // O
    (4 << 8) | 0b0110, // P
    (4 << 8) | 0b1011, // Q
    (3 << 8) | 0b010, // R
    (3 << 8) | 0b000, // S
    (1 << 8) | 0b1, // T
    (3 << 8) | 0b100, // U
    (4 << 8) | 0b1000, // V
    (3 << 8) | 0b110, // W
    (4 << 8) | 0b1001, // X
    (4 << 8) | 0b1101, // Y
    (4 << 8) | 0b0011, // Z
};

// Returns the given morse code as an ASCII character, or 0 if it does not match any
char translate_morse(uint8_t morse_count, uint8_t morse) {
    for(int i = 0; i < sizeof(morse_characters) / 2; i++) {
        uint16_t data = morse_characters[i];
        uint8_t char_len = (data >> 8) & 0xFF;
        uint8_t char_morse = data & 0xFF;
        if(morse_count == char_len && morse == char_morse) {
            return i + 'A';
        }
    }

    return 0;
}


int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
  Set_Clk(16);

  // Rotary encoder inputs
  P2DIR &= ~(BIT1 | BIT2); //
  P2IES = BIT1 | BIT2;     // Interrupt on rotary encoder rotation (falling edge)
  P2IFG = 0;               // Clear interrupt flags

  // Morse button
  P1DIR &= ~(BIT5);
  P1REN |= BIT5;           // Pull-up
  P1OUT |= BIT5;
  P1IES = BIT5;            // Interrupt on morse button press (falling edge)

  // Buzzer PWM, use timer 0
  P1DIR |= BIT2;
  P1SEL |= BIT2;
  P1SEL2 &= ~BIT2;
  TA0CCTL0 = 0;
  TA0CCTL1 = OUTMOD_7; // PWM reset/set
  TA0CTL = TASSEL_1 | MC_1; // ACLK (12kHz), do not divide, up to CCR0
  TA0CCR0 = 25; // Frequency: 12000 / 26 = ~440 Hz
  TA0CCR1 = 0; // Initial duty cycle 0% (off)

  // Use timer 1 for morse dit/dah classification
  TA1CTL = TASSEL_1 | ID_3 | MC_1; // ACLK (12kHz), divide by 8, up to CCR0
  TA1CCR0 = 0;                     // halt timer

  i2c_init(); // initialize I2C to use with OLED

  ssd1306_init();         // Initialize SSD1306 OLED
  ssd1306_clearDisplay(); // Clear OLED display

  __bis_SR_register(GIE);

  int32_t number = 50;

  while (1) {
    ssd1306_printUI32(0, 2, number, HCENTERUL_OFF);
    ssd1306_printText(0, 0, "Test");
    P2IE |= BIT1 | BIT2;
    P1IE |= BIT5;
    LPM0;
    if (rotated_rotary_encoder) {
        if (direction == Cw) {
          number++;
        } else {
          number--;
        }
    }
    P2IE = 0;
    P1IE = 0;
    ssd1306_printChar(0, 3, morse_button ? '1' : '0');
    ssd1306_printText(0, 1, (direction == Cw) ? "->" : "<-");

    char translated_char = translate_morse(state.current_morse_element, state.morse_buffer[state.current_msg_char]);
    if(translated_char) {
        ssd1306_printChar(0, 4, translated_char);
    }

    // Draw previous character
    if(state.current_morse_element > 0) {
        const uint8_t element_idx = state.current_morse_element - 1;
        ssd1306_printText(element_idx << 3, 5, (state.morse_buffer[state.current_msg_char] & (1 << element_idx)) ? "-" : ".");
    }
  }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void int_T1_0(void) {
    state.morse_buffer[state.current_msg_char] |= 1 << state.current_morse_element;
    state.current_morse_element++;
    // Stop timer and signal that the last element was a 'dah' to the morse button interrupt
    TA1CCR0 = 0;
    LPM0_EXIT; // Redraw current character
}

#pragma vector = PORT1_VECTOR
__interrupt void p1v() {
  // Change interrupt edge select to call interrupt when the button is released/pressed next time
  morse_button = !morse_button;
  if (morse_button) {
      // Start 'dah' timer with a period of ~500ms
      TA1CCR0 = 749;
      TA1CCTL0 = CCIE;
      // Turn on buzzer
      TA0CCR1 = 13;
  } else {
      // If the last morse element sent was a 'dah', the timer must have changed the comparator value to 0, so do not increment the morse element
      if(TA1CCR0 != 0) {
          state.current_morse_element++;
      }
      // Stop 'dah' timer
      TA1CCTL0 = 0;
      TA1CCR0 = 0;
      // Turn off buzzer
      TA0CCR1 = 0;
  }
  P1IES = ~P1IES;

  LPM0_EXIT;
  P1IFG = 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void p2v() {
  if ((P2IFG & BIT1) && (P2IN & BIT2)) {
    direction = Ccw;
  } else if ((P2IFG & BIT2) && (P2IN & BIT1)) {
    direction = Cw;
  }
  rotated_rotary_encoder = true;
  LPM0_EXIT;
  P2IFG = 0;
}
