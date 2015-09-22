/*
This software is subject to the license described in the license.txt file included with this software distribution.You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/

#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "debug_pin.h"

#if defined (DEBUGGING_PINS_ENABLE)

///////////////////////////////////////////////////////////////////////
// This function outputs a Manchester waveform for a
// given priority context and event number. The particular waveforms were pre-calculated
// so that the transmission speed is as fast as possible.
// With Manchester encoding, the clock is XOR'd with an oversampled data stream.
// This way, one is guaranteed to have a transitition every bit period and
// the clock can be extracted from the data stream.
// After oversampling the data (e.g. bABC... -> bAABBCC) and XORing with 0x55 or
// b01010101.
//
// The following waveform lookup table was pre-determined:
//    Data  Output Waveform
//    ---------------------
//    0     0x55
//    1     0x56
//    2     0x59
//    3     0x5A
//    4     0x65
//    5     0x66
//    6     0x69
//    7     0x6A
//    8     0x95
//    9     0x96
//    10    0x99
//    11    0x9A
//    12    0xA5
//    13    0xA6
//    14    0xA9
//    15    0xAA
///////////////////////////////////////////////////////////////////////

#define STACK_DEBUG_MANCHESTER_BYTE55(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE56(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE59(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE5A(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE65(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE66(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE69(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE6A(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE95(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE96(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE99(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTE9A(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTEA5(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTEA6(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTEA9(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_BYTEAA(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                              __nop(); __nop();\
                                              NRF_GPIO->OUTSET = 1UL << ucPin;\
                                              __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin) {NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                                    __nop(); __nop();\
                                                    NRF_GPIO->OUTSET = 1UL << ucPin;\
                                                    __nop(); __nop();}

#define STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin) {NRF_GPIO->OUTSET = 1UL << ucPin;\
                                                     __nop(); __nop();\
                                                     NRF_GPIO->OUTCLR = 1UL << ucPin;\
                                                     __nop(); __nop();}

void stack_debug_Manchester_Start(uint8_t ucPin, uint8_t ucCode)
{
   __disable_irq();

   NRF_GPIO->OUTCLR = 1UL << ucPin;
   __nop(); __nop();

   switch (ucCode)
   {
      case 0:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE55(ucPin);
         break;

      case 1:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE56(ucPin);
         break;

      case 2:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE59(ucPin);
         break;

      case 3:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE5A(ucPin);
         break;

      case 4:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE65(ucPin);
         break;

      case 5:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE66(ucPin);
         break;

      case 6:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE69(ucPin);
         break;

      case 7:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE6A(ucPin);
         break;

      case 8:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE95(ucPin);
         break;

      case 9:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE96(ucPin);
         break;

      case 10:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE99(ucPin);
         break;

      case 11:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTE9A(ucPin);
         break;

      case 12:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTEA5(ucPin);
         break;

      case 13:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTEA6(ucPin);
         break;

      case 14:
         STACK_DEBUG_MANCHESTER_PREAMBLE_ODD(ucPin);
         STACK_DEBUG_MANCHESTER_BYTEA9(ucPin);
         break;

      case 15:
         STACK_DEBUG_MANCHESTER_PREAMBLE_EVEN(ucPin);
         STACK_DEBUG_MANCHESTER_BYTEAA(ucPin);
         break;

      default:
         break;

   }

   // clear last manchester code state
   NRF_GPIO->OUTCLR = 1UL << ucPin;
   __nop(); __nop();

   // start window
   NRF_GPIO->OUTSET = 1UL << ucPin;
   __nop(); __nop();

   __enable_irq();
}

void stack_debug_Manchester_Stop(uint8_t ucPin)
{
   __disable_irq();

   // end window
   NRF_GPIO->OUTCLR = 1UL << ucPin;
   __nop(); __nop();

   __enable_irq();
}

#endif // DEBUGGING_PINS_ENABLE
