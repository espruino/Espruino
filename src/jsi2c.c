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
 * I2C Utility functions, and software I2C
 * ----------------------------------------------------------------------------
 */
#include "jsi2c.h"
#include "jsinteractive.h"

typedef struct {
  Pin pinSCL;
  Pin pinSDA;
  bool started;
  int delay;
} i2cInfo;

bool jsi2cPopulateI2CInfo(
    JshI2CInfo *inf,
    JsVar      *options
  ) {
  jshI2CInitInfo(inf);

  jsvConfigObject configs[] = {
      {"scl", JSV_PIN, &inf->pinSCL},
      {"sda", JSV_PIN, &inf->pinSDA},
      {"bitrate", JSV_INTEGER, &inf->bitrate}
  };
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return true;
  } else
    return false;
}

#ifndef SAVE_ON_FLASH

// -------------------------------------------------------- I2C Implementation

const int I2C_TIMEOUT = 100000;

static void dly(i2cInfo *inf) {
  volatile int i;
  for (i=inf->delay;i>0;i--);
}

static void err(const char *s) {
  jsExceptionHere(JSET_ERROR, "I2C Error: %s", s);
}

static void i2c_start(i2cInfo *inf) {
  if (inf->started) {
    // reset
    jshPinSetValue(inf->pinSDA, 1);
    dly(inf);
    jshPinSetValue(inf->pinSCL, 1);
    int timeout = I2C_TIMEOUT;
    while (!jshPinGetValue(inf->pinSCL) && --timeout); // clock stretch
    if (!timeout) err("Timeout (start)");
    dly(inf);
  }
  if (!jshPinGetValue(inf->pinSDA)) err("Arbitration (start)");
  jshPinSetValue(inf->pinSDA, 0);
  dly(inf);
  jshPinSetValue(inf->pinSCL, 0);
  dly(inf);
  inf->started = true;
}

static void i2c_stop(i2cInfo *inf) {
  jshPinSetValue(inf->pinSDA, 0);
  dly(inf);
  jshPinSetValue(inf->pinSCL, 1);
  int timeout = I2C_TIMEOUT;
  while (!jshPinGetValue(inf->pinSCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (stop)");
  dly(inf);
  jshPinSetValue(inf->pinSDA, 1);
  dly(inf);
  if (!jshPinGetValue(inf->pinSDA)) err("Arbitration (stop)");
  dly(inf);
  inf->started = false;
}

static void i2c_wr_bit(i2cInfo *inf, bool b) {
  jshPinSetValue(inf->pinSDA, b);
  dly(inf);
  jshPinSetValue(inf->pinSCL, 1); // stop forcing SCL
  dly(inf);
  int timeout = I2C_TIMEOUT;
  while (!jshPinGetValue(inf->pinSCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (wr)");
  jshPinSetValue(inf->pinSCL, 0);
  dly(inf);  
}

static bool i2c_rd_bit(i2cInfo *inf) {
  jshPinSetValue(inf->pinSDA, 1); // stop forcing SDA
  dly(inf);
  jshPinSetValue(inf->pinSCL, 1); // stop forcing SCL
  int timeout = I2C_TIMEOUT;
  while (!jshPinGetValue(inf->pinSCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (rd)");
  dly(inf);
  bool b = jshPinGetValue(inf->pinSDA);
  jshPinSetValue(inf->pinSCL, 0);
  dly(inf);
  return b;
}

// true on ack, false on nack
static bool i2c_wr(i2cInfo *inf, int data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(inf, data&128);
    data <<= 1;
  }
  return !i2c_rd_bit(inf);
}

static int i2c_rd(i2cInfo *inf, bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | (i2c_rd_bit(inf)?1:0);
  i2c_wr_bit(inf, nack);
  jshPinSetValue(inf->pinSDA, 1); // stop forcing SDA
  return data;
}


static void i2c_initstruct(i2cInfo *inf, JshI2CInfo *i) {
  inf->pinSDA = i->pinSDA;
  inf->pinSCL = i->pinSCL;
  inf->started = i->started;
  inf->delay = 4000000/i->bitrate;
}

// ----------------------------------------------------------------------------

void jsi2cWrite(JshI2CInfo *inf, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  if (inf->pinSCL==PIN_UNDEFINED || inf->pinSDA==PIN_UNDEFINED)
    return;
  i2cInfo d;
  i2c_initstruct(&d, inf);
  i2c_start(&d);
  i2c_wr(&d, address<<1);
  int i;
  for (i=0;i<nBytes;i++)
    i2c_wr(&d, data[i]);
  if (sendStop) i2c_stop(&d);
  inf->started = d.started;
}

void jsi2cRead(JshI2CInfo *inf, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  if (inf->pinSCL==PIN_UNDEFINED || inf->pinSDA==PIN_UNDEFINED)
    return;
  i2cInfo d;
  i2c_initstruct(&d, inf);
  i2c_start(&d);
  i2c_wr(&d, 1|(address<<1));
  int i;
  for (i=0;i<nBytes;i++)
    data[i] = (unsigned char)i2c_rd(&d, i==nBytes-1);
  if (sendStop) i2c_stop(&d);
  inf->started = d.started;
}

#endif // SAVE_ON_FLASH
