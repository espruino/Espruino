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
 * SPI Utility functions, and software SPI
 * ----------------------------------------------------------------------------
 */
#include "jsspi.h"
#include "jsinteractive.h"

/**
 * Dump the internal SPI Info data structure to the console.
 * This is an internal debugging function.
 */
void jsspiDumpSPIInfo(JshSPIInfo *inf) {
	jsiConsolePrintf("baudRate=%d, baudRateSpec=%d, pinSCK=%d, pinMISO=%d, pinMOSI=%d, spiMode=%d, spiMSB=%d, numBits=%d\n",
		inf->baudRate, inf->baudRateSpec, inf->pinSCK, inf->pinMISO, inf->pinMOSI, inf->spiMode, inf->spiMSB, inf->numBits);
}


void jsspiHardwareFunc(unsigned char *tx, unsigned char *rx, unsigned int len, spi_sender_data *info) {
  IOEventFlags device = *(IOEventFlags*)info;
  jshSPISendMany(device, tx, rx, len, NULL/*no callback - sync*/);
}

void jsspiFastSoftwareFunc(
  unsigned char *tx, unsigned char *rx, unsigned int len,
	spi_sender_data *info
  ) {
  JshSPIInfo *inf = (JshSPIInfo*)info;
  // fast path for common case
  for (unsigned int i=0;i<len;i++) {
    int data = tx[i];
    int bit;
    for (bit=inf->numBits - 1;bit>=0;bit--) {
      jshPinSetValue(inf->pinMOSI, (data>>bit)&1 );
      jshPinSetValue(inf->pinSCK, 1 );
      jshPinSetValue(inf->pinSCK, 0 );
    }
  }
}


void jsspiSoftwareFunc(
    unsigned char *tx, unsigned char *rx, unsigned int len,
    spi_sender_data *info
  ) {
  JshSPIInfo *inf = (JshSPIInfo*)info;
  // Debug
  //jsspiDumpSPIInfo(inf);

  bool CPHA = (inf->spiMode & SPIF_CPHA)!=0;
  bool CPOL = (inf->spiMode & SPIF_CPOL)!=0;

  const int bitDir = inf->spiMSB ? -1 : 1;
  const int endBit = inf->spiMSB ? -1 : inf->numBits;
  for (unsigned int i=0;i<len;i++) {
    int data = tx[i];
    int result = 0;
    int bit = inf->spiMSB ? (inf->numBits-1) : 0;
    for (;bit!=endBit;bit+=bitDir) {
      if (!CPHA) { // 'Normal' SPI, CPHA=0
        if (inf->pinMOSI != PIN_UNDEFINED)
          jshPinSetValue(inf->pinMOSI, (data>>bit)&1 );
        if (inf->pinSCK != PIN_UNDEFINED)
          jshPinSetValue(inf->pinSCK, !CPOL );
        if (inf->pinMISO != PIN_UNDEFINED)
          result |= (jshPinGetValue(inf->pinMISO )?1:0)<<bit;
        if (inf->pinSCK != PIN_UNDEFINED)
          jshPinSetValue(inf->pinSCK, CPOL );
      } else { // CPHA=1
        if (inf->pinSCK != PIN_UNDEFINED)
          jshPinSetValue(inf->pinSCK, !CPOL );
        if (inf->pinMOSI != PIN_UNDEFINED)
          jshPinSetValue(inf->pinMOSI, (data>>bit)&1 );
        if (inf->pinSCK != PIN_UNDEFINED)
          jshPinSetValue(inf->pinSCK, CPOL );
        if (inf->pinMISO != PIN_UNDEFINED)
          result |= (jshPinGetValue(inf->pinMISO )?1:0)<<bit;
      }
    }
    if (rx) rx[i] = result;
  }
}


/**
 * Populate a JshSPIInfo structure from a JS Object.
 * The object properties that are examined are:
 * * `sck` - The pin to use for the clock.
 * * `miso` - The pin to use for Master In/Slave Out.
 * * `mosi` - The pin to use for Master Out/Slave In.
 * * `baud` - The baud rate value.
 * * `mode` - The SPI mode.
 * * `order` - The bit order (one of "msb" or "lsb")
 * * `bits` - number of bits per send.
 */
bool jsspiPopulateSPIInfo(
    JshSPIInfo *inf,    //!< The JshSPIInfo structure to populate.
    JsVar      *options //!< The JS object var to parse.
  ) {
  jshSPIInitInfo(inf);

  JsVar *order = 0;
  int spiMode = inf->spiMode;
  jsvConfigObject configs[] = {
      {"sck", JSV_PIN, &inf->pinSCK},
      {"miso", JSV_PIN, &inf->pinMISO},
      {"mosi", JSV_PIN, &inf->pinMOSI},
      {"baud", JSV_INTEGER, &inf->baudRate},
      {"mode", JSV_INTEGER, &spiMode}, // don't reference direct as this is just a char, not unsigned integer
      {"order", JSV_OBJECT /* a variable */, &order},
      {"bits", JSV_INTEGER, &inf->numBits},
  };
  bool ok = true;
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    inf->spiMode = spiMode&3;

    if (jsvIsString(order) && jsvIsStringEqual(order, "msb")) {
      inf->spiMSB = true;
    } else if (jsvIsString(order) && jsvIsStringEqual(order, "lsb")) {
      inf->spiMSB = false;
    } else if (!jsvIsUndefined(order)) {
      jsExceptionHere(JSET_ERROR, "SPI order should be 'msb' or 'lsb'");
      ok = false;
    }

    if (inf->baudRate < 100) {
      jsExceptionHere(JSET_ERROR, "Invalid SPI baud rate");
      ok = false;
    }
  }
  jsvUnLock(order);
  return ok;
}

