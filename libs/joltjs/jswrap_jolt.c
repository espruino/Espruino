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
 * Contains JavaScript interface for Jolt.js
 * ----------------------------------------------------------------------------

FIXME:

See https://docs.google.com/document/d/1Tw7fUkpj9dwBYASBCX9TjPcpgdqEw_7VL6MSRqVUUXY

 */


#include "jswrap_jolt.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jsflags.h"
#include "jstimer.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf5x_utils.h"
#include "ble_gap.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h"
#include "app_timer.h"

/// IO pins on Qwiic headers - check for shorts
const Pin JOLT_IO_PINS[] = {
  3,29,7,  // q0
  2,31,27, // q1
  44,45,36,43, // q2
  39,38,37,42  // q3
};

typedef enum {
  JDM_AUTO,    // Automatic (JDM_OUTPUT whenever a pin is set as output mode)
  JDM_AUTO_ON, // Automatic (currently on)
  JDM_OFF,
  JDM_OUTPUT, // Independent bridge
  JDM_MOTOR,   // 4 pin
  JDM_UNKNOWN  // just used when parsing to show we don't know
} JoltDriverMode;
#define DRIVERCOUNT 2
JoltDriverMode driverMode[DRIVERCOUNT];

/*JSON{
  "type": "class",
  "class" : "Jolt",
  "ifdef" : "JOLTJS"
}
Class containing utility functions for the [Jolt.js Smart Bluetooth driver](http://www.espruino.com/Jolt.js)
*/

void setJoltProperty(const char *name, JsVar* prop) {
  JsVar *jolt = jsvObjectGetChildIfExists(execInfo.root, "Jolt");
  if (!jolt) return;
  jsvObjectSetChild(jolt, name, prop);
  jsvUnLock(jolt);
}

/*JSON{
  "type" : "variable",
  "name" : "Q0",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q0",
  "return" : ["JsVar","An object containing the pins for the Q0 connector on Jolt.js `{sda,scl,fet}`"],
  "return_object" : "Qwiic"
}
`Q0` and `Q1` Qwiic connectors can have their power controlled by a 500mA FET (`Q0.fet`) which switches GND.

The `sda` and `scl` pins on this port are also analog inputs - use `analogRead(Q0.sda)`/etc

To turn this connector on run `Q0.setPower(1)`
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Jolt",
  "name" : "Q0",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q0",
  "return" : ["JsVar","An object containing the pins for the Q0 connector on Jolt.js `{sda,scl,fet}`"],
  "return_object" : "Qwiic"
}
`Q0` and `Q1` Qwiic connectors can have their power controlled by a 500mA FET (`Jolt.Q0.fet`) which switches GND.

The `sda` and `scl` pins on this port are also analog inputs - use `analogRead(Jolt.Q0.sda)`/etc

To turn this connector on run `Jolt.Q0.setPower(1)`
*/
JsVar *jswrap_jolt_q0() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC0_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC0_PIN_SCL));
  jsvObjectSetChildAndUnLock(o, "fet", jsvNewFromPin(QWIIC0_PIN_FET));
  setJoltProperty("Q0", o);
  return o;
}

/*JSON{
  "type" : "variable",
  "name" : "Q1",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q1",
  "return" : ["JsVar","An object containing the pins for the Q1 connector on Jolt.js `{sda,scl,fet}`"],
  "return_object" : "Qwiic"
}
`Q0` and `Q1` Qwiic connectors can have their power controlled by a 500mA FET (`Q1.fet`) which switches GND.

The `sda` and `scl` pins on this port are also analog inputs - use `analogRead(Q1.sda)`/etc

To turn this connector on run `Q1.setPower(1)`
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Jolt",
  "name" : "Q1",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q1",
  "return" : ["JsVar","An object containing the pins for the Q1 connector on Jolt.js `{sda,scl,fet}`"],
  "return_object" : "Qwiic"
}
`Q0` and `Q1` Qwiic connectors can have their power controlled by a 500mA FET  (`Jolt.Q1.fet`) which switches GND.

The `sda` and `scl` pins on this port are also analog inputs - use `analogRead(Jolt.Q1.sda)`/etc

To turn this connector on run `Jolt.Q1.setPower(1)`
*/
JsVar *jswrap_jolt_q1() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC1_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC1_PIN_SCL));
  jsvObjectSetChildAndUnLock(o, "fet", jsvNewFromPin(QWIIC1_PIN_FET));
  setJoltProperty("Q1", o);
  return o;
}

