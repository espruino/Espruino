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

#include "nrf_gpio.h"

#define MAG_PWR 18
#define MAG_INT 17
#define MAG_SDA 20
#define MAG_SCL 19
#define MAG3110_ADDR 0x0E

void wr(int pin, bool state) {
  if (state) nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
  else {  nrf_gpio_pin_clear(pin); nrf_gpio_cfg_output(pin); }
}

bool rd(int pin) {
  return nrf_gpio_pin_read(pin);
}

void dly() {
  jshDelayMicroseconds(10);
}

void err(const char *s) {
}

bool started = false;

void i2c_start() {
  if (started) {
    // reset
    wr(MAG_SDA, 1);
    dly();
    wr(MAG_SCL, 1);
    while (!rd(MAG_SCL)); // clock stretch
    dly();
  }
  if (rd(MAG_SDA)) err("Arbitration");
  wr(MAG_SDA, 0);
  dly();
  wr(MAG_SCL, 0);
  started = true;
}

void i2c_stop() {
  wr(MAG_SDA, 0);
  dly();
  wr(MAG_SCL, 1);
  while (!rd(MAG_SCL)); // clock stretch
  dly();
  wr(MAG_SDA, 1);
  dly();
  if (rd(MAG_SDA)) err("Arbitration");
  dly();
  started = false;
}

void i2c_wr_bit(bool b) {
  wr(MAG_SDA, b);
  dly();
  wr(MAG_SCL, 1);
  dly();
  while (!rd(MAG_SCL)); // clock stretch
  wr(MAG_SDA, 1); // stop forcing SDA (needed?)
  wr(MAG_SCL, 0);
}

bool i2c_rd_bit() {
  wr(MAG_SDA, 1); // stop forcing SDA
  dly();
  wr(MAG_SCL, 1); // stop forcing SDA
  while (!rd(MAG_SCL)); // clock stretch
  dly();
  bool b = rd(MAG_SDA);
  wr(MAG_SCL, 0);
  return b;
}

bool i2c_wr(uint8_t data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(data&128);
    data <<= 1;
  }
  return i2c_rd_bit(); // nack
}

uint8_t i2c_rd(bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | i2c_rd_bit();
  i2c_wr_bit(nack);
  return data;
}



/*JSON{
  "type" : "function",
  "name" : "mag",
  "generate" : "jswrap_puck_mag"
}
*/
void jswrap_puck_mag() {
  nrf_gpio_pin_set(MAG_PWR);
  nrf_gpio_cfg_input(MAG_INT, NRF_GPIO_PIN_NOPULL);
  wr(MAG_SDA, 1);
  wr(MAG_SCL, 1);
  jshDelayMicroseconds(20000); // 1.7ms from power on to ok
  jsiConsolePrintf("Init.\n");
  i2c_start();
  i2c_wr(MAG3110_ADDR); // CTRL_REG2, AUTO_MRST_EN
  i2c_wr(0x11);
  i2c_wr(0x80);
  i2c_stop();
  i2c_start();
  i2c_wr(MAG3110_ADDR); // CTRL_REG1, active mode 80 Hz ODR with OSR = 1
  i2c_wr(0x10);
  i2c_wr(0x01);
  i2c_stop();
  jsiConsolePrintf("Inited\n");
  while (!nrf_gpio_pin_read(MAG_INT));
  jsiConsolePrintf("Int.\n");
  i2c_start();
  i2c_wr(MAG3110_ADDR);
  i2c_wr(1);
  i2c_stop();
  i2c_start();
  i2c_wr(MAG3110_ADDR);
  i2c_wr(6);
  uint8_t d[6] = {
      i2c_wr(false),
      i2c_wr(false),
      i2c_wr(false),
      i2c_wr(false),
      i2c_wr(false),
      i2c_wr(false),
  };
  i2c_stop();
  jsiConsolePrintf("%d,%d,%d,%d,%d,%d\n",d[0],d[1],d[2],d[3],d[4],d[5],d[6]);
  // power off
  nrf_gpio_cfg_input(MAG_SDA, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(MAG_SCL, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(MAG_PWR, NRF_GPIO_PIN_NOPULL);
}


/*JSON{
  "type" : "kill",
  "generate" : "jswrap_puck_kill"
}*/
void jswrap_puck_kill() {
}
