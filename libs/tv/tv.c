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
#include "tv.h"

// PAL timing specs - which we're ignoring :)
// http://martin.hinner.info/vga/pal.html


void tv_info_pal_init(tv_info_pal *inf) {
  inf->width =384;
  inf->height = 270;
};

void tv_info_vga_init(tv_info_vga *inf) {
  inf->width = 220;
  inf->height = 480;
  inf->lineRepeat = 1;
}


unsigned short tvWidth, tvHeight; // width and height of buffer
unsigned char tvLineRepeat;
Pin tvPinVideo, tvPinSync, tvPinSyncV;
const char *tvPixelPtr;
int tvCurrentLine;
unsigned short ticksPerLine; // timer ticks

#define PAL_VBLANK 25 // amount of extra height that is just blank

#define PAL_LINE 64
#define PAL_HALF_LINE (PAL_LINE/2)
#define PAL_PULSE_SHORT_ON 5
#define PAL_PULSE_LONG_ON 27
#define PAL_PULSE_SHORT_OFF (PAL_HALF_LINE-PAL_PULSE_SHORT_ON)
#define PAL_PULSE_LONG_OFF (PAL_HALF_LINE-PAL_PULSE_LONG_ON)
#define PAL_FRONTPORCH 8
#define PAL_BACKPORCH 7

// See STM32 reference on DMA for the mappings to SPI1_TX
#ifdef STM32F4
#define TVSPIDEVICE            EV_SPI1
#define TVSPI                  SPI1
#define DMA_TVSPI_TX   DMA2_Stream3
#define DMA_TVSPI_FLAG_TCIF DMA_FLAG_TCIF3
#define DMA_Channel_TVSPI_TX DMA_Channel_3
#define RCC_AHB1Periph_TVDMA   RCC_AHB1Periph_DMA2

#define TVTIMER               TIM4
#define RCC_APB1Periph_TVTIMER  RCC_APB1Periph_TIM4
#define TVTIMER_IRQHandler TIM4_IRQHandler
#define TVTIMER_IRQn TIM4_IRQn
#else
#define TVSPIDEVICE            EV_SPI1
#define TVSPI                  SPI1
#define DMA_TVSPI_TX   DMA1_Channel3
#define RCC_AHBPeriph_TVDMA    RCC_AHBPeriph_DMA1

#define TVTIMER               TIM6
#define RCC_APB1Periph_TVTIMER  RCC_APB1Periph_TIM6
#define TVTIMER_IRQHandler TIM6_IRQHandler
#define TVTIMER_IRQn TIM6_IRQn
#endif

unsigned int jshGetTimerFreq(TIM_TypeDef *TIMx);

void nullFunc() {};
void (*tvTimerFunc)();

static ALWAYS_INLINE void setTVTimerIRQ(void (*f)()) {
  tvTimerFunc = f;
}

void TVTIMER_IRQHandler() {
  jshInterruptOff();
  TIM_ClearITPendingBit(TVTIMER, TIM_IT_Update);
  tvTimerFunc();
  jshInterruptOn();
}


void dma_setup(int bitRate) {
  // init SPI
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate =  bitRate;
  inf.baudRateSpec = SPIB_MINIMUM; // we don't want SPI to be any slower than this
  inf.spiMSB = false;
  inf.pinMOSI = tvPinVideo;
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
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
#ifdef STM32F4
  DMA_InitStructure.DMA_Channel = DMA_Channel_TVSPI_TX; // needed for SPI TX
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)tvPixelPtr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_MemoryBurst =DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst =DMA_PeripheralBurst_Single;
#else
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tvPixelPtr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
#endif
  DMA_InitStructure.DMA_BufferSize = tvWidth>>3/*bytes*/;

  DMA_DeInit(DMA_TVSPI_TX);
  DMA_Init(DMA_TVSPI_TX, &DMA_InitStructure);
  SPI_I2S_DMACmd(TVSPI, SPI_I2S_DMAReq_Tx, ENABLE);
}

void dma_start(uint32_t addr, uint32_t count) {
#ifdef STM32F4
    DMA_TVSPI_TX->NDTR = count;
    DMA_TVSPI_TX->M0AR = addr;
    DMA_ClearFlag(DMA_TVSPI_TX, DMA_TVSPI_FLAG_TCIF);
    DMA_Cmd(DMA_TVSPI_TX, ENABLE);
#else
    DMA_TVSPI_TX->CCR &= ~DMA_CCR5_EN; // disable
    DMA_TVSPI_TX->CNDTR = count;
    DMA_TVSPI_TX->CMAR = addr;
    DMA_TVSPI_TX->CCR |= DMA_CCR5_EN; // enable
#endif
}



