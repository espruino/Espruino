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
#include "hrm_vc31.h"
#include "jsutils.h"
#include "platform_config.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jsi2c.h"

#ifndef EMULATED

extern JshI2CInfo i2cHRM;
HrmCallback hrmCallback;
uint16_t hrmPollInterval = HRM_POLL_INTERVAL_DEFAULT; // in msec, so 20 = 50hz

// VC31
#define VC31A_DEV_ID        0x00 // read only
#define VC31A_STATUS        0x01 // read only
#define VC31A_PPG           0x02 // read only
#define VC31A_CURRENT       0x04 // read only
#define VC31A_PRE           0x06 // read only
#define VC31A_PS            0x08 // read only
#define VC31A_ENV           0x0A // read only
#define VC31A_CTRL          0x20
#define VC31A_PPG_DIV       0x21
#define VC31A_PS_DIV        0x23
#define VC31A_TIA_WAIT      0x24
#define VC31A_AMP_WAIT      0x25
#define VC31A_GREEN_WAIT    0x26
#define VC31A_GREEN_IR_GAP  0x27
#define VC31A_IR_WAIT       0x28
#define VC31A_GREEN_ADJ     0x29
// VC31B
#define VC31B_REG1         0x01 // read only Sensor Status
// see VC31B_STATUS_*
#define VC31B_REG2         0x02 // read only IRQ status
// see VC31B_INT_*
#define VC31B_REG3         0x03 // FIFO write pos (where it'll write NEXT)
#define VC31B_REG7         0x07 // 2x 16 bit counter from internal oscillator (if 2nd is less than 1st there's some kind of rollover)
#define VC31B_REG10        0x3B // write 0x5A for a soft reset
#define VC31B_REG11        0x10 // CTRL
 // 0x80 = enable
 // 0x40 = ??? set for HRM measurement
 // 0x10 = if SPO2 w edisable this when not worn
 // 0x08 = LED control? disable when not worn
 // 0x04 = SLOT2 enable for env/wear detect sensor
 // 0x02 = SLOT1 enable for SPO2
 // 0x01 = SLOT0 enable for HRM/PPG
 // totalSlots = amount of bits set in 0x02/0x01
#define VC31B_REG12        0x11 // INT?
// 0x80 appears to enable interrupts?
// 0x10 = WearStatusDetection (ENV sensor IRQ?)
#define VC31B_REG13        0x12 // ???
#define VC31B_REG14        0x13 // FIFO 0x40=some flag, + assert FIFO IRQ every X samples
#define VC31B_REG15        0x14 // 16 bit time calibration (x>>8,x) (0x31F default)
#define VC31B_REG16        0x16 // ENV samplerate. samplesPerSec-6. After how many samples is the 0x10 IRQ asserted for ENV data
#define VC31B_REG17        0x17 // SLOT0 LED Current - 0xEF = maxLedCur
#define VC31B_REG18        0x18 // SLOT1 LED Current - 0xEF = maxLedCur
#define VC31B_REG19        0x19 // LED current, 0x30=10mA,0x50=40mA,0x5A=60mA,0xE0=80mA
#define VC31B_REG20        0x1A // SLOT0 ENV sensitivity - 0x77 = PDResMax
#define VC31B_REG21        0x1B // SLOT1 ENV sensitivity - 0x77 = PDResMax
#define VC31B_REG22        0x1C // ? set to 0x67 for HRM mode

// Interrupts
// 0x10 = WearStatusDetection (ENV sensor IRQ?)
/* Bit fields for VC31B_REG1 */
#define VC31B_STATUS_CONFLICT                       0x10
#define VC31B_STATUS_INSAMPLE                       0x08
#define VC31B_STATUS_OVERLOAD_MASK                  0x07 // 3x bits for each of the 3 channels
/* Bit fields for VC31B_REG2 */
#define VC31B_INT_PS                          0x10 // used for wear detection
#define VC31B_INT_OV                          0x08 // OvloadAdjust
#define VC31B_INT_FIFO                        0x04
#define VC31B_INT_ENV                         0x02 // EnvAdjust
#define VC31B_INT_PPG                         0x01 // PpgAdjust

#define VC31B_PS_TH                     6 // threshold for wearing/not
#define VC31B_PPG_TH                    10 // Causes of PPG interruption PPG_TH = 300
#define VC31B_ADJUST_INCREASE                   22 // 1.4 << 4 = 22.4//1.4f
#define VC31B_ADJUST_DECREASE                     11 // 0.7 << 4 = 11.2//0.7f
#define VC31B_ADJUST_STEP_MAX                   32
#define VC31B_ADJUST_STEP_MIN                   1


#define VC31A_CTRL_RESET_VALUE       0x03 // Default value for VC31A_CTRL
#define VC31A_CTRL_MASK              0xF7 // Mask for VC31A_CTRL
#define VC31A_CTRL_WORK_MODE         0x80 // Enter work mode. Start sample
#define VC31A_CTRL_ENABLE_PPG        0x40 // Enable green led sample
#define VC31A_CTRL_ENABLE_PRE        0x20 // Enable pre sample
#define VC31A_CTRL_LDO_EXTERN        0x10 // External LDO
#define VC31A_CTRL_INT_DIR_RISING    0x03 // IRQ trigger on raising edge
#define VC31A_CTRL_OPA_GAIN_12_5     0x00 // OPA3 Gain 12.5.
#define VC31A_CTRL_OPA_GAIN_25       0x01 // OPA3 Gain 25.
#define VC31A_CTRL_OPA_GAIN_50       0x02 // OPA3 Gain 50.
#define VC31A_CTRL_OPA_GAIN_100      0x03 // OPA3 Gain 100.

