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
 * Platform Specific entry point for RP2350 - the Pico SDK crt0/runtime_init
 * has already set up XIP, .data/.bss and clocks before main() is called
 * ----------------------------------------------------------------------------
 */
#include "platform_config.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jswrapper.h"

int main(void) {
  jshInit();
  jswHWInit();

  bool buttonState = false;
#ifdef BTN1_PININDEX
  buttonState = jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE;
#endif

  jsvInit(JSVAR_CACHE_SIZE);
  jsiInit(!buttonState); // pressing button on boot skips autoload from flash

  while (1) {
    jsiLoop();
  }
  return 0;
}
