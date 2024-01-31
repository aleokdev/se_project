#include "i2c.h"

#include <msp430.h>
#include <stdint.h>

#define SDA BIT7 // i2c sda pin
#define SCL BIT6 // i2c scl pin

volatile unsigned char* PTxData;  // Pointer to TX data
volatile unsigned char TxByteCtr; // number of bytes to TX
volatile unsigned char* PRxData;  // Pointer to RX data
volatile unsigned char RxByteCtr; // number of bytes to RX

void i2c_init(void) {
  P1SEL |= SCL + SDA;                   // Assign I2C pins to USCI_B0
  P1SEL2 |= SCL + SDA;                  // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                  // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;        // Use SMCLK, keep SW reset
  UCB0BR0 = 10;                         // fSCL = SMCLK/10 = ~100kHz with SMCLK = 1MHz
  UCB0BR1 = 0;                          //
  UCB0CTL1 &= ~UCSWRST;                 // Clear SW reset, resume operation
  IE2 |= UCB0RXIE | UCB0TXIE;           // Enable TX & RX interrupt

} // end i2c_init

void i2c_write(unsigned char slave_address, unsigned char* DataBuffer, unsigned char ByteCtr) {
  UCB0I2CSA = slave_address;

  PTxData = DataBuffer;
  TxByteCtr = ByteCtr;

  while (UCB0CTL1 & UCTXSTP)
    ;                              // Ensure stop condition got sent
  UCB0CTL1 |= UCTR + UCTXSTT;      // I2C TX, start condition
  while (UCB0CTL1 & UCTXSTP)       //
    ;                              // Ensure stop condition got sent
  __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts
                                   // Remain in LPM0 until all data is TX'd
}

void i2c_read(unsigned char slave_address, unsigned char* DataBuffer, unsigned char ByteCtr) {
  UCB0I2CSA = slave_address;

  PRxData = DataBuffer;
  RxByteCtr = ByteCtr;

  while (UCB0CTL1 & UCTXSTP)
    ;                              // Ensure stop condition got sent
  UCB0CTL1 &= ~UCTR;               // Clear UCTR
  UCB0CTL1 |= UCTXSTT;             // I2C start condition
  while (UCB0CTL1 & UCTXSTT)       //
    ;                              // Start condition sent?
  UCB0CTL1 |= UCTXSTP;             // I2C stop condition
  __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts
}

/* =============================================================================
 * The USCIAB0TX_ISR is structured such that it can be used to transmit any
 * number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
 * points to the next byte to transmit.
 * =============================================================================
 */
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void) {
  if (TxByteCtr) {          // Check TX byte counter
    UCB0TXBUF = *PTxData++; // Load TX buffer
    TxByteCtr--;            // Decrement TX byte counter
  } else {
    UCB0CTL1 |= UCTXSTP;               // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                // Clear USCI_B0 TX int flag
    __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
  }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void) {
  if (RxByteCtr) {        // Check RX byte counter
    *PRxData = UCB0RXBUF; // Load RX buffer
    PRxData++;
    RxByteCtr--; // Decrement RX byte counter
  } else {
    UCB0CTL1 |= UCTXSTP;               // I2C stop condition
    IFG2 &= ~UCB0RXIFG;                // Clear USCI_B0 RX int flag
    __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
  }
}