/*JSON{
  "type" : "variable",
  "name" : "Q2",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q2",
  "return" : ["JsVar","An object containing the pins for the Q2 connector on Jolt.js `{sda,scl,gnd,vcc}`"],
  "return_object" : "Qwiic"
}
`Q2` and `Q3` have all 4 pins connected to Jolt.js's GPIO (including those usually used for power).
As such only around 8mA of power can be supplied to any connected device.

To use this as a normal Qwiic connector, run `Q2.setPower(1)`
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Jolt",
  "name" : "Q2",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q2",
  "return" : ["JsVar","An object containing the pins for the Q2 connector on Jolt.js `{sda,scl,gnd,vcc}`"],
  "return_object" : "Qwiic"
}
`Q2` and `Q3` have all 4 pins connected to Jolt.js's GPIO (including those usually used for power).
As such only around 8mA of power can be supplied to any connected device.

To use this as a normal Qwiic connector, run `Jolt.Q2.setPower(1)`
*/
JsVar *jswrap_jolt_q2() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC2_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC2_PIN_SCL));
  jsvObjectSetChildAndUnLock(o, "gnd", jsvNewFromPin(QWIIC2_PIN_GND));
  jsvObjectSetChildAndUnLock(o, "vcc", jsvNewFromPin(QWIIC2_PIN_VCC));
  setJoltProperty("Q2", o);
  return o;
}

/*JSON{
  "type" : "variable",
  "name" : "Q3",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q3",
  "return" : ["JsVar","An object containing the pins for the Q3 connector on Jolt.js `{sda,scl,gnd,vcc}`"],
  "return_object" : "Qwiic"
}
`Q2` and `Q3` have all 4 pins connected to Jolt.js's GPIO (including those usually used for power).
As such only around 8mA of power can be supplied to any connected device.

To use this as a normal Qwiic connector, run `Q3.setPower(1)`
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Jolt",
  "name" : "Q3",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_q3",
  "return" : ["JsVar","An object containing the pins for the Q3 connector on Jolt.js `{sda,scl,gnd,vcc}`"],
  "return_object" : "Qwiic"
}
`Q2` and `Q3` have all 4 pins connected to Jolt.js's GPIO (including those usually used for power).
As such only around 8mA of power can be supplied to any connected device.

To use this as a normal Qwiic connector, run `Jolt.Q3.setPower(1)`
*/
JsVar *jswrap_jolt_q3() {
  JsVar *o = jspNewObject(0, "Qwiic");
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o, "sda", jsvNewFromPin(QWIIC3_PIN_SDA));
  jsvObjectSetChildAndUnLock(o, "scl", jsvNewFromPin(QWIIC3_PIN_SCL));
  jsvObjectSetChildAndUnLock(o, "gnd", jsvNewFromPin(QWIIC3_PIN_GND));
  jsvObjectSetChildAndUnLock(o, "vcc", jsvNewFromPin(QWIIC3_PIN_VCC));
  setJoltProperty("Q3", o);
  return o;
}


/// Actually sets the driver mode
static void _jswrap_jolt_setDriverMode_int(int driver, JoltDriverMode dMode) {
  if (driver==0) {
    if (dMode == JDM_OUTPUT)
      jshPinSetState(DRIVER0_PIN_MODE, JSHPINSTATE_GPIO_IN); // Z for independent bridge
    else
      jshPinOutput(DRIVER0_PIN_MODE, 0); // 0 for 4 pin interface
    jshPinOutput(DRIVER0_PIN_NSLEEP, dMode != JDM_OFF);
  } else if (driver==1) {
    if (dMode == JDM_OUTPUT)
      jshPinSetState(DRIVER1_PIN_MODE, JSHPINSTATE_GPIO_IN); // Z for independent bridge
    else
      jshPinOutput(DRIVER1_PIN_MODE, 0); // 0 for 4 pin interface
    jshPinOutput(DRIVER1_PIN_NSLEEP, dMode != JDM_OFF);
  } else assert(0);
}

