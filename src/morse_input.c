#include "morse_input.h"

#include "io.h"
#include "settings.h"

#include "ssd1306.h"

// Where the morse display is located on the screen.
#define MORSE_DISPLAY_X (127 - 8 * 8)
#define MORSE_DISPLAY_Y 0

// Prints a single morse element on its corresponding location
inline void display_element(uint8_t position, char element) {
  ssd1306_printChar(MORSE_DISPLAY_X + (position << 3), MORSE_DISPLAY_Y, element, true);
}

inline void clear_morse_display(void) {
  ssd1306_printText(MORSE_DISPLAY_X, MORSE_DISPLAY_Y, "          ", true);
}

MorseCharacter process_morse_input(MorseInputData* data, const IoActions* actions) {
  if (actions->timer1_finished) {
    // Check why the timer 1 was set up in the first place
    switch (data->last_timer_reason) {
      case TimerReason_Dah: {
        // We are waiting for a 'dah' and the time to wait has finished:
        // Set the current morse element to a dah
        if (data->current_morse_element < 8) {
          data->morse_buffer |= 1 << data->current_morse_element;
          display_element(data->current_morse_element, '-');
        }
        break;
      }

      case TimerReason_NextChar:
        // The button isn't pressed, and the time to go to the next character has finished:
        // Return the current character as-is and reset the buffer to prepare for the next one
        if (data->current_morse_element > 0) {
          const MorseCharacter ch = {.length = data->current_morse_element,
                                     .morse = data->morse_buffer};
          data->current_morse_element = 0;
          data->morse_buffer = 0;
          clear_morse_display();
          return ch;
        }
        break;
    }
  }

  if (actions->pressed_morse_button) {
    // Started to press the morse button:
    // Play the tone given in settings, draw a dot in the display, set up timer for dah
    play_tone(settings.tone_value);
    if (data->current_morse_element < 8) {
      display_element(data->current_morse_element, '.');
    }

    // Set up timer for 'dah' detection
    data->last_timer_reason = TimerReason_Dah;
    setup_timer(settings.dah_time);
  }
  if (actions->released_morse_button && playing_tone()) {
    // Released the morse button:
    // Silence the tone being played, skip to the next morse element, set up timer to skip to next
    // char
    silence_tone();
    if (data->current_morse_element < 8) {
      data->current_morse_element++;
    }

    // Set up timer for skipping to the next character
    data->last_timer_reason = TimerReason_NextChar;
    setup_timer(settings.dah_time);
  }

  // No character has been finished being input yet: Return empty MorseCharacter
  return (MorseCharacter){};
}
