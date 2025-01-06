/*
 * Copyright (c) 2023, Dominic Szablewski - https://phoboslab.org
 * SPDX-License-Identifier: MIT
 *
 * QOA - The "Quite OK Audio" format for fast, lossy audio compression
 */

#include "qoa.h"


/* -----------------------------------------------------------------------------
	Implementation */

typedef unsigned long long qoa_uint64_t;

/* The dequant_tab maps each of the scalefactors and quantized residuals to
their unscaled & dequantized version.

Since qoa_div rounds away from the zero, the smallest entries are mapped to 3/4
instead of 1. The dequant_tab assumes the following dequantized values for each
of the quant_tab indices and is computed as:
float dqt[8] = {0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7};
dequant_tab[s][q] <- round_ties_away_from_zero(scalefactor_tab[s] * dqt[q])

The rounding employed here is "to nearest, ties away from zero",  i.e. positive
and negative values are treated symmetrically.
*/

// note: this is a modified version that uses a quarter of the storage the original one used

static const int16_t qoa_dequant_tab[16][4] = {
  {  1,     3,    5,     7},
  {  5,    18,   32,    49},
  { 16,    53,   95,   147},
  { 34,   113,  203,   315},
  { 63,   210,  378,   588},
  { 104,  345,  621,   966},
  { 158,  528,  950,  1477},
  { 228,  760, 1368,  2128},
  { 316, 1053, 1895,  2947},
  { 422, 1405, 2529,  3934},
  { 548, 1828, 3290,  5117},
  { 696, 2320, 4176,  6496},
  { 868, 2893, 5207,  8099},
  {1064, 3548, 6386,  9933},
  {1286, 4288, 7718, 12005},
  {1536, 5120, 9216, 14336},
};


/* The Least Mean Squares Filter is the heart of QOA. It predicts the next
sample based on the previous 4 reconstructed samples. It does so by continuously
adjusting 4 weights based on the residual of the previous prediction.

The next sample is predicted as the sum of (weight[i] * history[i]).

The adjustment of the weights is done with a "Sign-Sign-LMS" that adds or
subtracts the residual to each weight, based on the corresponding sample from
the history. This, surprisingly, is sufficient to get worthwhile predictions.

This is all done with fixed point integers. Hence the right-shifts when updating
the weights and calculating the prediction. */

static int qoa_lms_predict(qoa_lms_t *lms) {
  int prediction = 0;
  for (int i = 0; i < QOA_LMS_LEN; i++) {
    prediction += lms->weights[i] * lms->history[i];
  }
  return prediction >> 13;
}

static void qoa_lms_update(qoa_lms_t *lms, int sample, int residual) {
  int delta = residual >> 4;
  for (int i = 0; i < QOA_LMS_LEN; i++) {
    lms->weights[i] += lms->history[i] < 0 ? -delta : delta;
  }

  for (int i = 0; i < QOA_LMS_LEN - 1; i++) {
    lms->history[i] = lms->history[i + 1];
  }
  lms->history[QOA_LMS_LEN - 1] = sample;
}

static inline int qoa_clamp(int v, int min, int max) {
  if (v < min) { return min; }
  if (v > max) { return max; }
  return v;
}

/* This specialized clamp function for the signed 16 bit range improves decode
performance quite a bit. The extra if() statement works nicely with the CPUs
branch prediction as this branch is rarely taken. */

static inline int qoa_clamp_s16(int v) {
  if ((unsigned int) (v + 32768) > 65535) {
    if (v < -32768) { return -32768; }
    if (v > 32767) { return 32767; }
  }
  return v;
}

static inline qoa_uint64_t qoa_read_u64(const unsigned char *bytes, unsigned int *p) {
  bytes += *p;
  *p += 8;
  return
      ((qoa_uint64_t) (bytes[0]) << 56) | ((qoa_uint64_t) (bytes[1]) << 48) |
      ((qoa_uint64_t) (bytes[2]) << 40) | ((qoa_uint64_t) (bytes[3]) << 32) |
      ((qoa_uint64_t) (bytes[4]) << 24) | ((qoa_uint64_t) (bytes[5]) << 16) |
      ((qoa_uint64_t) (bytes[6]) << 8) | ((qoa_uint64_t) (bytes[7]) << 0);
}


/* -----------------------------------------------------------------------------
	Decoder */

