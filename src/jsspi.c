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

int jsspiHardwareFunc(int data, spi_sender_data *info) {
  IOEventFlags device = *(IOEventFlags*)info;
  return jshSPISend(device, data);
}

int jsspiSoftwareFunc(int data, spi_sender_data *info) {
  if (data<0) return -1;
  JshSPIInfo *inf = (JshSPIInfo*)info;

  bool CPHA = (inf->spiMode & SPIF_CPHA)!=0;
  bool CPOL = (inf->spiMode & SPIF_CPOL)!=0;

  int result = 0;
  int bit = inf->spiMSB ? 7 : 0;
  int bitDir = inf->spiMSB ? -1 : 1;
  int endBit = inf->spiMSB ? -1 : 8;

  for (;bit!=endBit;bit+=bitDir) {
    if (!CPHA) { // 'Normal' SPI, CPHA=0
      if (inf->pinMOSI != PIN_UNDEFINED)
        jshPinSetValue(inf->pinMOSI, (data>>bit)&1 );
      if (inf->pinSCK != PIN_UNDEFINED)
        jshPinSetValue(inf->pinSCK, CPOL ? 0 : 1 );
      if (inf->pinMISO != PIN_UNDEFINED)
         result = (result<<1) | (jshPinGetValue(inf->pinMISO )?1:0);
      if (inf->pinSCK != PIN_UNDEFINED)
        jshPinSetValue(inf->pinSCK, CPOL ? 1 : 0 );
    } else { // CPHA=1
      if (inf->pinSCK != PIN_UNDEFINED)
        jshPinSetValue(inf->pinSCK, CPOL ? 0 : 1 );
      if (inf->pinMOSI != PIN_UNDEFINED)
        jshPinSetValue(inf->pinMOSI, (data>>bit)&1 );
      if (inf->pinSCK != PIN_UNDEFINED)
         jshPinSetValue(inf->pinSCK, CPOL ? 1 : 0 );
      if (inf->pinMISO != PIN_UNDEFINED)
         result = (result<<1) | (jshPinGetValue(inf->pinMISO )?1:0);
    }
  }

  return result;
}

void jsspiPopulateSPIInfo(JshSPIInfo *inf, JsVar *options) {
  jshSPIInitInfo(inf);
  if (jsvIsObject(options)) {
    inf->pinSCK = jshGetPinFromVarAndUnLock(jsvObjectGetChild(options, "sck", 0));
    inf->pinMISO = jshGetPinFromVarAndUnLock(jsvObjectGetChild(options, "miso", 0));
    inf->pinMOSI = jshGetPinFromVarAndUnLock(jsvObjectGetChild(options, "mosi", 0));
    JsVar *v;
    v = jsvObjectGetChild(options, "baud", 0);
    if (jsvIsNumeric(v))
      inf->baudRate = (int)jsvGetInteger(v);
    v = jsvObjectGetChild(options, "mode", 0);
    if (jsvIsNumeric(v))
      inf->spiMode = ((int)jsvGetInteger(v))&3;
    v = jsvObjectGetChild(options, "order", 0);
    if (jsvIsString(v) && jsvIsStringEqual(v, "msb")) {
      inf->spiMSB = true;
    } else if (jsvIsString(v) && jsvIsStringEqual(v, "lsb")) {
      inf->spiMSB = false;
    } else if (!jsvIsUndefined(v))
      jsWarn("SPI order should be 'msb' or 'lsb'");
    jsvUnLock(v);
  }
}

// Get the correct SPI send function (and the data to send to it)
bool jsspiGetSendFunction(JsVar *spiDevice, spi_sender *spiSend, spi_sender_data *spiSendData) {
  IOEventFlags device = jsiGetDeviceFromClass(spiDevice);
  if (DEVICE_IS_SPI(device)) {
    if (!jshIsDeviceInitialised(device)) {
      JshSPIInfo inf;
      jshSPIInitInfo(&inf);
      jshSPISetup(device, &inf);
    }
    *spiSend = jsspiHardwareFunc;
    *(IOEventFlags*)spiSendData = device;
    return true;
  } else if (device == EV_NONE) {
    JsVar *options = jsvObjectGetChild(spiDevice, DEVICE_OPTIONS_NAME, 0);
    static JshSPIInfo inf;
    jsspiPopulateSPIInfo(&inf, options);
    jsvUnLock(options);
    *spiSend = jsspiSoftwareFunc;
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

  size_t txPtr = 0;
  size_t rxPtr = 0;
  // transmit the data
  while (txPtr<len && !jspIsInterrupted()) {
    int data = spiSend(buf[txPtr++], &spiSendData);
    if (data>=0) buf[rxPtr++] = (char)data;
  }
  // clear the rx buffer
  while (rxPtr<len && !jspIsInterrupted()) {
    buf[rxPtr++] = (char)spiSend(-1, &spiSendData);
  }
  // wait if we need to
  if (flags & JSSPI_WAIT) {
    IOEventFlags device = jsiGetDeviceFromClass(spiDevice);
    if (DEVICE_IS_SPI(device)) jshSPIWait(device);
  }
  return true;
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
