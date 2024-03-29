#include "menus.h"

#include "morse.h"
#include "morse_input.h"
#include "ssd1306.h"

#include <stdbool.h>

typedef struct {
  // The character that the user has to write in morse
  char current_char;

  // Amount of errors accumulated until now trying to write the current character in morse
  uint8_t error_count;

  MorseInputData input_data;

  // Stats
  // Number of successful translations since the menu was opened
  uint16_t correct_translations;
  // Number of unsuccessful translations since the menu was opened
  uint16_t incorrect_translations;

  // What the guide menu is doing right now
  enum {
    GuideState_GenChar_ReadingAdc,
    GuideState_Guide,
    GuideState_PlayCorrectChime,
    GuideState_ShowingCorrectTranslation
  } state;

  // Tones to play in GuideState_PlayCorrectChime & GuideState_ShowingCorrectTranslation
  uint16_t tone_buffer[2];
  // Index of the next tone from tone_buffer to be played
  uint8_t tone_idx;
} GuideState;
GuideState gstate;

#define ERROR_DISPLAY_X (2 + 12)
#define ERROR_DISPLAY_Y 7

inline void reset_error_display(void) {
  for (uint8_t i = 0; i < 3; i++) {
    ssd1306_printChar(ERROR_DISPLAY_X + (i << 3), ERROR_DISPLAY_Y, 'o', false);
  }
}

void redraw_guide_menu(void) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Modo guia", true);
  // Error display
  ssd1306_printText(2, 6, "Intentos", false);
  reset_error_display();
  ssd1306_printText(48 + 6, 2, "Aciertos", false);
  ssd1306_printUI32(100 + 6, 2, gstate.correct_translations, false);
  ssd1306_printText(48 + 6, 3, "Falladas", false);
  ssd1306_printUI32(100 + 6, 3, gstate.incorrect_translations, false);
}

void start_char_gen(void) {
  gstate.current_char = 'A';
  // Generate new random character by using the ADC
  start_adc_conv();
  gstate.state = GuideState_GenChar_ReadingAdc;
}

void query_next_char(void) {
  start_char_gen();
  gstate.error_count = 0;
  reset_error_display();
}

void check_char_written(MorseCharacter ch) {
  if (translate_morse(ch) == gstate.current_char) {
    // The user wrote the correct morse translation for the character shown
    gstate.correct_translations++;
    ssd1306_printUI32(100 + 6, 2, gstate.correct_translations, false);

    play_tone(AUDIO_NOTE(880));               // A5 (880Hz)
    gstate.tone_buffer[0] = AUDIO_NOTE(988);  // B5 (988Hz)
    gstate.tone_buffer[1] = AUDIO_NOTE(1047); // C6 (1047Hz)
    gstate.tone_idx = 0;
    gstate.state = GuideState_PlayCorrectChime;
    setup_timer(1500 / 16);
  } else {
    // The user wrote an incorrect morse translation for the character shown
    ssd1306_printChar(ERROR_DISPLAY_X + (gstate.error_count << 3), ERROR_DISPLAY_Y, 'x', false);
    if (++gstate.error_count >= 3) {
      // ...and also had too many errors
      gstate.incorrect_translations++;
      ssd1306_printUI32(100 + 6, 3, gstate.incorrect_translations, false);

      gstate.state = GuideState_ShowingCorrectTranslation;
      ssd1306_printText(64, 6, "Incorrecto", false);
      ssd1306_printChar(64, 7, gstate.current_char, false);
      ssd1306_printText(76, 7, "es", false);

      MorseCharacter translation = get_morse_translation(gstate.current_char);
      for (uint8_t m = 0; m < translation.length; m++) {
        ssd1306_printChar(76 + 6 * 3 + m * 6, 7, translation.morse & (1 << m) ? '-' : '.', false);
      }

      play_tone(AUDIO_NOTE(1047));             // C6 (1047Hz)
      gstate.tone_buffer[1] = AUDIO_NOTE(988); // E5 (659Hz)
      gstate.tone_idx = 1;
      setup_timer(1500 / 16);
    }
  }
}

void process_guide_menu(Menu* menu_open, const IoActions* actions) {
  switch (gstate.state) {
    case GuideState_GenChar_ReadingAdc: {
      if (actions->adc10_conv_finished) {
        // We need a value between 0 (A) and 25 (Z)
        // This method is biased towards G, T, D, Q, J & W, but it should be fine
        static uint8_t conversion_idx = 0;
        const uint8_t rand_table[] = {13, 6, 3, 2, 1};
        // Use the first bit of the ADC conversion to determine whether to add the conversion_idx-th
        // value of rand_table to current_char or not
        gstate.current_char += (finish_adc_conv() & 1) ? rand_table[conversion_idx] : 0;
        if (++conversion_idx >= 5) {
          ssd1306_printChar2x(20, 2, gstate.current_char, false);
          gstate.state = GuideState_Guide;
          conversion_idx = 0;
        } else {
          start_adc_conv();
        }
      }
      break;
    }

    case GuideState_Guide: {
      if (actions->pressed_encoder) {
        silence_tone();
        open_selection_menu(menu_open);
      }

      const MorseCharacter char_input = process_morse_input(&gstate.input_data, actions);
      if (char_input.length > 0) {
        check_char_written(char_input);
      }

      break;
    }

    case GuideState_PlayCorrectChime:
      if (actions->timer1_finished) {
        if (gstate.tone_idx >= 2) {
          silence_tone();
          query_next_char();
        } else {
          play_tone(gstate.tone_buffer[gstate.tone_idx++]);
          setup_timer(1500 / 16);
        }
      }
      break;

    case GuideState_ShowingCorrectTranslation:
      if (actions->pressed_morse_button) {
        gstate.state = GuideState_Guide;
        ssd1306_printText(64, 6, "          ", false);
        ssd1306_printText(64, 7, "          ", false);
        gstate.input_data = (MorseInputData){0};
        query_next_char();
      }
      if (gstate.tone_idx >= 2) {
        silence_tone();
      } else {
        play_tone(gstate.tone_buffer[gstate.tone_idx++]);
        setup_timer(1500 / 16);
      }
      break;
  }
}

void open_guide_menu(Menu* menu_open) {
  *menu_open = Menu_Guide;
  gstate =
      (GuideState){.current_char = 'A', .error_count = 0, .state = GuideState_GenChar_ReadingAdc};
  redraw_guide_menu();

  // Generate new random character by using the ADC
  start_adc_conv();
}
