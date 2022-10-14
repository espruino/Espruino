/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Hardware interface Layer common functions
 * ----------------------------------------------------------------------------
 */
#include "jshardware.h"
#include "jsinteractive.h"
#include "platform_config.h"

void jshUSARTInitInfo(JshUSARTInfo *inf) {
  inf->baudRate = DEFAULT_BAUD_RATE;
  inf->pinRX    = PIN_UNDEFINED;
  inf->pinTX    = PIN_UNDEFINED;
  inf->pinCK    = PIN_UNDEFINED;
  inf->pinCTS   = PIN_UNDEFINED;
  inf->bytesize = DEFAULT_BYTESIZE;
  inf->parity   = DEFAULT_PARITY; // PARITY_NONE = 0, PARITY_ODD = 1, PARITY_EVEN = 2 FIXME: enum?
  inf->stopbits = DEFAULT_STOPBITS;
  inf->xOnXOff = false;
  inf->errorHandling = false;
}

void jshSPIInitInfo(JshSPIInfo *inf) {
  inf->baudRate     = 100000;
  inf->baudRateSpec = SPIB_DEFAULT;
  inf->pinSCK       = PIN_UNDEFINED;
  inf->pinMISO      = PIN_UNDEFINED;
  inf->pinMOSI      = PIN_UNDEFINED;
  inf->spiMode      = SPIF_SPI_MODE_0;
  inf->spiMSB       = true; // MSB first is default
  inf->numBits      = 8;
}

void jshI2CInitInfo(JshI2CInfo *inf) {
  inf->pinSCL = PIN_UNDEFINED;
  inf->pinSDA = PIN_UNDEFINED;
  inf->bitrate = 100000;
  inf->started = false;
  inf->clockStretch = true;
#ifdef I2C_SLAVE
  inf->slaveAddr = -1; // master default
#endif
}

void jshFlashWriteAligned(void *buf, uint32_t addr, uint32_t len) {
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH))) {
    // If using external flash it doesn't care about alignment, so don't bother
    jshFlashWrite(buf, addr, len);
    return;
  }
#endif
  unsigned char *dPtr = (unsigned char *)buf;
  uint32_t alignOffset = addr & (JSF_ALIGNMENT-1);
  if (alignOffset) {
    char buf[JSF_ALIGNMENT];
    jshFlashRead(buf, addr-alignOffset, JSF_ALIGNMENT);
    uint32_t alignRemainder = JSF_ALIGNMENT-alignOffset;
    if (alignRemainder > len)
      alignRemainder = len;
    memcpy(&buf[alignOffset], dPtr, alignRemainder);
    dPtr += alignRemainder;
    jshFlashWrite(buf, addr-alignOffset, JSF_ALIGNMENT);
    addr += alignRemainder;
    if (alignRemainder >= len)
      return; // we're done!
    len -= alignRemainder;
  }
  // Do aligned write
  alignOffset = len & (JSF_ALIGNMENT-1);
  len -= alignOffset;
  if (len)
    jshFlashWrite(dPtr, addr, len);
  addr += len;
  dPtr += len;
  // Do final unaligned write
  if (alignOffset) {
    char buf[JSF_ALIGNMENT];
    jshFlashRead(buf, addr, JSF_ALIGNMENT);
    memcpy(buf, dPtr, alignOffset);
    jshFlashWrite(buf, addr, JSF_ALIGNMENT);
  }
}
/** Send data in tx through the given SPI device and return the response in
 * rx (if supplied). Returns true on success */
__attribute__((weak)) bool jshSPISendMany(IOEventFlags device, unsigned char *tx, unsigned char *rx, size_t count, void (*callback)()) {
  size_t txPtr = 0;
  size_t rxPtr = 0;
  // transmit the data
  while (txPtr<count && !jspIsInterrupted()) {
    int data = jshSPISend(device, tx[txPtr++]);
    if (data>=0) {
      if (rx) rx[rxPtr++] = (char)data;
    }
  }
  // clear the rx buffer
  if (rx) {
    while (rxPtr<count && !jspIsInterrupted()) {
      int data = jshSPISend(device, -1);
      rx[rxPtr++] = (char)data;
    }
  } else {
    // wait for it to finish so we can clear the buffer
    jshSPIWait(device);
  }
  // call the callback
  if (callback) callback();
  return true;
}

// Only define this if it's not used elsewhere
__attribute__((weak)) void jshBusyIdle() {
}

// Only define this if it's not used elsewhere
__attribute__((weak)) bool jshIsPinStateDefault(Pin pin, JshPinState state) {
  return state == JSHPINSTATE_GPIO_IN || state == JSHPINSTATE_ADC_IN;
}

__attribute__((weak)) void jshUSARTUnSetup(IOEventFlags device) {
  NOT_USED(device);
  // placeholder - not all platforms implement this
}

/// Erase the flash pages containing the address.
__attribute__((weak)) bool jshFlashErasePages(uint32_t startAddr, uint32_t byteLength) {
  uint32_t endAddr = startAddr + byteLength;
  uint32_t addr, len;
  if (!jshFlashGetPage(startAddr, &addr, &len))
    return false; // not a valid page
  while (addr<endAddr && !jspIsInterrupted()) {
    jshFlashErasePage(addr);
    if (!jshFlashGetPage(addr+len, &addr, &len))
      return true; // no more pages
    // Erasing can take a while, so kick the watchdog throughout
    jshKickWatchDog();
  }
  return !jspIsInterrupted();
}
