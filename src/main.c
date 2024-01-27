#include "i2c.h"
#include "io.h"
#include "menus.h"
#include "morse.h"
#include "rotary_encoder.h"
#include "settings.h"
#include "ssd1306.h"
#include "state.h"
#include "menus.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

  setup_clocks();
  load_settings();
  setup_io();

  __bis_SR_register(GIE);

  const uint32_t hz_to_time = 1000000;
  play_tone(hz_to_time / 647); // E5 (647.27Hz)
  _delay_cycles(1000000);
  play_tone(hz_to_time / 864); // A5 (864Hz)
  _delay_cycles(1000000);
  play_tone(hz_to_time / 1027); // C6 (1027.47Hz)
  _delay_cycles(1000000);
  silence_tone();

  State state = {0};

  redraw_morse_transmission_screen(&state);

  for (;;) {
    // Re-enable button interrupts
    P2IE |= BIT1 | BIT2 | BIT5;
    P1IE |= BIT4;

    // Sleep (either deeply if in LPM or on mode 0 otherwise) until an interruption is received
    if(state.in_low_power_mode) {
        ssd1306_command(SSD1306_DISPLAYOFF);
        LPM4;

        // Got woken up, turn on the display and disable LPM
        ssd1306_command(SSD1306_DISPLAYON);
        state.in_low_power_mode = false;
        // Also ignore the actions sent to avoid doing things while the screen is off
        io_actions = (IoActions){0};
        continue;
    } else {
        LPM0;
    }
    // Got waken up, reset the low power mode time to go asleep
    lpm_reset_time();

    // Process the IO actions sent via interruptions
    const IoActions actions_to_process = io_actions;
    // Disable button interruptions to decrease I2C comms errors
    P2IE = 0;
    P1IE = 0;
    switch(state.menu_open) {
    case Menu_MorseTx:
        process_morse_tx_menu(&state, &actions_to_process);
        break;

    case Menu_Settings:
        process_settings_menu(&state, &actions_to_process);
        break;

    case Menu_MorseTable:
        process_morse_table_menu(&state, &actions_to_process);
        break;

    case Menu_SelectMenu:
        process_selection_menu(&state, &actions_to_process);
        break;
    }

    if(io_actions.low_power_mode_requested) {
        state.in_low_power_mode = true;
    }
    // Clear the actions since they have been processed
    io_actions = (IoActions){0};
  }
}
