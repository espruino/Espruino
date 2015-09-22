/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/



#ifndef N5SK_LED_H_
#define N5SK_LED_H_

#include "stdint.h"


////////////////////////////////////////////////////////////////////////////////
// Module Description
////////////////////////////////////////////////////////////////////////////////

/*
 * This module is responsible for controlling the N5 Starter Kit IO board LEDs.
 * Includes methods to initialise and control LEDs.
 */


////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void
n5_io_init_leds(void);

void
n5_io_clear_leds(void);

void
n5_io_set_leds(void);

void
n5_io_turn_off_led(uint8_t led_num);

void
n5_io_turn_on_led(uint8_t led_num);

void
n5_io_toggle_led(uint8_t led_num);

#endif /* N5SK_LED_H_ */
