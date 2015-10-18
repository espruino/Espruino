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
#include "platform_config.h"
#ifdef USB
 #if defined(STM32F1) || defined(STM32F3)
  #include "legacy_usb.h"
 #else
  #include "usb_device.h"
 #endif
#endif
#include "jsinteractive.h"
#include "jshardware.h"

extern void _VECTOR_TABLE;

void main_loop(void) {

  jshInit();
#ifdef USB
  MX_USB_DEVICE_Init();
#endif

#if 0
  bool buttonState = false;
#ifdef BTN1_PININDEX
  buttonState = jshPinInput(BTN1_PININDEX) == BTN1_ONSTATE;
#endif
  jsvInit();
  jsiInit(!buttonState); // pressing USER button skips autoload
#else
  jsvInit();
  jsiInit(true);
#endif

  while (1) {
    jsiLoop();
  }
  //jsiKill();
  //jsvKill();
  //jshKill();
}

