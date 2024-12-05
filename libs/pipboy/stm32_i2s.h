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
 * ---------------------------------------------------------------------------- */
#ifndef __STM32_I2S_H
#define __STM32_I2S_H

#include "jsutils.h"

#define I2S_DMA_BUFFER_SIZE 2048 // size of i2sDMAbuf (DMA direct to I2S) in u16
// 16kHz sample rate, 2xu16 = ~16Hz IRQ rate
#define I2S_RING_BUFFER_SIZE 8192 // size of ringbuffer used for audio input in u16
// 8192 seems fine to use - still enough for 8 DMA packets worth/0.5sec...

/* jswrap_pb_audioFrame sends data in 2048 byte chunks and STM32_I2S_AddSamples
starts playback at 3*I2S_DMA_BUFFER_SIZE. So I2S_RING_BUFFER_SIZE=8192
is the least we can use, since any less and 3*I2S_DMA_BUFFER_SIZE would be
big enough that the next sample from jswrap_pb_audioFrame would fill the buffer */


typedef enum {
  STM32_I2S_STOPPED,
  STM32_I2S_PLAYING
} STM32_I2S_Status;

/// Initialise the I2S peripheral and IO (do this at startup)
void STM32_I2S_Init();
/// Stop the I2S peripheral so we can sleep ok
void STM32_I2S_Kill();
/// Prepare for start of playback
void STM32_I2S_Prepare(int audioFreq);
/// Return the amount of free samples available for STM32_I2S_AddSamples
int STM32_I2S_GetFreeSamples();
/// Add new Samples - playback will start when we have enough in buffer. count=# of samples (not bytes)
void STM32_I2S_AddSamples(int16_t *data, unsigned int count);
/// Start playback (ideally don't use this - just add samples and playback will start)
void STM32_I2S_Start();
/// Stop playback
void STM32_I2S_Stop();
/// Input stream has ended - so no new data is coming. If we didn't have enough data in our buffer to start playing yet, start playing anyway
void STM32_I2S_StreamEnded();
/// Get status
STM32_I2S_Status STM32_I2S_GetStatus();
/// Get a pointer to the pending buffer of samples (least likely to be overwritten!)
uint16_t *STM32_I2S_GetSampleBufferPtr();

#endif
