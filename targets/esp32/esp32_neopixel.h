/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains neopixel (WS2812B) specific function definitions.
 * ----------------------------------------------------------------------------
 */
#ifndef ESP32_NEOPIXEL_H_
#define ESP32_NEOPIXEL_H_
#include "jspin.h"

//===== neopixel Library

bool esp32_neopixelWrite(Pin pin,unsigned char *rgbData, size_t rgbSize);

#endif /* ESP32_NEOPIXEL_H_ */