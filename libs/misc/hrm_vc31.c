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
 * VC31 heart rate sensor
 * ----------------------------------------------------------------------------
 */

#include "hrm.h"
#include "jsutils.h"
#include "platform_config.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jsi2c.h"


extern JshI2CInfo i2cHRM;
HrmCallback hrmCallback;

//Read only.
#define VC31_DEV_ID        0x00
#define VC31_STATUS        0x01
#define VC31_PPG           0x02
#define VC31_CURRENT       0x04
#define VC31_PRE           0x06
#define VC31_PS            0x08
#define VC31_ENV           0x0A

//Read or write.
#define VC31_CTRL          0x20
#define VC31_PPG_DIV       0x21
#define VC31_PS_DIV        0x23
#define VC31_TIA_WAIT      0x24
#define VC31_AMP_WAIT      0x25
#define VC31_GREEN_WAIT    0x26
#define VC31_GREEN_IR_GAP  0x27
#define VC31_IR_WAIT       0x28
#define VC31_GREEN_ADJ     0x29

#define VC31_CTRL_RESET_VALUE       0x03 // Default value for VC31_CTRL
#define VC31_CTRL_MASK              0xF7 // Mask for VC31_CTRL
#define VC31_CTRL_WORK_MODE         0x80 // Enter work mode. Start sample
#define VC31_CTRL_ENABLE_PPG        0x40 // Enable green led sample
#define VC31_CTRL_ENABLE_PRE        0x20 // Enable pre sample
#define VC31_CTRL_LDO_EXTERN        0x10 // External LDO
#define VC31_CTRL_INT_DIR_RISING    0x03 // IRQ trigger on raising edge
#define VC31_CTRL_OPA_GAIN_12_5     0x00 // OPA3 Gain 12.5.
#define VC31_CTRL_OPA_GAIN_25       0x01 // OPA3 Gain 25.
#define VC31_CTRL_OPA_GAIN_50       0x02 // OPA3 Gain 50.
#define VC31_CTRL_OPA_GAIN_100      0x03 // OPA3 Gain 100.

#define VC31_PPG_DIV_10_HZ          0x0CC5
#define VC31_PPG_DIV_12_5_HZ        0x0A35
#define VC31_PPG_DIV_25_HZ          0x0516
#define VC31_PPG_DIV_50_HZ          0x0287
#define VC31_PPG_DIV_100_HZ         0x013F
#define VC31_PPG_DIV_1000_HZ        0x0018

#define VC31_STATUS_D_ENV_OK        0x10
#define VC31_STATUS_D_PS_OK         0x08
#define VC31_STATUS_D_PRE_OK        0x04
#define VC31_STATUS_D_CUR_OK        0x02
#define VC31_STATUS_D_PPG_OK        0x01

#define VC31_GREEN_ADJ_RESET_VALUE  0x0000          // Default value for VC31_GREEN_ADJ.
#define VC31_GREEN_ADJ_MASK         0xFFFF          // Mask for VC31_GREEN_ADJ.
#define VC31_GREEN_ADJ_ENABLE       0x8000          // Enable current adjust.
#define VC31_GREEN_ADJ_DISABLE      0               // Disable current adjust.
#define VC31_GREEN_ADJ_UP           0x4000          // Turn up the current.
#define VC31_GREEN_ADJ_DOWN         0               // Turn down the current.
#define VC31_GREEN_ADJ_VALUE_MASK   0x3FFF          // Mask for VC31_ADJ_CUR value.


#define VC31_ADJUST_FACTOR_INCREASE   22       // 1.4 << 4 = 22.4
#define VC31_ADJUST_FACTOR_DECREASE   11       // 0.7 << 4 = 11.2
#define VC31_ADJUST_FACTOR_MAX        1536000
#define VC31_ADJUST_FACTOR_MIN        15360
#define VC31_ADJUST_STEP_MAX          1000
#define VC31_ADJUST_STEP_MIN          2

#define VC31_ENV_LIMIT                2500
#define VC31_PS_LIMIT                 150//350

