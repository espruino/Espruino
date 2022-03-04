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
#include "jsinteractive.h"
#include "jshardware.h"
#include "nrf5x_utils.h"
#include "jswrapper.h"

int main() {

  // Ensure UICR flags are set correctly for the current device
  // Will cause a reboot if flags needed to be reset
  nrf_configure_uicr_flags();

  jshInit();
  jswHWInit();

  bool buttonState = false;
#ifdef BTN1_PININDEX
  buttonState = jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE;
#endif
  jsvInit(0);
  jsiInit(!buttonState /* load from flash by default */); // pressing USER button skips autoload

  while (1) 
  {
    jsiLoop();
  }
  //jsiKill();
  //jsvKill();
  //jshKill();
}

#ifdef LD_NOSTARTFILES
void _start(){
  main();
}
#endif
