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
 * Software Serial
 * ----------------------------------------------------------------------------
 */

#include "jshardware.h"

typedef JshUSARTInfo serial_sender_data; // the larger of JshSPIInfo or IOEventFlags
typedef void (*serial_sender)(unsigned char data, serial_sender_data *info);

bool jsserialPopulateSerialInfo(JshUSARTInfo *inf, JsVar *baud,  JsVar *options);

// Get the correct Serial send function (and the data to send to it)
bool jsserialGetSendFunction(JsVar *serialDevice, serial_sender *serialSend, serial_sender_data *serialSendData) ;

void jsserialEventCallbackInit(JshUSARTInfo *inf);
void jsserialEventCallbackIdle();

// This is used with jshSetEventCallback to allow Serial data to be received in software
void jsserialEventCallback(bool state);
