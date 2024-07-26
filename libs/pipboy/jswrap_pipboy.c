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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Pipboy
 *
 * FFMPEG example to convert "Atomic Command" intro animation (frames.avi) extracted from SWF file in Fallout Pip-Boy APK:
 * - change colour to green
 * - crop 1289x937 to 1080x810 (to remove offset for phone screen)
 * - re-encode as 320x240 RLE at 12fps
 * ffmpeg -i frames.avi -filter_complex 'colorchannelmixer=0:0:0:0:0:1:0:0:0:0:0:0,crop=1080:810:0:0' -vcodec msrle -s 320x240 -r 12 intro-320x240-RLE-12fps.avi
 *
 * Convert the Pip-Boy bootup animation from Kilter:
 * - crop 1920x1080 to 1440x1080 to remove offset
 * - convert audio to 16kHz mono, 16-bit little-endian PCM
 * - re-encode video as 320x240 RLE at 12fps
 * ffmpeg -i ND-PB_Bootup01-v01_Waudio.mp4 -ar 16000 -ac 1 -acodec pcm_s16le -vf crop=1440:1080:0:0 -vcodec msrle -s 320x240 -r 12 bootup-320x240-RLE-12fps.avi
 *
 * ----------------------------------------------------------------------------
 */
/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */


#include "jswrap_pipboy.h"
#include "platform_config.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jsflags.h"
#include "jshardware.h"
#include "jswrap_fs.h"
#include "jswrap_file.h"
#include "graphics.h"
#include "stm32_i2s.h"
#ifndef LINUX

#include "lcd_fsmc.h"
#endif

#include "avi.h"

#define STREAM_BUFFER_SIZE 40960
uint8_t streamBuffer[STREAM_BUFFER_SIZE+4] __attribute__ ((aligned (8))); // we add 4 to allow our unaligned fread hack to work
// Can't go in 64k CCM RAM because no DMA is allowed to CCM!
// Maybe if we modified DMA to read to a buffer first?

typedef enum {
  ST_NONE,
  ST_AVI,
  ST_WAV
} StreamType;

StreamType streamType = ST_NONE;
int streamBufferLen;
uint16_t streamPacketId;
uint32_t streamPacketLen;        // length of current stream
uint32_t streamPacketRemaining;  // length left to read in current stream
uint32_t streamPacketBufferLen;  // length of stream in current buffer
File_Handle streamFile; // The Video/Audio stream's file

JsSysTime videoFrameTime;
JsSysTime videoNextFrameTime;
bool debugInfo = false;
int startX=0;
int startY=0;
AviInfo videoInfo;

typedef enum {
  DM_OFF,
  DM_Output
} DACMode;


#ifdef LINUX // Linux -> FatFS hacks
void f_close(File_Handle *f) {
  fclose(*f);
}
void f_lseek(File_Handle *f, uint32_t offset) {
  fseek(*f, offset, SEEK_SET);
}
FRESULT f_read(File_Handle* fp, void* buff, uint32_t btr, size_t* br) {
  assert((((size_t)buff)&7)==0);
  *br = fread(buff, 1, btr, *fp);
  return *br!=0;
}
#else // not LINUX


#endif

