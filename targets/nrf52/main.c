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

// For debugging with LEDS.
#include "nrf_gpio.h"
#define LED_1 17
#define LED_2 18
#define LED_3 19
#define LED_4 20

// Clearing an LED actually lights it for the NRF52 platform and setting the LED turns it off.
void debug_all_leds_on(void) {
  nrf_gpio_cfg_output(LED_1);
  nrf_gpio_pin_clear(LED_1);
  nrf_gpio_cfg_output(LED_2);
  nrf_gpio_pin_clear(LED_2);
  nrf_gpio_cfg_output(LED_3);
  nrf_gpio_pin_clear(LED_3);
  nrf_gpio_cfg_output(LED_4);
  nrf_gpio_pin_clear(LED_4);
}

void debug_all_leds_off(void) {
  nrf_gpio_pin_set(LED_1);
  nrf_gpio_pin_set(LED_2);
  nrf_gpio_pin_set(LED_3);
  nrf_gpio_pin_set(LED_4);
}

int main() {
  jshInit();
  jsvInit();
  jsiInit(false);
  debug_all_leds_on();

  while (1) 
  {
    jsiLoop();
  }
  //jsiKill();
  //jsvKill();
  //jshKill();
}