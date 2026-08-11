/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Small SWD driver for Bangle.js 3
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
// based on https://qcentlabs.com/posts/swd_banger/ and https://github.com/atc1441/ESP32_nRF52_SWD

// IO defines - these vary depending on device
#include "py32f07x_hal.h"
#include "py32f07x_hal_gpio.h"
#define ENABLE_SWDPINS(enable) { \
  GPIO_InitTypeDef  GPIO_InitStruct; \
  GPIO_InitStruct.Mode = enable ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_ANALOG/*full disconnect*/; \
  GPIO_InitStruct.Pull = GPIO_NOPULL; \
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1; \
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); \
}
#define SET_SWDIO_OUT(isOut) { \
  GPIO_InitTypeDef  GPIO_InitStruct; \
  GPIO_InitStruct.Mode = isOut ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT; \
  GPIO_InitStruct.Pull = isOut ? GPIO_NOPULL : GPIO_PULLUP; \
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
  GPIO_InitStruct.Pin = GPIO_PIN_0; \
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); \
}
#define WRITE_SWDIO(x) HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, x?GPIO_PIN_SET:GPIO_PIN_RESET)
#define WRITE_SWDCK(x) HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, x?GPIO_PIN_SET:GPIO_PIN_RESET)
#define READ_SWDIO() HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0)
#define DELAY_US(n) for (volatile int z=0;z<n;z++); // about right on PY32

typedef enum {
  SWDTX_START = 1, // always set
  SWDTX_APnDP = 2, // 0=DP, 1=AP
  SWDTX_RnW = 4, // 0=Write, 1=Read
  SWDTX_A1 = 8,
  SWDTX_A2 = 16,
  SWDTX_PARITY = 32, // calculated - APnDP,RnW,A1,A2; If cnt(1) is even, parity is 0
  SWDTX_STOP = 64, //
  SWDTX_PARK = 128, // always set
  // easier reading SWDTX_APnDP
  SWDTX_AP = SWDTX_APnDP,
  SWDTX_DP = 0,
  // easier reading SWDTX_RnW
  SWDTX_READ = SWDTX_RnW,
  SWDTX_WRITE = 0,
  // combinations of SWDTX_A1/2
  SWDTX_DP_IDCODE = 0,
  SWDTX_DP_ABORT = 0,
  SWDTX_DP_CTRL_STAT = SWDTX_A1,
  SWDTX_DP_SELECT = SWDTX_A2,
  SWDTX_DP_RDBUF = SWDTX_A1|SWDTX_A2,

  SWDTX_AP_CSW = 0,
  SWDTX_AP_TAR = SWDTX_A1,          // set target address
  SWDTX_AP_DRW = SWDTX_A1|SWDTX_A2, // write
  SWDTX_AP_NRF_RESET = 0,
  SWDTX_AP_NRF_ERASE_ALL = SWDTX_A1,
  SWDTX_AP_NRF_ERASE_STATUS = SWDTX_A2,
  SWDTX_AP_NRF_APP_PROTECT = SWDTX_A1|SWDTX_A2,
} SWD_TX_Flags;

bool swdIsWriting = false;

// Calculate parity of a 32-bit value by repeated XOR
bool swdParity(uint32_t v) {
  v = (v&0xFFFF) ^ (v>>16);
  v = (v&0xFF) ^ (v>>8);
  v = (v&0xF) ^ (v>>4);
  v = (v&0x3) ^ (v>>2);
  return (v&0x1) ^ (v>>1);
}

void swdTurnaround(SWD_TX_Flags readOrWrite) {
  WRITE_SWDIO(1);
  SET_SWDIO_OUT(false)
  WRITE_SWDCK(0);
  DELAY_US(2);
  WRITE_SWDCK(1);
  DELAY_US(2);
  swdIsWriting = false;
  if ((readOrWrite&SWDTX_RnW) == SWDTX_WRITE) { // if writing
    SET_SWDIO_OUT(true);
    swdIsWriting = true;
  }
}

void swdWriteBits(uint32_t data, int bits) {
  if (!swdIsWriting)
    swdTurnaround(SWDTX_WRITE);
  for (int i=0;i<bits;i++) {
    WRITE_SWDIO(data&1);
    WRITE_SWDCK(0);
    DELAY_US(2);
    WRITE_SWDCK(1);
    DELAY_US(2);
    data >>= 1;
  }
}

uint32_t swdReadBits(int bits) {
  if (swdIsWriting)
    swdTurnaround(SWDTX_READ);
  uint32_t data = 0;
  for (int i=0;i<bits;i++) {
    if (READ_SWDIO())
      data |= (1<<i);
    WRITE_SWDCK(0);
    DELAY_US(2);
    WRITE_SWDCK(1);
    DELAY_US(2);
  }
  return data;
}

