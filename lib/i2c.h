// Original version found at https://github.com/sdp8483/MSP430G2_SSD1306_OLED

#pragma once

void i2c_init(void);
void i2c_write(unsigned char, unsigned char*, unsigned char);

void i2c_read(unsigned char slave_address, unsigned char* DataBuffer, unsigned char ByteCtr);