#define VC31_PPG_LIMIT_L              200
#define VC31_PPG_LIMIT_H              3900
#define VC31_CURRENT_LIMIT_L          12
#define VC31_CURRENT_LIMIT_H          1000
#define VC31_UNWEAR_CNT               3
#define VC31_ISWEAR_CNT               1

#define VC31_PPG_ADJUSTED             0x1000
#define VC31_PPG_MASK                 0x0FFF

typedef enum
{
    AdjustDirection_Null    = 0,
    AdjustDirection_Up      = 1,
    AdjustDirection_Down    = 2,
} VC31AdjustDirection;

typedef struct
{
    VC31AdjustDirection directionLast;
    VC31AdjustDirection directionLastBefore;
    int32_t step;
} VC31AdjustInfo_t;

typedef struct {
  uint8_t status;
  uint16_t ppgValue;
  uint16_t currentValue;
  uint16_t preValue;
  uint16_t psValue;
  uint16_t envValue;
  VC31AdjustInfo_t adjustInfo;
  uint8_t raw[11];
  int ppgOffset; ///< when we adjust the PPG settings, we tweak ppgOffset to ensure there's no big 'jump'
} PACKED_FLAGS VC31Info;

VC31Info vcInfo;


static void vc31_w(uint8_t reg, uint8_t data) {
  uint8_t buf[2] = {reg, data};
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, 2, buf, true);
}

static uint8_t vc31_r(uint8_t reg) {
  uint8_t buf[1] = {reg};
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, 1, buf, false);
  jsi2cRead(&i2cHRM, HEARTRATE_ADDR, 1, buf, true);
  return buf[0];
}
static void vc31_rx(uint8_t reg, uint8_t *data, int cnt) {
  uint8_t buf[1] = {reg};
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, 1, buf, false);
  jsi2cRead(&i2cHRM, HEARTRATE_ADDR, cnt, data, true);
}

static void vc31_w16(uint8_t reg, uint16_t data) {
  uint8_t buf[3] = {reg, data&0xFF, data>>8};
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, 3, buf, true);
}


