/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

#ifndef N5STARTERKIT_H
#define N5STARTERKIT_H

#include "nrf_gpio.h"

//IO board active low leds
#define LED_A        8  //LED A on N5 Starter Kit IO Board
#define LED_B        3  //LED B on N5 Starter Kit IO Board
#define LED_C        15 //LED C on N5 Starter Kit IO Board
#define LED_D        30 //LED D on N5 Starter Kit IO Board

//IO board pull-up buttons
#define BUTTON_A     23 //BUTTON A on N5 Starter Kit IO Board
#define BUTTON_B     2  //BUTTON B on N5 Starter Kit IO Board
#define BUTTON_C     12 //BUTTON C on N5 Starter Kit IO Board
#define BUTTON_D     11 //BUTTON D on N5 Starter Kit IO Board
#define BUTTON_PULL  NRF_GPIO_PIN_PULLUP

//Battery board pull-up switches
#define SWITCH_1     5  // Switch 1 on N5 Starter Kit Battery Board
#define SWITCH_2     0  // Switch 2 on N5 Starter Kit Battery Board
#define SWITCH_3     6  // Switch 3 on N5 Starter Kit Battery Board
#define SWITCH_4     24 // Switch 4 on N5 Starter Kit Battery Board
#define SWITCH_5     9  // Switch 5 on N5 Starter Kit Battery Board
#define SWITCH_PULL  NRF_GPIO_PIN_PULLUP

#endif
