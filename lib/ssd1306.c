/*
 * ssd1306.c
 */

#include "ssd1306.h"
#include "i2c.h"
#include <font_5x8.h>
#include <msp430.h>
#include <stdint.h>
#include <string.h>

unsigned char buffer[32]; // buffer for data transmission to screen

/* ====================================================================
 * Horizontal Centering Number Array
 * ==================================================================== */
const unsigned char HcenterUL[] = {
    // Horizontally center number with separators on screen
    0,  // 0 digits, not used but included to size array correctly
    61, // 1 digit
    58, // 2 digits
    55, // 3 digits
    49, // 4 digits and 1 separator
    46, // 5 digits and 1 separator
    43, // 6 digits and 1 separator
    37, // 7 digits and 2 separators
    34, // 8 digits and 2 separators
    31, // 9 digits and 2 separators
    25  // 10 digits and 3 separators
};

void ssd1306_init(void) {
  // SSD1306 init sequence
  ssd1306_command(SSD1306_DISPLAYOFF);         // 0xAE
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV); // 0xD5
  ssd1306_command(0x80);                       // the suggested ratio 0x80

  ssd1306_command(SSD1306_SETMULTIPLEX); // 0xA8
  ssd1306_command(SSD1306_LCDHEIGHT - 1);

  ssd1306_command(SSD1306_SETDISPLAYOFFSET);   // 0xD3
  ssd1306_command(0x0);                        // no offset
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0
  ssd1306_command(SSD1306_CHARGEPUMP);         // 0x8D
  ssd1306_command(0x14);                       // generate high voltage from 3.3v line internally
  ssd1306_command(SSD1306_MEMORYMODE);         // 0x20
  ssd1306_command(0x00);                       // 0x0 act like ks0108
  ssd1306_command(SSD1306_SEGREMAP | 0x1);
  ssd1306_command(SSD1306_COMSCANDEC);

  ssd1306_command(SSD1306_SETCOMPINS); // 0xDA
  ssd1306_command(0x12);
  ssd1306_command(SSD1306_SETCONTRAST); // 0x81
  ssd1306_command(0xCF);

  ssd1306_command(SSD1306_SETPRECHARGE); // 0xd9
  ssd1306_command(0xF1);
  ssd1306_command(SSD1306_SETVCOMDETECT); // 0xDB
  ssd1306_command(0x40);
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
  ssd1306_command(SSD1306_NORMALDISPLAY);       // 0xA6

  ssd1306_command(SSD1306_DEACTIVATE_SCROLL);

  ssd1306_command(SSD1306_DISPLAYON); //--turn on oled panel
} // end ssd1306_init

void ssd1306_command(unsigned char command) {
  buffer[0] = 0x80;
  buffer[1] = command;

  i2c_write(SSD1306_I2C_ADDRESS, buffer, 2);
} // end ssd1306_command

void ssd1306_clearDisplay(void) {
  for (uint8_t i = 8; i > 0; i--) { ssd1306_clearPage(i - 1u, false); }
} // end ssd1306_clearDisplay

void ssd1306_setPosition(uint8_t column, uint8_t page) {
  if (column > 128) {
    column = 0; // constrain column to upper limit
  }

  if (page > 8) {
    page = 0; // constrain page to upper limit
  }

  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(column);               // Column start address (0 = reset)
  ssd1306_command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(page); // Page start address (0 = reset)
  ssd1306_command(7);    // Page end address
} // end ssd1306_setPosition

void ssd1306_setDrawingRect(uint8_t start_column,
                            uint8_t start_page,
                            uint8_t end_column,
                            uint8_t end_page) {
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(start_column); // Column start address (0 = reset)
  ssd1306_command(end_column);   // Column end address (127 = reset)

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(start_page); // Page start address (0 = reset)
  ssd1306_command(end_page);   // Page end address
}

void ssd1306_printChar(uint8_t x, uint8_t y, char ch, bool inverted) {
  ssd1306_setPosition(x, y);

  buffer[0] = 0x40;

  uint8_t inverting_mask = inverted ? 0xFF : 0;

  for (uint8_t i = 0; i < 5; i++) { buffer[i + 1u] = font_5x8[ch - ' '][i] ^ inverting_mask; }

  buffer[6] = inverting_mask;

  i2c_write(SSD1306_I2C_ADDRESS, buffer, 7);
}