static void vc31_adjust() {
  uint16_t adjustParam = 0;
  uint32_t adjustStep = 0;

  vcInfo.currentValue += 10;

  if (vcInfo.ppgValue > VC31_PPG_LIMIT_H) {
    if (vcInfo.currentValue < VC31_CURRENT_LIMIT_H) {
      if (vcInfo.adjustInfo.directionLast == AdjustDirection_Down) {
        vcInfo.adjustInfo.step *= VC31_ADJUST_FACTOR_DECREASE;
      } else if ((vcInfo.adjustInfo.directionLast == AdjustDirection_Up)
          && (vcInfo.adjustInfo.directionLastBefore == AdjustDirection_Up)) {
        vcInfo.adjustInfo.step *= VC31_ADJUST_FACTOR_INCREASE;
      } else {
        vcInfo.adjustInfo.step *= 16;
      }
      vcInfo.adjustInfo.step = vcInfo.adjustInfo.step >> 4;
      vcInfo.adjustInfo.step =
          (vcInfo.adjustInfo.step > VC31_ADJUST_FACTOR_MAX) ?
              VC31_ADJUST_FACTOR_MAX : vcInfo.adjustInfo.step;
      vcInfo.adjustInfo.step =
          (vcInfo.adjustInfo.step < VC31_ADJUST_FACTOR_MIN) ?
              VC31_ADJUST_FACTOR_MIN : vcInfo.adjustInfo.step;
      adjustStep = (vcInfo.adjustInfo.step / (3072 - vcInfo.currentValue));
      adjustStep =
          (adjustStep <= VC31_ADJUST_STEP_MIN) ?
              VC31_ADJUST_STEP_MIN : adjustStep;
      adjustStep =
          (adjustStep >= VC31_ADJUST_STEP_MAX) ?
              VC31_ADJUST_STEP_MAX : adjustStep;
      adjustParam = (uint16_t)(
          adjustStep) | VC31_GREEN_ADJ_ENABLE | VC31_GREEN_ADJ_UP;
      vc31_w16(VC31_GREEN_ADJ, adjustParam);
      vcInfo.ppgValue |= VC31_PPG_ADJUSTED;
    }
    vcInfo.adjustInfo.directionLastBefore = vcInfo.adjustInfo.directionLast;
    vcInfo.adjustInfo.directionLast = AdjustDirection_Up;
  } else if (vcInfo.ppgValue < VC31_PPG_LIMIT_L) {
    //if (vcInfo.currentValue > VC31_CURRENT_LIMIT_L)
    {
      if (vcInfo.adjustInfo.directionLast == AdjustDirection_Up) {
        vcInfo.adjustInfo.step *= VC31_ADJUST_FACTOR_DECREASE;
      } else if ((vcInfo.adjustInfo.directionLast == AdjustDirection_Down)
          && (vcInfo.adjustInfo.directionLastBefore == AdjustDirection_Down)) {
        vcInfo.adjustInfo.step *= VC31_ADJUST_FACTOR_INCREASE;
      } else {
        vcInfo.adjustInfo.step *= 16;
      }
      vcInfo.adjustInfo.step = vcInfo.adjustInfo.step >> 4;
      vcInfo.adjustInfo.step =
          (vcInfo.adjustInfo.step > VC31_ADJUST_FACTOR_MAX) ?
              VC31_ADJUST_FACTOR_MAX : vcInfo.adjustInfo.step;
      vcInfo.adjustInfo.step =
          (vcInfo.adjustInfo.step < VC31_ADJUST_FACTOR_MIN) ?
              VC31_ADJUST_FACTOR_MIN : vcInfo.adjustInfo.step;
      adjustStep = (vcInfo.adjustInfo.step / (1024 + vcInfo.currentValue));
      adjustStep =
          (adjustStep <= VC31_ADJUST_STEP_MIN) ?
              VC31_ADJUST_STEP_MIN : adjustStep;
      adjustStep =
          (adjustStep >= VC31_ADJUST_STEP_MAX) ?
              VC31_ADJUST_STEP_MAX : adjustStep;
      adjustParam = (uint16_t)(
          adjustStep) | VC31_GREEN_ADJ_ENABLE | VC31_GREEN_ADJ_DOWN;
      vc31_w16(VC31_GREEN_ADJ, adjustParam);
      vcInfo.ppgValue |= VC31_PPG_ADJUSTED;
    }
    vcInfo.adjustInfo.directionLastBefore = vcInfo.adjustInfo.directionLast;
    vcInfo.adjustInfo.directionLast = AdjustDirection_Down;
  } else {
    vcInfo.adjustInfo.directionLastBefore = vcInfo.adjustInfo.directionLast;
    vcInfo.adjustInfo.directionLast = AdjustDirection_Null;
  }
}

void vc31_irqhandler(bool state, IOEventFlags flags) {
  if (!state || !hrmCallback) return;

  // Have we adjusted settings since this value?
  uint16_t lastPPG = vcInfo.ppgValue;

  uint8_t *buf = vcInfo.raw;
  vc31_rx(VC31_STATUS, buf, 11);
  vcInfo.status = buf[0];
  vcInfo.ppgValue = (buf[2] << 8) | buf[1];
  vcInfo.currentValue = ((buf[4] << 8) | buf[3]) + 10;
  vcInfo.preValue = (buf[6] << 8) | buf[5];
  vcInfo.psValue = (buf[8] << 8) | buf[7];
  vcInfo.envValue = (buf[10] << 8) | buf[9];
  if (vcInfo.status & VC31_STATUS_D_PPG_OK)
    vc31_adjust();
  // wear status?

  if (lastPPG & VC31_PPG_ADJUSTED) {
    // did we adjust settings last time? if so, tweak ppgOffset to ensure that this value is the same as last
    vcInfo.ppgOffset += (lastPPG & VC31_PPG_MASK) - (vcInfo.ppgValue & VC31_PPG_MASK);
  }
  // sanity check ppgOffset to ensure we stay in range
  int value = (vcInfo.ppgValue & VC31_PPG_MASK) + vcInfo.ppgOffset;
  if (value > 511) { vcInfo.ppgOffset -= value-511; value=511; }
  else if (value < 0) { vcInfo.ppgOffset += -value; value = 0; }
  hrmCallback(value>>2); // stay in 0..127 range
}

