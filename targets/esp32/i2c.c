/*
 * This file is designed to support i2c functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Rhys Williams (wilberforce)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include "esp_log.h"
 
#include "i2c.h"
// Using arduino library
#include "esp32-hal-i2c.h"

// Let's get this working with only one device
i2c_t * i2c=NULL;

/** Set-up I2C master for ESP32, default pins are SCL:21, SDA:22. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  if (device != EV_I2C1) {
    jsError("Only I2C1 supported"); 
	return;
  }
  Pin scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 21;
  Pin sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 22;

  //jshPinSetState(scl, JSHPINSTATE_I2C);
  //jshPinSetState(sda, JSHPINSTATE_I2C);
   
  int num=1; // Master mode only 0 is slave mode..
 
  i2c_err_t err;
  i2c = i2cInit(num, 0, false);
  //jsError("jshI2CSetup: Frequency: %d", info->bitrate);
  err=i2cSetFrequency(i2c, (uint32_t)info->bitrate);
  if ( err != I2C_ERROR_OK ) {
    jsError( "jshI2CSetup: i2cSetFrequency error: %d", err);
	return;
  }
  err=i2cAttachSDA(i2c, pinToESP32Pin(sda));
  if ( err != I2C_ERROR_OK ) {
    jsError( "jshI2CSetup: i2cAttachSDA error: %d", err);
	return;
  }  
  err=i2cAttachSCL(i2c, pinToESP32Pin(scl));
  if ( err != I2C_ERROR_OK ) {
    jsError(  "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}

void jshI2CWrite(IOEventFlags device,
  unsigned char address,
  int nBytes,
  const unsigned char *data,
  bool sendStop) {
// i2cWrite(i2c_t * i2c, uint16_t address, bool addr_10bit, uint8_t * data, uint8_t len, bool sendStop);
  i2c_err_t err=i2cWrite(i2c,address,false,data,nBytes,sendStop);
  if ( err != I2C_ERROR_OK ) {
    jsError(  "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}

void jshI2CRead(IOEventFlags device,
  unsigned char address,
  int nBytes,
  unsigned char *data,
  bool sendStop) {
  i2c_err_t err=i2cRead(i2c,address,false,data,nBytes,sendStop);
  if ( err != I2C_ERROR_OK ) {
    jsError(  "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}