/// Called when drivermode=auto to figure out when to turn the driver on
static void _jswrap_jolt_autoDriverMode(int driver) {
  assert((driver>=0) && (driver<2));
  // check the state of the 4 pins
  bool pinIsOutput = false; // set if >0 pins are outputs
  Pin pinBase = JSH_PORTH_OFFSET + driver*4;
  for (int i=0;i<4;i++) {
    uint32_t ipin = (uint32_t)pinInfo[pinBase + i].pin;
    NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&ipin);
    if ((reg->PIN_CNF[ipin] & GPIO_PIN_CNF_DIR_Msk) ==
        (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos))
      pinIsOutput = true;
  }
  // update driver state if different
  bool isCurrentlyOn = driverMode[driver] == JDM_AUTO_ON;
  if (pinIsOutput != isCurrentlyOn) {
    //jsiConsolePrintf("autoDriverMode %d %d\n", driver, pinIsOutput);
    if (pinIsOutput) driverMode[driver] = JDM_AUTO_ON;
    else driverMode[driver] = JDM_AUTO;
    _jswrap_jolt_setDriverMode_int(driver, pinIsOutput ? JDM_OUTPUT : JDM_OFF);
  }
}


/*JSON{
  "type" : "staticmethod",
  "class" : "Jolt",
  "name" : "setDriverMode",
  "ifdef" : "JOLTJS",
  "generate" : "jswrap_jolt_setDriverMode",
  "params" : [
    ["driver","int","The number of the motor driver (0 or 1)"],
    ["mode","JsVar","The mode of the motor driver (see below)"]
  ]
}
Sets the mode of the motor drivers. Jolt.js has two motor drivers,
one (`0`) for outputs H0..H3, and one (`1`) for outputs H4..H7. They
can be controlled independently.

Mode can be:

* `undefined` / `false` / `"off"` - the motor driver is off, all motor driver pins are open circuit (the motor driver still has a ~2.5k pulldown to GND)
* `"auto"` - (default) - if any pin in the set of 4 pins (H0..H3, H4..H7) is set as an output, the driver is turned on. Eg `H0.set()` will
turn the driver on with a high output, `H0.reset()` will pull the output to GND and `H0.read()` (or `H0.mode("input")` to set the state explicitly) is needed to
turn the motor driver off.
* `true` / `"output"` - **[recommended]** driver is set to "Independent bridge" mode. All 4 outputs in the bank are enabled
* `"motor"` - driver is set to "4 pin interface" mode where pins are paired up (H0+H1, H2+H3, etc). If both
in a pair are 0 the output is open circuit (motor coast), if both are 1 both otputs are 0 (motor brake), and
if both are different, those values are on the output:

`output`/`auto` mode:

| H0 | H1 | Out 0 | Out 1 |
|----|----|-------|-------|
| 0  | 0  | Low   | Low   |
| 0  | 1  | Low   | High  |
| 1  | 0  | High  | Low   |
| 1  | 1  | High  | High  |

`motor` mode

| H0 | H1 | Out 0 | Out 1 |
|----|----|-------|-------|
| 0  | 0  | Open  | Open  |
| 0  | 1  | Low   | High  |
| 1  | 0  | High  | Low   |
| 1  | 1  | Low   | Low   |


*/

void jswrap_jolt_setDriverMode(int driver, JsVar *mode) {
  if (driver<0 || driver>1) {
    jsExceptionHere(JSET_ERROR, "Invalid driver %d", driver);
    return;
  }
  JoltDriverMode dMode = JDM_UNKNOWN;
  if (jsvIsBoolean(mode) || jsvIsUndefined(mode)) {
    dMode = jsvGetBool(mode) ? JDM_OUTPUT : JDM_OFF;
  } else if (jsvIsString(mode)) {
    if (jsvIsStringEqual(mode, "off")) dMode = JDM_OFF;
    else if (jsvIsStringEqual(mode, "auto")) dMode = JDM_AUTO;
    else if (jsvIsStringEqual(mode, "output")) dMode = JDM_OUTPUT;
    else if (jsvIsStringEqual(mode, "motor")) dMode = JDM_MOTOR;
  }
  if (dMode == JDM_UNKNOWN) {
    jsExceptionHere(JSET_ERROR, "Unknown driver mode %q", mode);
    return;
  }
  driverMode[driver] = dMode;
  if (dMode==JDM_AUTO) {
    _jswrap_jolt_setDriverMode_int(driver, JDM_OFF); // turn off
    _jswrap_jolt_autoDriverMode(driver); // now let autoDriverMode update
  } else
    _jswrap_jolt_setDriverMode_int(driver, dMode);
}

