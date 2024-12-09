/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2024 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *
 * STM32 I2S DMA/I2S handling
 *
 * ----------------------------------------------------------------------------

 Works as follows:

* We have 2x buffers in i2sDMAbuf which we always use - we DMA audio out of these
* In DMA1_Stream4_IRQHandler we fill the currently unused buffers (we have to pad mono->stereo for these)
  from the audioRingBuffer

*/


#include "jsinteractive.h"
#include "stm32_i2s.h"

#ifdef LINUX // stubs
void STM32_I2S_Init() {};
void STM32_I2S_Kill() {};
void STM32_I2S_Prepare(int audioFreq) {};
int STM32_I2S_GetFreeSamples() { return I2S_RING_BUFFER_SIZE; }
void STM32_I2S_AddSamples(int16_t *data, unsigned int count) {};
void STM32_I2S_Start() {};
void STM32_I2S_Stop() {};
void STM32_I2S_StreamEnded() {};
uint16_t *STM32_I2S_GetSampleBufferPtr() { return 0; }
STM32_I2S_Status STM32_I2S_GetStatus() { return STM32_I2S_STOPPED; }
#else // not LINUX

#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"

volatile STM32_I2S_Status i2sStatus = STM32_I2S_STOPPED;

STM32_I2S_Status STM32_I2S_GetStatus() { return i2sStatus; }

volatile uint8_t i2sDMAidx = 0; // index in i2sDMAbuf we're playing
volatile int16_t i2sDMAbuf[2][I2S_DMA_BUFFER_SIZE] __attribute__ ((aligned (8))); // Set of audio buffers

volatile uint16_t audioRingIdxIn = 0;  // index in audioRingBuf we're writing to
volatile uint16_t audioRingIdxOut = 0; // index in audioRingBuf we're reading from
/// ringbuffer for audio data we're outputting
//int16_t audioRingBuf[I2S_RING_BUFFER_SIZE];
volatile int16_t *audioRingBuf = (int16_t*)0x10000000; // force in in CCM
// FIXME ideally we use linker and __attribute__((section(".CCM_RAM"))) for this, but attempts so far have failed (it makes a massive binary!)

// Get how many samples are in the buffer
int audioRingBufGetSamples() {
  int s = (int)audioRingIdxIn - (int)audioRingIdxOut;
  if (s<0) s+=I2S_RING_BUFFER_SIZE;
  return s;
}

// Fill the DMA buffer, return false if it fails
bool fillDMAFromRingBuffer() {
  int samples = audioRingBufGetSamples();
  if (!samples) return false; // no data to play at all!

  volatile int16_t *dmaPtr = i2sDMAbuf[i2sDMAidx ? 0 : 1];
  int count = I2S_DMA_BUFFER_SIZE>>1; // Mono->stereo
  int padding = 0;
  if (count > samples) {
    padding = count-samples;
    count = samples; // not enough samples in our ring buffer
  }
  static int16_t lastSample = 0;
  int16_t sample = lastSample;
  // mono -> stereo
  while (count--) {
    sample = audioRingBuf[audioRingIdxOut];
    audioRingIdxOut = (audioRingIdxOut+1) & (I2S_RING_BUFFER_SIZE-1);
    *(dmaPtr++) = sample; // L
    *(dmaPtr++) = sample; // R
  }
  // if we don't have enough samples, pad it out
  while (padding--) {
    *(dmaPtr++) = sample; // L
    *(dmaPtr++) = sample; // R
  }
  lastSample = sample;
  return true;
}

// DMA IRQ handler
void DMA1_Stream4_IRQHandler(void) {
  if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4)==SET) {
    DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
    //jshPinOutput(LED1_PININDEX, LED1_ONSTATE); // debug

    i2sDMAidx = (uint8_t)DMA_GetCurrentMemoryTarget(DMA1_Stream4);
    // if we can't fill the next buffer, stop playback
    if (!fillDMAFromRingBuffer()) { // this takes around 0.1ms
      STM32_I2S_Stop();
    }

    //jshPinOutput(LED1_PININDEX, !LED1_ONSTATE); // debug
  }
}

