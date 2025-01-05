/*
 * Copyright (c) 2023, Dominic Szablewski - https://phoboslab.org
 * SPDX-License-Identifier: MIT
 *
 * QOA - The "Quite OK Audio" format for fast, lossy audio compression
 */

#ifndef QOA_H
#define QOA_H

#include "jsvar.h"
#include "jsvariterator.h"

#define QOA_MIN_FILESIZE 16
#define QOA_MAX_CHANNELS 8

#define QOA_SLICE_LEN 20
#define QOA_SLICES_PER_FRAME 256
#define QOA_FRAME_LEN (QOA_SLICES_PER_FRAME * QOA_SLICE_LEN)
#define QOA_LMS_LEN 4
#define QOA_MAGIC 0x716f6166 /* 'qoaf' */

#define QOA_FRAME_SIZE(channels, slices) \
(8 + QOA_LMS_LEN * 4 * channels + 8 * slices * channels)

typedef struct {
  int history[QOA_LMS_LEN];
  int weights[QOA_LMS_LEN];
} qoa_lms_t;

typedef struct {
  unsigned int channels;
  unsigned int samplerate;
  unsigned int samples;
  qoa_lms_t lms[QOA_MAX_CHANNELS];
} qoa_desc;

unsigned int qoa_decode_header(const unsigned char *bytes, int size, qoa_desc *qoa);

unsigned int qoa_decode_frame(const unsigned char *bytes, unsigned int size, qoa_desc *qoa,
                              JsvStringIterator *sample_data_it, unsigned int *frame_len, bool use16Bit);

#endif //QOA_H