static void jswrap_jolt_setDriverMode_(int driver, bool mode) {
  JsVar *b = jsvNewFromBool(mode);
  jswrap_jolt_setDriverMode(driver,b);
  jsvUnLock(b);
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
  for (i=0;i<sizeof(JOLT_IO_PINS)/sizeof(Pin);i++)
    if (JOLT_IO_PINS[i]!=pin)
      jshPinOutput(JOLT_IO_PINS[i], 0);
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
  for (i=0;i<sizeof(JOLT_IO_PINS)/sizeof(Pin);i++)
     if (JOLT_IO_PINS[i]!=pin)
       jshPinOutput(JOLT_IO_PINS[i], 1);
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
    "class" : "Jolt",
    "name" : "selfTest",
    "ifdef" : "JOLTJS",
    "generate" : "jswrap_jolt_selfTest",
    "return" : ["bool", "True if the self-test passed" ]
}
Run a self-test, and return true for a pass. This checks for shorts between
pins, so your Jolt shouldn't have anything connected to it.

**Note:** This self-test auto starts if you hold the button on your Jolt down
while inserting the battery, leave it pressed for 3 seconds (while the green LED
is lit) and release it soon after all LEDs turn on. 5 red blinks is a fail, 5
green is a pass.

If the self test fails, it'll set the Jolt.js Bluetooth advertising name to
`Jolt.js !ERR` where ERR is a 3 letter error code.
*/
bool _jswrap_jolt_selfTest(bool advertisePassOrFail) {
  unsigned int timeout;
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

  v = jshReadVRef();
  if (v<3.2 || v>3.4) {
    if (!err[0]) strcpy(err,"VRG");
    jsiConsolePrintf("VREG out of range (%fv)\n", v);
    ok = false;
  }

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


  jswrap_jolt_setDriverMode_(0,true);
  jswrap_jolt_setDriverMode_(1,true);
  // test every pin on the motor driver one at a time
  for (int i=0;i<8;i++) {
    for (int p=0;p<8;p++)
      jshPinSetValue(JSH_PORTH_OFFSET+p, p == i);
    nrf_delay_ms(5);
    // we can only read H0/2/4/8
    for (int p=0;p<8;p+=2) {
      v = jshPinAnalog(JSH_PORTH_OFFSET+p);
      if (i==p) {
        if (v<2) {
          if (!err[0]) { strcpy(err,"OLx"); err[2]='0'+i; }
          jsiConsolePrintf("H%d low (%f) when H%d set\n", p, v, i);
          ok = false;
        }
      } else {
        if (v>1) {
          if (!err[0]) { strcpy(err,"OHx"); err[2]='0'+i; }
          jsiConsolePrintf("H%d high (%f) when H%d set\n", p, v, i);
          ok = false;
        }
      }
    }
  }
  // all drivers off
  for (int p=0;p<8;p++)
    jshPinSetValue(JSH_PORTH_OFFSET+p, 0);
  jswrap_jolt_setDriverMode_(0,false);
  jswrap_jolt_setDriverMode_(1,false);

  // TODO: what about checking motor drivers if there's a short using nFault pins?
  for (unsigned int i=0;i<sizeof(JOLT_IO_PINS)/sizeof(Pin);i++)
    selftest_check_pin(JOLT_IO_PINS[i], err);
  for (unsigned int i=0;i<sizeof(JOLT_IO_PINS)/sizeof(Pin);i++)
    jshPinSetState(JOLT_IO_PINS[i], JSHPINSTATE_GPIO_IN);

  if (err[0] || advertisePassOrFail) {
    char deviceName[BLE_GAP_DEVNAME_MAX_LEN];
    if (advertisePassOrFail) {
      if (err[0]) {
        strcpy(deviceName,"FAIL ");
        strcat(deviceName,err);
      } else {
        strcpy(deviceName,"PASS");
      }
    } else {
      strcpy(deviceName,"Jolt.js !");
      strcat(deviceName,err);
    }
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    sd_ble_gap_device_name_set(&sec_mode,
                              (const uint8_t *)deviceName,
                              strlen(deviceName));
    jsiConsolePrintf("Error code %s\n",err);
  }

  return ok;
}
bool jswrap_jolt_selfTest() {
  return _jswrap_jolt_selfTest(false);
}