void STM32_I2S_Prepare(int audioFreq) {
  if (i2sStatus == STM32_I2S_PLAYING) return; // if we're started then it's all ok!

  DMA_DeInit(DMA1_Stream4);
  while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}
  DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_FEIF4|DMA_IT_DMEIF4|DMA_IT_TEIF4|DMA_IT_HTIF4|DMA_IT_TCIF4);

  RCC_PLLI2SCmd(ENABLE);

  DMA_InitTypeDef  DMA_InitStructure;
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)i2sDMAbuf[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)I2S_DMA_BUFFER_SIZE; // it's in 16 bit samples (not bytes)
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);
  DMA_DoubleBufferModeConfig(DMA1_Stream4,(uint32_t)i2sDMAbuf[1], DMA_Memory_0);
  /* In an IRQ we might use:
    DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t)i2sDMAbuf[0], DMA_Memory_0);
    DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t)i2sDMAbuf[1], DMA_Memory_1);
  But between them DMA_Init and DMA_DoubleBufferModeConfig set the same registers
  */
  DMA_DoubleBufferModeCmd(DMA1_Stream4, ENABLE);

  // setup relevant counters
  i2sDMAidx = (uint8_t)DMA_GetCurrentMemoryTarget(DMA1_Stream4);
  audioRingIdxIn = 0;
  audioRingIdxOut = 0;
  i2sStatus = STM32_I2S_STOPPED;

  // debugging - various beeps (we shouldn't hear them at all if it's working right!)
  /*for (int i=0;i<I2S_DMA_BUFFER_SIZE;i++) {
    i2sDMAbuf[0][i] = ((i&3)<2) ? -30000 : 30000;
    i2sDMAbuf[1][i] = ((i&7)<4) ? -30000 : 30000;
  }
  for (int i=0;i<I2S_RING_BUFFER_SIZE;i++) {
    audioRingBuf[i] = ((i&15)<8) ? -30000 : 30000;
  }*/

  DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
}


void STM32_I2S_Init() {
  i2sStatus = STM32_I2S_STOPPED;

  I2S_InitTypeDef I2S_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13; // PB12=LRCK, PB13=SCLK
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // SCLK runs at 1.04 MHz - ST datasheet says "Low Speed" is OK at 8 MHz if VDD>2.7V and pin load capacitance < 10pF
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6; // PC2=ASDOUT (ouput from IC, so should not be set as STM32 output), PC3=DSDIN, PC6=MCLK
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // MCLK runs at 4.16 MHz - ST datasheet says "Low Speed" is OK at 8 MHz if VDD>2.7V and pin load capacitance < 10pF
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_SPI2); // PB12,AF5  I2S_LRCK
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); // PB13,AF5  I2S_SCLK
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);  // PC3 ,AF5  I2S_DACDATA
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_SPI2);  // PC6 ,AF5  I2S_MCK
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource2,GPIO_AF_SPI3);  // PC2 ,AF6  I2S_ADCDATA (AF6 apparently?) - RB 2024-11-25: we're not using this, so should we remove it?

  I2S_InitStructure.I2S_Mode=I2S_Mode_MasterTx;
  I2S_InitStructure.I2S_Standard=I2S_Standard_Phillips;
  I2S_InitStructure.I2S_DataFormat=I2S_DataFormat_16bextended;
  I2S_InitStructure.I2S_MCLKOutput=I2S_MCLKOutput_Enable;
  I2S_InitStructure.I2S_AudioFreq=I2S_AudioFreq_16k;
  I2S_InitStructure.I2S_CPOL=I2S_CPOL_Low;
  I2S_Init(SPI2,&I2S_InitStructure);

  SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);
  I2S_Cmd(SPI2,ENABLE);

  NVIC_InitTypeDef   NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void STM32_I2S_Kill() {

  I2S_Cmd(SPI2,DISABLE);
  DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, DISABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
  RCC_PLLI2SCmd(DISABLE);

  // deconfigure pins
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource2,GPIO_AF_MCO);

  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/// Return the amount of free samples available for STM32_I2S_AddSamples