void ssd1306_printChar2x(uint8_t x, uint8_t y, char ch, bool inverted) {
  ssd1306_setDrawingRect(x, y, x + 10, 7);

  buffer[0] = 0x40; // Upload data

  uint8_t inverting_mask = inverted ? 0xFF : 0;

  for (uint8_t i = 0; i < 10; i++) {
    uint8_t column = font_5x8[ch - ' '][i >> 1] ^ inverting_mask;
    uint8_t upper_half_2x = (column & 0b0001) | ((column & 0b0001) << 1) |
                            ((column & 0b0010) << 1) | ((column & 0b0010) << 2) |
                            ((column & 0b0100) << 2) | ((column & 0b0100) << 3) |
                            ((column & 0b1000) << 3) | ((column & 0b1000) << 4);
    buffer[i + 1] = upper_half_2x;
  }

  buffer[11] = inverting_mask;

  for (uint8_t i = 0; i < 10; i++) {
    uint8_t column = (font_5x8[ch - ' '][i >> 1] ^ inverting_mask) >> 4;
    uint8_t lower_half_2x = (column & 0b0001) | ((column & 0b0001) << 1) |
                            ((column & 0b0010) << 1) | ((column & 0b0010) << 2) |
                            ((column & 0b0100) << 2) | ((column & 0b0100) << 3) |
                            ((column & 0b1000) << 3) | ((column & 0b1000) << 4);
    buffer[i + 12] = lower_half_2x;
  }

  buffer[22] = inverting_mask;

  i2c_write(SSD1306_I2C_ADDRESS, buffer, 23);
}

void ssd1306_stopScroll() {
  ssd1306_command(0x2E); // Deactivate scrolling
}

void ssd1306_startHorzScroll(uint8_t start_page, uint8_t end_page, uint8_t time_interval) {
  ssd1306_command(0x2E); // Deactivate scrolling

  ssd1306_command(0x26);
  ssd1306_command(0x00);
  ssd1306_command(start_page);
  ssd1306_command(time_interval);
  ssd1306_command(end_page);
  ssd1306_command(0x00);
  ssd1306_command(0xFF);

  ssd1306_command(0x2F); // Activate scrolling
}

void ssd1306_clearPage(uint8_t page, bool value) {
  ssd1306_setDrawingRect(0, page, SSD1306_LCDWIDTH - 1, page);

  buffer[0] = 0x40;                // Upload data
  const uint8_t fill = value ? 0xFF : 0x00;
  for (uint8_t i = 16; i > 0; i--) {
    buffer[i] = fill; // Empty column
  }
  for (uint8_t i = SSD1306_LCDWIDTH / 16; i > 0; i--) { i2c_write(SSD1306_I2C_ADDRESS, buffer, 17); }
}

void ssd1306_printText(uint8_t x, uint8_t y, const char* ptString, bool inverted) {
  ssd1306_setPosition(x, y);

  while (*ptString != '\0') {
    if ((x + 5) >= 127) {        // char will run off screen
      x = 0;                     // set column to 0
      y++;                       // jump to next page
      ssd1306_setPosition(x, y); // send position change to oled
    }

    ssd1306_printChar(x, y, *ptString, inverted);

    ptString++;
    x += 6;
  }
} // end ssd1306_printText

void ssd1306_printTextBlock(uint8_t x, uint8_t y, const char* ptString, bool inverted) {
  char word[12];
  uint8_t i;
  uint8_t endX = x;
  while (*ptString != '\0') {
    i = 0;
    while ((*ptString != ' ') && (*ptString != '\0')) {
      word[i] = *ptString;
      ptString++;
      i++;
      endX += 6;
    }

    word[i++] = '\0';

    if (endX >= 127) {
      x = 0;
      y++;
      ssd1306_printText(x, y, word, inverted);
      endX = (i * 6);
      x = endX;
    } else {
      ssd1306_printText(x, y, word, inverted);
      endX += 6;
      x = endX;
    }

    if (*ptString == ' ') {
      ptString++;
    }
  }
}

void ssd1306_printUI32(uint8_t x, uint8_t y, uint32_t val, bool inverted) {
  char text[14];

  ultoa(val, text);
  ssd1306_printText(x, y, text, inverted);
} // end ssd1306_printUI32

void ultoa(uint32_t val, char* string) {
  do {
    *(string++) = val % 10 + '0'; // Add the ith digit to the number string
  } while ((val /= 10) > 0);

  *(string++) = '\0'; // Add termination to string
  reverse(string);    // string was built in reverse, fix that
} // end ultoa

void reverse(char* s) {
  for (uint8_t i = 0, j = strlen(s) - 1; i < j; i++, j--) {
    const uint8_t c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
} // end reverse
