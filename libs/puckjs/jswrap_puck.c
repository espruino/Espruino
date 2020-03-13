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


Puck.js v2 TODO
---------------

LIS3MDL Magnetometer
LSM6DS3TR Accel / Gyro
PCT2075TP temp sensor
FET pin
New IR output
IR input?
NFC tuning
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
#include "jsflash.h" // for jsfRemoveCodeFromFlash

#define MAG3110_ADDR 0x0E
#define I2C_TIMEOUT 100000

JshI2CInfo i2cMag;
JshI2CInfo i2cAccel;
JshI2CInfo i2cTemp;
bool isPuckV2 = false;

const Pin PUCK_IO_PINS[] = {1,2,4,6,7,8,23,24,28,29,30,31};

// Has the magnetometer been turned on?
bool mag_enabled = false;
int16_t mag_reading[3];  //< magnetometer xyz reading


/* TODO: Use software I2C for this instead. Since we're relying
 * on the internal pullup resistors there might be some gotchas
 * since we force high here for 0.1uS here before going open circuit. */

// Turn magnetometer on and configure
bool mag_on(int milliHz) {
  int reg1 = 0;
  if (milliHz == 80000) reg1 = (0x00)<<3; // 900uA
  else if (milliHz == 40000) reg1 = (0x04)<<3; // 550uA
  else if (milliHz == 20000) reg1 = (0x08)<<3; // 275uA
  else if (milliHz == 10000) reg1 = (0x0C)<<3; // 137uA
  else if (milliHz == 5000) reg1 = (0x10)<<3; // 69uA
  else if (milliHz == 2500) reg1 = (0x14)<<3; // 34uA
  else if (milliHz == 1250) reg1 = (0x18)<<3; // 17uA
  else if (milliHz == 630) reg1 = (0x1C)<<3; // 8uA
  else if (milliHz == 310) reg1 = (0x1D)<<3; // 8uA
  else if (milliHz == 160) reg1 = (0x1E)<<3; // 8uA
  else if (milliHz == 80) reg1 = (0x1F)<<3; // 8uA
  else return false;

  nrf_gpio_pin_set(MAG_PWR);
  nrf_gpio_cfg_output(MAG_PWR);
  nrf_gpio_cfg_input(MAG_INT, NRF_GPIO_PIN_NOPULL);
  wr(MAG_SDA, 1);
  wr(MAG_SCL, 1);
  jshDelayMicroseconds(2000); // 1.7ms from power on to ok

  unsigned char buf[2];
  buf[0] = 0x11;
  buf[1] = 0x80/*AUTO_MRST_EN*/ + 0x20/*RAW*/;
  jsi2cWrite(&i2cMag, MAG3110_ADDR, 2, buf, true);
  reg1 |= 1; // Active bit
  buf[0] = 0x10;
  buf[1] = reg1;
  jsi2cWrite(&i2cMag, MAG3110_ADDR, 2, buf, true);
  return true;
}

// Wait for magnetometer IRQ line to be set
void mag_wait() {
  int timeout = I2C_TIMEOUT*2;
  while (!nrf_gpio_pin_read(MAG_INT) && --timeout);
  if (!timeout) jsExceptionHere(JSET_INTERNALERROR, "Timeout (Magnetometer)");
}

// Read a value
void mag_read() {
  unsigned char buf[6];
  buf[0] = 1;
  jsi2cWrite(&i2cMag, MAG3110_ADDR, 1, buf, false);
  jsi2cRead(&i2cMag, MAG3110_ADDR, 6, buf, true);
  mag_reading[0] = (buf[0]<<8) | buf[1];
  mag_reading[1] = (buf[2]<<8) | buf[3];
  mag_reading[2] = (buf[4]<<8) | buf[5];
}

// Get temperature
int8_t mag_read_temp() {
  unsigned char buf[1];
  buf[0] = 15; // DIE_TEMP
  jsi2cWrite(&i2cMag, MAG3110_ADDR, 1, buf, false);
  jsi2cRead(&i2cMag, MAG3110_ADDR, 1, buf, true);
  return buf[0];
}

