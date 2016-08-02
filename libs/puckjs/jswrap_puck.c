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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Puck.js
 * ----------------------------------------------------------------------------
 */


#include "jswrap_puck.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jshardware.h"
#include "jstimer.h"

#include "nrf_gpio.h"

#define MAG_PWR 18
#define MAG_INT 17
#define MAG_SDA 20
#define MAG_SCL 19
#define MAG3110_ADDR 0x0E
#define I2C_TIMEOUT 100000

void wr(int pin, bool state) {
  if (state) {
    nrf_gpio_pin_set(pin); nrf_gpio_cfg_output(pin);
    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
  } else {
    nrf_gpio_pin_clear(pin);
    nrf_gpio_cfg_output(pin);
  }
}

bool rd(int pin) {
  return nrf_gpio_pin_read(pin);
}

void dly() {
  volatile int i;
  for (i=0;i<10;i++);
}

void err(const char *s) {
  jsiConsolePrintf("I2C: %s\n", s);
}

bool started = false;

void i2c_start() {
  if (started) {
    // reset
    wr(MAG_SDA, 1);
    dly();
    wr(MAG_SCL, 1);
    int timeout = I2C_TIMEOUT;
    while (!rd(MAG_SCL) && --timeout); // clock stretch
    if (!timeout) err("Timeout (start)");
    dly();
  }
  if (!rd(MAG_SDA)) err("Arbitration (start)");
  wr(MAG_SDA, 0);
  dly();
  wr(MAG_SCL, 0);
  dly();
  started = true;
}

void i2c_stop() {
  wr(MAG_SDA, 0);
  dly();
  wr(MAG_SCL, 1);
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (stop)");
  dly();
  wr(MAG_SDA, 1);
  dly();
  if (!rd(MAG_SDA)) err("Arbitration (stop)");
  dly();
  started = false;
}

void i2c_wr_bit(bool b) {
  wr(MAG_SDA, b);
  dly();
  wr(MAG_SCL, 1);
  dly();
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (wr)");
  wr(MAG_SCL, 0);
  wr(MAG_SDA, 1); // stop forcing SDA (needed?)
}

bool i2c_rd_bit() {
  wr(MAG_SDA, 1); // stop forcing SDA
  dly();
  wr(MAG_SCL, 1); // stop forcing SDA
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (rd)");
  dly();
  bool b = rd(MAG_SDA);
  wr(MAG_SCL, 0);
  return b;
}

// true on ack, false on nack
bool i2c_wr(uint8_t data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(data&128);
    data <<= 1;
  }
  return !i2c_rd_bit();
}

uint8_t i2c_rd(bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | (i2c_rd_bit()?1:0);
  i2c_wr_bit(nack);
  return data;
}


void mag_on() {
  nrf_gpio_pin_set(MAG_PWR);
  nrf_gpio_cfg_output(MAG_PWR);
  nrf_gpio_cfg_input(MAG_INT, NRF_GPIO_PIN_NOPULL);
  wr(MAG_SDA, 1);
  wr(MAG_SCL, 1);
  jshDelayMicroseconds(2000); // 1.7ms from power on to ok
  i2c_start();
  i2c_wr(MAG3110_ADDR<<1); // CTRL_REG2, AUTO_MRST_EN
  if (!i2c_wr(0x11));
  if (!i2c_wr(0x80));
  i2c_stop();
  i2c_start();
  i2c_wr(MAG3110_ADDR<<1); // CTRL_REG1, active mode 80 Hz ODR with OSR = 1
  i2c_wr(0x10);
  i2c_wr(0x01);
  i2c_stop();
}

void mag_read(int16_t d[3]) {
  int timeout = I2C_TIMEOUT*2;
  while (!nrf_gpio_pin_read(MAG_INT) && --timeout);
  if (!timeout) err("Timeout (wait reading)");
  i2c_start();
  i2c_wr(MAG3110_ADDR<<1);
  i2c_wr(1);
  i2c_start();
  i2c_wr(1|(MAG3110_ADDR<<1));
  d[0] = i2c_rd(false)<<8;
  d[0] |= i2c_rd(false);
  d[1] = i2c_rd(false)<<8;
  d[1] |= i2c_rd(false);
  d[2] = i2c_rd(false)<<8;
  d[2] |= i2c_rd(true);
  i2c_stop();
}

void mag_off() {
  nrf_gpio_cfg_input(MAG_SDA, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(MAG_SCL, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(MAG_PWR, NRF_GPIO_PIN_NOPULL);
}

/*JSON{
  "type" : "function",
  "name" : "mag",
  "generate" : "jswrap_puck_mag",
  "return" : ["JsVar", "An Object `{x,y,z}` of magnetometer readings" ]
}
Turn on the magnetometer, take a single reading, and then turn it off again.
*/
JsVar *jswrap_puck_mag() {
  mag_on();
  int16_t d[3];
  mag_read(d);
  mag_off();
  JsVar *obj = jsvNewObject();
  if (obj) {
    jsvObjectSetChildAndUnLock(obj,"x",jsvNewFromInteger(d[0]));
    jsvObjectSetChildAndUnLock(obj,"y",jsvNewFromInteger(d[1]));
    jsvObjectSetChildAndUnLock(obj,"z",jsvNewFromInteger(d[2]));
  }
  return obj;
}

// Called when we're done with the IR transmission
void _jswrap_puck_IR_done(JsSysTime t) {
  // set as input - so no signal
  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_IN);
  // this one also stops the PWM
  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_IN);
}

/*JSON{
  "type" : "function",
  "name" : "IR",
  "generate" : "jswrap_puck_IR",
  "params" : [
      ["data","JsVar","An array of pulse lengths, in milliseconds"]
  ]
}
Transmit the given set of IR pulses - data should be an array of pulse times
in milliseconds (as `[on, off, on, off, on, etc]`).
*/
void jswrap_puck_IR(JsVar *data) {
  if (!jsvIsIterable(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting an array, got %t", data);
    return;
  }

  jshPinSetValue(IR_ANODE_PIN, 1);
  Pin pin = IR_ANODE_PIN;
  jshPinAnalogOutput(IR_CATHODE_PIN, 0.5, 38000, 0);

  JsSysTime time = jshGetSystemTime();
  bool hasPulses = false;
  bool pulsePolarity = true;

  JsvIterator it;
  jsvIteratorNew(&it, data);
  while (jsvIteratorHasElement(&it)) {
    JsVarInt pulseTime = jsvIteratorGetIntegerValue(&it);
    if (hasPulses) jstPinOutputAtTime(time, &pin, 1, pulsePolarity);
    else jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_OUT);
    hasPulses = true;
    time += jshGetTimeFromMilliseconds(pulseTime);
    pulsePolarity = !pulsePolarity;
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  if (hasPulses) {
    jstExecuteFn(_jswrap_puck_IR_done, time, 0);
  }
}


/*JSON{
  "type" : "kill",
  "generate" : "jswrap_puck_kill"
}*/
void jswrap_puck_kill() {
}
