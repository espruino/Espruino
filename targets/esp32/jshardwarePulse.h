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

#ifndef JSHARDWAREPULSE_H_
#define JSHARDWAREPULSE_H_

#include "jspininfo.h"
#include <stdbool.h>

#define RMTChannelMax 8  // maximum RMT channels available

typedef struct {
    Pin pin;
#if ESP_IDF_VERSION_MAJOR >= 5
    void *rmt_handle;   // rmt_channel_handle_t in IDF5, stored as void* for compatibility
#endif
} RMTChannel;

extern RMTChannel RMTChannels[RMTChannelMax];

void RMTInit();
void RMTReset();

void sendPulse(
    Pin pin,
    bool pulsePolarity,
    int duration
);

#endif /* JSHARDWAREPULSE_H_ */