/*JSON{
    "type": "class",
    "class" : "Pip"
}
*/
/*JSON{
  "type" : "event",
  "class" : "Pip",
  "name" : "streamStarted"
}
The video had ended
*/
/*JSON{
  "type" : "event",
  "class" : "Pip",
  "name" : "streamStopped"
}
The video had ended
*/
void jswrap_pb_sendEvent(const char *eventName) { // eg JS_EVENT_PREFIX"streamStarted"
   JsVar *pip =jsvObjectGetChildIfExists(execInfo.root, "Pip");
  if (pip)
    jsiQueueObjectCallbacks(pip, eventName, NULL, 0);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "videoStart",
  "generate" : "jswrap_pb_videoStart",
  "params" : [
      ["fn","JsVar","Filename"],
      ["options","JsVar","[Optional] object `{x:0, y:0, debug:false}`"]
   ]
}
*/
void jswrap_pb_videoStart(JsVar *fn, JsVar *options) {

  startX=0;
  startY=0;
  debugInfo=false;
  JsVar *v;
  if (jsvIsObject(options)) {
    v = jsvObjectGetChildIfExists(options, "x");
    if (v) {
      startX = jsvGetIntegerAndUnLock(v);
    }
    v = jsvObjectGetChildIfExists(options, "y");
    if (v) {
      startY = jsvGetIntegerAndUnLock(v);
    }
    v = jsvObjectGetChildIfExists(options, "debug");
    if (v) {
      if (jsvGetBoolAndUnLock(v)) debugInfo = true;
      else debugInfo = false;
    }
  }
  if (debugInfo)
    jsiConsolePrintf("Playing video at x0=%d, y0=%d\n", startX, startY);

  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsfsGetPathString(pathStr, fn)) return;

  if (jsfsInit()) {
    if (streamType) {
      f_close(&streamFile);
      streamType = ST_NONE;
    }

  FRESULT res;
#ifdef LINUX
  if ((streamFile = fopen(pathStr, "r"))) {
#else
    BYTE ff_mode = FA_READ | FA_OPEN_EXISTING;
    if ((res=f_open(&streamFile, pathStr, ff_mode)) == FR_OK) {
#endif

      streamType = ST_AVI;
      size_t actual = 0;
      res = f_read(&streamFile, (uint8_t*)streamBuffer, STREAM_BUFFER_SIZE, &actual);
      if (debugInfo) {
        jsiConsolePrintf("AVI read %d %d %c%c%c%c\n", STREAM_BUFFER_SIZE, actual, streamBuffer[0],streamBuffer[1],streamBuffer[2],streamBuffer[3]);
      }
      if (aviLoad(streamBuffer, (int)actual, &videoInfo, debugInfo)) {
        streamPacketId = *(uint16_t*)&streamBuffer[videoInfo.streamOffset+2]; // +0 = '01'/'00' stream index?
        streamPacketLen = *(uint32_t*)&streamBuffer[videoInfo.streamOffset+4]; // +0 = '01'/'00' stream index?
        f_lseek(&streamFile, (uint32_t)(videoInfo.streamOffset+8)); // go back to start of video data
        videoFrameTime = jshGetTimeFromMilliseconds(videoInfo.usPerFrame/1000.0);
        videoNextFrameTime = jshGetSystemTime() + videoFrameTime;
#ifndef LINUX
        // Set up Audio
        if (videoInfo.audioBufferSize <= I2S_RING_BUFFER_SIZE*2) { // IF we have audio
          STM32_I2S_Prepare(videoInfo.audioSampleRate);
          // playback will start when the buffer is full enough
        } else if (videoInfo.audioBufferSize) {
          jsiConsolePrintf("Audio stream too big (%db)\n", videoInfo.audioBufferSize);
        }
#endif
        jswrap_pb_sendEvent(JS_EVENT_PREFIX"streamStarted");
      } else {
        jsExceptionHere(JSET_ERROR, "Corrupt video\n");
        jswrap_pb_videoStop();
      }
    }
  } else {
    jsExceptionHere(JSET_ERROR, "Can't load file\n");
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "videoStop",
  "generate" : "jswrap_pb_videoStop"
}
*/
void jswrap_pb_videoStopLetAudioRun() {
  if (streamType) {
    f_close(&streamFile);
    streamType = ST_NONE;
    jswrap_pb_sendEvent(JS_EVENT_PREFIX"streamStopped");
  }
}
void jswrap_pb_videoStop() {
  if (streamType) {
    STM32_I2S_Stop(); // Stop audio immediately
  }
  jswrap_pb_videoStopLetAudioRun();
}


/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "audioStart",
  "generate" : "jswrap_pb_audioStart",
  "params" : [
      ["fn","JsVar","Filename"],
      ["options","JsVar","[Optional] object `{debug:false}`"]
   ]
}
*/
void jswrap_pb_audioStart(JsVar *fn, JsVar *options) {
  debugInfo=false;
  JsVar *v;
  if (jsvIsObject(options)) {
    v = jsvObjectGetChildIfExists(options, "debug");
    if (v) {
      if (jsvGetBoolAndUnLock(v)) debugInfo = true;
      else debugInfo = false;
    }
  }
  if (debugInfo)
    jsiConsolePrintf("Playing audio\n");

  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsfsGetPathString(pathStr, fn)) return;

  if (jsfsInit()) {
    if (streamType) {
      f_close(&streamFile);
      streamType = ST_NONE;
    }

  FRESULT res;
#ifdef LINUX
  if ((streamFile = fopen(pathStr, "r"))) {
#else
    BYTE ff_mode = FA_READ | FA_OPEN_EXISTING;
    if ((res=f_open(&streamFile, pathStr, ff_mode)) == FR_OK) {
#endif

      streamType = ST_WAV;
      size_t actual = 0;
      const int WAVHEADER_MAX = 256;
      res = f_read(&streamFile, (uint8_t*)streamBuffer, WAVHEADER_MAX, &actual);
      if (debugInfo) {
        jsiConsolePrintf("WAV read %d %d %c%c%c%c\n", WAVHEADER_MAX, actual, streamBuffer[0],streamBuffer[1],streamBuffer[2],streamBuffer[3]);
      }
      if (wavLoad(streamBuffer, (int)actual, &videoInfo, debugInfo)) {
        f_lseek(&streamFile, (uint32_t)(videoInfo.streamOffset)); // go back to start of video data
        STM32_I2S_Prepare(videoInfo.audioSampleRate);
        jswrap_pb_sendEvent(JS_EVENT_PREFIX"streamStarted");
        jsiConsolePrintf("Started...\n");
      } else {
        jsExceptionHere(JSET_ERROR, "Corrupt audio\n");
        jswrap_pb_videoStop();
      }
    }
  } else {
    jsExceptionHere(JSET_ERROR, "Can't load file\n");
  }
}


