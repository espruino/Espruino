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