#define VC31A_PPG_DIV_10_HZ          0x0CC5
#define VC31A_PPG_DIV_12_5_HZ        0x0A35
#define VC31A_PPG_DIV_25_HZ          0x0516
#define VC31A_PPG_DIV_50_HZ          0x0287
#define VC31A_PPG_DIV_100_HZ         0x013F
#define VC31A_PPG_DIV_1000_HZ        0x0018

#define VC31A_STATUS_D_ENV_OK        0x10
#define VC31A_STATUS_D_PS_OK         0x08
#define VC31A_STATUS_D_PRE_OK        0x04
#define VC31A_STATUS_D_CUR_OK        0x02
#define VC31A_STATUS_D_PPG_OK        0x01

#define VC31A_GREEN_ADJ_RESET_VALUE  0x0000          // Default value for VC31A_GREEN_ADJ.
#define VC31A_GREEN_ADJ_MASK         0xFFFF          // Mask for VC31A_GREEN_ADJ.
#define VC31A_GREEN_ADJ_ENABLE       0x8000          // Enable current adjust.
#define VC31A_GREEN_ADJ_DISABLE      0               // Disable current adjust.
#define VC31A_GREEN_ADJ_UP           0x4000          // Turn up the current.
#define VC31A_GREEN_ADJ_DOWN         0               // Turn down the current.
#define VC31A_GREEN_ADJ_VALUE_MASK   0x3FFF          // Mask for VC31A_ADJ_CUR value.


#define VC31A_ADJUST_FACTOR_INCREASE   22       // 1.4 << 4 = 22.4
#define VC31A_ADJUST_FACTOR_DECREASE   11       // 0.7 << 4 = 11.2
#define VC31A_ADJUST_FACTOR_MAX        1536000
#define VC31A_ADJUST_FACTOR_MIN        15360
#define VC31A_ADJUST_STEP_MAX          1000
#define VC31A_ADJUST_STEP_MIN          2

#define VC31A_ENV_LIMIT                2500
#define VC31A_PS_LIMIT                 350//150

#define VC31A_PPG_LIMIT_L              200
#define VC31A_PPG_LIMIT_H              3900
#define VC31A_CURRENT_LIMIT_L          12
#define VC31A_CURRENT_LIMIT_H          1000
#define VC31A_UNWEAR_CNT               3
#define VC31A_ISWEAR_CNT               1

typedef enum
{
    AdjustDirection_Null    = 0,
    AdjustDirection_Up      = 1,
    AdjustDirection_Down    = 2,
} VC31AdjustDirection;

typedef struct
{
    VC31AdjustDirection directionLast;
    VC31AdjustDirection direction;// was directionLastBefore for VC31A
    int32_t step;
} VC31AdjustInfo_t;

typedef enum {
  VC31_DEVICE,
  VC31B_DEVICE
} VC31Type;

// VC31A-specific info
typedef struct {
  uint8_t ctrl; // current VC31A_CTRL reg value
  uint16_t currentValue;
  uint16_t preValue;
  uint16_t envValue;
  uint16_t psValue;
  VC31AdjustInfo_t adjustInfo;
} PACKED_FLAGS VC31AInfo;
// VC31B-specific info
typedef struct {
  uint8_t maxLedCur;
  uint8_t pdResValue[3];
  uint8_t currentValue[3];
  uint8_t psValue;      //PS Sample value.
  uint8_t preValue[2];  //Environment Sample value.
  uint8_t envValue[3];  //Environment Sample value.
} PACKED_FLAGS VC31BSample;
typedef struct {
  uint8_t vcHr02SampleRate; // Heart rate sample frequency
  uint8_t status; // REG2
  uint8_t fifoReadIndex; // last index we read from
  VC31BSample sampleData;
  VC31AdjustInfo_t adjustInfo[2];

  uint8_t ledCurrent[3];
  uint8_t ledMaxCurrent[3];
  uint8_t pdRes[3],pdResMax[3];
  uint8_t pdResSet[3],ppgGain[3];
  bool slot0EnvIsExceedFlag,slot1EnvIsExceedFlag;

  uint8_t regConfig[17]; // all config registers (written to VC31B_REG11)
  bool psBiasReadInPdFlag;

  // SETUP (computed from regConfig)
  uint8_t totalSlots;  // 2 is SPO2 enabled, 1 otherwise or 0 if all disabled
  uint8_t fifoIntDiv;  // when should IRQ enable (i guess?)
  bool enFifo; // FIFO enabled (otherwise data is at 0x80)

} PACKED_FLAGS VC31BInfo;

// Actual vars
VC31Type vcType;
VC31Info vcInfo;
VC31AInfo vcaInfo;
VC31BInfo vcbInfo;