// Turn magnetometer off
void mag_off() {
  nrf_gpio_cfg_input(MAG_SDA, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(MAG_SCL, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_pin_clear(MAG_PWR);
  nrf_gpio_cfg_output(MAG_PWR);
}

JsVar *mag_to_xyz(int16_t d[3]) {
  JsVar *obj = jsvNewObject();
  if (!obj) return 0;
  jsvObjectSetChildAndUnLock(obj,"x",jsvNewFromInteger(d[0]));
  jsvObjectSetChildAndUnLock(obj,"y",jsvNewFromInteger(d[1]));
  jsvObjectSetChildAndUnLock(obj,"z",jsvNewFromInteger(d[2]));
  return obj;
}

/*JSON{
    "type": "class",
    "class" : "Puck",
    "ifdef" : "PUCKJS"
}
Class containing [Puck.js's](http://www.puck-js.com) utility functions.
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
    mag_on(80000);
    mag_wait();
    mag_read();
    mag_off();
  }
  return mag_to_xyz(mag_reading);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Puck",
  "name" : "magTemp",
  "ifdef" : "PUCKJS",
  "generate" : "jswrap_puck_magTemp",
  "return" : ["int", "Temperature in degrees C" ]
}
Turn on the magnetometer, take a single temperature reading from the MAG3110 chip, and then turn it off again.

(If the magnetometer is already on, this just returns the last reading obtained)

`E.getTemperature()` uses the microcontroller's temperature sensor, but this uses the magnetometer's.

The reading obtained is an integer (so no decimal places), but the sensitivity is factory trimmed. to 1&deg;C, however the temperature
offset isn't - so absolute readings may still need calibrating.
*/
JsVarInt jswrap_puck_magTemp() {
  JsVarInt t;
  if (!mag_enabled) {
    mag_on(80000);
    mag_wait();
    t = mag_read_temp();
    mag_off();
  } else
    t = mag_read_temp();
  return t;
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

*/
void jswrap_puck_magOn(JsVarFloat hz) {
  if (mag_enabled) {
    jswrap_puck_magOff();
    // wait 1ms for power-off
    jshDelayMicroseconds(1000);
  }
  int milliHz = (int)((hz*1000)+0.5);
  if (milliHz==0) milliHz=630;
  if (!mag_on(milliHz)) {
    jsExceptionHere(JSET_ERROR, "Invalid sample rate %f - must be 80, 40, 20, 10, 5, 2.5, 1.25, 0.63, 0.31, 0.16 or 0.08 Hz", hz);
  }
  jshPinWatch(MAG_INT, true);
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
    jshPinWatch(MAG_INT, false);
    mag_off();
  }
  mag_enabled = false;
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


*/
void jswrap_puck_IR(JsVar *data, Pin cathode, Pin anode) {
  if (!jsvIsIterable(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting an array, got %t", data);
    return;
  }

  if (!jshIsPinValid(anode)) anode = IR_ANODE_PIN;
  if (!jshIsPinValid(cathode)) cathode = IR_CATHODE_PIN;
  jshPinAnalogOutput(cathode, 0.75, 38000, 0);

  JsSysTime time = jshGetSystemTime();
  bool hasPulses = false;
  bool pulsePolarity = true;
  jshPinSetValue(anode, pulsePolarity);

  JsvIterator it;
  jsvIteratorNew(&it, data, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it)) {
    JsVarFloat pulseTime = jsvIteratorGetFloatValue(&it);
    if (hasPulses) jstPinOutputAtTime(time, &anode, 1, pulsePolarity);
    else jshPinSetState(anode, JSHPINSTATE_GPIO_OUT);
    hasPulses = true;
    time += jshGetTimeFromMilliseconds(pulseTime);
    pulsePolarity = !pulsePolarity;
   jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  if (hasPulses) {
    uint32_t d = cathode | anode<<8;
    jstExecuteFn(_jswrap_puck_IR_done, (void*)d, time, 0);
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

static bool selftest_check_pin(Pin pin) {
  unsigned int i;
  bool ok = true;
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 1);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced low\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
    if (PUCK_IO_PINS[i]!=pin)
      jshPinOutput(PUCK_IO_PINS[i], 0);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p shorted low\n", pin);
    ok = false;
  }

  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 0);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLDOWN);
  if (jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced high\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
     if (PUCK_IO_PINS[i]!=pin)
       jshPinOutput(PUCK_IO_PINS[i], 1);
   if (jshPinGetValue(pin)) {
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

*/
bool jswrap_puck_selfTest() {
  unsigned int timeout, i;
  JsVarFloat v;
  bool ok = true;

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
  if (v<0.3 || v>0.65) {
    jsiConsolePrintf("LED1 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(LED2_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED2_PININDEX);
  jshPinSetState(LED2_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.55 || v>0.85) {
    jsiConsolePrintf("LED2 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(LED3_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED3_PININDEX);
  jshPinSetState(LED3_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.65 || v>0.90) {
    jsiConsolePrintf("LED3 pullup voltage out of range (%f) - disconnected?\n", v);
    ok = false;
  }

  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_IN_PULLDOWN);
  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(IR_CATHODE_PIN, 1);
  nrf_delay_ms(1);
  if (jshPinGetValue(IR_ANODE_PIN)) {
    jsiConsolePrintf("IR LED wrong way around/shorted?\n");
    ok = false;
  }

  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_IN_PULLDOWN);
  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(IR_ANODE_PIN, 1);
  nrf_delay_ms(1);
  if (!jshPinGetValue(IR_CATHODE_PIN)) {
    jsiConsolePrintf("IR LED disconnected?\n");
    ok = false;
  }

  jshPinSetState(IR_ANODE_PIN, JSHPINSTATE_GPIO_IN);
  jshPinSetState(IR_CATHODE_PIN, JSHPINSTATE_GPIO_IN);

  mag_on(80000);
  mag_wait();
  mag_read();
  mag_off();
  mag_enabled = false;
  if (mag_reading[0]==-1 && mag_reading[1]==-1 && mag_reading[2]==-1) {
    jsiConsolePrintf("Magnetometer not working?\n");
    ok = false;
  }

  jshPinSetState(CAPSENSE_TX_PIN, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(CAPSENSE_RX_PIN, JSHPINSTATE_GPIO_IN);
  jshPinSetValue(CAPSENSE_TX_PIN, 1);
  nrf_delay_ms(1);
  if (!jshPinGetValue(CAPSENSE_RX_PIN)) {
    jsiConsolePrintf("Capsense resistor disconnected? (pullup)\n");
    ok = false;
  }
  jshPinSetValue(CAPSENSE_TX_PIN, 0);
  nrf_delay_ms(1);
  if (jshPinGetValue(CAPSENSE_RX_PIN)) {
    jsiConsolePrintf("Capsense resistor disconnected? (pulldown)\n");
    ok = false;
  }
  jshPinSetState(CAPSENSE_TX_PIN, JSHPINSTATE_GPIO_IN);


  ok &= selftest_check_pin(1);
  ok &= selftest_check_pin(2);
  ok &= selftest_check_pin(6);
  ok &= selftest_check_pin(7);
  ok &= selftest_check_pin(8);
  ok &= selftest_check_pin(28);
  ok &= selftest_check_pin(29);
  ok &= selftest_check_pin(30);
  ok &= selftest_check_pin(31);

  for (i=0;i<sizeof(PUCK_IO_PINS)/sizeof(Pin);i++)
    jshPinSetState(PUCK_IO_PINS[i], JSHPINSTATE_GPIO_IN);


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
  jshPinSetValue(MAG_PIN_PWR, 1);
  jshPinSetState(i2cMag.MAG_PIN_PWR, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(i2cMag.pinSCL, 1);
  jshPinSetState(i2cMag.pinSCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(i2cMag.pinSDA, 1);
  jshPinSetState(i2cMag.pinSDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  // MAG3110 WHO_AM_I
  unsigned char buf[2];
  buf[0] = 0x07;
  jsi2cWrite(&i2cMag, MAG3110_ADDR, 1, buf, false);
  jsi2cRead(&i2cMag, MAG3110_ADDR, 1, buf, true);
  if (buf[0]!=0xC4) {
    // Not found, check for LIS3MDL - Puck.js v2
    buf[0] = 0x0F;
    jsi2cWrite(&i2cMag, MAG3110_ADDR, 1, buf, false);
    jsi2cRead(&i2cMag, MAG3110_ADDR, 1, buf, true);
    if (buf[0] == 0b00111101) {
      // all good, MAG3110 - Puck.js v2
      isPuckV2 = true;
    } else {
      // ERROR - magnetometer not found!
    }
  } else {
    // all good, MAG3110 - Puck.js v1
  }
  // Power off
  jshPinSetValue(MAG_PIN_PWR, 0);


  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;
  if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    firstStart = false; // don't do it during a software reset - only first hardware reset
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
  /* jshPinWatch should mean that we wake up whenever a new
   * magnetometer reading is ready */
  if (mag_enabled && nrf_gpio_pin_read(MAG_INT)) {
    mag_read();
    JsVar *xyz = mag_to_xyz(mag_reading);
    JsVar *puck = jsvObjectGetChild(execInfo.root, "Puck", 0);
    if (jsvHasChildren(puck))
        jsiQueueObjectCallbacks(puck, JS_EVENT_PREFIX"mag", &xyz, 1);
    jsvUnLock2(puck, xyz);
    return true; // don't sleep - handle this now
  }
  return false;
}
