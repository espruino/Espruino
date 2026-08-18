/*
 * This file is designed to support UART functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Juergen Marsch
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include "jshardware.h"

void initConsole();
void initSerial(IOEventFlags device,JshUSARTInfo *inf);
void uninitSerial(IOEventFlags device);
void pollSerialDevices();

#if defined(ESPR_USE_USB_SERIAL_JTAG) && ESP_IDF_VERSION_MAJOR < 5
// not in IDF 4, make our own
bool usb_serial_jtag_is_driver_installed();
bool usb_serial_jtag_is_connected();
#endif