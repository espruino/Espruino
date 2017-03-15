/*
 * This file is designed to support spi functions in Espruino for ESP32,
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
#include "driver/spi_master.h"

// Convert an Espruino pin to an ESP32 pin number.
gpio_num_t pinToESP32Pin(Pin pin);

#define SPIMax 2
struct SPIChannel{
  spi_device_handle_t spi;
  bool spi_read;
  uint32_t g_lastSPIRead;
  spi_host_device_t HOST;
};
struct SPIChannel SPIChannels[SPIMax];
void SPIChannelsInit();
void SPIReset();

void jshSPISetup( IOEventFlags device, JshSPIInfo *inf );

int jshSPISend( IOEventFlags device, int data );
void jshSPISend16( IOEventFlags device, int data );
void jshSPISet16( IOEventFlags device, bool is16 );
void jshSPIWait( IOEventFlags device );
void jshSPISetReceive(IOEventFlags device, bool isReceive);
