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
 * Contains JavaScript interface for the Nordic Thingy:52
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
// SX1509 IO Expander
#define SX_I2C_ADDR 0x3e
#define SX_RESET 16
#define SX_POWER 30
#define SX_REG_DIRA 0x0F
#define SX_REG_DIRB 0x0E
#define SX_REG_DATAA 0x11
#define SX_REG_DATAB 0x10

unsigned short sxValues = 0;
unsigned short sxDirection = 0;

JshI2CInfo i2cInfo = {
    .bitrate = 0x7FFFFFFF, // FAST
    .pinSCL = I2C_SCL,
    .pinSDA = I2C_SDA,
    .started = false
};

// Write to IO expander
void sxWriteReg(unsigned reg, unsigned d) {
  unsigned char data[2] = {reg,d};
  jsi2cWrite(&i2cInfo, SX_I2C_ADDR, sizeof(data), data, true);
}
// Read from IO expander
unsigned char sxReadReg(unsigned reg) {
  unsigned char data = reg;
  jsi2cWrite(&i2cInfo, SX_I2C_ADDR, 1, &data, true);
  jsi2cRead(&i2cInfo, SX_I2C_ADDR, 1, &data, true);
  return data;
}

void jshVirtualPinInitialise() {
  jshPinOutput(SX_POWER, 1); // enable IO expander power
  jshPinSetValue(i2cInfo.pinSDA, 1);
  jshPinSetState(i2cInfo.pinSCL,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(i2cInfo.pinSDA, 1);
  jshPinSetState(i2cInfo.pinSDA,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshDelayMicroseconds(10);
  // these are the power-on defaults
  sxDirection = 0xFFFF;
  sxValues    = 0xFFFF;
  sxWriteReg(SX_REG_DATAA, sxValues&0xFF);
  sxWriteReg(SX_REG_DATAB, sxValues>>8);
  sxWriteReg(SX_REG_DIRA, sxDirection&0xFF);
  sxWriteReg(SX_REG_DIRB, sxDirection>>8);
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  int p = pinInfo[pin].pin;
  if (state) sxValues |= 1<<p;
  else sxValues &= ~(1<<p);
  if (p<8) sxWriteReg(SX_REG_DATAA, sxValues&0xFF);
  else sxWriteReg(SX_REG_DATAB, sxValues>>8);
}

bool jshVirtualPinGetValue(Pin pin) {
  return 0;
}

void jshVirtualPinSetState(Pin pin, JshPinState state) {
  int p = pinInfo[pin].pin;
  if (JSHPINSTATE_IS_OUTPUT(state))
    sxDirection &= ~(1<<p);
  else
    sxDirection |= (1<<p);
  if (p<8) sxWriteReg(SX_REG_DIRA, sxDirection&0xFF);
  else sxWriteReg(SX_REG_DIRB, sxDirection>>8);
}

/*JSON{"type" : "variable", "name" : "MOS1", "generate_full" : "18", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "MOS2", "generate_full" : "19", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "MOS3", "generate_full" : "20", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "MOS4", "generate_full" : "21", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "IOEXT0", "generate_full" : "JSH_PORTV_OFFSET + 0", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "IOEXT1", "generate_full" : "JSH_PORTV_OFFSET + 1", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "IOEXT2", "generate_full" : "JSH_PORTV_OFFSET + 2", "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "variable", "name" : "IOEXT3", "generate_full" : "JSH_PORTV_OFFSET + 3", "return" : ["pin","A Pin"]
}*/


/*JSON{
  "type" : "init",
  "generate" : "jswrap_thingy_init"
}*/
void jswrap_thingy_init() {
  // Force the Thingy js module to be loaded to 'Thingy' global var
  jsvUnLock(jspEvaluate("global.Thingy=require(\"Thingy\");",true));
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_thingy_kill"
}*/
void jswrap_thingy_kill() {

}

