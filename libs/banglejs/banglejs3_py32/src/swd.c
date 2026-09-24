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
 *
 * based on https://qcentlabs.com/posts/swd_banger/ and https://github.com/atc1441/ESP32_nRF52_SWD
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdbool.h>

#ifdef BANGLEJS3
#include "jshardware.h"
#include "jsinteractive.h"
#define lcd_print(s) jsiConsolePrintf("%s",s)
#define lcd_println(s) jsiConsolePrintf("%s\n",s)
#define lcd_print_hex(x) jsiConsolePrintf("0x%08x",x)
// IO defines - these vary depending on device

#define ENABLE_SWDPINS(enable) { \
  jshPinSetState(MISC_PIN_PY32_SWDIO, enable ? JSHPINSTATE_GPIO_OUT : JSHPINSTATE_GPIO_IN ); \
  jshPinSetState(MISC_PIN_PY32_SWDCK, enable ? JSHPINSTATE_GPIO_OUT : JSHPINSTATE_GPIO_IN ); \
}
#define SET_SWDIO_OUT(isOut) { \
  jshPinSetState(MISC_PIN_PY32_SWDIO, isOut ? JSHPINSTATE_GPIO_OUT : JSHPINSTATE_GPIO_IN_PULLUP ); \
}
#define WRITE_SWDIO(x) jshPinSetValue(MISC_PIN_PY32_SWDIO, x)
#define WRITE_SWDCK(x) jshPinSetValue(MISC_PIN_PY32_SWDCK, x)
#define READ_SWDIO() jshPinGetValue(MISC_PIN_PY32_SWDIO)
#define DELAY_US(n) for (volatile int z=0;z<n*10;z++);

#else // PY32
#include "lcd.h"
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

#endif


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

// Perform an SWD transfer depending on the flags. true on success
bool swdTransfer(SWD_TX_Flags flags, uint32_t *data) {
  int retries = 100;
  int response = 2;
  while (response==2 && retries>0) {
    retries--;
    bool parity = (((flags&SWDTX_APnDP)?1:0) ^
                  ((flags&SWDTX_RnW)?1:0) ^
                  ((flags&SWDTX_A1)?1:0) ^
                  ((flags&SWDTX_A2)?1:0));
    flags |= SWDTX_START | (parity ? SWDTX_PARITY : 0) | SWDTX_PARK;
    swdWriteBits(flags, 8);
    response = swdReadBits(3);
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
    if (response!=2) jsiConsolePrintf("SWD ERR %d\n", response);
    swdWriteBits(0, 32); // send reset on failure
  }
  if (retries==0) jsiConsolePrintf("SWD ERR %d (timeout)\n", response);
  return false;
}

// read from SWD, true on success
bool swdRead(SWD_TX_Flags flags, uint32_t *data) {
  for (int i=0;i<1;i++)
    if (swdTransfer(flags|SWDTX_READ, data))
      return true;
  return false;
}
// write to SWD, true on success
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

/// Read from a specific memory address
uint32_t swdReadMem(uint32_t addr) {
  uint32_t result;
  swdWrite(SWDTX_AP|SWDTX_AP_TAR, addr);
  swdRead(SWDTX_AP|SWDTX_AP_DRW, &result);
  swdRead(SWDTX_DP|SWDTX_DP_RDBUF, &result);
  //jsiConsolePrintf("swdReadMem(0x%08x)=0x%08x\n", addr, result);
  return result;
}

/// Write to a specific memory address
void swdWriteMem(uint32_t addr, uint32_t value) {
  uint32_t t;
  swdWrite(SWDTX_AP|SWDTX_AP_TAR, addr);
  swdWrite(SWDTX_AP|SWDTX_AP_DRW, value);
  swdRead(SWDTX_DP|SWDTX_DP_RDBUF, &t);
  //jsiConsolePrintf("swdWriteMem(0x%08x,0x%08x)\n", addr, value);
}

void swdPY32Unlock() {
  uint32_t lock = swdReadMem(0x40022014);
  if (lock & 0x80000000) { // locked
    swdWriteMem(0x40022008, 0x45670123);
    swdWriteMem(0x40022008, 0xCDEF89AB);
  }
}

void swdPY32WaitFlashBusy() {
  while (swdReadMem(0x40022010)&1); // FLASH_SR.BUSY
  swdWriteMem(0x40022010, 0x00008011); // FLASH_SR clear flags
}

