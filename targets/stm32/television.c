/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Code to handle output to a television
 * ----------------------------------------------------------------------------
 */
#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"

// http://martin.hinner.info/vga/pal.html

#define TV_OUT_HEIGHT 144 // 288 max
#define TV_OUT_WIDTH 192 // 4/3 of 144
#define TV_OUT_STRIDE (TV_OUT_WIDTH>>3)

#define PAL_LINE 64
#define PAL_HALF_LINE (PAL_LINE/2)
#define PAL_PULSE_SHORT_ON 4
#define PAL_PULSE_LONG_ON 28
#define PAL_PULSE_SHORT_OFF (PAL_HALF_LINE-PAL_PULSE_SHORT_ON)
#define PAL_PULSE_LONG_OFF (PAL_HALF_LINE-PAL_PULSE_LONG_ON)
#define PAL_FRONTPORCH 10
#define PAL_BACKPORCH 4

#define TVSPIDEVICE            EV_SPI1
#define TVSPI                  SPI1
#define DMA_Channel_TVSPI_TX   DMA1_Channel3
#define DMA_FLAG_TVSPI_TC_TX   DMA1_FLAG_TC3
#define RCC_AHBPeriph_TVDMA    RCC_AHBPeriph_DMA1

#define TVTIMER               TIM6
#define RCC_APB1Periph_TVTIMER  RCC_APB1Periph_TIM6
#define TVTIMER_IRQHandler TIM6_IRQHandler
#define TVTIMER_IRQn TIM6_IRQn

Pin videoPin = 0;
Pin syncPin = 0;
DMA_InitTypeDef DMA_InitStructure;
const char *screenPtr = (const char *)0;
int line = 0;
unsigned int ticksPerLine = 0; // timer ticks

static ALWAYS_INLINE void sync_start() {
  jshPinSetValue(syncPin, 0);
}

static ALWAYS_INLINE void sync_end() {
  jshPinSetValue(syncPin, 1);
}

ALWAYS_INLINE void tv_start_line_video() {
  int lineIdx;
  if (line <= 313) {
    lineIdx = (line-5) >> 1;
  } else {
    lineIdx = (line-317) >> 1;
  }

  DMA_Channel_TVSPI_TX->CCR &= ~DMA_CCR5_EN; // disable
  DMA_Channel_TVSPI_TX->CNDTR = TV_OUT_STRIDE;
  DMA_Channel_TVSPI_TX->CMAR = (uint32_t)(screenPtr + lineIdx*TV_OUT_STRIDE);
  DMA_Channel_TVSPI_TX->CCR |= DMA_CCR5_EN; // enable
}


typedef enum {
  TVS_SYNC1_START,
  TVS_SYNC1_END,
  TVS_VID_START,  // output white
  TVS_VID_VIDEO, // actual start of video
  TVS_VID_BACKPORCH, // back porch
  TVS_SYNC2_START,
  TVS_SYNC2_END,
} TvState;
TvState tvState = TVS_SYNC1_START;

static inline void setTimer(unsigned int mSec) {
  TVTIMER->ARR = (uint16_t)(ticksPerLine * mSec / 64);
}

bool tvIsVideo() {
  return (line>=5 && line<=309) || (line>=317 && line<=622);
}

bool tvIsSync1Long() {
  return (line<=2) || (line==313) || (line==314);
}

bool tvIsSync2Long() {
  return (line<=1) || ((line>=312) && (line<=314));
}