static ALWAYS_INLINE void sync_start() {
  jshPinSetValue(tvPinSync, 0);
}

static ALWAYS_INLINE void sync_end() {
  jshPinSetValue(tvPinSync, 1);
}

ALWAYS_INLINE void tv_start_line_video() {
  uint32_t lineIdx;
  if (tvCurrentLine <= 313) {
    lineIdx = ((uint32_t)tvCurrentLine-(5+PAL_VBLANK)) ;
  } else {
    lineIdx = ((uint32_t)tvCurrentLine-(317+PAL_VBLANK));
  }
  if (lineIdx<270) {
    lineIdx = lineIdx*tvHeight/270;
    jshPinSetState(tvPinVideo, JSHPINSTATE_AF_OUT); // re-enable output for SPI
    uint32_t lsize = tvWidth>>3/*bytes*/;
    dma_start((uint32_t)tvPixelPtr + lineIdx*lsize, lsize);
  }
}

static ALWAYS_INLINE void setTimer(unsigned int mSec) {
  TVTIMER->ARR = (uint16_t)(ticksPerLine * mSec / 64);
}

bool tvIsVideo() {
  return (tvCurrentLine>=5 && tvCurrentLine<=309) || (tvCurrentLine>=317 && tvCurrentLine<=622);
}

bool tvIsSync1Long() {
  return (tvCurrentLine<=2) || (tvCurrentLine==313) || (tvCurrentLine==314);
}

bool tvIsSync2Long() {
  return (tvCurrentLine<=1) || ((tvCurrentLine>=312) && (tvCurrentLine<=314));
}

void tv_pal_irq_sync1_start();
void tv_pal_irq_sync1_end();
void tv_pal_irq_vid_start();
void tv_pal_irq_vid_backporch();
void tv_pal_irq_sync2_start();
void tv_pal_irq_sync2_end();

void tv_pal_irq_sync1_start() {
  if (tvCurrentLine++ > 624) tvCurrentLine=0; // count lines

  if (tvIsVideo() || !tvIsSync1Long()) {
    setTimer(PAL_PULSE_SHORT_ON);
  } else {
    setTimer(PAL_PULSE_LONG_ON);
  }
  sync_start();
  setTVTimerIRQ(tv_pal_irq_sync1_end);
}

void tv_pal_irq_sync1_end() {
  if (tvIsVideo()) {
    setTimer(PAL_FRONTPORCH);
    setTVTimerIRQ(tv_pal_irq_vid_start);
  } else {
    if (tvIsSync1Long()) {
      setTimer(PAL_PULSE_LONG_OFF);
    } else { // short
      setTimer(PAL_PULSE_SHORT_OFF);
    }
    setTVTimerIRQ(tv_pal_irq_sync2_start);
  }
  sync_end();
}

void tv_pal_irq_vid_start() {
  setTimer(PAL_LINE-(PAL_PULSE_SHORT_ON+PAL_FRONTPORCH+PAL_BACKPORCH));
  if (tvCurrentLine>PAL_VBLANK) {
    tv_start_line_video();
  }
  setTVTimerIRQ(tv_pal_irq_vid_backporch);
}

void tv_pal_irq_vid_backporch() {
  setTimer(PAL_BACKPORCH);
  jshPinSetState(tvPinVideo, JSHPINSTATE_GPIO_OUT);
  setTVTimerIRQ(tv_pal_irq_sync1_start);
}

void tv_pal_irq_sync2_start() {
  if (tvIsSync2Long()) {
    setTimer(PAL_PULSE_LONG_ON);
  } else { // short
    setTimer(PAL_PULSE_SHORT_ON);
  }
  sync_start();
  setTVTimerIRQ(tv_pal_irq_sync2_end);
}

void tv_pal_irq_sync2_end() {
  if (tvIsSync1Long()) {
    setTimer(PAL_PULSE_LONG_OFF);
  } else { // short
    setTimer(PAL_PULSE_SHORT_OFF);
  }
  sync_end();
  setTVTimerIRQ(tv_pal_irq_sync1_start);
}


