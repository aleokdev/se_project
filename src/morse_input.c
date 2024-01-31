#include "morse_input.h"

#include "settings.h"
#include "io.h"

#include "ssd1306.h"

#define MORSE_DISPLAY_X (127-8*8)
#define MORSE_DISPLAY_Y 0

MorseCharacter process_morse_input(MorseInputData* data, const IoActions* actions) {
  const TimerReason timer_reason = data->last_timer_reason;
  if (actions->pressed_morse_button) {
      play_tone(settings.tone_value);
      if(data->current_morse_element < 8) {
        ssd1306_printChar(MORSE_DISPLAY_X + (data->current_morse_element << 3), MORSE_DISPLAY_Y, '.', true);
      }

      // Set up timer for 'dah' detection
      data->last_timer_reason = TimerReason_Dah;
      setup_timer(settings.dah_time);
  }
  if (actions->released_morse_button && TA0CCR0) {
      silence_tone();
      reset_timer();
      if(data->current_morse_element < 8) {
        data->current_morse_element++;
      }

      // Setup next character timer
      data->last_timer_reason = TimerReason_NextChar;
      setup_timer(settings.dah_time);
  }

  if (actions->timer1_finished) {
      switch(timer_reason) {
      case TimerReason_Dah: {
        if(data->current_morse_element < 8) {
          data->morse_buffer |= 1 << data->current_morse_element;
          ssd1306_printChar(MORSE_DISPLAY_X + (data->current_morse_element << 3), MORSE_DISPLAY_Y, '-', true);
        }
        break;
      }

      case TimerReason_NextChar:
          if(data->current_morse_element > 0) {
            const MorseCharacter ch = { .length = data->current_morse_element, .morse = data->morse_buffer };
            data->current_morse_element = 0;
            data->morse_buffer = 0;
            return ch;
          }
          break;
      }
  }

  return (MorseCharacter) { 0 };
}

void clear_morse_display(void) {
  ssd1306_printText(MORSE_DISPLAY_X, 0, "          ", true);
}
