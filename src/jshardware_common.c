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
}

void jshSPIInitInfo(JshSPIInfo *inf) {
  inf->baudRate     = 100000;
  inf->baudRateSpec = SPIB_DEFAULT;
  inf->pinSCK       = PIN_UNDEFINED;
  inf->pinMISO      = PIN_UNDEFINED;
  inf->pinMOSI      = PIN_UNDEFINED;
  inf->spiMode      = SPIF_SPI_MODE_0;
  inf->spiMSB       = true; // MSB first is default
}

void jshI2CInitInfo(JshI2CInfo *inf) {
  inf->pinSCL = PIN_UNDEFINED;
  inf->pinSDA = PIN_UNDEFINED;
  inf->bitrate = 50000; // Is what we used - shouldn't it be 100k?
  inf->started = false;
}