void tv_vga_irq() {
  jshPinSetValue(tvPinSync, 0);
  jshPinSetValue(tvPinSyncV, tvCurrentLine>=2); // 2 lines of sync
  jshDelayMicroseconds(1);
  jshPinSetValue(tvPinSync, 1);
  jshDelayMicroseconds(2);

  uint32_t lineIdx = ((uint32_t)tvCurrentLine - 20) / tvLineRepeat; // 20px = front porch
  if (lineIdx < tvHeight) {
    uint32_t lsize = tvWidth>>3/*bytes*/;
    dma_start((uint32_t)tvPixelPtr + lineIdx*lsize, lsize);
  }

  if (tvCurrentLine++ > tvHeight*tvLineRepeat + 40) tvCurrentLine=0; // count lines
}


JsVar *tv_setup_pal(tv_info_pal *inf) {
  tvWidth = (unsigned short)((inf->width+7)&~7); // to the nearest byte
  tvHeight = (unsigned short)inf->height;
  tvPinVideo = inf->pinVideo;
  tvPinSync = inf->pinSync;
  tvPinSyncV = 0;
  tvCurrentLine = 0;

  jshPinOutput(tvPinSync, 0); // setup output state

  JsVar *gfx = jswrap_graphics_createArrayBuffer(tvWidth,tvHeight,1,0);
  if (!gfx) return 0;
  JsVar *buffer = jsvObjectGetChild(gfx, "buffer", 0);
  JsVar *ab = jsvGetArrayBufferBackingString(buffer);
  jsvUnLock(buffer);
  tvPixelPtr = (char*)(ab+1);
  jsvUnLock(ab);

  dma_setup(tvWidth * 1000000 / 52); // 52uS of picture

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
  setTVTimerIRQ(tv_pal_irq_sync1_start);

  //TIM_ARRPreloadConfig(TVTIMER, DISABLE);
  TIM_ITConfig(TVTIMER, TIM_IT_Update, ENABLE);
  TIM_Cmd(TVTIMER, ENABLE);  /* enable counter */

  return gfx;
}

JsVar *tv_setup_vga(tv_info_vga *inf) {
  tvWidth = (unsigned short)((inf->width+7)&~7); // to the nearest byte
  tvHeight = (unsigned short)inf->height;
  tvLineRepeat = (unsigned char)inf->lineRepeat;
  tvPinVideo = inf->pinVideo;
  tvPinSync = inf->pinSync;
  tvPinSyncV = inf->pinSyncV;
  tvCurrentLine = 0;

  //JshPinFunction timer = jshPinAnalogOutput(tvPinSync, 1-0.12 /* */, 31468, JSAOF_NONE);
  //if (!timer) return 0; // couldn't set up the timer

  jshPinOutput(tvPinSync, 1); // setup output state
  jshPinOutput(tvPinSyncV, 1); // setup output state


  JsVar *gfx = jswrap_graphics_createArrayBuffer(tvWidth,tvHeight,1,0);
  if (!gfx) return 0;
  JsVar *buffer = jsvObjectGetChild(gfx, "buffer", 0);
  JsVar *ab = jsvGetArrayBufferBackingString(buffer);
  jsvUnLock(buffer);
  tvPixelPtr = (char*)(ab+1);
  jsvUnLock(ab);

  dma_setup(tvWidth * 1000000 / 25); // 25uS of picture
  jshPinSetState(tvPinVideo, JSHPINSTATE_AF_OUT); // enable output for SPI

  /*Timer configuration------------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TVTIMER, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  ticksPerLine = (unsigned short)(jshGetTimerFreq(TVTIMER) / 31468);
  TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
  TIM_TimeBaseInitStruct.TIM_Period = (uint16_t)ticksPerLine;
  TIM_TimeBaseInit(TVTIMER, &TIM_TimeBaseInitStruct);

  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = TVTIMER_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
  nvicStructure.NVIC_IRQChannelSubPriority = 0;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);
  setTVTimerIRQ(tv_vga_irq);

  //TIM_ARRPreloadConfig(TVTIMER, DISABLE);
  TIM_ITConfig(TVTIMER, TIM_IT_Update, ENABLE);
  TIM_Cmd(TVTIMER, ENABLE);  /* enable counter */

  return gfx;
}

