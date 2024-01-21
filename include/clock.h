#pragma once

#include <msp430.h>

void Set_Clk(char VEL) {
  BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
  switch (VEL) {
    case 1:
      if (CALBC1_1MHZ != 0xFF) {
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ; /* Set DCO to 1MHz */
        DCOCTL = CALDCO_1MHZ;
      }
      break;
    case 8:

      if (CALBC1_8MHZ != 0xFF) {
        __delay_cycles(100000);
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_8MHZ; /* Set DCO to 8MHz */
        DCOCTL = CALDCO_8MHZ;
      }
      break;
    case 12:
      if (CALBC1_12MHZ != 0xFF) {
        __delay_cycles(100000);
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_12MHZ; /* Set DCO to 12MHz */
        DCOCTL = CALDCO_12MHZ;
      }
      break;
    case 16:
      if (CALBC1_16MHZ != 0xFF) {
        __delay_cycles(100000);
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_16MHZ; /* Set DCO to 16MHz */
        DCOCTL = CALDCO_16MHZ;
      }
      break;
    default:
      if (CALBC1_1MHZ != 0xFF) {
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ; /* Set DCO to 1MHz */
        DCOCTL = CALDCO_1MHZ;
      }
      break;
  }
  BCSCTL1 |= XT2OFF | DIVA_0;
  BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;
}
