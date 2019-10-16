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
      if (rx) rx[rxPtr] = (char)data;
      rxPtr++;
    }
  }
  // clear the rx buffer
  while (rxPtr<count && !jspIsInterrupted()) {
    int data = jshSPISend(device, -1);
    if (rx) rx[rxPtr] = (char)data;
    rxPtr++;
  }
  // call the callback
  if (callback) callback();
  return true;
}

// Only define this if it's not used elsewhere
__attribute__((weak)) void jshBusyIdle() {
}
