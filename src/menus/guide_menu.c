#include "menus.h"

#include "ssd1306.h"
#include "morse.h"
#include "morse_input.h"

#include <stdbool.h>

void redraw_guide_screen(const State* _state) {
  ssd1306_clearDisplay();
  ssd1306_clearPage(0, true);
  ssd1306_printText(2, 0, "Modo guia", true);
  // Error display
  ssd1306_printText(2, 6, "Intentos", false);
  for(uint8_t i = 0; i < 3; i++) {
    ssd1306_printChar(2 + i << 3, 7, 'o', false);
  }
}

typedef struct {
  // The character that the user has to write in morse
  char current_char;

  // Amount of errors accumulated until now trying to write the current character in morse
  uint8_t error_count;

  MorseInputData input_data;

  // What the guide menu is doing right now
  enum {
    GuideState_GenChar_ReadingAdc,
    GuideState_Guide,
    GuideState_ShowingCorrectTranslation
  } state;
} GuideState;
GuideState gstate;

void start_char_gen(void) {
  gstate.current_char = 'A';
  // Generate new random character by using the ADC
  start_adc_conv();
  gstate.state = GuideState_GenChar_ReadingAdc;
}

void query_next_char(void) {
  start_char_gen();
  gstate.error_count = 0;
  // Reset error display
  for(uint8_t i = 0; i < 3; i++) {
    ssd1306_printChar(2 + i << 3, 7, 'o', false);
  }
}

void check_char_written(MorseCharacter ch) {
  if(translate_morse(ch) == gstate.current_char) {
    // The user wrote the correct morse translation for the character shown
    query_next_char();
  } else {
    // The user wrote an incorrect morse translation for the character shown
    ssd1306_printChar(2 + gstate.error_count << 3, 7, 'x', false);
    if (++gstate.error_count >= 3) {
      // ...and also had too many errors
      gstate.state = GuideState_ShowingCorrectTranslation;
      ssd1306_printText(64, 6, "Incorrecto", false);
      ssd1306_printChar(64, 7, gstate.current_char, false);
      ssd1306_printText(76, 7, "es", false);

      MorseCharacter translation = get_morse_translation(gstate.current_char);
      for(uint8_t m = 0; m < translation.length; m++) {
        ssd1306_printChar(76+6*3 + m * 6, 7, translation.morse & (1 << m) ? '-' : '.', false);
      }
    }
  }

  // Clear morse display
  ssd1306_printText(127-5*6, 0, "     ", true);
}

void process_guide_menu(State* state, const IoActions* actions) {
  switch(gstate.state) {
  case GuideState_GenChar_ReadingAdc: {
    if(actions->adc10_conv_finished) {
      // We need a value between 0 (A) and 25 (Z)
      static uint8_t conversion_idx = 0;
      const uint8_t rand_table[] = {13, 6, 3, 2, 1}; // Not perfectly weighted, but it'll do
      gstate.current_char += (finish_adc_conv() & 1) ? rand_table[conversion_idx] : 0;
      if(++conversion_idx >= 5) {
        ssd1306_printChar2x(128/2-5, 2, gstate.current_char, false);
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
      open_selection_menu(state);
    }

    const MorseCharacter char_input = process_morse_input(&gstate.input_data, actions);
    if(char_input.length > 0) {
      check_char_written(char_input);
    }

    break;
  }

  case GuideState_ShowingCorrectTranslation:
    if(actions->pressed_morse_button) {
      gstate.state = GuideState_Guide;
      ssd1306_printText(64, 6, "          ", false);
      ssd1306_printText(64, 7, "          ", false);
      gstate.input_data = (MorseInputData) { 0 };
      query_next_char();
    }
    break;
  }
}

void open_guide_menu(State* state) {
  state->menu_open = Menu_Guide;
  redraw_guide_screen(state);

  gstate = (GuideState) {
    .current_char = 'A',
    .error_count = 0,
    .state = GuideState_GenChar_ReadingAdc
  };

  // Generate new random character by using the ADC
  start_adc_conv();
}