void TVTIMER_IRQHandler() {
  TIM_ClearITPendingBit(TVTIMER, TIM_IT_Update);
  switch (tvState) {
  case TVS_SYNC1_START:
    if (tvIsVideo() || !tvIsSync1Long()) {
      setTimer(PAL_PULSE_SHORT_ON);
    } else {
      setTimer(PAL_PULSE_LONG_ON);
    }
    sync_start();
    tvState = TVS_SYNC1_END;
    break;
  case TVS_SYNC1_END:
    sync_end();
    if (tvIsVideo()) {
      setTimer(PAL_FRONTPORCH);
      tvState = TVS_VID_START;
    } else {
      if (tvIsSync1Long()) {
        setTimer(PAL_PULSE_LONG_OFF);
      } else { // short
        setTimer(PAL_PULSE_SHORT_OFF);
      }
      tvState = TVS_SYNC2_START;
    }
    break;
  case TVS_VID_START:
    setTimer(PAL_LINE-(PAL_PULSE_SHORT_ON+PAL_FRONTPORCH+PAL_BACKPORCH));
    jshPinSetState(videoPin, JSHPINSTATE_AF_OUT); // re-enable output for SPI
    tv_start_line_video();
    tvState = TVS_VID_BACKPORCH;
    break;
  case TVS_VID_BACKPORCH:
    setTimer(PAL_BACKPORCH);
    jshPinSetState(videoPin, JSHPINSTATE_GPIO_OUT);
    tvState = TVS_SYNC1_START;
    break;
  case TVS_SYNC2_START:
    if (tvIsSync2Long()) {
      setTimer(PAL_PULSE_LONG_ON);
    } else { // short
      setTimer(PAL_PULSE_SHORT_ON);
    }
    sync_start();
    tvState = TVS_SYNC2_END;
    break;
  case TVS_SYNC2_END:
  default:
    if (tvIsSync1Long()) {
      setTimer(PAL_PULSE_LONG_OFF);
    } else { // short
      setTimer(PAL_PULSE_SHORT_OFF);
    }
    sync_end();
    tvState = TVS_SYNC1_START;
    break;
  }

  if (tvState == TVS_SYNC1_START) {
    if (line++ > 624) line=0; // count lines
  }
}

unsigned int jshGetTimerFreq(TIM_TypeDef *TIMx);

void tv_init(JsVar *v) {
  JsVar *ab = jsvGetArrayBufferBackingString(v);
  screenPtr = (char*)(ab+1);
  jsvUnLock(ab);
  // init SPI
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate =  TV_OUT_WIDTH * 1000000 / 52;
  inf.spiMSB = false;
  videoPin = jshGetPinFromString("A7");
  syncPin = jshGetPinFromString("A6");
  inf.pinMOSI = videoPin;
  jshPinOutput(syncPin, 0); // setup output state
  jshPinSetValue(videoPin, 0); // set default video output state
  jshSPISetup(TVSPIDEVICE, &inf);
  // disable IRQs - because jsHardware enabled them
  SPI_I2S_ITConfig(TVSPI, SPI_I2S_IT_RXNE, DISABLE);
  // init DMA
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_TVDMA, ENABLE);

  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(TVSPI->DR));
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // DMA_PeripheralDataSize_HalfWord and 16 bit?
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)screenPtr;
  DMA_InitStructure.DMA_BufferSize = TV_OUT_STRIDE;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_DeInit(DMA_Channel_TVSPI_TX);
  DMA_Init(DMA_Channel_TVSPI_TX, &DMA_InitStructure);
  DMA_Cmd(DMA_Channel_TVSPI_TX, ENABLE);

  SPI_I2S_DMACmd(TVSPI, SPI_I2S_DMAReq_Tx, ENABLE);


  /*Timer configuration------------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TVTIMER, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  ticksPerLine = jshGetTimerFreq(TVTIMER) / 15625;
  TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
  TIM_TimeBaseInitStruct.TIM_Period = (uint16_t)ticksPerLine;
  TIM_TimeBaseInit(TVTIMER, &TIM_TimeBaseInitStruct);

  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = TVTIMER_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
  nvicStructure.NVIC_IRQChannelSubPriority = 0;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);

  //TIM_ARRPreloadConfig(TVTIMER, DISABLE);
  TIM_ITConfig(TVTIMER, TIM_IT_Update, ENABLE);
  TIM_Cmd(TVTIMER, ENABLE);  /* enable counter */
}
