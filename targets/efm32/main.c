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
 * Platform Specific entry point
 * ----------------------------------------------------------------------------
 */
/* APM comment
#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"
*/
#include "em_gpio.h"
#include "em_cmu.h"

int main() {

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 1);
  GPIO_PinOutSet(gpioPortE, 2);
  GPIO_PinOutClear(gpioPortE, 3);
  while(1);
  /* APM comment
  jshInit();
  GPIO_PinOutSet(gpioPortE, 3);
  jsvInit();
  GPIO_PinOutClear(gpioPortE, 2);
  jsiInit(true); // load from flash by default
  GPIO_PinOutClear(gpioPortE, 3);
  while (1) 
  {
    jsiLoop();
  }
*/
  //jsiKill();
  //jsvKill();
  //jshKill();
}