int STM32_I2S_GetFreeSamples() {
  return I2S_RING_BUFFER_SIZE - audioRingBufGetSamples();
}

// Add samples to the ringbuffer
void STM32_I2S_AddSamples(int16_t *data, unsigned int count) {
  // Try and fill until ringbuffer is full
  unsigned int freeSamples = STM32_I2S_GetFreeSamples();
  unsigned int c = count;
  if (c > freeSamples) {
    c = freeSamples;
    count -= freeSamples;
  } else count = 0;
  while (c--) {
    audioRingBuf[audioRingIdxIn] = *(data++)
                                 + (audioRingIdxIn&1); // add 1 bit of noise to stop DAC from turning off!
    audioRingIdxIn = (audioRingIdxIn+1) & (I2S_RING_BUFFER_SIZE-1);
  }

  //jsiConsolePrintf("add %d %d %d %d\n", i2sDMAidx, i2sStatus, count, audioRingBufGetSamples());
  // start playback if we have enough
  if (i2sStatus == STM32_I2S_STOPPED && audioRingBufGetSamples()>=I2S_DMA_BUFFER_SIZE*3) {
    // if audioRingBufGetSamples()>I2S_DMA_BUFFER_SIZE*3 we should have enough here to fill 6 buffers
    i2sDMAidx = !DMA_GetCurrentMemoryTarget(DMA1_Stream4);
    fillDMAFromRingBuffer(); // fill the first buffer with what we're currently reading from!
    i2sDMAidx = !i2sDMAidx;
    fillDMAFromRingBuffer(); // fill the second buffer
    STM32_I2S_Start();
  }

  if (!count) return;
  // otherwise we still have more to put in - try and wait a bit
  if (STM32_I2S_GetFreeSamples() < (int)count) {
    // wait until we have space
    int timeout = 1000000;
    while ((STM32_I2S_GetFreeSamples() < (int)count) && --timeout); // wait here for space
  }
  // now attempt to put in the rest but drop any we don't have space for
  c = count;
  while (c-- && STM32_I2S_GetFreeSamples()) {
    audioRingBuf[audioRingIdxIn] = *(data++)
                                 + (audioRingIdxIn&1); // add 1 bit of noise to stop DAC from turning off!
    audioRingIdxIn = (audioRingIdxIn+1) & (I2S_RING_BUFFER_SIZE-1);
  }
}

void STM32_I2S_Start() {
  i2sStatus = STM32_I2S_PLAYING;
  DMA_Cmd(DMA1_Stream4, ENABLE);
}

void STM32_I2S_Stop() {
  i2sStatus = STM32_I2S_STOPPED;
  DMA_Cmd(DMA1_Stream4, DISABLE);
}

/// Input stream has ended - so no new data is coming. If we didn't have enough data in our buffer to start playing yet, start playing anyway
void STM32_I2S_StreamEnded() {
  if (STM32_I2S_GetStatus() == STM32_I2S_STOPPED) {
    i2sDMAidx = !DMA_GetCurrentMemoryTarget(DMA1_Stream4);
    fillDMAFromRingBuffer();
    i2sDMAidx = !i2sDMAidx;
    fillDMAFromRingBuffer(); // fill the second buffer
    STM32_I2S_Start();
  }
}

/// Get a pointer to the pending buffer of samples (least likely to be overwritten!)
uint16_t *STM32_I2S_GetSampleBufferPtr() {
  return &i2sDMAbuf[i2sDMAidx];
}

#endif
