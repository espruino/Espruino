/*
 * This file is designed to support Timer functions in Espruino,
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
#include <stdio.h>

#include "esp_log.h"
 
#include "esp_attr.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "jshardwareTimer.h"
#include "rtosutil.h"
#include "soc/timer_group_struct.h"
#include "driver/timer.h"
#include "jstimer.h"

void startTimer(int timer_idx,uint64_t duration){
  timer_Start(timer_idx,duration);
}

void disableTimer(int timer_idx){
  timer_disable_intr(TIMER_GROUP_0, timer_idx);
}

void rescheduleTimer(int timer_idx,uint64_t duration){
  timer_Reschedule(timer_idx,duration);
}
