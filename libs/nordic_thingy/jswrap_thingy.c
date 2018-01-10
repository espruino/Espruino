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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for the hexagonal Espruino badge
 * ----------------------------------------------------------------------------
 */

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

#include <jswrap_thingy.h>

#include "jsinteractive.h"
#include "jsdevices.h"
#include "jshardware.h"
#include "jsi2c.h"

#define I2C_SDA 7
#define I2C_SCL 8
#define SX_I2C_ADDR 0x3e
#define SX_RESET 16
#define SX_POWER 30

unsigned short sxValues = 0;
unsigned short sxDirection = 0;

JshI2CInfo i2cInfo = {
    .bitrate = 400000,
    .pinSCL = I2C_SCL,
    .pinSDA = I2C_SDA,
    .started = false
};

void sxWriteReg(unsigned reg, unsigned d) {
  unsigned char data[2] = {reg,d};
  jsi2cWrite(&i2cInfo, SX_I2C_ADDR, sizeof(data), data, true);
}

unsigned char sxReadReg(unsigned reg) {
  unsigned char data = reg;
  jsi2cWrite(&i2cInfo, SX_I2C_ADDR, 1, &data, true);
  jsi2cRead(&i2cInfo, SX_I2C_ADDR, 1, &data, true);
  return data;
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  if (state) sxValues |= 1<<pinInfo[pin].pin;
  else sxValues &= ~(1<<pinInfo[pin].pin);
  sxWriteReg(0x11, sxValues); // DATAA
}

bool jshVirtualPinGetValue(Pin pin) {
  return 0;
}

void jshVirtualPinSetState(Pin pin, JshPinState state) {
  if (JSHPINSTATE_IS_OUTPUT(state))
    sxDirection &= ~(1<<pinInfo[pin].pin);
  else
    sxDirection |= (1<<pinInfo[pin].pin);
  sxWriteReg(0x0F, sxDirection); // DIRA
}


/*JSON{
  "type" : "init",
  "generate" : "jswrap_thingy_init"
}*/
void jswrap_thingy_init() {
  jshPinOutput(SX_POWER, 1); // enable IO expander power
  sxDirection = sxReadReg(0x0F); // DIRA
  sxValues    = sxReadReg(0x11); // DATAA
  sxWriteReg(0x21, 0); // disable LED driver
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_thingy_kill"
}*/
void jswrap_thingy_kill() {

}

