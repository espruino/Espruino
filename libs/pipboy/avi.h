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
 * AVI/WAV file decode
 *
 * ---------------------------------------------------------------------------- */
#ifndef __AVI_H
#define __AVI_H

typedef struct {
  int width, height, usPerFrame;
  int streamOffset; // offset in the file of the start of data
  uint16_t palette[256];
  int audioSampleRate;
  int audioBufferSize;
} AviInfo;
typedef struct {
  int streamOffset; // offset in the file of the start of data
  int sampleSize; // 16=16 bit, or 4 for ADPCM
  int audioSampleRate;
  int blockAlign; // for ADPCM, blocks we pass to decoder should be a multiple of this size
  uint16_t formatTag;
} WavInfo;

const uint16_t WAVFMT_RAW = 1;
const uint16_t WAVFMT_IMA_ADPCM = 0x11; // https://wiki.multimedia.cx/index.php/Microsoft_IMA_ADPCM

#define AVI_STREAM_AUDIO 0x6277
#define AVI_STREAM_VIDEO 0x6364

bool aviLoad(uint8_t *buf, int len, AviInfo *result, bool debugInfo);
bool wavLoad(uint8_t *buf, int len, WavInfo *result, bool debugInfo);


// decode IMA-encoded data, return number of samples created
int wavIMADecode(WavInfo *wavInfo, uint8_t *bufin, int16_t *bufout, int len);

#endif
