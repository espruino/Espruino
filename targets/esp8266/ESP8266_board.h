/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific function definitions.
 * ----------------------------------------------------------------------------
 */
#ifndef TARGETS_ESP8266_ESP8266_BOARD_H_
#define TARGETS_ESP8266_ESP8266_BOARD_H_
#include <user_interface.h>

// Define the task ids for the APP event handler
#define TASK_APP_MAINLOOP ((os_signal_t)1)
#define TASK_APP_RX_DATA ((os_signal_t)2)

// Task priority for main loop
#define TASK_APP_QUEUE USER_TASK_PRIO_0

#endif /* TARGETS_ESP8266_ESP8266_BOARD_H_ */
