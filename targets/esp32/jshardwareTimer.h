/*
 * This file is designed to support Timer functions in Espruino for ESP32,
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

void initTimer();

void startTimer(int timer_idx,uint64_t duration);

void disableTimer(int timer_idx);

void rescheduleTimer(int timer_idx,uint64_t duration);

