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
 * AVI file decode
 *
 * ---------------------------------------------------------------------------- */
#ifndef __AVI_H
#define __AVI_H

typedef struct {
  int width, height, usPerFrame;
  int videoOffset;
  uint16_t palette[256];
  int audioSampleRate;
  int audioBufferSize;
} AviInfo;

#define AVI_STREAM_AUDIO 0x6277
#define AVI_STREAM_VIDEO 0x6364

bool aviLoad(uint8_t *buf, int len, AviInfo *result, bool debugInfo);

#endif
