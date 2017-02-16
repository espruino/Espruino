/*
 * This file is designed to support Pulse functions in Espruino for ESP32,
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

#include "jspininfo.h"

#define RMTChannelMax 8 //maximum RMT channel

struct RMTChannel{Pin pin;}; //will be extended once we know more about RMT functions for Espruino on ESP32
struct RMTChannel RMTChannels[RMTChannelMax];

void RMTInit();
void sendPulse(Pin pin,              //!< The pin to be pulsed.
    bool pulsePolarity,   //!< The value to be pulsed into the pin.
    int duration  //!< The duration in microseconds to hold the pin.
);
