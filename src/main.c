#include "i2c.h"
#include "io.h"
#include "menus.h"
#include "morse.h"
#include "settings.h"
#include "ssd1306.h"
#include "menus.h"

#include <msp430.h>

#include <stdbool.h>
#include <stdint.h>

void play_startup_chime(void);

int main(void) {
  WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

  setup_clocks();
  load_settings();
  setup_io();

  __bis_SR_register(GIE);

  bool in_low_power_mode = false;

  Menu menu_open;
  open_morse_tx_menu(&menu_open);

  ssd1306_command(SSD1306_DISPLAYON); // Turn on the display when everything's in order

  play_startup_chime();

  // Clear premature actions (e.g. from startup chime)
  io_actions = (IoActions){};

  for (;;) {
    // Re-enable button interrupts
    P2IE |= BIT1 | BIT2 | BIT5;
    P1IE |= BIT4;

    // Only sleep if there are no actions to process
    if(!io_actions.u16) {
      // Start ADC conversion if required
      ADC10CTL0 |= ADC10SC;
      // Sleep (either deeply if in LPM or on mode 0 otherwise) until an interruption is received
      if(in_low_power_mode) {
          ssd1306_command(SSD1306_DISPLAYOFF);
          LPM4;

          // Got woken up, turn on the display and disable LPM
          ssd1306_command(SSD1306_DISPLAYON);
          in_low_power_mode = false;
          // Also ignore the actions sent to avoid doing things while the screen is off
          io_actions = (IoActions){0};
          continue;
      } else {
          LPM0;
      }
      ADC10CTL0 &= ~ADC10SC;
      // Got waken up, reset the low power mode time to go asleep
      lpm_reset_time();
    }

    // Process the IO actions sent via interruptions
    const IoActions actions_to_process = io_actions;
    // Disable button interruptions to decrease I2C comms errors
    P2IE = 0;
    P1IE = 0;
    switch(menu_open) {
    case Menu_MorseTx:
      process_morse_tx_menu(&menu_open, &actions_to_process);
      break;

    case Menu_Settings:
      process_settings_menu(&menu_open, &actions_to_process);
      break;

    case Menu_MorseTable:
      process_morse_table_menu(&menu_open, &actions_to_process);
      break;

    case Menu_SelectMenu:
      process_selection_menu(&menu_open, &actions_to_process);
      break;

    case Menu_Guide:
      process_guide_menu(&menu_open, &actions_to_process);
      break;
    }

    if(io_actions.low_power_mode_requested) {
        in_low_power_mode = true;
    }
    // Clear the actions that have been processed
    io_actions.u16 &= ~actions_to_process.u16;
  }
}

void play_startup_chime(void) {
  play_tone(AUDIO_NOTE(647)); // E5 (647.27Hz)
  setup_timer(100);
  uint8_t note_idx = 0;
  for(;;) {
    LPM0;
    switch(note_idx++) {
    case 0: play_tone(AUDIO_NOTE(864)); break; // A5 (864Hz)
    case 1: play_tone(AUDIO_NOTE(1027)); break; // C6 (1027.47Hz)
    default: silence_tone(); return;
    }
    setup_timer(100);
  }
}
