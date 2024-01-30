#include "morse_input.h"

#include "settings.h"
#include "io.h"

#include "ssd1306.h"

MorseCharacter process_morse_input(MorseInputData* data, const IoActions* actions) {
  const TimerReason timer_reason = data->last_timer_reason;
  if (actions->pressed_morse_button) {
      play_tone(settings.tone_value);
      ssd1306_printChar(127-5*6 + (data->current_morse_element << 3), 0, '.', true);

      // Set up timer for 'dah' detection
      data->last_timer_reason = TimerReason_Dah;
      setup_timer(settings.dah_time);
  }
  if (actions->released_morse_button && TA0CCR0) {
      silence_tone();
      reset_timer();
      data->current_morse_element++;

     if (data->current_morse_element >= 4) {
       // No consonant or vowel is longer than 4 morse elements, so go to next character directly
       const MorseCharacter ch = { .length = data->current_morse_element, .morse = data->morse_buffer };
       data->current_morse_element = 0;
       data->morse_buffer = 0;
       return ch;
     } else {
       // Setup next character timer
       data->last_timer_reason = TimerReason_NextChar;
       setup_timer(settings.dah_time);
     }
  }

  if (actions->timer1_finished) {
      switch(timer_reason) {
      case TimerReason_Dah: {
        data->morse_buffer |= 1 << data->current_morse_element;
        ssd1306_printChar(127-5*6 + (data->current_morse_element << 3), 0, '-', true);
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