#ifdef LINUX
int px,py,ox,oy;
void lcdFSMC_blitStart(JsGraphics *gfx, int x, int y, int w, int h) {
  ox=x;
  oy=y;
  px=0;
  py=0;
}
void lcdFSMC_setCursor(JsGraphics *gfx, int x, int y) {
  px=x;
  py=y;
}
void lcdFSMC_blitPixel(unsigned int col) {
  if (py+oy>319 || px+ox>479) {
    jsiConsolePrintf("OOB pixel %d,%d\n", px+ox, py+oy);
  } else
    graphicsSetPixel(&graphicsInternal, px+ox, py+oy, col);
  px++;
}
void lcdFSMC_blitEnd() {
}
#endif

void jswrap_pb_videoFrame() {
  if (streamType != ST_AVI) return;
  //JsSysTime tStart = jshGetSystemTime();
  //if (debugInfo) jsiConsolePrintf("Stream 0x%04x, %d\n", streamPacketId, streamPacketLen);
  streamPacketRemaining = 0;
  streamPacketBufferLen = streamPacketLen+8; // 8 bytes contains info for next stream
  if (streamPacketLen > STREAM_BUFFER_SIZE) {
    streamPacketRemaining = streamPacketBufferLen-STREAM_BUFFER_SIZE;
    streamPacketBufferLen = STREAM_BUFFER_SIZE;
  }
  size_t actual = 0;
  //jsiConsolePrintf("=========== FIRST READ %d (%d)\n", ((size_t)streamBuffer)&7, streamPacketBufferLen);
  f_read(&streamFile, streamBuffer,streamPacketBufferLen, &actual);
  if (streamPacketId==AVI_STREAM_AUDIO) {
    if (streamPacketRemaining) {
      jsiConsolePrintf("Audio stream too big for streamBuffer\n");
      jswrap_pb_videoStop();
    } else {
      STM32_I2S_AddSamples((int16_t*)streamBuffer, streamPacketLen>>1); // l is in bytes, not samples
    }
  } else if (streamPacketId==AVI_STREAM_VIDEO) {
    lcdFSMC_blitStart(&graphicsInternal, startX,startY,videoInfo.width,videoInfo.height);
    int x=0, y=videoInfo.height-1;
    uint8_t *b = streamBuffer;
    uint8_t *endBuffer = &streamBuffer[streamPacketBufferLen];
    while (b < endBuffer) {
      // Loop doing RLE until end *or* we have more data and no RLE command could use up more than we have left
      uint8_t *nearlyEndOfBuffer = streamPacketRemaining ? &streamBuffer[streamPacketBufferLen-260] : endBuffer;
      while (b < nearlyEndOfBuffer) {
        // next RLE byte!
        uint8_t num = *(b++);
        if (num==0)  { // RLE code is 0 -> escape code for commands
          uint8_t cmd = *(b++);
          if (cmd==0) { // 0 = EOL
            x=0;
            y--;
            lcdFSMC_setCursor(&graphicsInternal, x+startX,y+startY);
          } else if (cmd==1) {
            //jsiConsolePrintf("end of bitmap!\n");
            b = endBuffer;
            // but why is there data after the EOB opcode??
          } else if (cmd==2) { // 2 = DELTA
            x += *(b++);
            y -= *(b++);
            lcdFSMC_setCursor(&graphicsInternal, x+startX,y+startY);
          } else {
            bool extraByte = cmd&1; // copy is padded
            while (cmd--) {
              lcdFSMC_blitPixel(videoInfo.palette[*(b++)]);
              x++;
            }
            if (extraByte) b++; // trailing 0 if
          }
        } else { // just a run of pixels
          uint8_t col = *(b++);
          while (num--) {
            lcdFSMC_blitPixel(videoInfo.palette[col]);
            x++;
          }
        }
      }
      // check if we need to refill our buffer
      if (streamPacketRemaining) {
        uint32_t amountToShift = (uint32_t)(b-streamBuffer) & ~7ul; // aligned to the nearest word (STM32 f_read fails otherwise!)
        uint32_t leftInStream = streamPacketBufferLen - amountToShift;
        memmove(streamBuffer, &streamBuffer[amountToShift], leftInStream); // move data back
        b -= amountToShift;
        streamPacketBufferLen = leftInStream;
        uint32_t bufferRemaining = STREAM_BUFFER_SIZE-leftInStream;
        uint32_t len = streamPacketRemaining;
        if (len>bufferRemaining) len=bufferRemaining;
        //jsiConsolePrintf("=========== READ %d (%d)\n", leftInStream, leftInStream&3);
        f_read(&streamFile, &streamBuffer[leftInStream], len, &actual);
        streamPacketRemaining -= len;
        streamPacketBufferLen += len;
      }
    }
    lcdFSMC_blitEnd();
    if (debugInfo) {
      //jswrap_pb_videoStop(); // first frame only
      //JsSysTime tEnd = jshGetSystemTime();
      //jsiConsolePrintf("%dms\n", (int)jshGetMillisecondsFromTime(tEnd-tStart));
    }
  } else {
    // unknown stream - assume end and stop!
    //jsiConsolePrintf("Unknown stream ID");
    jswrap_pb_videoStopLetAudioRun();
  }
  // get IDs for next stream
  streamPacketId = *(uint16_t*)&streamBuffer[streamPacketBufferLen-6]; // +0 = '01'/'00' stream index?
  streamPacketLen = *(uint32_t*)&streamBuffer[streamPacketBufferLen-4]; // +0 = '01'/'00' stream index?
  if (streamPacketId==AVI_STREAM_VIDEO) {
    // the next frame is a video frame, so ensure we parse it at the right time
    // if the frame is audio we want to parse it ASAP
    videoNextFrameTime += videoFrameTime;
  }
}

