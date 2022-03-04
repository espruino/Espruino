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
  int delay; ///< delay per bit
  int timeout; ///< how long do we wait for clock stretching
} i2cInfo;

bool jsi2cPopulateI2CInfo(
    JshI2CInfo *inf,
    JsVar      *options
  ) {
  jshI2CInitInfo(inf);

  jsvConfigObject configs[] = {
      {"scl", JSV_PIN, &inf->pinSCL},
      {"sda", JSV_PIN, &inf->pinSDA},
#ifdef I2C_SLAVE
      {"addr", JSV_INTEGER, &inf->slaveAddr},
#endif
      {"bitrate", JSV_INTEGER, &inf->bitrate}
  };
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    bool ok = true;
    if (inf->bitrate < 100) {
      jsExceptionHere(JSET_ERROR, "Invalid I2C bitrate");
      ok = false;
    }
    return ok;
  } else
    return false;
}

#ifndef SAVE_ON_FLASH

// Do we check to see if we got a NACK or not?
// Some devices (like Bangle.js IO controller) don't ever ACK
//#define I2C_NACK_CHECK

// -------------------------------------------------------- I2C Implementation

static void dly(i2cInfo *inf) {
  if (inf->delay) jshDelayMicroseconds(inf->delay);
}

static void err(const char *s) {
  jsExceptionHere(JSET_ERROR, "I2C Error: %s", s);
}

static void i2c_pin_wr1(Pin pin) {
  jshPinSetValue(pin, 1);
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT); // force up first, to allow higher clock speeds
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP);
}

static void i2c_pin_wr0(Pin pin) {
  jshPinSetValue(pin, 0);
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
}

static void i2c_start(i2cInfo *inf) {
  if (inf->started) {
    // reset
    i2c_pin_wr1(inf->pinSDA);
    dly(inf);
    i2c_pin_wr1(inf->pinSCL);
    int timeout = inf->timeout;
    while (!jshPinGetValue(inf->pinSCL) && timeout) timeout--; // clock stretch
    if (inf->timeout && !timeout) err("Timeout (start)");
    dly(inf);
  }
  if (inf->timeout && !jshPinGetValue(inf->pinSDA)) err("Arbitration (start)");
  i2c_pin_wr0(inf->pinSDA);
  dly(inf);
  i2c_pin_wr0(inf->pinSCL);
  dly(inf);
  inf->started = true;
}

static void i2c_stop(i2cInfo *inf) {
  i2c_pin_wr0(inf->pinSDA);
  dly(inf);
  i2c_pin_wr1(inf->pinSCL);
  int timeout = inf->timeout;
  while (!jshPinGetValue(inf->pinSCL) && timeout) timeout--; // clock stretch
  if (inf->timeout && !timeout) err("Timeout (stop)");
  dly(inf);
  i2c_pin_wr1(inf->pinSDA);
  dly(inf);
  if (inf->timeout && !jshPinGetValue(inf->pinSDA)) err("Arbitration (stop)");
  dly(inf);
  inf->started = false;
}

static void i2c_wr_bit(i2cInfo *inf, bool b) {
  if (b) i2c_pin_wr1(inf->pinSDA);
  else i2c_pin_wr0(inf->pinSDA);
  dly(inf);
  i2c_pin_wr1(inf->pinSCL); // stop forcing SCL
  dly(inf);
  dly(inf);
  int timeout = inf->timeout;
  while (!jshPinGetValue(inf->pinSCL) && timeout) timeout--; // clock stretch
  if (inf->timeout && !timeout) err("Timeout (wr)");
  i2c_pin_wr0(inf->pinSCL);
  i2c_pin_wr1(inf->pinSDA); // stop forcing SDA (needed?)
  dly(inf);  
}

static bool i2c_rd_bit(i2cInfo *inf) {
  i2c_pin_wr1(inf->pinSDA); // stop forcing SDA
  dly(inf);
  i2c_pin_wr1(inf->pinSCL); // stop forcing SCL
  dly(inf);
  int timeout = inf->timeout;
  while (!jshPinGetValue(inf->pinSCL) && timeout--); // clock stretch
  if (inf->timeout && !timeout) err("Timeout (rd)");
  dly(inf);
  bool b = jshPinGetValue(inf->pinSDA);
  i2c_pin_wr0(inf->pinSCL);
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
#if I2C_NACK_CHECK
  return !i2c_rd_bit(inf);
#else
  i2c_rd_bit(inf);
  return true; // always ok
#endif
}

static int i2c_rd(i2cInfo *inf, bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | (i2c_rd_bit(inf)?1:0);
  i2c_wr_bit(inf, nack);
  i2c_pin_wr1(inf->pinSDA); // stop forcing SDA
  return data;
}


static void i2c_initstruct(i2cInfo *inf, JshI2CInfo *i) {
  inf->pinSDA = i->pinSDA;
  inf->pinSCL = i->pinSCL;
  inf->started = i->started;
  inf->delay = 250000/i->bitrate;
  inf->timeout = i->clockStretch ? 100000 : 0;
}

// ----------------------------------------------------------------------------

void jsi2cSetup(JshI2CInfo *inf) {
  jshPinSetValue(inf->pinSCL, 1);
  jshPinSetState(inf->pinSCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(inf->pinSDA, 1);
  jshPinSetState(inf->pinSDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
}

void jsi2cUnsetup(JshI2CInfo *inf) {
  jshPinSetState(inf->pinSCL, JSHPINSTATE_UNDEFINED);
  jshPinSetState(inf->pinSDA, JSHPINSTATE_UNDEFINED);
}

bool jsi2cWrite(JshI2CInfo *inf, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  if (inf->pinSCL==PIN_UNDEFINED || inf->pinSDA==PIN_UNDEFINED)
    return false;
  i2cInfo d;
  i2c_initstruct(&d, inf);
  i2c_start(&d);
  if (!i2c_wr(&d, address<<1)) // NACK happened when writing to I2C address
    return false;
  int i;
  for (i=0;i<nBytes;i++)
    i2c_wr(&d, data[i]);
  if (sendStop) i2c_stop(&d);
  inf->started = d.started;
  return true;
}

bool jsi2cRead(JshI2CInfo *inf, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  if (inf->pinSCL==PIN_UNDEFINED || inf->pinSDA==PIN_UNDEFINED)
    return false;
  i2cInfo d;
  i2c_initstruct(&d, inf);
  i2c_start(&d);
  if (!i2c_wr(&d, 1|(address<<1))) // NACK happened when reading from I2C address
    return false;
  int i;
  for (i=0;i<nBytes;i++)
    data[i] = (unsigned char)i2c_rd(&d, i==nBytes-1);
  if (sendStop) i2c_stop(&d);
  inf->started = d.started;
  return true;
}

#endif // SAVE_ON_FLASH