// Erase a page, addr=0x08000000 onwards?
void swdPY32FlashErase() {
  swdPY32Unlock();
  swdPY32WaitFlashBusy();
  swdWriteMem(0x40022014, 0x00000004); // FLASH_CR mass erase
  swdWriteMem(0x08000000, 0x12344321); // force erase
  swdPY32WaitFlashBusy();
  swdWriteMem(0x40022014, 0x00000000); // FLASH_CR disable erase bit
}

void swdPY32FlashWriteInit() {
#if 0
  swdWriteMem(0x40022030/*FLASH_TPS*/,  16); // 16 mhz
  swdWriteMem(0x4002201C/*FLASH_TS0*/,  0x00000078);
  swdWriteMem(0x40022020/*FLASH_TS1*/,  0x0000002B);
  swdWriteMem(0x40022024/*FLASH_TS2P*/, 0x00000078);
  swdWriteMem(0x40022028/*FLASH_TS3P*/, 0x00000035);
  swdWriteMem(0x4002202C/*FLASH_PERT*/, 0x00001388);
#else
  swdWriteMem(0x40022030/*FLASH_TPS*/, 24); // 24 mhz
  swdWriteMem(0x4002201C/*FLASH_TS0*/,  0x000000B4);
  swdWriteMem(0x40022020/*FLASH_TS1*/,  0x00000040);
  swdWriteMem(0x40022024/*FLASH_TS2P*/, 0x000000B4);
  swdWriteMem(0x40022028/*FLASH_TS3P*/, 0x00000050);
  swdWriteMem(0x4002202C/*FLASH_PERT*/, 0x00001D4B);
#endif
}

// write to flash - len in bytes. Start at 256b boundary, write 256b
void swdPY32FlashWrite(uint32_t addr, uint32_t *buf, int len) {
  swdPY32Unlock();
  swdPY32WaitFlashBusy();
  swdWriteMem(0x40022014, 0x00000001); // FLASH_CR PG
  for (int i=0;i<len;i+=4) {
    if (((i>>2)&63)==63) // set PGSTART before final word
      swdWriteMem(0x40022014, 0x00080001); // FLASH_CR PG + PGSTRT
    swdWriteMem(addr+i, buf[i>>2]); // write data
  }
  swdPY32WaitFlashBusy();
  swdWriteMem(0x40022014, 0x00000000); // FLASH_CR disable write bit
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
#ifdef BANGLEJS3 // this is to program the PY32
  // bring up SWD
  swdWrite(SWDTX_DP|SWDTX_DP_CTRL_STAT, 0x50000000); // CDBGPWRUPREQ | CSYSPWRUPREQ
  // Poll CTRL/STAT until both Acknowledge bits are set
  uint32_t ctrlstat = 0;
  uint32_t timeout = 1000;
  do {
    swdRead(SWDTX_DP|SWDTX_DP_CTRL_STAT, &ctrlstat);
    if ((ctrlstat & 0xA0000000) == 0xA0000000) // CDBGPWRUPACK | CSYSPWRUPACK
      break; // Power domains are fully active
  } while (--timeout > 0);
  if (timeout == 0) {
    lcd_println("SWD TIMEOUT");
    return;
  }
  // Initialize MEM-AP CSW register (32-bit width, auto-increment off or on)
  swdWrite(SWDTX_DP|SWDTX_DP_SELECT, 0x00000000); // Select AP 0, Bank 0
  swdWrite(SWDTX_AP|SWDTX_AP_CSW, 0x23000002); // Set CSW: 32-bit access width
  // halt the core
  // Bit 0 (C_DEBUGEN) = 1 (Enable Debugging)
  // Bit 1 (C_HALT)    = 1 (Halt Core)
  swdWriteMem(0xE000EDF0, 0xA05F0003); // DHCSR

  swdReadMem(0);
  swdReadMem(4);
  swdReadMem(8);
  jsiConsolePrintf("Erase\n");
  swdPY32FlashErase();
  jsiConsolePrintf("Read\n");
  swdReadMem(0);
  swdReadMem(4);
  swdReadMem(8);
  swdReadMem(128);
  jsiConsolePrintf("Write\n");
  swdPY32FlashWriteInit();
  uint32_t b[64] = { 0x1234, 0x4567, 0x89AB, 0xCDEF };
  swdPY32FlashWrite(0x08000000, b, sizeof(b));
  jsiConsolePrintf("Read\n");
  swdReadMem(0);
  swdReadMem(4);
  swdReadMem(8);
  swdReadMem(128);
  swdReadMem(0x20000000);
  swdWriteMem(0x20000000,0xDEADBEEF);
  swdReadMem(0x20000000);

  swdWriteMem(0xE000EDF0, 0xA05F0001); // DHCSR resume
#endif


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
