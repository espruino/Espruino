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
#include "jsdevices.h"
#include "jspin.h"
#include "jstimer.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf5x_utils.h"
#include "ble_gap.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "jsi2c.h" // accelerometer/etc
#include "bluetooth.h"
#include "app_timer.h"

#define MAG3110_ADDR 0x0E
#define LIS3MDL_ADDR 0x1E // MAG_ADDR
#define MMC5603NJ_ADDR 0x30
#define I2C_TIMEOUT 100000

JshI2CInfo i2cMag;
JshI2CInfo i2cAccel;
JshI2CInfo i2cTemp;
PuckVersion puckVersion;
APP_TIMER_DEF(m_poll_timer_id);

const Pin PUCK_IO_PINS[] = {1,2,4,6,7,8,23,24,28,29,30,31};
#define IR_INPUT_PIN 25 // Puck v2
#define IR_FET_PIN 27 // Puck v2
#define FET_PIN 26 // Puck v2

bool mag_enabled = false; //< Has the magnetometer been turned on?
int16_t mag_reading[3];  //< magnetometer xyz reading
//int mag_zero[3]; //< magnetometer 'zero' reading, only for Puck 2.1 right now
volatile bool mag_data_ready = false;

bool accel_enabled = false; //< Has the accelerometer been turned on?
int16_t accel_reading[3];
int16_t gyro_reading[3];

bool mag_wait();
void mag_read();

int getMagAddr() {
  switch (puckVersion) {
    case PUCKJS_1V0: return MAG3110_ADDR;
    case PUCKJS_2V0: return LIS3MDL_ADDR;
    case PUCKJS_2V1: return MMC5603NJ_ADDR;
    default: return 0;
  }
}

JsVar *jswrap_puck_getHardwareVersion() {
  switch (puckVersion) {
    case PUCKJS_1V0: return jsvNewFromInteger(1);
    case PUCKJS_2V0: return jsvNewFromInteger(2);
    case PUCKJS_2V1: return jsvNewFromFloat(2.1);
    default: return NULL;
  }
}

JsVar *to_xyz(int16_t d[3], double scale) {
  JsVar *obj = jsvNewObject();
  if (!obj) return 0;
  jsvObjectSetChildAndUnLock(obj,"x",jsvNewFromFloat(d[0]*scale));
  jsvObjectSetChildAndUnLock(obj,"y",jsvNewFromFloat(d[1]*scale));
  jsvObjectSetChildAndUnLock(obj,"z",jsvNewFromFloat(d[2]*scale));
  return obj;
}

/// MAG3110 I2C implementation - write pin
void wr(int pin, bool state) {
  if (state) {
    nrf_gpio_pin_set(pin); nrf_gpio_cfg_output(pin);
    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
  } else {
    nrf_gpio_pin_clear(pin);
    nrf_gpio_cfg_output(pin);
  }
}

/// MAG3110 I2C implementation - read pin
bool rd(int pin) {
  return nrf_gpio_pin_read(pin);
}

/// MAG3110 I2C implementation - delay
void dly() {
  volatile int i;
  for (i=0;i<10;i++);
}

/// MAG3110 I2C implementation - show error
void err(const char *s) {
  jsiConsolePrintf("I2C: %s\n", s);
}

/// MAG3110 I2C implementation has i2c started?
bool started = false;
/// MAG3110 I2C implementation - start bit
void i2c_start() {
  if (started) {
    // reset
    wr(MAG_PIN_SDA, 1);
    dly();
    wr(MAG_PIN_SCL, 1);
    int timeout = I2C_TIMEOUT;
    while (!rd(MAG_PIN_SCL) && --timeout); // clock stretch
    if (!timeout) err("Timeout (start)");
    dly();
  }
  if (!rd(MAG_PIN_SDA)) err("Arbitration (start)");
  wr(MAG_PIN_SDA, 0);
  dly();
  wr(MAG_PIN_SCL, 0);
  dly();
  started = true;
}
/// MAG3110 I2C implementation - stop bit
void i2c_stop() {
  wr(MAG_PIN_SDA, 0);
  dly();
  wr(MAG_PIN_SCL, 1);
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_PIN_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (stop)");
  dly();
  wr(MAG_PIN_SDA, 1);
  dly();
  if (!rd(MAG_PIN_SDA)) err("Arbitration (stop)");
  dly();
  started = false;
}
/// MAG3110 I2C implementation - write bit
void i2c_wr_bit(bool b) {
  wr(MAG_PIN_SDA, b);
  dly();
  wr(MAG_PIN_SCL, 1);
  dly();
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_PIN_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (wr)");
  wr(MAG_PIN_SCL, 0);
  wr(MAG_PIN_SDA, 1); // stop forcing SDA (needed?)
}
/// MAG3110 I2C implementation - read bit
bool i2c_rd_bit() {
  wr(MAG_PIN_SDA, 1); // stop forcing SDA
  dly();
  wr(MAG_PIN_SCL, 1); // stop forcing SDA
  int timeout = I2C_TIMEOUT;
  while (!rd(MAG_PIN_SCL) && --timeout); // clock stretch
  if (!timeout) err("Timeout (rd)");
  dly();
  bool b = rd(MAG_PIN_SDA);
  wr(MAG_PIN_SCL, 0);
  return b;
}
/// MAG3110 I2C implementation - write byte, true on ack, false on nack
bool i2c_wr(uint8_t data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(data&128);
    data <<= 1;
  }
  return !i2c_rd_bit();
}
/// MAG3110 I2C implementation - read byte
uint8_t i2c_rd(bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | (i2c_rd_bit()?1:0);
  i2c_wr_bit(nack);
  return data;
}
/// Write to I2C register on magnetometer
void mag_wr(int addr, int data) {
  int iaddr = getMagAddr();
  if (puckVersion == PUCKJS_1V0) { // MAG3110
    i2c_start();
    i2c_wr(iaddr<<1);
    i2c_wr(addr);
    i2c_wr(data);
    i2c_stop();
    wr(MAG_PIN_SDA, 1);
    wr(MAG_PIN_SCL, 1);
  } else {
    unsigned char buf[2];
    buf[0] = addr;
    buf[1] = data;
    jsi2cWrite(&i2cMag, iaddr, 2, buf, true);
  }
}
/// Read from I2C register on magnetometer
void mag_rd(int addr, unsigned char *data, int cnt) {
  int iaddr = getMagAddr();
  if (puckVersion == PUCKJS_1V0) { // MAG3110
    i2c_start();
    i2c_wr(iaddr<<1);
    i2c_wr(addr);
    i2c_start();
    i2c_wr(1|(iaddr<<1));
    for (int i=0;i<cnt;i++) {
      data[i] = i2c_rd(i==(cnt-1));
    }
    i2c_stop();
    wr(MAG_PIN_SDA, 1);
    wr(MAG_PIN_SCL, 1);
  } else {
    unsigned char buf[1];
    buf[0] = addr;
    jsi2cWrite(&i2cMag, iaddr, 1, buf, false);
    jsi2cRead(&i2cMag, iaddr, cnt, data, true);
  }
}

