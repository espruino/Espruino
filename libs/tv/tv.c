/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * TV output capability on STM32 devices
 * ----------------------------------------------------------------------------
 */

#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jswrap_graphics.h"

// PAL timing specs - which we're ignoring :)
// http://martin.hinner.info/vga/pal.html

unsigned short tvWidth, tvHeight; // width and height of buffer
Pin tvPinVideo, tvPinSync;
const char *screenPtr = (const char *)0;
int line = 0;
unsigned short ticksPerLine = 0; // timer ticks

#define PAL_VBLANK 25 // amount of extra height that is just blank

#define PAL_LINE 64
#define PAL_HALF_LINE (PAL_LINE/2)
#define PAL_PULSE_SHORT_ON 5
#define PAL_PULSE_LONG_ON 27
#define PAL_PULSE_SHORT_OFF (PAL_HALF_LINE-PAL_PULSE_SHORT_ON)
#define PAL_PULSE_LONG_OFF (PAL_HALF_LINE-PAL_PULSE_LONG_ON)
#define PAL_FRONTPORCH 8
#define PAL_BACKPORCH 7

#ifdef STM32F4
#define TVSPIDEVICE            EV_SPI1
#define TVSPI                  SPI1
#define DMA_Channel_TVSPI_TX   DMA1_Stream3
#define DMA_FLAG_TVSPI_TC_TX   DMA1_FLAG_TC3
#define RCC_AHB1Periph_TVDMA    RCC_AHB1Periph_DMA1

#define TVTIMER               TIM4
#define RCC_APB1Periph_TVTIMER  RCC_APB1Periph_TIM4
#define TVTIMER_IRQHandler TIM4_IRQHandler
#define TVTIMER_IRQn TIM4_IRQn
#else
#define TVSPIDEVICE            EV_SPI1
#define TVSPI                  SPI1
#define DMA_Channel_TVSPI_TX   DMA1_Channel3
#define DMA_FLAG_TVSPI_TC_TX   DMA1_FLAG_TC3
#define RCC_AHBPeriph_TVDMA    RCC_AHBPeriph_DMA1

#define TVTIMER               TIM6
#define RCC_APB1Periph_TVTIMER  RCC_APB1Periph_TIM6
#define TVTIMER_IRQHandler TIM6_IRQHandler
#define TVTIMER_IRQn TIM6_IRQn
#endif





static ALWAYS_INLINE void sync_start() {
  jshPinSetValue(tvPinSync, 0);
}

static ALWAYS_INLINE void sync_end() {
  jshPinSetValue(tvPinSync, 1);
}

ALWAYS_INLINE void tv_start_line_video() {
  int lineIdx;
  if (line <= 313) {
    lineIdx = (line-(5+PAL_VBLANK)) ;
  } else {
    lineIdx = (line-(317+PAL_VBLANK));
  }
  if (lineIdx>=0 && lineIdx<270) {
    lineIdx = lineIdx*tvHeight/270;
    jshPinSetState(tvPinVideo, JSHPINSTATE_AF_OUT); // re-enable output for SPI
#ifdef STM32F4
    DMA_Channel_TVSPI_TX->CR &= ~DMA_SxCR_EN; // disable
    DMA_Channel_TVSPI_TX->NDTR = tvWidth>>3/*bytes*/;
    DMA_Channel_TVSPI_TX->M0AR = (uint32_t)(screenPtr + lineIdx*DMA_Channel_TVSPI_TX->NDTR);
    DMA_Channel_TVSPI_TX->CR |= DMA_SxCR_EN; // enable
#else
    DMA_Channel_TVSPI_TX->CCR &= ~DMA_CCR5_EN; // disable
    DMA_Channel_TVSPI_TX->CNDTR = tvWidth>>3/*bytes*/;
    DMA_Channel_TVSPI_TX->CMAR = (uint32_t)(screenPtr + lineIdx*DMA_Channel_TVSPI_TX->CNDTR);
    DMA_Channel_TVSPI_TX->CCR |= DMA_CCR5_EN; // enable
#endif
  }
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

static ALWAYS_INLINE void setTimer(unsigned int mSec) {
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
  jshInterruptOff();
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
    sync_end();
    break;
  case TVS_VID_START:
    setTimer(PAL_LINE-(PAL_PULSE_SHORT_ON+PAL_FRONTPORCH+PAL_BACKPORCH));
    if (line>PAL_VBLANK) {
      tv_start_line_video();
    }
    tvState = TVS_VID_BACKPORCH;
    break;
  case TVS_VID_BACKPORCH:
    setTimer(PAL_BACKPORCH);
    jshPinSetState(tvPinVideo, JSHPINSTATE_GPIO_OUT);
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
  jshInterruptOn();
}

unsigned int jshGetTimerFreq(TIM_TypeDef *TIMx);


JsVar *tv_setup_pal(Pin pinVideo, Pin pinSync, int width, int height) {
  tvWidth = (unsigned short)((width+7)&~7); // to the nearest byte
  tvHeight = (unsigned short)height;
  tvPinVideo = pinVideo;
  tvPinSync = pinSync;
  JsVar *gfx = jswrap_graphics_createArrayBuffer(tvWidth,tvHeight,1,0);
  if (!gfx) return 0;
  JsVar *buffer = jsvObjectGetChild(gfx, "buffer", 0);
  JsVar *ab = jsvGetArrayBufferBackingString(buffer);
  jsvUnLock(buffer);
  screenPtr = (char*)(ab+1);
  jsvUnLock(ab);
  // init SPI
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate =  tvWidth * 1000000 / 52;
  inf.spiMSB = false;
  inf.pinMOSI = tvPinVideo;
  jshPinOutput(tvPinSync, 0); // setup output state
  jshPinSetValue(tvPinVideo, 0); // set default video output state
  jshSPISetup(TVSPIDEVICE, &inf);
  // disable IRQs - because jsHardware enabled them
  SPI_I2S_ITConfig(TVSPI, SPI_I2S_IT_RXNE, DISABLE);
  // init DMA
#ifdef STM32F4
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_TVDMA, ENABLE);
#else
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_TVDMA, ENABLE);
#endif


  DMA_InitTypeDef DMA_InitStructure;
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(TVSPI->DR));
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // DMA_PeripheralDataSize_HalfWord and 16 bit?
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
#ifdef STM32F4
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)screenPtr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
#else
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)screenPtr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
#endif
  DMA_InitStructure.DMA_BufferSize = tvWidth>>3/*bytes*/;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_DeInit(DMA_Channel_TVSPI_TX);
  DMA_Init(DMA_Channel_TVSPI_TX, &DMA_InitStructure);
  DMA_Cmd(DMA_Channel_TVSPI_TX, ENABLE);
  SPI_I2S_DMACmd(TVSPI, SPI_I2S_DMAReq_Tx, ENABLE);


  /*Timer configuration------------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TVTIMER, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  ticksPerLine = (unsigned short)(jshGetTimerFreq(TVTIMER) / 15625);
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

  return gfx;
}
