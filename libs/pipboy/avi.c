#include "jsutils.h"
#include "jsinteractive.h"
#include "avi.h"

typedef struct {
  uint32_t dwMicroSecPerFrame; // frame display rate (or 0)
  uint32_t dwMaxBytesPerSec; // max. transfer rate
  uint32_t dwPaddingGranularity; // pad to multiples of this size
  uint32_t dwFlags; // the ever-present flags
  uint32_t dwTotalFrames; // # frames in file
  uint32_t dwInitialFrames;
  uint32_t dwStreams;
  uint32_t dwSuggestedBufferSize;
  uint32_t dwWidth;
  uint32_t dwHeight; // 480
  uint32_t dwReserved[4];
} PACKED_FLAGS MainAVIHeader;

typedef struct {
  uint32_t fccType;
  uint32_t fccHandler;
  uint32_t dwFlags;
  uint16_t wPriority;
  uint16_t wLanguage;
  uint32_t dwInitialFrames;
  uint32_t dwScale;
  uint32_t dwRate; /* dwRate / dwScale == samples/second */
  uint32_t dwStart;
  uint32_t dwLength; /* In units above... */
  uint32_t dwSuggestedBufferSize;
  uint32_t dwQuality;
  uint32_t dwSampleSize;
  struct
  {
    uint16_t Left;
    uint16_t Top;
    uint16_t Right;
    uint16_t Bottom;
  } PACKED_FLAGS  rcFrame;
} PACKED_FLAGS AVIStreamHeader;

bool is4CC(uint8_t *ptr, const char *fourcc) {
  return
    ptr[0]==fourcc[0] &&
    ptr[1]==fourcc[1] &&
    ptr[2]==fourcc[2] &&
    ptr[3]==fourcc[3];
}
uint32_t getU32(uint8_t *buf) {
  return *(uint32_t*)buf; // little endian
}
// find the entry in the list, or return 0. Returns the offset of the data (so -4=size)
uint8_t *riffListFind(uint8_t *buf, const char *fourcc) {
  assert(is4CC(buf, "LIST"));
  uint32_t listEnd = getU32(&buf[4])+8;
  uint32_t offs=12;
  while (offs<listEnd) {
    if (is4CC(&buf[offs], fourcc)) {
      return &buf[offs+8];
    }
    offs += (getU32(&buf[offs+4])+8+1) & ~1U; // round to nearest 2
  }
  return 0;
}
uint8_t *riffGetIndex(uint8_t *buf, int idx, const char *fourcc) {
  assert(is4CC(buf, "LIST"));
  uint32_t listEnd = getU32(&buf[4])+8;
  uint32_t offs=12;
  while (offs<listEnd) {
    if (idx <= 0) {
      if (!is4CC(&buf[offs], fourcc)) {
        jsExceptionHere(JSET_ERROR, "RIFF LIST idx %d %c%c%c%c not correct\n", idx, buf[offs],buf[offs+1],buf[offs+2],buf[offs+3]);
        return 0;
      }
      return &buf[offs+8];
    }
    idx--;
    offs += (getU32(&buf[offs+4])+8+1) & ~1U; // round to nearest 2
  }
  return 0;
}
void riffListShow(uint8_t *buf, int pad) {
  assert(is4CC(buf, "LIST"));
  for (int i=0;i<pad;i++) jsiConsolePrintf("  ");
  jsiConsolePrintf("LIST %c%c%c%c\n", buf[8],buf[9],buf[10],buf[11]);
  uint32_t listEnd = getU32(&buf[4])+8;
  uint32_t offs=12;
  while (offs<listEnd) {
    for (int i=0;i<pad;i++) jsiConsolePrintf("  ");
    if (is4CC(&buf[offs], "LIST")) {
      riffListShow(&buf[offs], pad+1);
    } else
      jsiConsolePrintf("- %c%c%c%c\n", buf[offs+0],buf[offs+1],buf[offs+2],buf[offs+3]);
    offs += (getU32(&buf[offs+4])+8+1) & ~1U; // round to nearest 2
  }
}

bool aviLoad(uint8_t *buf, int len) {
  if (!is4CC(buf,"RIFF")) {
    jsExceptionHere(JSET_ERROR, "Not RIFF\n");
    return 0;
  }
  if (!is4CC(&buf[8],"AVI ")) {
    jsExceptionHere(JSET_ERROR, "Not AVI\n");
    return 0;
  }
  buf += 12; // skip RIFF+length+AVI tag
  if (!is4CC(buf,"LIST") || !is4CC(&buf[8],"hdrl")) {
    jsExceptionHere(JSET_ERROR, "Not LIST hdrl\n");
    return 0;
  }
  uint8_t *listPtr = buf;
  riffListShow(listPtr,0);
  MainAVIHeader *aviHeader = (MainAVIHeader*)riffGetIndex(listPtr, 0, "avih");
  if (!aviHeader) {
    jsExceptionHere(JSET_ERROR, "No header found\n");
    return 0;
  }
  jsiConsolePrintf("AVI w=%d h=%d fps=%d\n",aviHeader->dwWidth, aviHeader->dwHeight, 1000000/aviHeader->dwMicroSecPerFrame);
  for (int stream=0;stream<aviHeader->dwStreams;stream++) {
    uint8_t *streamListPtr = riffGetIndex(listPtr, 1+stream, "LIST")-8;
    if (!streamListPtr) {
      jsExceptionHere(JSET_ERROR, "No stream list %d\n");
      return 0;
    }
    AVIStreamHeader *streamHeader = (AVIStreamHeader*)riffGetIndex(streamListPtr, 0, "strh");
    if (!streamHeader) {
      jsExceptionHere(JSET_ERROR, "No stream %d\n");
      return 0;
    }
    char buf[10];
    memcpy(buf,&streamHeader->fccType,4);
    buf[4]=' ';
    memcpy(&buf[5],&streamHeader->fccHandler,4);
    buf[9]=0;
    jsiConsolePrintf("Stream %d %s %d\n", stream, buf, streamHeader->dwStart);
    if (is4CC(&streamHeader->fccType,"vids")) {
    } else if (is4CC(&streamHeader->fccType,"auds")) {
    }
  }
}