void mag_pin_on() {
  jshPinSetValue(MAG_PIN_PWR, 1);
  jshPinSetValue(MAG_PIN_SCL, 1);
  jshPinSetValue(MAG_PIN_SDA, 1);
  jshPinSetState(MAG_PIN_SCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetState(MAG_PIN_SDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetState(MAG_PIN_PWR, JSHPINSTATE_GPIO_OUT);
  if (puckVersion == PUCKJS_1V0) {
    // IRQ line on Puck.js 1v0 is often 0 - don't pull up
    jshPinSetState(MAG_PIN_INT, JSHPINSTATE_GPIO_IN);
  } else if (puckVersion == PUCKJS_2V0) {
    // using DRDY for data
    jshPinSetState(MAG_PIN_DRDY, JSHPINSTATE_GPIO_IN);
  }
}

// Turn magnetometer on and configure. If instant=true we take a reading right away
bool mag_on(int milliHz, bool instant) {
  //jsiConsolePrintf("mag_on\n");
  mag_pin_on();
  if (puckVersion == PUCKJS_1V0) { // MAG3110
    jshDelayMicroseconds(2500); // 1.7ms from power on to ok
    if (instant) milliHz = 80000;
    int reg1 = 0;
    if (milliHz == 80000) reg1 |= (0x00)<<3; // 900uA
    else if (milliHz == 40000) reg1 |= (0x04)<<3; // 550uA
    else if (milliHz == 20000) reg1 |= (0x08)<<3; // 275uA
    else if (milliHz == 10000) reg1 |= (0x0C)<<3; // 137uA
    else if (milliHz == 5000) reg1 |= (0x10)<<3; // 69uA
    else if (milliHz == 2500) reg1 |= (0x14)<<3; // 34uA
    else if (milliHz == 1250) reg1 |= (0x18)<<3; // 17uA
    else if (milliHz == 630) reg1 |= (0x1C)<<3; // 8uA
    else if (milliHz == 310) reg1 |= (0x1D)<<3; // 8uA
    else if (milliHz == 160) reg1 |= (0x1E)<<3; // 8uA
    else if (milliHz == 80) reg1 |= (0x1F)<<3; // 8uA
    else return false;

    jshDelayMicroseconds(2000); // 1.7ms from power on to ok
    mag_wr(0x11, 0x80/*AUTO_MRST_EN*/ + 0x20/*RAW*/); // CTRL_REG2
    mag_wr(0x10, reg1 | 1); // CTRL_REG1, samplerate + active
  } else if (puckVersion == PUCKJS_2V0) { // LIS3MDL
    jshDelayMicroseconds(10000); // takes ages to start up
    if (instant) milliHz = 80000;
    bool lowPower = false;
    int reg1 = 0x80; // temp sensor, low power
    if (milliHz == 80000) reg1 |= 7<<2; // 900uA
    else if (milliHz == 40000) reg1 |= 6<<2; // 550uA
    else if (milliHz == 20000) reg1 |= 5<<2; // 275uA
    else if (milliHz == 10000) reg1 |= 4<<2; // 137uA
    else if (milliHz == 5000) reg1 |= 3<<2; // 69uA
    else if (milliHz == 2500) reg1 |= 2<<2; // 34uA
    else if (milliHz == 1250) reg1 |= 1<<2; // 17uA
    else if (milliHz <= 630) { /*if (milliHz == 630 || milliHz == 625)*/
      // We just go for the lowest power mode
      reg1 |= 0<<2; // 8uA
      lowPower = true;
    }
    else return false;

    mag_wr(0x21, 0x00); // CTRL_REG2 - full scale +-4 gauss
    mag_wr(0x20, reg1); // CTRL_REG1
    mag_wr(0x23, 0x02); // CTRL_REG4 - low power, LSb at higher address (to match MAG3110)
    mag_wr(0x22, lowPower ? 0x20 : 0x00); // CTRL_REG3 - normal or low power, continuous measurement
    mag_wr(0x24, 0x40); // CTRL_REG5 - block data update
 } else if (puckVersion == PUCKJS_2V1) { // MMC5603NJ
    jshDelayMicroseconds(20000); // wait for boot
    int hz = (milliHz+500) / 1000;
    if (hz<1) hz=1;
    bool highPower = false; // for 1kHz
    if (instant || hz>255) {
      hz=255; // max speed in default mode
      highPower = true;
    }

/*    mag_zero[0]=0;
    mag_zero[1]=0;
    mag_zero[2]=0;
    int mag_min[3];
    int mag_max[3];
    mag_wr(0x1B, 0b00001001); // IC0 - SET - plus single measurement
    mag_wait();
    mag_read();
    memcpy(mag_min, mag_reading, sizeof(mag_reading));
    mag_wr(0x1B, 0b00010001); // IC0 - RESET - plus single measurement
    mag_wait();
    mag_read();
    memcpy(mag_max, mag_reading, sizeof(mag_reading));*/


    mag_wr(0x1A, hz); // ODR - hz
    mag_wr(0x1C, 3); // IC1 - 1.2ms (fastest sample time)
    mag_wr(0x1B, 0b10100000); // IC0 - auto SR, continuous mode enable
    mag_wr(0x1D,  0b00011001| (highPower?0x80:0)); // IC2 - periodic set every 25, continuous mode enable

/*    mag_zero[0] = (mag_min[0]+mag_max[0])/2;
    mag_zero[1] = (mag_min[1]+mag_max[1])/2;
    mag_zero[2] = (mag_min[2]+mag_max[2])/2;
    jsiConsolePrintf("min %d,%d,%d\n", mag_min[0], mag_min[1], mag_min[2]);
    jsiConsolePrintf("max %d,%d,%d\n", mag_max[0], mag_max[1], mag_max[2]);
    jsiConsolePrintf("zer %d,%d,%d\n", mag_zero[0], mag_zero[1], mag_zero[2]);
    mag_reading[0]=0;
    mag_reading[1]=0;
    mag_reading[2]=0;*/

    // read data anyway, to ensure data ready status bit it cleared
    mag_read();

    if (!instant) // if we're instant, don't start a timer as we just want to read in the main thread
      app_timer_start(m_poll_timer_id, APP_TIMER_TICKS(1000000 / milliHz, APP_TIMER_PRESCALER), NULL);
  } else {
    jsWarn("Unknown Puck version!");
    return false;
  }

  if (instant) {
    mag_wait();
    mag_read();
  }

  return true;
}

// Wait for magnetometer IRQ line to be set
bool mag_wait() {
  int timeout = 0;
  unsigned char buf[1];

  if (puckVersion == PUCKJS_1V0) { // MAG3110
    timeout = I2C_TIMEOUT*2;
    while (!nrf_gpio_pin_read(MAG_PIN_INT) && --timeout);
  } else if (puckVersion == PUCKJS_2V0) { // LIS3MDL
    timeout = 400;
    do {
      mag_rd(0x27, buf, 1); // STATUS_REG
    } while (!(buf[0]&0x08) && --timeout); // ZYXDA
  } else if (puckVersion == PUCKJS_2V1) { // MMC5603NJ
    timeout = 400;
    do {
      mag_rd(0x18, buf, 1); // Status
    } while ((!(buf[0]&0x40)) && --timeout); // check for Meas_m_done
  }

  if (!timeout) {
    jsExceptionHere(JSET_INTERNALERROR, "Timeout (Magnetometer)");
    return false;
  }
  return true;
}



// Read a value
void mag_read() {
  unsigned char buf[9];
  if (puckVersion == PUCKJS_1V0) { // MAG3110
    mag_rd(0x01, buf, 6); // OUT_X_MSB
    mag_reading[0] = (int16_t)((buf[0]<<8) | buf[1]);
    mag_reading[1] = (int16_t)((buf[2]<<8) | buf[3]);
    mag_reading[2] = (int16_t)((buf[4]<<8) | buf[5]);
  } else if (puckVersion == PUCKJS_2V0) { // LIS3MDL
    mag_rd(0x28, buf, 6);
    mag_reading[0] = (int16_t)((buf[0]<<8) | buf[1]);
    mag_reading[1] = (int16_t)((buf[2]<<8) | buf[3]);
    mag_reading[2] = (int16_t)((buf[4]<<8) | buf[5]);
  } else if (puckVersion == PUCKJS_2V1) { // MMC5603NJ
    /*mag_rd(0x00, buf, 9);
    mag_reading[0] = ((buf[0]<<12) | (buf[1]<<4) | (buf[6]>>4)) - (1<<19);
    mag_reading[1] = ((buf[2]<<12) | (buf[3]<<4) | (buf[7]>>4)) - (1<<19);
    mag_reading[2] = ((buf[4]<<12) | (buf[5]<<4) | (buf[8]>>4)) - (1<<19);*/
    mag_rd(0x00, buf, 6);
    mag_reading[0] = ((buf[0]<<8) | buf[1]) - 32768;
    mag_reading[1] = ((buf[2]<<8) | buf[3]) - 32768;
    mag_reading[2] = ((buf[4]<<8) | buf[5]) - 32768;
  }
}

// Get temperature, shifted left 8 bits
int mag_read_temp() {
  unsigned char buf[2];
  if (puckVersion == PUCKJS_1V0) { // MAG3110
    mag_rd(0x0F, buf, 1); // DIE_TEMP
    return ((int8_t)buf[0])<<8;
  } else if (puckVersion == PUCKJS_2V0) { // LIS3MDL
    mag_rd(0x2E, buf, 2); // TEMP_OUT_L
    int16_t t = buf[0] | (buf[1]<<8);
    // 20 degree offset based on tests here
    return (int)(t >> 3) + (20 << 8);
  } else if (puckVersion == PUCKJS_2V1) { // MMC5603NJ
    mag_wr(0x1B, 0x02); // Take temperature measurement
    // Wait for completion
    int timeout = 100;
    do {
      mag_rd(0x18, buf, 1); // Status
    } while (!(buf[0]&0x80) && --timeout); // check for Meas_t_done
    // get measurement
    mag_rd(0x07, buf, 1); // Status
    return (buf[0] - 75) << 8;
  } else
    return -1;
}

// Turn magnetometer off
void mag_off() {
  if (puckVersion == PUCKJS_2V1)
    app_timer_stop(m_poll_timer_id);
  //jsiConsolePrintf("mag_off\n");
  nrf_gpio_cfg_default(MAG_PIN_SDA);
  nrf_gpio_cfg_default(MAG_PIN_SCL);
  nrf_gpio_cfg_default(MAG_PIN_INT);
  nrf_gpio_cfg_default(MAG_PIN_DRDY);
  nrf_gpio_pin_clear(MAG_PIN_PWR);
  nrf_gpio_cfg_output(MAG_PIN_PWR);
}

void peripheralPollHandler() {
  if (puckVersion == PUCKJS_2V1) {
    unsigned char buf[1];
    mag_rd(0x18, buf, 1); // Status
    if (buf[0]&0x40) {// check for Meas_m_done
      mag_read();
      mag_data_ready = true;
      jshHadEvent();
    }
  }
}

bool accel_on(int milliHz) {
  // CTRL1_XL / CTRL2_G
  int reg = 0;
  bool gyro = true;
  if (milliHz<12500) { // 1.6Hz, no gyro
    reg = 11<<4;
    gyro = false;
  } else if (milliHz==12500) reg=1<<4; // 12.5 Hz (low power)
  else if (milliHz==26000) reg=2<<4; // 26 Hz (low power)
  else if (milliHz==52000) reg=3<<4; // 52 Hz (low power)
  else if (milliHz==104000) reg=4<<4; // 104 Hz (normal mode)
  else if (milliHz==208000) reg=5<<4; // 208 Hz (normal mode)
  else if (milliHz==416000) reg=6<<4; // 416 Hz (high performance)
  else if (milliHz==833000) reg=7<<4; // 833 Hz (high performance)
  else if (milliHz==1660000) reg=8<<4; // 1.66 kHz (high performance)
  else return false;


  jshPinSetState(ACCEL_PIN_INT, JSHPINSTATE_GPIO_IN);
#ifdef ACCEL_PIN_PWR
  jshPinSetState(ACCEL_PIN_PWR, JSHPINSTATE_GPIO_OUT);
#endif
  jshPinSetState(ACCEL_PIN_SCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetState(ACCEL_PIN_SDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
#ifdef ACCEL_PIN_PWR
  jshPinSetValue(ACCEL_PIN_PWR, 1);
#endif
  jshPinSetValue(ACCEL_PIN_SCL, 1);
  jshPinSetValue(ACCEL_PIN_SDA, 1);
  jshDelayMicroseconds(20000); // 20ms boot from app note

  // LSM6DS3TR
  unsigned char buf[2];
  buf[0] = 0x15; buf[1]=0x10; // CTRL6-C - XL_HM_MODE=1, low power accelerometer
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x16; buf[1]=0x80; //  CTRL6-C - G_HM_MODE=1, low power gyro
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x18; buf[1]=0x38; // CTRL9_XL  Acc X, Y, Z axes enabled
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x10; buf[1]=reg | 0b00001011; // CTRL1_XL Accelerometer, +-4g, 50Hz AA filter
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x11; buf[1]=gyro ? reg : 0; // CTRL2_G  Gyro, 250 dps, no 125dps limit
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x12; buf[1]=0x44; // CTRL3_C, BDU, irq active high, push pull, auto-inc
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x0D; buf[1]=3; // INT1_CTRL - Gyro/accel data ready IRQ
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);

  return true;
}

// Read a value
void accel_read() {
  unsigned char buf[12];
  buf[0] = 0x22; // OUTX_L_G
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(&i2cAccel, ACCEL_ADDR, 12, buf, true);
  gyro_reading[0] = (buf[1]<<8) | buf[0];
  gyro_reading[1] = (buf[3]<<8) | buf[2];
  gyro_reading[2] = (buf[5]<<8) | buf[4];
  accel_reading[0] = (buf[7]<<8) | buf[6];
  accel_reading[1] = (buf[9]<<8) | buf[8];
  accel_reading[2] = (buf[11]<<8) | buf[10];
}

void accel_wait() {
  int timeout;
  unsigned char buf[1];
  timeout = 400;
  do {
    buf[0] = 0x1E; // STATUS_REG
    jsi2cWrite(&i2cAccel, ACCEL_ADDR, 1, buf, false);
    jsi2cRead(&i2cAccel, ACCEL_ADDR, 1, buf, true);
    //jsiConsolePrintf("M %d\n", buf[0]);
  } while (!(buf[0]&3) && --timeout); // ZYXDA
  if (!timeout) jsExceptionHere(JSET_INTERNALERROR, "Timeout (Accelerometer)");
}

// Turn accelerometer off
void accel_off() {
#ifdef ACCEL_PIN_PWR
  nrf_gpio_cfg_input(ACCEL_PIN_SDA, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(ACCEL_PIN_SCL, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(ACCEL_PIN_INT, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_pin_clear(ACCEL_PIN_PWR);
  nrf_gpio_cfg_output(ACCEL_PIN_PWR);
#else
  unsigned char buf[2];
  buf[0] = 0x10; buf[1]=0; // CTRL1_XL  - power down
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
  buf[0] = 0x11; buf[1]=0; // CTRL2_G   - power down
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
#endif
}


/*JSON{
    "type": "class",
    "class" : "Puck",
    "ifdef" : "PUCKJS"
}
Class containing [Puck.js's](http://www.puck-js.com) utility functions.
*/
/*JSON{
  "type" : "variable",
  "name" : "FET",
  "generate_full" : "26",
  "ifdef" : "PUCKJS",
  "return" : ["pin",""]
}
On Puck.js V2 (not v1.0) this is the pin that controls the FET, for high-powered outputs.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "mag",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_mag",
  "return" : ["JsVar", "An Object `{x,y,z}` of magnetometer readings as integers" ]
}
Turn on the magnetometer, take a single reading, and then turn it off again.

An object of the form `{x,y,z}` is returned containing magnetometer readings.
Due to residual magnetism in the Puck and magnetometer itself, with
no magnetic field the Puck will not return `{x:0,y:0,z:0}`.

Instead, it's up to you to figure out what the 'zero value' is for your
Puck in your location and to then subtract that from the value returned. If
you're not trying to measure the Earth's magnetic field then it's a good idea
to just take a reading at startup and use that.

With the aerial at the top of the board, the `y` reading is vertical, `x` is
horizontal, and `z` is through the board.

Readings are in increments of 0.1 micro Tesla (uT). The Earth's magnetic field
varies from around 25-60 uT, so the reading will vary by 250 to 600 depending
on location.
*/
JsVar *jswrap_puck_mag() {
  /* If not enabled, turn on and read. If enabled,
   * just pass out the last reading */
  if (!mag_enabled) {
    mag_on(0, true /*instant*/); // takes a reading right away
    mag_off();
  } else if (puckVersion == PUCKJS_2V1) { // MMC5603NJ
    // magnetometer is on, but let's poll it quickly
    // to see if we have any new data
    unsigned char buf[1];
    mag_rd(0x18, buf, 1); // Status
    if (buf[0]&0x40) // check for Meas_m_done
      mag_read();
  }
  return to_xyz(mag_reading, 1);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "magTemp",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_magTemp",
  "return" : ["float", "Temperature in degrees C" ]
}
Turn on the magnetometer, take a single temperature reading from the MAG3110 chip, and then turn it off again.

(If the magnetometer is already on, this just returns the last reading obtained)

`E.getTemperature()` uses the microcontroller's temperature sensor, but this uses the magnetometer's.

The reading obtained is an integer (so no decimal places), but the sensitivity is factory trimmed. to 1&deg;C, however the temperature
offset isn't - so absolute readings may still need calibrating.
*/
JsVarFloat jswrap_puck_magTemp() {
  int t;
  if (!mag_enabled) {
    mag_on(0, true /*instant*/); // takes a reading right away
    t = mag_read_temp();
    mag_off();
  } else
    t = mag_read_temp();
  return ((JsVarFloat)t) / 256.0;
}

/*JSON{
  "type" : "event",
  "class" : "Puck",
  "name" : "mag",
  "ifdef" : "PUCKJS"
}
Called after `Puck.magOn()` every time magnetometer data
is sampled. There is one argument which is an object
of the form `{x,y,z}` containing magnetometer readings
as integers (for more information see `Puck.mag()`).

Check out [the Puck.js page on the magnetometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information.
 */

/*JSON{
  "type" : "event",
  "class" : "Puck",
  "name" : "accel",
  "ifdef" : "PUCKJS"
}
Only on Puck.js v2.0

Called after `Puck.accelOn()` every time accelerometer data
is sampled. There is one argument which is an object
of the form `{acc:{x,y,z}, gyro:{x,y,z}}` containing the data.

The data is as it comes off the accelerometer and is not
scaled to 1g. For more information see `Puck.accel()` or
[the Puck.js page on the magnetometer](http://www.espruino.com/Puck.js#on-board-peripherals).
 */

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "magOn",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_magOn",
  "params" : [
      ["samplerate","float","The sample rate in Hz, or undefined"]
  ]
}
Turn the magnetometer on and start periodic sampling. Samples will then cause
a 'mag' event on 'Puck':

```
Puck.magOn();
Puck.on('mag', function(xyz) {
  console.log(xyz);
  // {x:..., y:..., z:...}
});
// Turn events off with Puck.magOff();
```

This call will be ignored if the sampling is already on.

If given an argument, the sample rate is set (if not, it's at 0.63 Hz). 
The sample rate must be one of the following (resulting in the given power consumption):

* 80 Hz - 900uA
* 40 Hz - 550uA
* 20 Hz - 275uA
* 10 Hz - 137uA
* 5 Hz - 69uA
* 2.5 Hz - 34uA
* 1.25 Hz - 17uA
* 0.63 Hz - 8uA
* 0.31 Hz - 8uA
* 0.16 Hz - 8uA
* 0.08 Hz - 8uA

When the battery level drops too low while sampling is turned on,
the magnetometer may stop sampling without warning, even while other
Puck functions continue uninterrupted.

Check out [the Puck.js page on the magnetometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information.

*/
void jswrap_puck_magOn(JsVarFloat hz) {
  if (mag_enabled) {
    jswrap_puck_magOff();
    // wait 1ms for power-off
    jshDelayMicroseconds(1000);
  }
  int milliHz = (int)((hz*1000)+0.5);
  if (milliHz<=0) milliHz=630;
  if (!mag_on(milliHz, false /*not instant*/)) {
    jsExceptionHere(JSET_ERROR, "Invalid sample rate %f - must be 80, 40, 20, 10, 5, 2.5, 1.25, 0.63, 0.31, 0.16 or 0.08 Hz", hz);
  }
  /* Setting a watch will push an event which will let Espruino
   * go around the idle loop and call jswrap_puck_idle - which
   * is where we actually collect the data. */
  if (puckVersion == PUCKJS_1V0) {
    jshPinWatch(MAG_PIN_INT, true, JSPW_NONE);
    jshPinSetState(MAG_PIN_INT, JSHPINSTATE_GPIO_IN);
  } else if (puckVersion == PUCKJS_2V0) {
    jshPinWatch(MAG_PIN_DRDY, true, JSPW_NONE);
    jshPinSetState(MAG_PIN_DRDY, JSHPINSTATE_GPIO_IN);
  } // 2v1 doesn't have an IRQ line - we poll with peripheralPollHandler
  mag_enabled = true;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "magOff",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_magOff"
}
Turn the magnetometer off
*/
void jswrap_puck_magOff() {
  if (mag_enabled) {
    if (puckVersion == PUCKJS_1V0) {
      jshPinWatch(MAG_PIN_INT, false, JSPW_NONE);
    } else if (puckVersion == PUCKJS_2V0) {
      jshPinWatch(MAG_PIN_DRDY, false, JSPW_NONE);
    } // 2v1 doesn't have an IRQ line
    mag_off();
  }
  mag_enabled = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "magWr",
    "generate" : "jswrap_puck_magWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "ifdef" : "PUCKJS"
}
Writes a register on the LIS3MDL / MAX3110 Magnetometer. Can be used for configuring advanced functions.

Check out [the Puck.js page on the magnetometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information and links to modules that use this function.
*/
void jswrap_puck_magWr(JsVarInt reg, JsVarInt data) {
  mag_wr(reg, data);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "magRd",
    "generate" : "jswrap_puck_magRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""],
    "ifdef" : "PUCKJS"
}
Reads a register from the LIS3MDL / MAX3110 Magnetometer. Can be used for configuring advanced functions.

Check out [the Puck.js page on the magnetometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information and links to modules that use this function.
*/
int jswrap_puck_magRd(JsVarInt reg) {
  unsigned char buf[1];
  mag_rd(reg, buf, 1);
  return buf[0];
}


// Turn temp sensor on
void temp_on() {
  jshPinSetValue(TEMP_PIN_PWR, 1);
  jshPinSetValue(TEMP_PIN_SCL, 1);
  jshPinSetValue(TEMP_PIN_SDA, 1);
  jshPinSetState(TEMP_PIN_PWR, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TEMP_PIN_SCL, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TEMP_PIN_SDA, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(TEMP_PIN_SCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetState(TEMP_PIN_SDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshDelayMicroseconds(50000); // wait for startup and first reading... could this be faster?
}

// Turn temp sensor off
void temp_off() {
  nrf_gpio_cfg_input(TEMP_PIN_SDA, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(TEMP_PIN_SCL, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_pin_clear(TEMP_PIN_PWR);
  nrf_gpio_cfg_output(TEMP_PIN_PWR);
}



/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "getTemperature",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_getTemperature",
  "return" : ["float", "Temperature in degrees C" ]
}
On Puck.js v2.0 this will use the on-board PCT2075TP temperature sensor, but on Puck.js the less accurate on-chip Temperature sensor is used.
*/
JsVarFloat jswrap_puck_getTemperature() {
  if (PUCKJS_HAS_TEMP_SENSOR) {
    temp_on();
    unsigned char buf[2];
    // 'on' is the default
    //buf[1] = 1; // CONF
    //buf[0] = 0; // on
    //jsi2cWrite(&i2cTemp,TEMP_ADDR, 1, buf, false);
    buf[0] = 0; // TEMP
    jsi2cWrite(&i2cTemp,TEMP_ADDR, 1, buf, false);
    jsi2cRead(&i2cTemp, TEMP_ADDR, 2, buf, true);
    int t = (buf[0]<<3) | (buf[1]>>5);
    if (t&1024) t-=2048; // negative
    temp_off();
    return t / 8.0;
  } else {
    return jshReadTemperature();
  }
}


/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "accelOn",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_accelOn",
  "params" : [
      ["samplerate","float","The sample rate in Hz, or undefined"]
  ]
}

Accepted values are:

* 1.6 Hz (no Gyro) - 40uA (2v05 and later firmware)
* 12.5 Hz (with Gyro)- 350uA
* 26 Hz (with Gyro) - 450 uA
* 52 Hz (with Gyro) - 600 uA
* 104 Hz (with Gyro) - 900 uA
* 208 Hz (with Gyro) - 1500 uA
* 416 Hz (with Gyro) (not recommended)
* 833 Hz (with Gyro) (not recommended)
* 1660 Hz (with Gyro) (not recommended)

Once `Puck.accelOn()` is called, the `Puck.accel` event will be called each time data is received. `Puck.accelOff()` can be called to turn the accelerometer off.

For instance to light the red LED whenever Puck.js is face up:

```
Puck.on('accel', function(a) {
 digitalWrite(LED1, a.acc.z > 0);
});
Puck.accelOn();
```

Check out [the Puck.js page on the accelerometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information.

*/
void jswrap_puck_accelOn(JsVarFloat hz) {
  if (!PUCKJS_HAS_ACCEL) {
    jsExceptionHere(JSET_ERROR, "Not available on Puck.js v1");
    return;
  }
  if (accel_enabled) {
    jswrap_puck_accelOff();
    // wait 1ms for power-off
    jshDelayMicroseconds(1000);
  }
  int milliHz = (int)((hz*1000)+0.5);
  if (milliHz==0) milliHz=12500;
  if (!accel_on(milliHz)) {
    jsExceptionHere(JSET_ERROR, "Invalid sample rate %f - must be 1660, 833, 416, 208, 104, 52, 26, 12.5, 1.6 Hz", hz);
  }
  jshPinWatch(ACCEL_PIN_INT, true, JSPW_NONE);
  accel_enabled = true;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "accelOff",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_accelOff"
}
Turn the accelerometer off after it has been turned on by `Puck.accelOn()`. 

Check out [the Puck.js page on the accelerometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information.
*/
void jswrap_puck_accelOff() {
  if (!PUCKJS_HAS_ACCEL) {
    jsExceptionHere(JSET_ERROR, "Not available on Puck.js v1");
    return;
  }
  if (accel_enabled) {
    jshPinWatch(ACCEL_PIN_INT, false, JSPW_NONE);
    accel_off();
  }
  accel_enabled = false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "accel",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_accel",
  "return" : ["JsVar", "An Object `{acc:{x,y,z}, gyro:{x,y,z}}` of accelerometer/gyro readings" ]
}
Turn on the accelerometer, take a single reading, and then turn it off again.

The values reported are the raw values from the chip. In normal configuration:

* accelerometer: full-scale (32768) is 4g, so you need to divide by 8192 to get correctly scaled values
* gyro: full-scale (32768) is 245 dps, so you need to divide by 134 to get correctly scaled values

If taking more than one reading, we'd suggest you use `Puck.accelOn()` and the `Puck.accel` event.
*/
JsVar *jswrap_puck_accel() {
  /* If not enabled, turn on and read. If enabled,
   * just pass out the last reading */
  if (!accel_enabled) {
    accel_on(1660000);
    accel_wait();
    accel_read();
    accel_off();
  }
  JsVar *o = jsvNewObject();
  jsvObjectSetChildAndUnLock(o,"acc",to_xyz(accel_reading, 1));
  jsvObjectSetChildAndUnLock(o,"gyro",to_xyz(gyro_reading, 1));
  return o;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "accelWr",
    "generate" : "jswrap_puck_accelWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "ifdef" : "PUCKJS"
}
Writes a register on the LSM6DS3TR-C Accelerometer. Can be used for configuring advanced functions.

Check out [the Puck.js page on the accelerometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information and links to modules that use this function.
*/
void jswrap_puck_accelWr(JsVarInt reg, JsVarInt data) {
  if (!PUCKJS_HAS_ACCEL) {
    jsExceptionHere(JSET_ERROR, "Not available on Puck.js v1");
    return;
  }
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 2, buf, true);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "accelRd",
    "generate" : "jswrap_puck_accelRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""],
    "ifdef" : "PUCKJS"
}
Reads a register from the LSM6DS3TR-C Accelerometer. Can be used for configuring advanced functions.

Check out [the Puck.js page on the accelerometer](http://www.espruino.com/Puck.js#on-board-peripherals)
for more information and links to modules that use this function.
*/
int jswrap_puck_accelRd(JsVarInt reg) {
  if (!PUCKJS_HAS_ACCEL) {
    jsExceptionHere(JSET_ERROR, "Not available on Puck.js v1");
    return -1;
  }
  unsigned char buf[1];
  buf[0] = (unsigned char)reg;
  jsi2cWrite(&i2cAccel, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(&i2cAccel, ACCEL_ADDR, 1, buf, true);
  return buf[0];
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "IR",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_IR",
  "params" : [
      ["data","JsVar","An array of pulse lengths, in milliseconds"],
      ["cathode","pin","(optional) pin to use for IR LED cathode - if not defined, the built-in IR LED is used"],
      ["anode","pin","(optional) pin to use for IR LED anode - if not defined, the built-in IR LED is used"]
  ]
}
Transmit the given set of IR pulses - data should be an array of pulse times
in milliseconds (as `[on, off, on, off, on, etc]`).

For example `Puck.IR(pulseTimes)` - see http://www.espruino.com/Puck.js+Infrared
for a full example.

You can also attach an external LED to Puck.js, in which case
you can just execute `Puck.IR(pulseTimes, led_cathode, led_anode)`

It is also possible to just supply a single pin for IR transmission
with `Puck.IR(pulseTimes, led_anode)` (on 2v05 and above).
*/
Pin _jswrap_puck_IR_pin;
void _jswrap_puck_IR_on() {
  jshPinAnalogOutput(_jswrap_puck_IR_pin, 0.1, 38000, 0);
}
void _jswrap_puck_IR_off() {
  // normally we're doing single-pin IR with the cathode, so 1=off
  bool polarity = 1;
  // On Puck.js v2 we go through an (N)FET, which means 0=off.
  if (PUCKJS_HAS_IR_FET && (_jswrap_puck_IR_pin==IR_FET_PIN || _jswrap_puck_IR_pin==FET_PIN))
    polarity = 0;
  jshPinOutput(_jswrap_puck_IR_pin, polarity);
}
// Called when we're done with the IR transmission
void _jswrap_puck_IR_done(JsSysTime t, void *data) {
  uint32_t d = (uint32_t)data;
  Pin cathode = d&255;
  Pin anode = (d>>8)&255;
  // set as input - so no signal
  jshPinSetState(anode, JSHPINSTATE_GPIO_IN);
  // this one also stops the PWM
  jshPinSetState(cathode, JSHPINSTATE_GPIO_IN);
}
void jswrap_puck_IR(JsVar *data, Pin cathode, Pin anode) {
  if (!jsvIsIterable(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting an array, got %t", data);
    return;
  }
  if (jshIsPinValid(anode) && !jshIsPinValid(cathode)) {
    jsExceptionHere(JSET_TYPEERROR, "Invalid pin combination");
    return;
  }

  if (!(jshIsPinValid(cathode) || jshIsPinValid(anode))) {
    if (PUCKJS_HAS_IR_FET) {
      cathode = IR_FET_PIN;
    } else { // Puck v1
      anode = IR_ANODE_PIN;
      cathode = IR_CATHODE_PIN;
    }
  }
  bool twoPin = jshIsPinValid(anode) && jshIsPinValid(cathode);
  bool pulsePolarity = true;

  if (twoPin) {
    jshPinAnalogOutput(cathode, 0.75, 38000, 0);
    jshPinSetValue(anode, pulsePolarity);
  } else {
    // Otherwise we're just doing stuff on a single pin
    _jswrap_puck_IR_pin = cathode;
  }
  JsSysTime time = 0;
  uint32_t timerOffset = jstGetUtilTimerOffset();
  bool hasPulses = false;

  JsvIterator it;
  jsvIteratorNew(&it, data, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it)) {
    JsVarFloat pulseTime = jsvIteratorGetFloatValue(&it);
    if (twoPin) {
      if (hasPulses) jstPinOutputAtTime(time, &timerOffset, &anode, 1, pulsePolarity);
      else jshPinSetState(anode, JSHPINSTATE_GPIO_OUT);
    } else {
      if (pulsePolarity) jstExecuteFn(_jswrap_puck_IR_on, NULL, time, 0, &timerOffset);
      else jstExecuteFn(_jswrap_puck_IR_off, NULL, time, 0, &timerOffset);
    }
    hasPulses = true;
    time += jshGetTimeFromMilliseconds(pulseTime);
    pulsePolarity = !pulsePolarity;
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  if (hasPulses) {
    if (twoPin) {
      uint32_t d = cathode | anode<<8;
      jstExecuteFn(_jswrap_puck_IR_done, (void*)d, time, 0, &timerOffset);
    } else {
      jstExecuteFn(_jswrap_puck_IR_off, NULL, time, 0, &timerOffset);
    }
  }
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "capSense",
    "ifdef" : "PUCKJS",
    "generate" : "jswrap_puck_capSense",
    "params" : [
      ["tx","pin",""],
      ["rx","pin",""]
    ],
    "return" : ["int", "Capacitive sense counter" ]
}
Capacitive sense - the higher the capacitance, the higher the number returned.

If called without arguments, a value depending on the capacitance of what is 
attached to pin D11 will be returned. If you attach a length of wire to D11,
you'll be able to see a higher value returned when your hand is near the wire
than when it is away.

You can also supply pins to use yourself, however if you do this then
the TX pin must be connected to RX pin and sense plate via a roughly 1MOhm 
resistor.

When not supplying pins, Puck.js uses an internal resistor between D12(tx)
and D11(rx).
*/
int jswrap_puck_capSense(Pin tx, Pin rx) {
  if (jshIsPinValid(tx) && jshIsPinValid(rx)) {
    return (int)nrf_utils_cap_sense(tx, rx);
  }
  return (int)nrf_utils_cap_sense(CAPSENSE_TX_PIN, CAPSENSE_RX_PIN);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "light",
    "ifdef" : "PUCKJS",
    "generate" : "jswrap_puck_light",
    "return" : ["float", "A light value from 0 to 1" ]
}
Return a light value based on the light the red LED is seeing.

**Note:** If called more than 5 times per second, the received light value
may not be accurate.
*/
JsVarFloat jswrap_puck_light() {
  // If pin state wasn't an analog input before, make it one now,
  // read, and delay, just to make sure everything has time to settle
  // before the 'real' reading
  JshPinState s = jshPinGetState(LED1_PININDEX);
  if (s != JSHPINSTATE_GPIO_IN) {
    jshPinOutput(LED1_PININDEX,0);// discharge
    jshPinAnalog(LED1_PININDEX);// analog
    int delay = 5000;
    // if we were using a peripheral it can take longer
    // for everything to sort itself out
    if ((s&JSHPINSTATE_MASK) == JSHPINSTATE_AF_OUT)
      delay = 50000;
    jshDelayMicroseconds(delay);
  }
  JsVarFloat v = jswrap_ble_getBattery();
  JsVarFloat f = jshPinAnalog(LED1_PININDEX)*v/(3*0.45);
  if (f>1) f=1;
  // turn the red LED back on if it was on before
  if (s & JSHPINSTATE_PIN_IS_ON)
    jshPinOutput(LED1_PININDEX, 1);

  return f;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "ifdef" : "PUCKJS",
    "name" : "getBatteryPercentage",
    "generate" : "jswrap_espruino_getBattery",
    "return" : ["int", "A percentage between 0 and 100" ]
}
DEPRECATED - Please use `E.getBattery()` instead.

Return an approximate battery percentage remaining based on
a normal CR2032 battery (2.8 - 2.2v).
*/
JsVarInt jswrap_puck_getBattery() {
  JsVarFloat v = jshReadVRef();
  int pc = (v-2.2)*100/0.6;
  if (pc>100) pc=100;
  if (pc<0) pc=0;
  return pc;
}

static bool selftest_check_pin(Pin pin, char *err) {
  unsigned int i;
  char pinStr[4];
  pinStr[0]='-';
  itostr(pin,&pinStr[1],10);

  bool ok = true;
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 1);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP);
  if (!jshPinGetValue(pin)) {
    pinStr[0]='l';
    if (!err[0]) strcpy(err,pinStr);
    jsiConsolePrintf("Pin %p forced low\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
    if (PUCK_IO_PINS[i]!=pin)
      jshPinOutput(PUCK_IO_PINS[i], 0);
  if (!jshPinGetValue(pin)) {
    pinStr[0]='L';
    if (!err[0]) strcpy(err,pinStr);
    jsiConsolePrintf("Pin %p shorted low\n", pin);
    ok = false;
  }

  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 0);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLDOWN);
  if (jshPinGetValue(pin)) {
    pinStr[0]='h';
    if (!err[0]) strcpy(err,pinStr);
    jsiConsolePrintf("Pin %p forced high\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
     if (PUCK_IO_PINS[i]!=pin)
       jshPinOutput(PUCK_IO_PINS[i], 1);
   if (jshPinGetValue(pin)) {
     pinStr[0]='H';
     if (!err[0]) strcpy(err,pinStr);
     jsiConsolePrintf("Pin %p shorted high\n", pin);
     ok = false;
   }
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
  return ok;
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Puck",
    "name" : "selfTest",
    "ifdef" : "PUCKJS",
    "generate" : "jswrap_puck_selfTest",
    "return" : ["bool", "True if the self-test passed" ]
}
Run a self-test, and return true for a pass. This checks for shorts
between pins, so your Puck shouldn't have anything connected to it.

**Note:** This self-test auto starts if you hold the button on your Puck
down while inserting the battery, leave it pressed for 3 seconds (while
the green LED is lit) and release it soon after all LEDs turn on. 5
red blinks is a fail, 5 green is a pass.

If the self test fails, it'll set the Puck.js Bluetooth advertising name
to `Puck.js !ERR` where ERR is a 3 letter error code.

*/
bool jswrap_puck_selfTest() {
  unsigned int timeout, i;
  JsVarFloat v;
  bool ok = true;
  char err[4] = {0,0,0,0};

  // light up all LEDs white
  jshPinOutput(LED1_PININDEX, LED1_ONSTATE);
  jshPinOutput(LED2_PININDEX, LED2_ONSTATE);
  jshPinOutput(LED3_PININDEX, LED3_ONSTATE);
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);

  timeout = 2000;
  while (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE && timeout--)
    nrf_delay_ms(1);
  if (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("Timeout waiting for button to be released.\n");
    if (!err[0]) strcpy(err,"BTN");
    ok = false;
  }
  nrf_delay_ms(100);
  jshPinInput(LED1_PININDEX);
  jshPinInput(LED2_PININDEX);
  jshPinInput(LED3_PININDEX);
  nrf_delay_ms(500);


  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED1_PININDEX);
  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.2 || v>0.65) {
    if (!err[0]) strcpy(err,"LD1");
    jsiConsolePrintf("LED1 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(LED2_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED2_PININDEX);
  jshPinSetState(LED2_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.55 || v>0.85) {
    if (!err[0]) strcpy(err,"LD2");
    jsiConsolePrintf("LED2 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(LED3_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED3_PININDEX);
  jshPinSetState(LED3_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.55 || v>0.90) {
    if (!err[0]) strcpy(err,"LD3");
    jsiConsolePrintf("LED3 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_IN_PULLDOWN);
  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(IR_CATHODE_PIN, 1);
  nrf_delay_ms(1);
  if (jshPinGetValue(IR_ANODE_PIN)) {
    if (!err[0]) strcpy(err,"IRs");
    jsiConsolePrintf("IR LED wrong way around/shorted?\n");
    ok = false;
  }


  if (PUCKJS_HAS_IR_FET) {
    jshPinSetValue(IR_FET_PIN, 0);
    jshPinSetState(IR_FET_PIN, JSHPINSTATE_GPIO_OUT);
    jshPinSetState(IR_INPUT_PIN, JSHPINSTATE_GPIO_IN_PULLUP);
    nrf_delay_ms(1);
    if (!jshPinGetValue(IR_INPUT_PIN)) {
      if (!err[0]) strcpy(err,"IRs");
      jsiConsolePrintf("IR LED short?\n");
      ok = false;
    }
    jshPinSetValue(IR_FET_PIN, 1);
    nrf_delay_ms(1);
    if (jshPinGetValue(IR_INPUT_PIN)) {
      if (!err[0]) strcpy(err,"IRF");
      jsiConsolePrintf("IR FET disconnected?\n");
      ok = false;
    }
    jshPinSetState(IR_INPUT_PIN, JSHPINSTATE_GPIO_IN);
    jshPinSetValue(IR_FET_PIN, 0);
  } else {
    jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_IN_PULLDOWN);
    jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(IR_ANODE_PIN, 1);
    nrf_delay_ms(1);
    if (!jshPinGetValue(IR_CATHODE_PIN)) {
      if (!err[0]) strcpy(err,"IRd");
      jsiConsolePrintf("IR LED disconnected?\n");
      ok = false;
    }
  }

  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_IN);
  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_IN);

  mag_on(0, true /*instant*/); // takes a reading right away
  mag_off();
  mag_enabled = false;
  if (mag_reading[0]==-1 && mag_reading[1]==-1 && mag_reading[2]==-1) {
    if (!err[0]) strcpy(err,"MAG");
    jsiConsolePrintf("Magnetometer not working?\n");
    ok = false;
  }

  if (PUCKJS_HAS_ACCEL) {
    accel_on(1660000);
    unsigned char buf[1];
    buf[0] = 0x0F; // WHOAMI
    jsi2cWrite(&i2cAccel, ACCEL_ADDR, 1, buf, false);
    jsi2cRead(&i2cAccel, ACCEL_ADDR, 1, buf, true);
    accel_off();
    if (buf[0]!=106) {
      if (!err[0]) strcpy(err,"ACC");
      jsiConsolePrintf("Accelerometer WHOAMI failed\n");
      ok = false;
    }

    JsVarFloat t = jswrap_puck_getTemperature();
    if (t<0 || t>40) {
      if (!err[0]) strcpy(err,"TMP");
      jsiConsolePrintf("Unexpected temperature\n");
      ok = false;
    }

  }

  jshPinSetState(CAPSENSE_TX_PIN, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(CAPSENSE_RX_PIN, JSHPINSTATE_GPIO_IN);
  jshPinSetValue(CAPSENSE_TX_PIN, 1);
  nrf_delay_ms(1);
  if (!jshPinGetValue(CAPSENSE_RX_PIN)) {
    if (!err[0]) strcpy(err,"CPu");
    jsiConsolePrintf("Capsense resistor disconnected? (pullup)\n");
    ok = false;
  }
  jshPinSetValue(CAPSENSE_TX_PIN, 0);
  nrf_delay_ms(1);
  if (jshPinGetValue(CAPSENSE_RX_PIN)) {
    if (!err[0]) strcpy(err,"CPd");
    jsiConsolePrintf("Capsense resistor disconnected? (pulldown)\n");
    ok = false;
  }
  jshPinSetState(CAPSENSE_TX_PIN, JSHPINSTATE_GPIO_IN);


  ok &= selftest_check_pin(1,err);
  ok &= selftest_check_pin(2,err);
  if (!PUCKJS_HAS_TEMP_SENSOR) {
    ok &= selftest_check_pin(6,err);
    ok &= selftest_check_pin(7,err);
    ok &= selftest_check_pin(8,err);
  }
  ok &= selftest_check_pin(28,err);
  ok &= selftest_check_pin(29,err);
  ok &= selftest_check_pin(30,err);
  ok &= selftest_check_pin(31,err);

  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
    jshPinSetState(PUCK_IO_PINS[i], JSHPINSTATE_GPIO_IN);

  if (err[0]) {
    char deviceName[BLE_GAP_DEVNAME_MAX_LEN];
    strcpy(deviceName,"Puck.js !");
    strcat(deviceName,err);
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    sd_ble_gap_device_name_set(&sec_mode,
                              (const uint8_t *)deviceName,
                              strlen(deviceName));
    jsiConsolePrintf("Error code %s\n",err);
  }

  return ok;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_puck_init"
}*/
void jswrap_puck_init() {
  // Set up I2C
  jshI2CInitInfo(&i2cMag);
  i2cMag.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cMag.pinSDA = MAG_PIN_SDA;
  i2cMag.pinSCL = MAG_PIN_SCL;
  i2cMag.clockStretch = false;
  jshI2CInitInfo(&i2cAccel);
  i2cAccel.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cAccel.pinSDA = ACCEL_PIN_SDA;
  i2cAccel.pinSCL = ACCEL_PIN_SCL;
  i2cAccel.clockStretch = false;
  jshI2CInitInfo(&i2cTemp);
  i2cTemp.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cTemp.pinSDA = TEMP_PIN_SDA;
  i2cTemp.pinSCL = TEMP_PIN_SCL;
  i2cTemp.clockStretch = false;
  accel_off();
  temp_off();
  mag_pin_on();
  jshDelayMicroseconds(2500); // 1.7ms from power on to ok
  // MAG3110 WHO_AM_I - for some reason we have to use this slightly odd SW I2C implementation for this
  unsigned char buf[2];
  puckVersion = PUCKJS_1V0;
  mag_rd(0x07, buf, 1); // MAG3110 WHO_AM_I
  //jsiConsolePrintf("MAG3110 %d\n", buf[0]);
  if (buf[0]!=0xC4 && buf[0]!=0x00) { // sometimes MAG3110 reports ID 0!
    // Not found, check for LIS3MDL - Puck.js v2
    nrf_delay_ms(1); // LIS3MDL takes longer to boot
    puckVersion = PUCKJS_2V0;
    mag_rd(0x0F, buf, 1); // LIS3MDL WHO_AM_I
    //jsiConsolePrintf("LIS3 %d\n", buf[0]);
    if (buf[0] == 0b00111101) {
      // all good, MAG3110 - Puck.js v2.0
      puckVersion = PUCKJS_2V0;
    } else {
      // Is it 2v1?
      puckVersion = PUCKJS_2V1;
      mag_rd(0x39, buf, 1); // MMC5603NJ WHO_AM_I
      //jsiConsolePrintf("MMC5603NJ %d\n", buf[0]);
      if (buf[0] == 16) {
        puckVersion = PUCKJS_2V1;
      } else {
        // uh-oh, no magnetometer found
        jsWarn("No Magnetometer found");
        puckVersion = PUCKJS_UNKNOWN;
      }
    }
  } else {
    // all good, MAG3110 - Puck.js v1
    puckVersion = PUCKJS_1V0;
  }
  // Power off
  mag_off();
  // If Puck.js v2 ensure FETs are forced off
  if (PUCKJS_HAS_IR_FET) {
    jshPinSetValue(IR_FET_PIN, 0);
    jshPinSetState(IR_FET_PIN, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(FET_PIN, 0);
    jshPinSetState(FET_PIN, JSHPINSTATE_GPIO_OUT);
  }

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  bool firstStart = jsiStatus & JSIS_FIRST_BOOT; // is this the first time jswrap_puck_init was called?;
  if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    // don't do it during a software reset - only first hardware reset
    bool result = jswrap_puck_selfTest();
    // green if good, red if bad
    Pin indicator = result ? LED2_PININDEX : LED1_PININDEX;
    int i;
    for (i=0;i<5;i++) {
      jshPinOutput(indicator, LED1_ONSTATE);
      nrf_delay_ms(500);
      jshPinOutput(indicator, !LED1_ONSTATE);
      nrf_delay_ms(500);
    }
    jshPinInput(indicator);
    // If the button is *still* pressed, remove all code from flash memory too!
    if (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
      jsfRemoveCodeFromFlash();
    }
  }

  // requires APP_TIMER_OP_QUEUE_SIZE=5 in BOARD.py
  uint32_t err_code = app_timer_create(&m_poll_timer_id,
                      APP_TIMER_MODE_REPEATED,
                      peripheralPollHandler);
  jsble_check_error(err_code);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_puck_kill"
}*/
void jswrap_puck_kill() {
  if (mag_enabled) {
    mag_off();
    mag_enabled = false;
  }
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_puck_idle"
}*/
bool jswrap_puck_idle() {
  bool busy = false;
  /* jshPinWatch should mean that we wake up whenever a new
   * magnetometer reading is ready */
  if (mag_enabled) {
    if (puckVersion == PUCKJS_1V0) { // Puck.js v1
      if (nrf_gpio_pin_read(MAG_PIN_INT)) {
        mag_read();
        mag_data_ready = true;
      }
    } else if (puckVersion == PUCKJS_2V0) {
      if (nrf_gpio_pin_read(MAG_PIN_DRDY)) {
        mag_read();
        mag_data_ready = true;
      }
    } // Puck 2v1 uses periphPollHandler to set mag_data_ready
    if (mag_data_ready) {
      JsVar *xyz = to_xyz(mag_reading, 1);
      mag_data_ready = false;
      JsVar *puck = jsvObjectGetChild(execInfo.root, "Puck", 0);
      if (jsvHasChildren(puck))
          jsiQueueObjectCallbacks(puck, JS_EVENT_PREFIX"mag", &xyz, 1);
      jsvUnLock2(puck, xyz);
      busy = true; // don't sleep - handle this now
    }
  }
  // accel_enabled is set to true only on devices with an accelerometer
  if (accel_enabled && nrf_gpio_pin_read(ACCEL_PIN_INT)) {
    accel_read();
    JsVar *d = jswrap_puck_accel();
    JsVar *puck = jsvObjectGetChild(execInfo.root, "Puck", 0);
    if (jsvHasChildren(puck))
        jsiQueueObjectCallbacks(puck, JS_EVENT_PREFIX"accel", &d, 1);
    jsvUnLock2(puck, d);
    busy = true;
  }
  return busy;
}
