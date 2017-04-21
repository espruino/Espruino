/*
 * This file is designed to support PWM functions in Espruino for ESP32,
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

#define PWMMax 5 //maximum PWM channel for Analog output (freq = 50000)
#define PWMFreqMax 3 //maximum PWM channel with free frequency
#define PWMTimerBit 10 //10 bit for value
#define PWMTimerRange 1024

struct PWMChannel{Pin pin;}; //will be extended once we know more about PWM for Espruino on ESP32
struct PWMChannel PWMChannels[PWMMax];
struct PWMFreqChannel{Pin pin; int freq;};
struct PWMFreqChannel PWMFreqChannels[PWMFreqMax];

void PWMInit();
void writePWM(Pin pin, uint16_t value,int freq);

void setPWM(Pin pin,uint16_t value);