void jswrap_pb_audioFrame() {
  if (streamType != ST_WAV) return;
  const int WAV_CHUNK_SIZE = 4096; // how much do we want to read in one chunk?
  int WAV_SAMPLES = WAV_CHUNK_SIZE>>1;

  int freeSamples = STM32_I2S_GetFreeSamples();
  //if (debugInfo) jsiConsolePrintf("%d free\n", freeSamples);
  if (freeSamples <= WAV_SAMPLES)
    return; // if there's not space yet, don't do anything

  size_t actual = 0;

  f_read(&streamFile, streamBuffer, WAV_CHUNK_SIZE, &actual);
  //if (debugInfo) jsiConsolePrintf("A%d\n", actual);
  if (actual) {
    jswrap_pb_sendEvent(JS_EVENT_PREFIX"frame");
    STM32_I2S_AddSamples((int16_t*)streamBuffer, actual>>1);
  } else {
    // jsiConsolePrintf("End of WAV\n");
    jswrap_pb_videoStopLetAudioRun(); // Stop parsing, let audio finish
  }
}

void graphicsInternalFlip() {
  // No offscreen buffer (yet), no need to flip
}

void es8388_write_reg(int r,int d) {
  unsigned char data[2] = {(uint8_t)r,(uint8_t)d};
  jshI2CWrite(EV_I2C1, 0x10, 2, data, true);
}
int es8388_read_reg(int r) {
 unsigned char data[2] = {(uint8_t)r};
 jshI2CWrite(EV_I2C1, 0x10, 1, data, true);
 jshI2CRead(EV_I2C1, 0x10, 1, data, true);
 return data[0];
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "setVol",
  "generate" : "jswrap_pb_setVol",
  "params" : [
      ["vol","int",""]
   ]
}
*/
void jswrap_pb_setVol(int volume) {
  if (volume > 33) volume = 33;
  if (volume < 0) volume = 0;
  es8388_write_reg(0x2E, volume); // 46 LOUT1 (33 = max)
  es8388_write_reg(0x2F, volume); // 47 ROUT1
  es8388_write_reg(0x30, volume); // 48 LOUT2 (33 = max)
  es8388_write_reg(0x31, volume); // 49 ROUT2
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "setDACMode",
  "generate" : "jswrap_pb_setDACMode",
  "params" : [
      ["mode","JsVar","Mode string - see below"]
   ]
}
* 'off'/undefined -> off
* 'out' -> output
*/
void jswrap_pb_setDACMode_(DACMode mode) {
  if (mode == DM_OFF) {
    es8388_write_reg(0x0F, 0x34); // ADC Mute
    es8388_write_reg(0x19, 0x36); // DAC Mute
    es8388_write_reg(0x02, 0xF3); // Power down DEM and STM
    es8388_write_reg(0x03, 0xFC); // Power down ADC
    es8388_write_reg(0x04, 0xC0); // Power down DAC L/ROUT
  } else if (mode == DM_Output) {
    es8388_write_reg(0x19, 0x32); // Unmute DAC
    es8388_write_reg(0x04, 0x3C); // Power up DAC, Analog out
    es8388_write_reg(0x02, 0xAA); // power up DEM and STM
  } // TODO: passthru?
  //es8388_write_reg(0x, 0x); //
}
void jswrap_pb_setDACMode(JsVar *mode) {
  if (jsvIsUndefined(mode) || jsvIsStringEqual(mode, "off"))
    jswrap_pb_setDACMode_(DM_OFF);
  else if (jsvIsStringEqual(mode, "out"))
    jswrap_pb_setDACMode_(DM_Output);
  else {
    jsExceptionHere(JSET_ERROR, "Unknown mode %q\n", mode);
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "setLCDPower",
  "generate" : "jswrap_pb_setLCDPower",
  "params" : [
      ["isOn","bool",""]
   ]
}
*/
void jswrap_pb_setLCDPower(bool isOn) {
#ifndef LINUX
  lcdFSMC_setPower(isOn);
#endif
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_pb_init"
}*/
void jswrap_pb_init() {
#ifdef SD_POWER_PIN
  jshPinOutput(SD_POWER_PIN, 1); // Power supply enable for the SD card is also used for the ES8388 audio codec
#endif
#ifdef USE_AUDIO_CODEC
  // Initialise audio
  STM32_I2S_Init();
  // Initialise dac
  JshI2CInfo dacI2C;
  jshI2CInitInfo(&dacI2C);
  dacI2C.pinSCL = JSH_PORTB_COUNT+8;
  dacI2C.pinSDA = JSH_PORTB_COUNT+9;
  jshI2CSetup(EV_I2C1, &dacI2C);
  jshDelayMicroseconds(1000);
  es8388_write_reg(0x00, 0x80); // Reset the ES8388 control registers
  jshDelayMicroseconds(100);
  es8388_write_reg(0x00, 0x00);
  jshDelayMicroseconds(1000);
  // based on https://cdn.pcbartists.com/wp-content/uploads/2022/12/ES8388-user-guide-application-note.pdf
  // The sequence for Start up play back mode
  es8388_write_reg(0x08, 0x00); // slave mode
  es8388_write_reg(0x02, 0xF3); // power down DEM and STM
  es8388_write_reg(0x2B, 0x80); // Set same LRCK
  es8388_write_reg(0x00, 0x05); // FIXME Set Chip to Play&Record Mode
  es8388_write_reg(0x01, 0x40); // Power analog and IBIAS
  es8388_write_reg(0x04, 0x3C); // Power up DAC, Analog out
  //es8388_write_reg(0x17, 0b0001100); // GW: Set DAC SFI - I2S,16 bit
  es8388_write_reg(0x17, 0); // Set DAC SFI - I2S,24 bit
  es8388_write_reg(0x18, 0x02); // Set MCLK/LRCK ratio (256)
  //es8388_write_reg(0x18, 0b00110); // GW: Set MCLK/LRCK ratio (768) - default
  es8388_write_reg(0x1A, 0x00); // ADC volume 0dB
  es8388_write_reg(0x1B, 0x00); // ADC volume 0dB
  es8388_write_reg(0x1D, 0x20); // GW: DAC control - force MONO
  es8388_write_reg(0x19, 0x32); // Unmute DAC
  // Set mixer for DAC out
  es8388_write_reg(0x26, 0x09); // Select LIN2 + RIN2 for output mix
  es8388_write_reg(0x27, 0xF0); // L mixer enable DAC + LIN signals, gain = -12dB
  es8388_write_reg(0x28, 0x38);
  es8388_write_reg(0x29, 0x38);
  es8388_write_reg(0x2A, 0xF0); // R mixer enable DAC + RIN signals, gain = -12dB
  jswrap_pb_setVol(0x1E);       // Set volume: 0x1E = 0dB
  es8388_write_reg(0x02, 0xAA); // power up DEM and STM
  // Doc above also has notes on suspend/etc
#else
  jsiConsolePrintf("No audio codec - can be enabled by defining USE_AUDIO_CODEC");
#endif
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_pb_kill"
}*/
void jswrap_pb_kill() {
  jswrap_pb_videoStop();
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_pb_idle"
}*/
bool jswrap_pb_idle() {
  bool busy = streamType!=ST_NONE;
  if (streamType==ST_AVI) {
    busy = true;
    if (jshGetSystemTime() >= videoNextFrameTime)
      jswrap_pb_videoFrame();
  } else if (streamType==ST_WAV) {
    busy = true;
    jswrap_pb_audioFrame();
  }
  return busy;
}

/*
RDA5807M RADIO IC FUNCTIONS
- Using software I2C as I2C1 is already in use by the ES8388 codec
- Note that the radio can't tune when VBAT is supplied via an ST-Link - it needs the LiPo battery

var rd=new I2C();
rd.setup({sda: B7, scl:B6});
function rd_write_reg(r,d) {
  rd.writeTo(0x22>>1, [r,(d>>8 & 0xFF),(d & 0xFF)]);
}
function rd_read_reg(r) {
  rd.writeTo(0x22>>1, r);
  let bytes = rd.readFrom(0x22>>1,2);
  return (bytes[0]<<8 | bytes[1]);
}

function rd_init() {
  Pip.setDACMode("out");
  let id = rd_read_reg(0)>>8;
  if (id == 0x58)
    console.log(`RDA5807 ID: 0x${id.toString(16)} (as expected)`);
  else
    console.log(`Unexpected value reading RDA5807 ID: 0x${id.toString(16)}`);
  rd_write_reg(0x02,0x0003); // 0x0001:enable,    0x0002:reset
  rd_write_reg(0x02,0xF001); // 0x8000:output on, 0x4000:un-mute, 0x2000:mono, 0x1000:bass boost
  rd_write_reg(0x03,0x0008); // 0x0008:worldwide band (76-108 MHz)
  rd_write_reg(0x04,0x0600); // 0x0400:reserved,  0x0200:softmute
  rd_write_reg(0x05,0x86AF); // 0x8000:int_mode,  0x0600:seek threshold 6/15, 0x00F:volume 15/15
  rd_write_reg(0x06,0x8000); // 0x8000:reserved
  rd_write_reg(0x07,0x5F1A); // 0x5C00:noise soft blend threshold, 0x0002:soft blend enable
  rd.band=(rd_read_reg(0x03) & 0x000C)>>2;
  switch (rd.band) {
    case 0:
      rd.start = 8700;
      rd.end = 10800;
      break;
    case 1:
      rd.start = 7600;
      rd.end = 9100;
      break;
    case 2:
      rd.start = 7600;
      rd.end = 10800;
      break;
    case 3:
      if ((rd_read_reg(0x07) >> 9) & 0x01) {
        rd.start = 6500;
        rd.end = 7600;
      } else {
        rd.start = 5000;
        rd.end = 7600;
      }
  }
  rd.space=rd_read_reg(0x03) & 0x0003;
  switch (rd.space) {
    case 0:
      rd.chans_per_MHz = 10;
      break;
    case 1:
      rd.chans_per_MHz = 5;
      break;
    case 2:
      rd.chans_per_MHz = 20;
      break;
    case 3:
      rd.chans_per_MHz = 40;
      break;
  }
}

function rd_seek(seekUp) {
  let ctrlReg=rd_read_reg(0x02);
  ctrlReg |= 0x0100; // Seek
  if (seekUp) ctrlReg |= 0x0200; // SeekUp (0=down)
  else ctrlReg &= ~0x0200;
  rd_write_reg(0x02,ctrlReg);
  console.log(`Seeking ${seekUp?"up":"down"}...`);
  var i = setInterval(()=>{
    let status=rd_read_reg(0x0A);
    let chan=status&0x03FF;
    let freq=chan*rd.chans_per_MHz+rd.start;
    console.log(`- ch ${chan} (${freq/100} MHz) ${status&0x2000?"(failed)":(status&0x4000?"found":"")}`);
    if (status&0x6000) clearInterval(i);
  },400);
}

// rd_freq_set(f): tune to frequency in multiple of 10 kHz (e.g. 9930 for 99.3 MHz)
function rd_freq_set(f) {
  if (f<rd.start || f>rd.end) {
    console.log(`Invalid frequency (${f}) - must be between ${rd.start} and ${rd.end}`);
    return;
  }
  let chan=((f-rd.start)/rd.chans_per_MHz) & 0x03FF;
  console.log(`Band:${rd.band} (start:${rd.start}, end:${rd.end}), spacing:${1000/rd.chans_per_MHz} kHz, tuning to ${f/100} MHz (channel ${chan})`);
  let chanReg=(chan<<6) | (rd.band<<2) | rd.space | 0x0010; // 0x0010:tune
  rd_write_reg(0x03,chanReg);
  rd_write_reg(0x03,chanReg); // Need to write it twice (?!)
  var t = 0;
  var i = setInterval(()=>{
    let status=rd_read_reg(0x0A);
    if (status&0x6000) {
      console.log(`- set channel=${status&0x03FF} ${status&0x2000?"(failed)":"OK"}`);
      clearInterval(i);
    }
    if (t++>10) {
      console.log(`Giving up!`);
      clearInterval(i);
      rd_write_reg(0x03,chanReg & ~0x0010); // Stop tuning
    }
  },400);
}

Pip.setDACMode("off");
rd_init();
Pip.setDACMode("out");
rd_freq_set(9800); // Tune to 98.0 MHz
setWatch(()=>rd_seek(1),E1,{repeat:true,edge:"rising",debounce:25});
setWatch(()=>rd_seek(0),E2,{repeat:true,edge:"rising",debounce:25});
*/

/*
AUDIO CODEC INITIALISATION CODE
--------------------------------

function es8388_adda_cfg(dacen, adcen){
  var tempreg = 0;
  tempreg |= ((!dacen) << 0);
  tempreg |= ((!adcen) << 1);
  tempreg |= ((!dacen) << 2);
  tempreg |= ((!adcen) << 3);
  es8388_write_reg(0x02, tempreg);
}

function es8388_output_cfg(o1en, o2en){
  var tempreg = 0;
  tempreg |= o1en * (3 << 4);
  tempreg |= o2en * (3 << 2);
  es8388_write_reg(0x04, tempreg);
}

*/