bool swdTransfer(SWD_TX_Flags flags, uint32_t *data) {
  bool parity = (((flags&SWDTX_APnDP)?1:0) ^
                 ((flags&SWDTX_RnW)?1:0) ^
                 ((flags&SWDTX_A1)?1:0) ^
                 ((flags&SWDTX_A2)?1:0));
  flags |= SWDTX_START | (parity ? SWDTX_PARITY : 0) | SWDTX_PARK;
  swdWriteBits(flags, 8);
  int response = swdReadBits(3);
  if (response == 1) {
    if (flags & SWDTX_RnW) { // Read 32 bits from SWD
      *data = swdReadBits(32);
      if (swdReadBits(1) == swdParity(*data)) {
        swdWriteBits(0, 1);
        return true;
      }
    } else { // Writing 32 bits to SWD
      swdWriteBits(*data, 32);
      swdWriteBits(swdParity(*data), 1);
      swdWriteBits(0, 1);
      return true;
    }
  }
  swdWriteBits(0, 32); // send reset on failure
  return false;
}

bool swdRead(SWD_TX_Flags flags, uint32_t *data) {
  for (int i=0;i<1;i++)
    if (swdTransfer(flags|SWDTX_READ, data))
      return true;
  return false;
}
bool swdWrite(SWD_TX_Flags flags, uint32_t data) {
  for (int i=0;i<1;i++)
    if (swdTransfer(flags|SWDTX_WRITE, &data))
      return true;
  return false;
}

void nrfWrite(SWD_TX_Flags flags, uint32_t data) {
  swdWrite(flags,    data);
  uint32_t tmp;
  swdRead(SWDTX_DP|SWDTX_DP_RDBUF, &tmp);
  swdRead(SWDTX_DP|SWDTX_DP_RDBUF, &tmp);
}

// aborts (also initialises)
void nrfAbort() {
  nrfWrite(SWDTX_DP|SWDTX_DP_ABORT,    0x1e);
  nrfWrite(SWDTX_DP|SWDTX_DP_CTRL_STAT, 0x50000000); // Debug and System power-up (CDBGPWRUPREQ | CSYSPWRUPREQ).
  // poll DP.CTRL/STAT until bits 31 and 29 (CDBGPWRUPACK and CSYSPWRUPACK) high? or just assume ok...
}

void nrfHalt() {
  nrfWrite(SWDTX_AP|SWDTX_AP_CSW, 0xa2000002);
  nrfWrite(SWDTX_AP|SWDTX_AP_TAR, 0xe000edf0);
  for (int i=0;i<100;i++) { // keep writing just in case
    nrfWrite(SWDTX_AP|SWDTX_AP_DRW, 0xA05F0003);
  }
}

/*void nrfReset() {
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT,    0x01000000); // port select
  nrfWrite(SWDTX_AP|SWDTX_AP,    1); // NRF reset enable
  DELAY_US(100000); // 100ms
  nrfWrite(SWDTX_AP|0,    0); // NRF reset disable
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT,    0); // port select
}*/
void swdSoftReset() { // ARM Core System Reset
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT, 0x00000000);
  swdWrite(SWDTX_AP|SWDTX_AP_TAR,    0xE000ED0C); // target address
  swdWrite(SWDTX_AP|SWDTX_AP_DRW,    0x05FA0004); // value -> reset
}

void swdInit() {
  ENABLE_SWDPINS(true);
  // 50+ cycles of SWDIO=1 for startup
  swdWriteBits(0xFFFFFFFF, 32);
  swdWriteBits(0xFFFFFFFF, 32);
  // init sequence
  swdWriteBits(0xE79E, 16);
  // 50+ cycles of SWDIO=1 again
  swdWriteBits(0xFFFFFFFF, 32);
  swdWriteBits(0xFFFFFFFF, 32);
  // lots of clocks SWD low (12 recommended)
  swdWriteBits(0, 32);
  swdWriteBits(0, 32);
  swdIsWriting = true;

  uint32_t idcode = 0;
  if (!swdRead(SWDTX_DP|SWDTX_DP_IDCODE, &idcode))
    lcd_println("SWD ERR");
  lcd_print("ID ");
  lcd_print_hex(idcode);
  lcd_println("");

  nrfAbort(); // this initialises the interface
  //nrfHalt();
}


void swdKill() {
  /*swdWrite(SWDTX_AP|SWDTX_AP_TAR,    0xE000EDF0); // target address
  swdWrite(SWDTX_AP|SWDTX_AP_DRW,    0xA05F0000); // Un-halt core and disable debug trap logic
  swdWrite(SWDTX_AP|SWDTX_AP_TAR,    0xE000EDFC); // target address
  swdWrite(SWDTX_AP|SWDTX_AP_DRW,    0x00000000); // Disable reset/fault vector catches
  swdWrite(SWDTX_DP|SWDTX_DP_CTRL_STAT, 0x00000000); // clear CDBGPWRUPREQ (bit 28) and CSYSPWRUPREQ (bit 30).
  DELAY_US(1000); // could poll, but let's just delay
  ENABLE_SWDPINS(false);*/
  // issue a hard reset, which should disable SWD and its power draw
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT, 0x02000000); // APSEL = 0x02 (selects CTRL-AP) and APBANKSEL = 0x00
  swdWrite(SWDTX_AP|SWDTX_AP_CSW,    0x00000001); // reset bit
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT, 0x00000000); // APSEL = 0x00
  swdWrite(SWDTX_DP|SWDTX_DP_CTRL_STAT, 0x00000000); // clear CDBGPWRUPREQ (bit 28) and CSYSPWRUPREQ (bit 30).
  ENABLE_SWDPINS(false);
}
