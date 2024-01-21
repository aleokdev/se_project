/*
 * ssd1306.h
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdbool.h>
#include <stdint.h>

/* ====================================================================
 * Horizontal Centering Number Array
 * ==================================================================== */
#define HCENTERUL_OFF 0
#define HCENTERUL_ON 1

/* ====================================================================
 * SSD1306 OLED Settings and Command Definitions
 * ==================================================================== */

#define SSD1306_I2C_ADDRESS 0x3C

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#define SSD1306_128_64

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// currently no scroll functionality, left for possible future use
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

#define SSD1306_SCROLL_FREQ_1 0b111 // 2 frames per scroll
#define SSD1306_SCROLL_FREQ_2 0b100 // 3 frames per scroll
#define SSD1306_SCROLL_FREQ_3 0b101 // 4 frames per scroll
#define SSD1306_SCROLL_FREQ_4 0b000 // 5 frames per scroll
#define SSD1306_SCROLL_FREQ_5 0b110 // 25 frames per scroll
#define SSD1306_SCROLL_FREQ_6 0b001 // 64 frames per scroll
#define SSD1306_SCROLL_FREQ_7 0b010 // 128 frames per scroll
#define SSD1306_SCROLL_FREQ_8 0b011 // 256 frames per scroll

/* ====================================================================
 * SSD1306 OLED Prototype Definitions
 * ==================================================================== */
void ssd1306_init(void);
void ssd1306_command(unsigned char);
void ssd1306_clearDisplay(void);
void ssd1306_setPosition(uint8_t, uint8_t);
void ssd1306_setDrawingRect(uint8_t start_column,
                            uint8_t start_page,
                            uint8_t end_column,
                            uint8_t end_page);
void ssd1306_stopScroll();
void ssd1306_startHorzScroll(uint8_t start_page, uint8_t end_page, uint8_t time_interval);
void ssd1306_clearPage(uint8_t page, bool value);
void ssd1306_printChar(uint8_t, uint8_t, char, bool inverted);
void ssd1306_printChar2x(uint8_t, uint8_t, char, bool inverted);
void ssd1306_printText(uint8_t, uint8_t, char*, bool inverted);
void ssd1306_printTextBlock(uint8_t, uint8_t, char*, bool inverted);
void ssd1306_printUI32(uint8_t, uint8_t, uint32_t, uint8_t, bool inverted);

uint8_t digits(uint32_t);
void ultoa(uint32_t, char*);
void reverse(char*);

#endif /* SSD1306_H_ */
