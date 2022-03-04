/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Simple jshardware replacement functions
 * ----------------------------------------------------------------------------
 */


#include "platform_config.h"
#include "jspininfo.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

/// Because Nordic's library functions don't inline on NRF52840!
#ifdef NRF_P1
#define NRF_GPIO_PIN_SET_FAST(PIN) {if((PIN)<P0_PIN_NUM) NRF_P0->OUTSET=1<<(PIN); else NRF_P1->OUTSET=1<<((PIN)-P0_PIN_NUM);}
#define NRF_GPIO_PIN_CLEAR_FAST(PIN) {if((PIN)<P0_PIN_NUM) NRF_P0->OUTCLR=1<<(PIN); else NRF_P1->OUTCLR=1<<((PIN)-P0_PIN_NUM);}
#define NRF_GPIO_PIN_WRITE_FAST(PIN,V) {if((PIN)<P0_PIN_NUM) { if (V) NRF_P0->OUTSET=1<<(PIN); else NRF_P0->OUTCLR=1<<(PIN); } else { if (V) NRF_P1->OUTSET=1<<(PIN); else NRF_P1->OUTCLR=1<<(PIN); }}
#define NRF_GPIO_PIN_READ_FAST(PIN) (((PIN)<P0_PIN_NUM) ? (NRF_P0->IN >> (PIN))&1 : (NRF_P0->IN >> (PIN)) &1 )
#define NRF_GPIO_PIN_CNF(PIN,value) {((PIN<P0_PIN_NUM) ? NRF_P0 : NRF_P1)->PIN_CNF[PIN & 31]=value;}

#else
#define NRF_GPIO_PIN_SET_FAST(PIN) NRF_P0->OUTSET=1<<(PIN);
#define NRF_GPIO_PIN_CLEAR_FAST(PIN) NRF_P0->OUTCLR=1<<(PIN);
#define NRF_GPIO_PIN_WRITE_FAST(PIN,V) { if (V) NRF_P0->OUTSET=1<<(PIN); else NRF_P0->OUTCLR=1<<(PIN); }
#define NRF_GPIO_PIN_READ_FAST(PIN) ((NRF_P0->IN >> (PIN))&1)
#define NRF_GPIO_PIN_CNF(PIN,value) NRF_P0->PIN_CNF[PIN]=value;
#endif

// Using a macro means we hard-code values from pinInfo, and can ditch the pinInfo array
#define jshPinSetValue(PIN,value) \
  nrf_gpio_pin_write((uint32_t)pinInfo[PIN].pin, value ^ ((pinInfo[PIN].port&JSH_PIN_NEGATED)!=0))

#define jshPinGetValue(PIN) \
  (nrf_gpio_pin_read(pinInfo[PIN].pin) ^ ((pinInfo[PIN].port&JSH_PIN_NEGATED)!=0))

#define jshPinOutput(PIN,value) \
  if (pinInfo[PIN].port&JSH_PIN_NEGATED) \
    nrf_gpio_pin_write_output((uint32_t)pinInfo[PIN].pin, value==0); \
  else \
    nrf_gpio_pin_write_output((uint32_t)pinInfo[PIN].pin, value!=0);

#define jshDelayMicroseconds(US) \
  nrf_delay_us(US)

static void __attribute__((noinline)) nrf_gpio_pin_write_output(uint32_t pin, bool value)
{
  nrf_gpio_pin_write(pin, value);
  nrf_gpio_cfg_output(pin); //  TODO: could use high drive for these pins (default is std)
}

static void set_led_state(bool btn, bool progress)
{
#if defined(PIXLJS) || defined(BANGLEJS)
  // LED1 is backlight/HRM - don't use it!
#else
#if defined(LED2_PININDEX) && defined(LED3_PININDEX)
  jshPinOutput(LED3_PININDEX, progress);
  jshPinOutput(LED2_PININDEX, btn);
#elif defined(LED1_PININDEX)
  jshPinOutput(LED1_PININDEX, progress || btn);
#endif
#endif
}

#ifdef BTN1_PININDEX
static bool get_btn1_state() {
  return jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE;
}
#endif
#ifdef BTN2_PININDEX
static bool get_btn2_state() {
  return jshPinGetValue(BTN2_PININDEX)==BTN2_ONSTATE;
}
#endif

static void hardware_init(void) {
#if defined(PIXLJS)
  // LED1 is backlight - don't use it, but ensure it's off
  jshPinOutput(LED1_PININDEX, 0);
#endif
  set_led_state(false, false);
#ifdef BTN1_PININDEX
  bool polarity;
  uint32_t pin;
  if (pinInfo[BTN1_PININDEX].port&JSH_PIN_NEGATED)
    polarity = BTN1_ONSTATE!=1;
  else
    polarity = BTN1_ONSTATE==1;
  pin = pinInfo[BTN1_PININDEX].pin;
  nrf_gpio_cfg_input(pin,
          polarity ? NRF_GPIO_PIN_PULLDOWN : NRF_GPIO_PIN_PULLUP);
#endif
#ifdef BTN2_PININDEX
  if (pinInfo[BTN2_PININDEX].port&JSH_PIN_NEGATED)
    polarity = BTN2_ONSTATE!=1;
  else
    polarity = BTN2_ONSTATE==1;
  pin = pinInfo[BTN2_PININDEX].pin;
  nrf_gpio_cfg_input(pin,
          polarity ? NRF_GPIO_PIN_PULLDOWN : NRF_GPIO_PIN_PULLUP);
#endif
#ifdef VIBRATE_PIN
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
#endif
}