unsigned int qoa_decode_header(const unsigned char *bytes, int size, qoa_desc *qoa) {
  unsigned int p = 0;
  if (size < QOA_MIN_FILESIZE) {
    return 0;
  }


  /* Read the file header, verify the magic number ('qoaf') and read the
  total number of samples. */
  qoa_uint64_t file_header = qoa_read_u64(bytes, &p);

  if ((file_header >> 32) != QOA_MAGIC) {
    return 0;
  }

  qoa->samples = file_header & 0xffffffff;
  if (!qoa->samples) {
    return 0;
  }

  /* Peek into the first frame header to get the number of channels and
  the samplerate. */
  qoa_uint64_t frame_header = qoa_read_u64(bytes, &p);
  qoa->channels = (frame_header >> 56) & 0x0000ff;
  qoa->samplerate = (frame_header >> 32) & 0xffffff;

  if (qoa->channels == 0 || qoa->samples == 0 || qoa->samplerate == 0) {
    return 0;
  }

  return 8;
}

unsigned int qoa_decode_frame(const unsigned char *bytes, unsigned int size, qoa_desc *qoa,
                              JsvStringIterator *sample_data_it, unsigned int *frame_len, bool use16Bit) {
  unsigned int p = 0;
  *frame_len = 0;

  if (size < 8 + QOA_LMS_LEN * 4 * qoa->channels) {
    return 0;
  }

  /* Read and verify the frame header */
  qoa_uint64_t frame_header = qoa_read_u64(bytes, &p);
  unsigned int channels = (frame_header >> 56) & 0x0000ff;
  unsigned int samplerate = (frame_header >> 32) & 0xffffff;
  unsigned int samples = (frame_header >> 16) & 0x00ffff;
  unsigned int frame_size = (frame_header) & 0x00ffff;

  unsigned int data_size = frame_size - 8 - QOA_LMS_LEN * 4 * channels;
  unsigned int num_slices = data_size / 8;
  unsigned int max_total_samples = num_slices * QOA_SLICE_LEN;

  if (
    channels != qoa->channels ||
    samplerate != qoa->samplerate ||
    frame_size > size ||
    samples * channels > max_total_samples
  ) {
    return 0;
  }


  /* Read the LMS state: 4 x 2 bytes history, 4 x 2 bytes weights per channel */
  for (unsigned int c = 0; c < channels; c++) {
    qoa_uint64_t history = qoa_read_u64(bytes, &p);
    qoa_uint64_t weights = qoa_read_u64(bytes, &p);
    for (int i = 0; i < QOA_LMS_LEN; i++) {
      qoa->lms[c].history[i] = ((signed short) (history >> 48));
      history <<= 16;
      qoa->lms[c].weights[i] = ((signed short) (weights >> 48));
      weights <<= 16;
    }
  }


  /* Decode all slices for all channels in this frame */
  for (unsigned int sample_index = 0; sample_index < samples; sample_index += QOA_SLICE_LEN) {
    for (unsigned int c = 0; c < channels; c++) {
      qoa_uint64_t slice = qoa_read_u64(bytes, &p);

      int scalefactor = (slice >> 60) & 0xf;
      slice <<= 4;

      int slice_start = sample_index * channels + c;
      int slice_end = qoa_clamp(sample_index + QOA_SLICE_LEN, 0, samples) * channels + c;

      for (int si = slice_start; si < slice_end; si += channels) {
        int predicted = qoa_lms_predict(&qoa->lms[c]);
        int quantized = (slice >> 61) & 0x7;
        int dequantized = qoa_dequant_tab[scalefactor][quantized / 2];
        if (quantized % 2 == 1) {
          dequantized = -dequantized;
        }
        int reconstructed = qoa_clamp_s16(predicted + dequantized);

        if (use16Bit) {
          uint16_t value = (uint16_t) reconstructed ^ 0x8000;
          for (size_t j = 0; j < 2; j++) {
            uint8_t setValue = ((value >> (j * 8)) & 0xff);
            jsvStringIteratorSetChar(sample_data_it, (char) (setValue));
            jsvStringIteratorNext(sample_data_it);
          }
        } else {
          uint8_t value = ((uint16_t) reconstructed ^ 0x8000) >> 8;
          jsvStringIteratorSetChar(sample_data_it, (char) value);
          jsvStringIteratorNext(sample_data_it);
        }
        slice <<= 3;

        qoa_lms_update(&qoa->lms[c], reconstructed, dequantized);
      }
    }
  }

  *frame_len = samples;
  return p;
}