static void vc31_watch_on() {
  jshSetPinShouldStayWatched(HEARTRATE_PIN_INT,true);
  IOEventFlags channel = jshPinWatch(HEARTRATE_PIN_INT, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, vc31_irqhandler);
}

static void vc31_watch_off() {
  jshPinWatch(HEARTRATE_PIN_INT, false);
  jshSetPinShouldStayWatched(HEARTRATE_PIN_INT,false);
}

void hrm_sensor_on(HrmCallback callback) {
  hrmCallback = callback;
  jshDelayMicroseconds(1000); // wait for HRM to boot
  //if (vc31_r(0)!=0x11) jsiConsolePrintf("VC31 WHO_AM_I failed");
  vc31_w(VC31_GREEN_WAIT, 0xb4);
  vc31_w(VC31_TIA_WAIT, 0x3c);
  vc31_w(VC31_PS_DIV, 0x09);
  vc31_w(VC31_IR_WAIT, 0xa0);
  vc31_w16(VC31_PPG_DIV, VC31_PPG_DIV_100_HZ);
  vc31_w(VC31_GREEN_IR_GAP, 0x20);
  vc31_w(VC31_AMP_WAIT, 0x14);
  uint8_t ctrl = VC31_CTRL_OPA_GAIN_25 | VC31_CTRL_ENABLE_PPG | VC31_CTRL_ENABLE_PRE |
                 VC31_CTRL_WORK_MODE | VC31_CTRL_INT_DIR_RISING;
  vc31_w(VC31_CTRL, ctrl);

  memset(&vcInfo, 0, sizeof(vcInfo));
  vcInfo.ppgOffset = 0;
  vcInfo.adjustInfo.step  = 307200;
  vcInfo.adjustInfo.directionLastBefore = AdjustDirection_Null;
  vcInfo.adjustInfo.directionLast = AdjustDirection_Null;

  vc31_watch_on();

  //vc31_w16(VC31_GREEN_ADJ, 0xe8c3);
}

void hrm_sensor_off() {
  vc31_watch_off();
  vc31_w16(VC31_GREEN_ADJ, 0);
  uint8_t ctrl = VC31_CTRL_OPA_GAIN_25 | VC31_CTRL_ENABLE_PRE | VC31_CTRL_INT_DIR_RISING;
  vc31_w(VC31_CTRL, ctrl);
  hrmCallback = NULL;
}

JsVar *hrm_sensor_getJsVar() {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o,"vcStatus",jsvNewFromInteger(vcInfo.status));
    jsvObjectSetChildAndUnLock(o,"vcPPG",jsvNewFromInteger(vcInfo.ppgValue));
    jsvObjectSetChildAndUnLock(o,"vcPPGoffs",jsvNewFromInteger(vcInfo.ppgOffset));
    jsvObjectSetChildAndUnLock(o,"vcCurrent",jsvNewFromInteger(vcInfo.currentValue));
    jsvObjectSetChildAndUnLock(o,"vcPre",jsvNewFromInteger(vcInfo.preValue));
    jsvObjectSetChildAndUnLock(o,"vcPS",jsvNewFromInteger(vcInfo.psValue));
    jsvObjectSetChildAndUnLock(o,"vcEnv",jsvNewFromInteger(vcInfo.envValue));
    jsvObjectSetChildAndUnLock(o,"vcRaw",jsvNewArrayBufferWithData(sizeof(vcInfo.raw), vcInfo.raw));
  }
  return o;
}

/// Called when JS engine torn down (disable timer/watch/etc)
void hrm_sensor_kill() {
  if (hrmCallback!=NULL) // if is running
    vc31_watch_off();
}

/// Called when JS engine initialised
void hrm_sensor_init() {
  if (hrmCallback!=NULL) // if is running
    vc31_watch_on();
}