/*JSON{
  "type" : "hwinit",
  "generate" : "jswrap_jolt_hwinit"
}*/
void jswrap_jolt_hwinit() {
  // ensure motor drivers are off
  jshPinOutput(DRIVER0_PIN_NSLEEP, 0);
  jshPinOutput(DRIVER1_PIN_NSLEEP, 0);
  jshPinOutput(DRIVER0_PIN_MODE, 0);
  jshPinOutput(DRIVER1_PIN_MODE, 0);
  jshPinOutput(DRIVER0_PIN_TRQ, 0);
  jshPinOutput(DRIVER1_PIN_TRQ, 0);
  jshPinSetState(DRIVER0_PIN_NFAULT, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(DRIVER1_PIN_NFAULT, JSHPINSTATE_GPIO_IN_PULLUP);
  driverMode[0] = JDM_AUTO;
  driverMode[1] = JDM_AUTO;
  // set all outputs to 0 by default
  jshPinOutput(DRIVER0_PIN_D0, 0);
  jshPinOutput(DRIVER0_PIN_D1, 0);
  jshPinOutput(DRIVER0_PIN_D2, 0);
  jshPinOutput(DRIVER0_PIN_D3, 0);
  jshPinOutput(DRIVER1_PIN_D0, 0);
  jshPinOutput(DRIVER1_PIN_D1, 0);
  jshPinOutput(DRIVER1_PIN_D2, 0);
  jshPinOutput(DRIVER1_PIN_D3, 0);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_jolt_init"
}*/
void jswrap_jolt_init() {

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  bool firstStart = jsiStatus & JSIS_FIRST_BOOT; // is this the first time jswrapjolt_init was called?
  bool firstRunAfterFlash = false;
  uint32_t firstStartFlagAddr = FLASH_SAVED_CODE_START-4;
  if (firstStart) {
    // check the 4 bytes *right before* our saved code. If these are 0xFFFFFFFF
    // then we have just been programmed...
    uint32_t buf;
    jshFlashRead(&buf, firstStartFlagAddr, 4);
    if (buf==0xFFFFFFFF) {
      firstRunAfterFlash = true;
    }
  }

  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || firstRunAfterFlash)) {
    // don't do it during a software reset - only first hardware reset
    // if we're doing our first run after being flashed with new firmware, we set the advertising name
    // up to say PASS or FAIL, to work with the factory test process.
    bool result = _jswrap_jolt_selfTest(firstRunAfterFlash);
    // if we passed, set the flag in flash so we don't self-test again
    if (firstRunAfterFlash && result) {
      uint32_t buf = 0;
      bool oldFlashStatus = jsfGetFlag(JSF_UNSAFE_FLASH);
      jsfSetFlag(JSF_UNSAFE_FLASH, true);
      jshFlashWrite(&buf, firstStartFlagAddr, 4);
      jsfSetFlag(JSF_UNSAFE_FLASH, oldFlashStatus);
    }
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
  "generate" : "jswrap_jolt_kill"
}*/
void jswrap_jolt_kill() {
  // ensure motor drivers are off
  jshPinOutput(DRIVER0_PIN_NSLEEP, 0);
  jshPinOutput(DRIVER1_PIN_NSLEEP, 0);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_jolt_idle"
}*/
bool jswrap_jolt_idle() {
  bool busy = false;
  // handle automatic driver mode
  for (int i=0;i<DRIVERCOUNT;i++) {
    if (driverMode[i]==JDM_AUTO || driverMode[i]==JDM_AUTO_ON)
      _jswrap_jolt_autoDriverMode(i);
  }
  return busy;
}
