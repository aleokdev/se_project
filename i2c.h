/*
 * i2c.h
 *
 *  Created on: May 19, 2019
 *      Author: Sam
 */

#ifndef I2C_H_
#define I2C_H_

#include <msp430.h>

/* ====================================================================
 * I2C Prototype Definitions
 * ==================================================================== */
void i2c_init(void);
void i2c_write(unsigned char, unsigned char *, unsigned char);

void i2c_read(unsigned char slave_address, unsigned char *DataBuffer,
              unsigned char ByteCtr);

#endif /* I2C_H_ */
