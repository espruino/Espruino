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

#define uart_console 0
#define uart_Serial2 1
#define uart_Serial3 2

void initConsole();
void initSerial(IOEventFlags device,JshUSARTInfo *inf);
void writeSerial(IOEventFlags device,uint8_t c); 
void consoleToEspruino();
void serialToEspruino();