static void vc31_w(uint8_t reg, uint8_t data) {
  uint8_t buf[2] = {reg, data};
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, 2, buf, true);
}
static void vc31_wx(uint8_t reg, uint8_t *data, int cnt) {
  uint8_t buf[32];
  buf[0] = reg;
  memcpy(&buf[1], data, cnt);
  jsi2cWrite(&i2cHRM, HEARTRATE_ADDR, cnt+1, buf, true);
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


// we have a PPG value - save to vcInfo.ppgValue and send it to HRM monitor
void vc31_new_ppg(uint16_t value) {
  vcInfo.ppgLastValue = vcInfo.ppgValue;
  vcInfo.ppgValue = value;

  if (vcInfo.wasAdjusted) {
    // stop any sudden jerky changes to the HRM value
    vcInfo.ppgOffset = vcInfo.ppgLastValue + vcInfo.ppgOffset - vcInfo.ppgValue;
  }
  const int offsetAdjustment = 32;
  if (vcInfo.ppgOffset > offsetAdjustment) vcInfo.ppgOffset -= offsetAdjustment;
  else if (vcInfo.ppgOffset < -offsetAdjustment) vcInfo.ppgOffset += offsetAdjustment;
  else vcInfo.ppgOffset = 0;

  int v = vcInfo.ppgValue + vcInfo.ppgOffset;
  if (vcType == VC31B_DEVICE)
    v <<= 1; // on VC31B the PPG doesn't vary as much with pulse so try and bulk it up here a bit

  hrmCallback(v);
}


static void vc31_adjust() {
  uint16_t adjustParam = 0;
  uint32_t adjustStep = 0;

  vcaInfo.currentValue += 10;

  if (vcInfo.ppgValue > VC31A_PPG_LIMIT_H) {
    if (vcaInfo.currentValue < VC31A_CURRENT_LIMIT_H) {
      if (vcaInfo.adjustInfo.directionLast == AdjustDirection_Down) {
        vcaInfo.adjustInfo.step *= VC31A_ADJUST_FACTOR_DECREASE;
      } else if ((vcaInfo.adjustInfo.directionLast == AdjustDirection_Up)
          && (vcaInfo.adjustInfo.direction == AdjustDirection_Up)) {
        vcaInfo.adjustInfo.step *= VC31A_ADJUST_FACTOR_INCREASE;
      } else {
        vcaInfo.adjustInfo.step *= 16;
      }
      vcaInfo.adjustInfo.step = vcaInfo.adjustInfo.step >> 4;
      vcaInfo.adjustInfo.step =
          (vcaInfo.adjustInfo.step > VC31A_ADJUST_FACTOR_MAX) ?
              VC31A_ADJUST_FACTOR_MAX : vcaInfo.adjustInfo.step;
      vcaInfo.adjustInfo.step =
          (vcaInfo.adjustInfo.step < VC31A_ADJUST_FACTOR_MIN) ?
              VC31A_ADJUST_FACTOR_MIN : vcaInfo.adjustInfo.step;
      adjustStep = (vcaInfo.adjustInfo.step / (3072 - vcaInfo.currentValue));
      adjustStep =
          (adjustStep <= VC31A_ADJUST_STEP_MIN) ?
              VC31A_ADJUST_STEP_MIN : adjustStep;
      adjustStep =
          (adjustStep >= VC31A_ADJUST_STEP_MAX) ?
              VC31A_ADJUST_STEP_MAX : adjustStep;
      adjustParam = (uint16_t)(
          adjustStep) | VC31A_GREEN_ADJ_ENABLE | VC31A_GREEN_ADJ_UP;
      vc31_w16(VC31A_GREEN_ADJ, adjustParam);
      vcInfo.wasAdjusted = 2; // ignore 2 samples worth
    }
    vcaInfo.adjustInfo.direction = vcaInfo.adjustInfo.directionLast;
    vcaInfo.adjustInfo.directionLast = AdjustDirection_Up;
  } else if (vcInfo.ppgValue < VC31A_PPG_LIMIT_L) {
    if (vcaInfo.currentValue > VC31A_CURRENT_LIMIT_L)
    {
      if (vcaInfo.adjustInfo.directionLast == AdjustDirection_Up) {
        vcaInfo.adjustInfo.step *= VC31A_ADJUST_FACTOR_DECREASE;
      } else if ((vcaInfo.adjustInfo.directionLast == AdjustDirection_Down)
          && (vcaInfo.adjustInfo.direction == AdjustDirection_Down)) {
        vcaInfo.adjustInfo.step *= VC31A_ADJUST_FACTOR_INCREASE;
      } else {
        vcaInfo.adjustInfo.step *= 16;
      }
      vcaInfo.adjustInfo.step = vcaInfo.adjustInfo.step >> 4;
      vcaInfo.adjustInfo.step =
          (vcaInfo.adjustInfo.step > VC31A_ADJUST_FACTOR_MAX) ?
              VC31A_ADJUST_FACTOR_MAX : vcaInfo.adjustInfo.step;
      vcaInfo.adjustInfo.step =
          (vcaInfo.adjustInfo.step < VC31A_ADJUST_FACTOR_MIN) ?
              VC31A_ADJUST_FACTOR_MIN : vcaInfo.adjustInfo.step;
      adjustStep = (vcaInfo.adjustInfo.step / (1024 + vcaInfo.currentValue));
      adjustStep =
          (adjustStep <= VC31A_ADJUST_STEP_MIN) ?
              VC31A_ADJUST_STEP_MIN : adjustStep;
      adjustStep =
          (adjustStep >= VC31A_ADJUST_STEP_MAX) ?
              VC31A_ADJUST_STEP_MAX : adjustStep;
      adjustParam = (uint16_t)(
          adjustStep) | VC31A_GREEN_ADJ_ENABLE | VC31A_GREEN_ADJ_DOWN;
      vc31_w16(VC31A_GREEN_ADJ, adjustParam);
      vcInfo.wasAdjusted = 2;  // ignore 2 samples worth
    }
    vcaInfo.adjustInfo.direction = vcaInfo.adjustInfo.directionLast;
    vcaInfo.adjustInfo.directionLast = AdjustDirection_Down;
  } else {
    vcaInfo.adjustInfo.direction = vcaInfo.adjustInfo.directionLast;
    vcaInfo.adjustInfo.directionLast = AdjustDirection_Null;
  }
}


static void vc31a_wearstatus() {
  if (vcInfo.isWearing) {
    if ((vcaInfo.envValue >= VC31A_ENV_LIMIT) ||
        (vcaInfo.psValue < vcaInfo.envValue + VC31A_PS_LIMIT)) {
      if (--vcInfo.unWearCnt <= 0) {
        vcInfo.isWearing = false;
        vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
        vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
        vcaInfo.ctrl &= ~VC31A_CTRL_ENABLE_PPG;
        vc31_w(VC31A_CTRL, vcaInfo.ctrl);
      }
    } else {
      vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
    }
  } else { // not wearing
    if (vcaInfo.psValue >= vcaInfo.envValue + VC31A_PS_LIMIT) {
      if (--vcInfo.isWearCnt <= 0) {
        vcInfo.isWearing = true;
        vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
        vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
        vcaInfo.ctrl |= VC31A_CTRL_ENABLE_PPG;
        vc31_w(VC31A_CTRL, vcaInfo.ctrl);
        vcaInfo.adjustInfo.direction = AdjustDirection_Null;
        vcaInfo.adjustInfo.directionLast = AdjustDirection_Null;
      }
    } else {
      vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
    }
  }
}


static void vc31b_wearstatus() {
  if (vcInfo.isWearing) {
    if ((vcbInfo.sampleData.envValue[2] > VC31B_PS_TH) ||
        (vcbInfo.sampleData.psValue < VC31B_PS_TH) ||
        (vcbInfo.slot0EnvIsExceedFlag == true) ||
        (vcbInfo.slot1EnvIsExceedFlag == true)) { // FIXME (or slot0EnvIsExceedFlag?)
      if (--vcInfo.unWearCnt <= 0) {
        vcInfo.isWearing = false;
        vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
        vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
        // FIXME if SP02 we need to do extra work here
        // Disable SLOT0+1, enable SLOT2
        vcbInfo.regConfig[0] = (vcbInfo.regConfig[0]&0xF8) | 0x04;
        vc31_w(VC31B_REG11, vcbInfo.regConfig[0]);
      }
    } else {
      vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
    }
  } else { // not wearing
    if ((vcbInfo.sampleData.psValue >= VC31B_PS_TH) &&
        (vcbInfo.sampleData.envValue[2]<3)) {
      if (--vcInfo.isWearCnt <= 0) {
        vcInfo.isWearing = true;
        vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
        vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
        /* FIXME they used to do this but it seems like overkill!
        vc31_softreset();
        hrm_sensor_on(hrmCallback);*/
        // Re-enable SLOT2 and SLOT0
        vcbInfo.regConfig[0] = (vcbInfo.regConfig[0]&0xF8) | 0x05;
        vc31_w(VC31B_REG11, vcbInfo.regConfig[0]);
      }
    } else {
      vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
    }
  }
}


// Read a chunk of data from the FIFO and send it
static void vc31b_readfifo(uint8_t startAddr, uint16_t endAddr, uint8_t IndexFlag) {
  uint16_t i = 0;
  uint8_t sampleData[128];

  int dataLength = endAddr - startAddr;
  vc31_rx(startAddr, sampleData, dataLength);
  for(i = 0; i<dataLength; i+=2) {
    uint16_t ppgValue = ((sampleData[i] << 8) | sampleData[i+1]);
    // jsiConsolePrintf("ppg %d\n",ppgValue);
    // FIXME - we need a timestamp - hrmCallback creates one itself when it is called
    // but if we use the FIFO properly we'll need to stamp it ourselves because
    // we'll call it a bunch of times at once
    vc31_new_ppg(ppgValue); // send PPG value
  }
}


// Adjust ledCurrent Value and PDRes value
static void vc31b_adjust(uint8_t slotNum,uint8_t ledcur,uint8_t pdres) {
  uint8_t  oldPdRes,oldLedCurrent;
  uint8_t  newPdRes,newLedCurrent;

  oldLedCurrent = vcbInfo.ledCurrent[slotNum];
  newLedCurrent = oldLedCurrent;
  oldPdRes = vcbInfo.pdRes[slotNum];
  newPdRes = oldPdRes;

  vcbInfo.slot0EnvIsExceedFlag = false;
  vcbInfo.slot1EnvIsExceedFlag = false;
  if (oldLedCurrent == ledcur)
  {
    if(oldPdRes == pdres)
    {
      newPdRes = oldPdRes ;
      vcbInfo.slot0EnvIsExceedFlag = (slotNum == 0)? true:false;
      vcbInfo.slot1EnvIsExceedFlag = (slotNum == 1)? true:false;
      return /*PPGCANNOTADJUSTABLE*/;
    }
    else
    {
      if (vcbInfo.adjustInfo[slotNum].direction == AdjustDirection_Up)
      {
        newPdRes = (oldPdRes >= 7) ? 7 : (oldPdRes + 1);
      }
      else
      {
        newPdRes = (oldPdRes < 1) ? 0 : (oldPdRes - 1);
      }
      newLedCurrent = oldLedCurrent;
      vcInfo.wasAdjusted = 2;
    }
  }
  else
  {
    if(vcbInfo.adjustInfo[slotNum].directionLast == AdjustDirection_Null)
    {
      vcbInfo.adjustInfo[slotNum].step *= 16;
    }
    else if(vcbInfo.adjustInfo[slotNum].direction == vcbInfo.adjustInfo[slotNum].directionLast)
    {
      if((vcbInfo.adjustInfo[slotNum].step == 1) || (vcbInfo.adjustInfo[slotNum].step == 2))
      {
        vcbInfo.adjustInfo[slotNum].step = (vcbInfo.adjustInfo[slotNum].step + 1) * 16;
      }
      else
      {
        vcbInfo.adjustInfo[slotNum].step *= VC31B_ADJUST_INCREASE;
      }
    }
    else
    {
      vcbInfo.adjustInfo[slotNum].step *= VC31B_ADJUST_DECREASE;
    }

    vcbInfo.adjustInfo[slotNum].step = vcbInfo.adjustInfo[slotNum].step >> 4;

    vcbInfo.adjustInfo[slotNum].step = (vcbInfo.adjustInfo[slotNum].step <= VC31B_ADJUST_STEP_MIN) ? VC31B_ADJUST_STEP_MIN : vcbInfo.adjustInfo[slotNum].step;
    vcbInfo.adjustInfo[slotNum].step = (vcbInfo.adjustInfo[slotNum].step >= VC31B_ADJUST_STEP_MAX) ? VC31B_ADJUST_STEP_MAX : vcbInfo.adjustInfo[slotNum].step;

    if(vcbInfo.adjustInfo[slotNum].direction == AdjustDirection_Up)
    {
      newLedCurrent = ((oldLedCurrent + vcbInfo.adjustInfo[slotNum].step)> ledcur) ? ledcur:oldLedCurrent + vcbInfo.adjustInfo[slotNum].step;
    }
    else
    {
      newLedCurrent = (oldLedCurrent < vcbInfo.adjustInfo[slotNum].step) ? ledcur:oldLedCurrent - vcbInfo.adjustInfo[slotNum].step;
    }
    vcInfo.wasAdjusted = 2;
    newPdRes = oldPdRes;
    vcbInfo.adjustInfo[slotNum].directionLast = vcbInfo.adjustInfo[slotNum].direction;
  }

  vcbInfo.ledCurrent[slotNum] = newLedCurrent;
  vcbInfo.pdRes[slotNum] = newPdRes;

  vcbInfo.regConfig[slotNum+7] = vcbInfo.ledCurrent[slotNum] | vcbInfo.ppgGain[slotNum];
  vc31_w(VC31B_REG17+slotNum, vcbInfo.regConfig[slotNum+7]);
  vcbInfo.regConfig[slotNum + 10] = (vcbInfo.pdRes[slotNum] << 4) | vcbInfo.pdResSet[slotNum];
  vc31_w(VC31B_REG20+slotNum, vcbInfo.regConfig[slotNum+10]);
  return;
}


static void vc31b_slot_adjust(int slotNum) {
  int slotMask = 1<<slotNum;
  if (!(vcbInfo.regConfig[0]&slotMask)) return; // slot disabled

  //vcHr02AdjustPDResMax(pvcHr02,slotCount);??
  vcbInfo.ledCurrent[slotNum]= vcbInfo.regConfig[7+slotNum] & 0x7f;
  vcbInfo.ppgGain[slotNum] = vcbInfo.regConfig[7+slotNum] & 0x80;
  vcbInfo.pdRes[slotNum] = (vcbInfo.regConfig[10+slotNum] & 0x70) >> 4;
  vcbInfo.pdResSet[slotNum] = vcbInfo.regConfig[10+slotNum] & 0x8F;

  // check for LED overload
  if(vcInfo.irqStatus & VC31B_INT_OV) {
    // check LED overload status
    uint8_t overload = vcbInfo.status & VC31B_STATUS_OVERLOAD_MASK;
    if (overload&&slotMask) {
      // slot enabled
      if(vcbInfo.ledCurrent[slotNum] != 0) {
        vcbInfo.ledCurrent[slotNum] = vcbInfo.ledCurrent[slotNum] - 1;
        vcbInfo.ledMaxCurrent[slotNum] = vcbInfo.ledCurrent[slotNum];
        vcInfo.wasAdjusted = 1;
        vcbInfo.regConfig[slotNum+7] = (vcbInfo.ledCurrent[slotNum] | vcbInfo.ppgGain[slotNum]);
        vc31_w((VC31B_REG17 + slotNum), vcbInfo.regConfig[slotNum+7]);
      }
    }
  }

  if (slotNum>1) return;

  // PPG supersaturation
  if(vcInfo.ppgValue/*[slotNum]*/ > 4095 - VC31B_PPG_TH * 32 ) {
    //jsiConsolePrintf("%dup %d\n",slotNum,vcInfo.ppgValue);
    /* If the LED luminous current reaches the maximum current,
    it can only be adjusted by increasing the PD resistance */
    vcbInfo.adjustInfo[slotNum].direction = AdjustDirection_Up;
    vc31b_adjust(slotNum, vcbInfo.ledMaxCurrent[slotNum], vcbInfo.pdResMax[slotNum]);
  }
  // Lower saturation
  else if (vcInfo.ppgValue/*[slotNum]*/ < VC31B_PPG_TH * 32) {
    //jsiConsolePrintf("%ddn %d\n",slotNum,vcInfo.ppgValue);
    vcbInfo.adjustInfo[slotNum].direction = AdjustDirection_Down;
    vc31b_adjust(slotNum,0,0);
  }

}

void vc31_irqhandler(bool state, IOEventFlags flags) {
  if (!state || !hrmCallback) return;

  // Have we adjusted settings since this value?
  if (vcType == VC31_DEVICE) {
    uint8_t *buf = vcInfo.raw;
    vc31_rx(VC31A_STATUS, buf, 11);
    vcInfo.irqStatus = buf[0];
    uint16_t ppgValue = (buf[2] << 8) | buf[1];
    vcaInfo.currentValue = ((buf[4] << 8) | buf[3]) + 10;
    vcaInfo.preValue = (buf[6] << 8) | buf[5];
    vcaInfo.psValue = (buf[8] << 8) | buf[7];
    vcaInfo.envValue = (buf[10] << 8) | buf[9];
    vcInfo.envValue = vcaInfo.envValue;

    if (vcInfo.irqStatus & VC31A_STATUS_D_PPG_OK) {
      vc31_new_ppg(ppgValue); // send PPG value

      if (vcInfo.wasAdjusted>0) vcInfo.wasAdjusted--;
      vc31_adjust();
    }
    if (vcInfo.irqStatus & VC31A_STATUS_D_PS_OK)
      vc31a_wearstatus();


  }
  if (vcType == VC31B_DEVICE) {
    uint8_t *buf = &vcInfo.raw[0];
    vc31_rx(VC31B_REG1, buf, 6);
    vcbInfo.status = buf[0]; // VC31B_REG1
    vcInfo.irqStatus = buf[1]; // VC31B_REG2
    //jsiConsolePrintf("int 0x%02x\n",vcInfo.irqStatus);
    vcbInfo.sampleData.envValue[0] = buf[3] >> 4;
    vcbInfo.sampleData.preValue[0] = buf[3] & 0x0F;
    vcbInfo.sampleData.envValue[1] = buf[4] >> 4;
    vcbInfo.sampleData.preValue[1] = buf[4] & 0x0F;
    vcbInfo.sampleData.envValue[2] = buf[5] >> 4;
    vcbInfo.sampleData.psValue = buf[5] & 0x0F;    
    buf = &vcInfo.raw[6];
    vc31_rx(VC31B_REG17, buf, 6);
    vcbInfo.sampleData.pdResValue[0] = (buf[3] >> 4) & 0x07;
    vcbInfo.sampleData.currentValue[0] = buf[0] & 0x7F;
    vcbInfo.sampleData.pdResValue[1] = (buf[4] >> 4) & 0x07;
    vcbInfo.sampleData.currentValue[1] = buf[1] & 0x7F;
    vcbInfo.sampleData.pdResValue[2] = (buf[5] >> 4) & 0x07;
    vcbInfo.sampleData.currentValue[2] = buf[2] & 0x7F;

    // if we had environment sensing, check for wear status and update LEDs accordingly
    if (vcInfo.irqStatus & VC31B_INT_PS) {
      vcInfo.envValue = vcbInfo.sampleData.envValue[2];
      //jsiConsolePrintf("e %d %d\n", vcbInfo.sampleData.psValue, vcbInfo.sampleData.envValue[2] );
      vc31b_wearstatus();
    }
    // read data from FIFO (right now FIFO isn't enabled so this is just one sample)
    if(vcInfo.irqStatus & VC31B_INT_FIFO) {
      uint8_t fifoWriteIndex = vc31_r(VC31B_REG3);
      // FIFO is 128 entries from 0x80 to 0x255 - we need to handle wrapping ourselves
      // if not using FIFO the sampleData is storage in 0x80
      if(vcbInfo.fifoIntDiv) { // fifo enabled
        if(fifoWriteIndex > vcbInfo.fifoReadIndex) { // normal read
          vc31b_readfifo(vcbInfo.fifoReadIndex,fifoWriteIndex,0);
        } else { // fifo rolled over - 2 reads needed
          vc31b_readfifo(vcbInfo.fifoReadIndex,256,0);
          if (fifoWriteIndex != 0x80)
            vc31b_readfifo(0x80,fifoWriteIndex,(256-vcbInfo.fifoReadIndex)/2);
        }
        vcbInfo.fifoReadIndex = fifoWriteIndex;
      } else { // FIFO disabled - samples are at 0x80
        vcbInfo.fifoReadIndex = 0x80;
        vc31b_readfifo(vcbInfo.fifoReadIndex,vcbInfo.fifoReadIndex+vcbInfo.totalSlots*2,0);
      }
      // now we need to adjust the PPG
      if (vcInfo.wasAdjusted>0) vcInfo.wasAdjusted--;
      for (int slotNum=0;slotNum<3;slotNum++) {
        vc31b_slot_adjust(slotNum);
      }
    }
    // Read just one PPG sample (the FIFO usually reads lots)
    /*if (vcInfo.irqStatus & VC31B_INT_PPG) {
      int slotNum = 0;
      uint8_t fifoWriteIndex;
      // if vcbInfo.totalSlots==2 we need to check for both slots?
      if (vcbInfo.enFifo) {
        uint8_t fifoWriteIndex = vc31_r(VC31B_REG3); // current FIFO write indec
        fifoWriteIndex -= 2 * vcbInfo.totalSlots; // go to last sample
        if (fifoWriteIndex < 0x80) fifoWriteIndex += 0x80; // deal with overflow
      } else {
        fifoWriteIndex = 0x80;
      }
      fifoWriteIndex += slotNum*2;
      uint8_t buf[2];
      vc31_rx(fifoWriteIndex, buf, 2);
      uint16_t ppgValue = (buf[0]<<8) | buf[1];
      // ONLY do this here because we're not using the FIFO
      jsiConsolePrintf("ppg %d\n", vcInfo.ppgValue);
      vc31_new_ppg(ppgValue); // send PPG value
      // now we need to adjust the PPG
      for (int slotNum=0;slotNum<3;slotNum++) {
        vc31b_slot_adjust(slotNum);
      }
    } else {
      // FIXME if >1 slot?
      vcbInfo.adjustInfo[0].directionLast = AdjustDirection_Null;
      vcbInfo.adjustInfo[0].direction = AdjustDirection_Null;
    }*/

    // would usually do vcHr02CalculateOSCFreq but we don't care about calibrating the freq (we do it this end)
  }



}

static void vc31_watch_on() {
  jshSetPinShouldStayWatched(HEARTRATE_PIN_INT,true);
  IOEventFlags channel = jshPinWatch(HEARTRATE_PIN_INT, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, vc31_irqhandler);
}

static void vc31_watch_off() {
  jshPinWatch(HEARTRATE_PIN_INT, false, JSPW_NONE);
  jshSetPinShouldStayWatched(HEARTRATE_PIN_INT,false);
}

static void vc31_softreset() {
  if (vcType == VC31B_DEVICE) {
    vc31_w(VC31B_REG10, 0x5A);
    vcbInfo.fifoReadIndex = 0x80;
  }
}

void hrm_sensor_on(HrmCallback callback) {
  hrmCallback = callback;
  jshDelayMicroseconds(1000); // wait for HRM to boot
  unsigned int deviceId = vc31_r(0);
  //jsiConsolePrintf("HRM ID 0x%02x\n",deviceId);
  if (deviceId==17) vcType = VC31_DEVICE;
  else if (deviceId==33) vcType = VC31B_DEVICE;
  else jsiConsolePrintf("VC31 WHO_AM_I failed (%d)", deviceId);

  memset(&vcInfo, 0, sizeof(vcInfo));
  vcInfo.isWearing = true;
  vcInfo.unWearCnt = VC31A_UNWEAR_CNT;
  vcInfo.isWearCnt = VC31A_ISWEAR_CNT;
  vcInfo.ppgOffset = 0;

  if (vcType == VC31_DEVICE) {
    vcaInfo.adjustInfo.step  = 307200;
    vcaInfo.adjustInfo.direction = AdjustDirection_Null;
    vcaInfo.adjustInfo.directionLast = AdjustDirection_Null;

    vc31_w(VC31A_GREEN_WAIT, 0xb4);
    vc31_w(VC31A_TIA_WAIT, 0x54);
    vc31_w(VC31A_PS_DIV, 0x09);
    vc31_w(VC31A_IR_WAIT, 0x5F);
    vc31_w(VC31A_GREEN_IR_GAP, 0x20);
    vc31_w(VC31A_AMP_WAIT, 0x14);

    // seems to take 2 samples, so to get 50Hz (20ms) we need VC31A_PPG_DIV_100_HZ
    uint16_t div;
    if (hrmPollInterval<2) div = VC31A_PPG_DIV_1000_HZ; // 500Hz
    else if (hrmPollInterval<=10) div = 160; // 100Hz
    else if (hrmPollInterval<=20) div = VC31A_PPG_DIV_100_HZ; // 50Hz
    else if (hrmPollInterval<=40) div = VC31A_PPG_DIV_50_HZ; // 25Hz
    else if (hrmPollInterval<=80) div = VC31A_PPG_DIV_25_HZ; // 12.5Hz
    else if (hrmPollInterval<=160) div = VC31A_PPG_DIV_12_5_HZ; // 6.25Hz
    else div = VC31A_PPG_DIV_10_HZ; // 5Hz

    vc31_w16(VC31A_PPG_DIV, div);
    vcaInfo.ctrl = VC31A_CTRL_OPA_GAIN_25 | VC31A_CTRL_ENABLE_PPG | VC31A_CTRL_ENABLE_PRE |
                  VC31A_CTRL_WORK_MODE | VC31A_CTRL_INT_DIR_RISING;
    vc31_w(VC31A_CTRL, vcaInfo.ctrl);
  //  vc31_w16(VC31A_GREEN_ADJ, 0xe8c3);
  }
  if (vcType == VC31B_DEVICE) {
    vcbInfo.vcHr02SampleRate = 1000 / hrmPollInterval; // Hz
    // FIXME SAMPLE RATE. Right now this only changes the period for ENV readings
    const uint8_t _regConfig[17] = {
        0x01,      // VC31B_REG11 - just enable SLOT0
        VC31B_INT_OV|VC31B_INT_FIFO|VC31B_INT_ENV|VC31B_INT_PS,      // VC31B_REG12 IRQs - was 0x3F
        0x8A,      // VC31B_REG13 ??
        0x40,      // VC31B_REG14 0x40 + FIFO Interrupt length in bottom 6 bits
        0x03,0x1F, // VC31B_REG15 (2 bytes) 16 bit counter prescaler
        0x00,      // VC31B_REG16 SLOT2 ENV sample rate - 6
        0x00,      // VC31B_REG17 SLOT0 LED current
        0x80,      // VC31B_REG18 SLOT1 LED current
        0x00,      // VC31B_REG19 - LED current
        0x57,0x37, // VC31B_REG20/21 SLOT 0/1 ENV sensitivity?
        0x07,0x16, // 22,23
        0x56,0x16,0x00
    };

    /* for SPO2

      regConfig[0] = 0x47; // enable SLOT1
      regConfig[1] = 0x2F; // different IRQs..
      regConfig[6] = 0; // SLOT2 samplerate = every sample
      regConfig[7] = 0x80; // SLOT1? LED current same as normal slot
      regConfig[13] = 0x96; // was 0x16
      regConfig[16] = 0x04; // was 4 - OverSample?

     */
    memcpy(vcbInfo.regConfig, _regConfig, sizeof(_regConfig));
    //vcbInfo.regConfig[3]&0x3F==0 for FIFO disable, so FIFO is off now

    // vcbInfo.regConfig[3] |= vcbInfo.vcHr02SampleRate - 6; // Enable FIFO
    vcbInfo.regConfig[6] = vcbInfo.vcHr02SampleRate - 6; // VC31B_REG16 how often should ENV fire

    vcbInfo.regConfig[9] = 0xE0; //CUR = 80mA//write Hs equal to 1
    vcbInfo.regConfig[12] = 0x67; // VC31B_REG22
    vcbInfo.regConfig[0] = 0x45; // VC31B_REG11 heart rate calculation - SLOT2(env) and SLOT0(hr)
    // Set up HRM speed - from testing, 200=100hz/10ms, 400=50hz/20ms, 800=25hz/40ms
    uint16_t divisor = 20 * hrmPollInterval;
    vcbInfo.regConfig[4] = divisor>>8;
    vcbInfo.regConfig[5] = divisor&255;
    // write all registers in one go
    vc31_wx(VC31B_REG11, vcbInfo.regConfig, 17);
    vcbInfo.regConfig[0] |= 0x80; // actually enable now?
    vc31_w(VC31B_REG11, vcbInfo.regConfig[0]);
    // reset state
    vcbInfo.fifoReadIndex = 0x80;
    for (int slot=0;slot<3;slot++) {
      vcbInfo.ledMaxCurrent[slot] = 0x6f;
      vcbInfo.pdResMax[slot] = 7;
    }
    for (int slot=0;slot<2;slot++) {
      vcbInfo.adjustInfo[slot].direction = AdjustDirection_Null;
      vcbInfo.adjustInfo[slot].directionLast = AdjustDirection_Null;
    }

    // update status based on regConfig
    vcbInfo.totalSlots = ((vcbInfo.regConfig[0]&0x02)?1:0) + ((vcbInfo.regConfig[0]&0x01)?1:0);
    vcbInfo.fifoIntDiv = (vcbInfo.regConfig[3] & 0x3f);
    vcbInfo.enFifo = vcbInfo.fifoIntDiv != 0;
    // FIFO IRQs enabled = regConfig[1]&0x10
  }

  vc31_watch_on();
}

void hrm_sensor_off() {
  vc31_watch_off();
  if (vcType == VC31_DEVICE) {
    vc31_w16(VC31A_GREEN_ADJ, 0);
    vcaInfo.ctrl = VC31A_CTRL_OPA_GAIN_25 | VC31A_CTRL_ENABLE_PRE | VC31A_CTRL_INT_DIR_RISING;
    vc31_w(VC31A_CTRL, vcaInfo.ctrl);
  }
  if (vcType == VC31B_DEVICE) {
    vc31_w(VC31B_REG11, 0);
  }
  hrmCallback = NULL;
}

JsVar *hrm_sensor_getJsVar() {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o,"vcPPG",jsvNewFromInteger(vcInfo.ppgValue));
    jsvObjectSetChildAndUnLock(o,"vcPPGoffs",jsvNewFromInteger(vcInfo.ppgOffset));
    jsvObjectSetChildAndUnLock(o,"isWearing",jsvNewFromBool(vcInfo.isWearing));
    jsvObjectSetChildAndUnLock(o,"adjusted",jsvNewFromBool(vcInfo.wasAdjusted));
    if (vcType == VC31_DEVICE) {
      jsvObjectSetChildAndUnLock(o,"vcCurrent",jsvNewFromInteger(vcaInfo.currentValue));
      jsvObjectSetChildAndUnLock(o,"vcPre",jsvNewFromInteger(vcaInfo.preValue));
      jsvObjectSetChildAndUnLock(o,"vcPS",jsvNewFromInteger(vcaInfo.psValue));
      jsvObjectSetChildAndUnLock(o,"vcEnv",jsvNewFromInteger(vcaInfo.envValue));
    }
    if (vcType == VC31B_DEVICE) {
      jsvObjectSetChildAndUnLock(o,"vcPre",jsvNewArrayFromBytes(vcbInfo.sampleData.preValue, 2));
      jsvObjectSetChildAndUnLock(o,"vcPS",jsvNewFromInteger(vcbInfo.sampleData.psValue));
      jsvObjectSetChildAndUnLock(o,"vcEnv",jsvNewArrayFromBytes(vcbInfo.sampleData.envValue, 3));
    }
    jsvObjectSetChildAndUnLock(o,"vcIRQ",jsvNewFromInteger(vcInfo.irqStatus));
    //jsvObjectSetChildAndUnLock(o,"isWearCnt",jsvNewFromInteger(vcInfo.isWearCnt));
    //jsvObjectSetChildAndUnLock(o,"unWearCnt",jsvNewFromInteger(vcInfo.unWearCnt));
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

#endif