/**
 * Select the SPI send function.
 * Get the correct SPI send function (and the data to send to it).  We do this
 * by examining the device and determining if it is hardware, software fast
 * or software regular. Return True on success, false otherwise.
 */
bool jsspiGetSendFunction(
    JsVar           *spiDevice,  //!< The device that we want to get the SPI drivers for.
    spi_sender      *spiSend,    //!< Return the function to called to send SPI data.
    spi_sender_data *spiSendData //!< Return configuration data needed to drive SPI.
  ) {
  // The spiSendData is a little ugly.  The value set here is either an
  // JshSPIInfo which is a structure describing the configuration of SPI or else
  // it is a device id.

  IOEventFlags device = jsiGetDeviceFromClass(spiDevice);
  JshSPIInfo inf;

  // See if the device is hardware or software.
  if (DEVICE_IS_SPI(device)) {
    //
    // jsiConsolePrintf("SPI is hardware\n");
    if (!jshIsDeviceInitialised(device)) {
      jshSPIInitInfo(&inf);
      jshSPISetup(device, &inf);
    }
    *spiSend = jsspiHardwareFunc;
    *(IOEventFlags*)spiSendData = device;
    return true;
  } else if (device == EV_NONE) {
    // Debug
    // jsiConsolePrintf("SPI is software\n");
    JsVar *options = jsvObjectGetChild(spiDevice, DEVICE_OPTIONS_NAME, 0);
    jsspiPopulateSPIInfo(&inf, options);
    jsvUnLock(options);

    if (inf.pinMISO == PIN_UNDEFINED &&
        inf.pinMOSI != PIN_UNDEFINED &&
        inf.pinSCK  != PIN_UNDEFINED &&
        inf.spiMode == SPIF_SPI_MODE_0 &&
        inf.spiMSB) {
      *spiSend = jsspiFastSoftwareFunc;
    } else {
      *spiSend = jsspiSoftwareFunc;
    }
    *spiSendData = inf;
    return true;
  }
  return false;
}


// Send data over SPI. If andReceive is true, write it back into the same buffer
bool jsspiSend(JsVar *spiDevice, JsSpiSendFlags flags, char *buf, size_t len) {
  spi_sender spiSend;
  spi_sender_data spiSendData;
  if (!jsspiGetSendFunction(spiDevice, &spiSend, &spiSendData))
    return false;

  spiSend((unsigned char*)buf, (flags&JSSPI_NO_RECEIVE)?0:(unsigned char*)buf, len , &spiSendData);
  // wait if we need to
  if (flags & JSSPI_WAIT) {
    IOEventFlags device = jsiGetDeviceFromClass(spiDevice);
    if (DEVICE_IS_SPI(device)) jshSPIWait(device);
  }
  return !jspIsInterrupted();
}


// used by jswrap_spi_send4bit
void jsspiSend4bit(IOEventFlags device, unsigned char data, int bit0, int bit1) {
  unsigned char lookup[] = {
      (unsigned char)((bit0<<4) | bit0),
      (unsigned char)((bit0<<4) | bit1),
      (unsigned char)((bit1<<4) | bit0),
      (unsigned char)((bit1<<4) | bit1),
  };
  // Send each bit as 4 bits, MSB first
  /*jshSPISend(device, lookup[(data>>6)&3]);
  jshSPISend(device, lookup[(data>>4)&3]);
  jshSPISend(device, lookup[(data>>2)&3]);
  jshSPISend(device, lookup[(data   )&3]);*/
  jshSPISend16(device, (lookup[(data>>6)&3]<<8) | lookup[(data>>4)&3]);
  jshSPISend16(device, (lookup[(data>>2)&3]<<8) | lookup[(data   )&3]);
}

// used by jswrap_spi_send8bit
void jsspiSend8bit(IOEventFlags device, unsigned char data, int bit0, int bit1) {
  // Send each bit as 8 bits, MSB first
  /*int i;
  for (i=7;i>=0;i--)
    jshSPISend(device, (unsigned char)(((data>>i)&1) ? bit1 : bit0));*/
  jshSPISend(device, ((((data>>7)&1) ? bit1 : bit0)<<8) | (((data>>6)&1) ? bit1 : bit0));
  jshSPISend(device, ((((data>>5)&1) ? bit1 : bit0)<<8) | (((data>>4)&1) ? bit1 : bit0));
  jshSPISend(device, ((((data>>3)&1) ? bit1 : bit0)<<8) | (((data>>2)&1) ? bit1 : bit0));
  jshSPISend(device, ((((data>>1)&1) ? bit1 : bit0)<<8) | (((data>>0)&1) ? bit1 : bit0));
}
