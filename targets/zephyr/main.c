/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrapper.h"


int main(void)
{
	int ret;

  jshInit();
  jswHWInit();
  bool buttonState = false;
//  buttonState = jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE;
  jsvInit(JSVAR_CACHE_SIZE);
  jsiInit(!buttonState /* load from flash by default */); // pressing USER button skips autoload
  jsiOneSecondAfterStartup();

	while (1) {
    jsiLoop();
    // TODO: sleep? k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
