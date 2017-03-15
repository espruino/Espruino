/*
 * This file is designed to support i2c functions in Espruino for ESP32,
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

#include "jspininfo.h"
#include "jshardware.h"
#include "driver/gpio.h"

// Convert an Espruino pin to an ESP32 pin number.
gpio_num_t pinToESP32Pin(Pin pin);

void jshI2CSetup(IOEventFlags device, JshI2CInfo *info);
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop);
void jshI2CRead(IOEventFlags device,  unsigned char address, int nBytes, unsigned char *data, bool sendStop);
