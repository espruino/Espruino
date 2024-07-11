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

#define VIDEO_BUFFER_SIZE 40960
uint8_t videoBuffer[VIDEO_BUFFER_SIZE] __attribute__ ((aligned (8)));
// Can't go in 64k CCM RAM because no DMA is allowed to CCM!
// Maybe if we modified DMA to read to a buffer first?

int videoBufferLen;
uint16_t videoStreamId;
uint32_t videoStreamLen;        // length of current stream
uint32_t videoStreamRemaining;  // length left to read in current stream
uint32_t videoStreamBufferLen;  // length of stream in current buffer



JsSysTime videoFrameTime;
JsSysTime videoNextFrameTime;
bool videoLoaded = false;
bool debugInfo = false;
int startX=0;
int startY=0;
File_Handle videoFile;
AviInfo videoInfo;

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
  "name" : "videoStarted"
}
The video had ended
*/
/*JSON{
  "type" : "event",
  "class" : "Pip",
  "name" : "videoStopped"
}
The video had ended
*/
void jswrap_pb_sendEvent(const char *eventName) { // eg JS_EVENT_PREFIX"videoStarted"
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
    if (videoLoaded) {
      f_close(&videoFile);
      videoLoaded = false;
    }

  FRESULT res;
#ifdef LINUX
  if ((videoFile = fopen(pathStr, "r"))) {
#else
    BYTE ff_mode = FA_READ | FA_OPEN_EXISTING;
    if ((res=f_open(&videoFile, pathStr, ff_mode)) == FR_OK) {
#endif

      videoLoaded = true;
      size_t actual = 0;
      res = f_read(&videoFile, (uint8_t*)videoBuffer, sizeof(videoBuffer), &actual);
      if (debugInfo) {
        jsiConsolePrintf("AVI read %d %d %c%c%c%c\n", sizeof(videoBuffer), actual, videoBuffer[0],videoBuffer[1],videoBuffer[2],videoBuffer[3]);
      }
      if (aviLoad(videoBuffer, (int)actual, &videoInfo, debugInfo)) {
        videoStreamId = *(uint16_t*)&videoBuffer[videoInfo.videoOffset+2]; // +0 = '01'/'00' stream index?
        videoStreamLen = *(uint32_t*)&videoBuffer[videoInfo.videoOffset+4]; // +0 = '01'/'00' stream index?
        f_lseek(&videoFile, (DWORD)(videoInfo.videoOffset+8)); // go back to start of video data
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
        jswrap_pb_sendEvent(JS_EVENT_PREFIX"videoStarted");
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
void jswrap_pb_videoStop() {
  if (videoLoaded) {
    STM32_I2S_Stop(); // Stop audio (FIXME do we need/want this? might be better to stop when buffer is empty)
    f_close(&videoFile);
    videoLoaded = false;
    jswrap_pb_sendEvent(JS_EVENT_PREFIX"videoStopped");
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
  graphicsSetPixel(&graphicsInternal, px+ox, py+oy, col);
  px++;
}
void lcdFSMC_blitEnd() {
}
#endif

void jswrap_pb_videoFrame() {
  if (!videoLoaded) return;
  JsSysTime tStart = jshGetSystemTime();
  //if (debugInfo) jsiConsolePrintf("Stream 0x%04x, %d\n", videoStreamId, videoStreamLen);
  videoStreamRemaining = 0;
  videoStreamBufferLen = videoStreamLen+8; // 8 bytes contains info for next stream
  if (videoStreamLen > sizeof(videoBuffer)) {
    videoStreamRemaining = videoStreamBufferLen-sizeof(videoBuffer);
    videoStreamBufferLen = sizeof(videoBuffer);
  }
  size_t actual = 0;
  //jsiConsolePrintf("=========== FIRST READ %d (%d)\n", ((size_t)videoBuffer)&7, videoStreamBufferLen);
  f_read(&videoFile, videoBuffer,videoStreamBufferLen, &actual);
  if (videoStreamId==AVI_STREAM_AUDIO) {
    if (videoStreamRemaining) {
      jsiConsolePrintf("Audio stream too big for videoBuffer\n");
      jswrap_pb_videoStop();
    } else {
      STM32_I2S_AddSamples(videoBuffer, videoStreamLen>>1); // l is in bytes, not samples
    }
  } else if (videoStreamId==AVI_STREAM_VIDEO) {
    lcdFSMC_blitStart(&graphicsInternal, startX,startY,videoInfo.width,videoInfo.height);
    int x=0, y=videoInfo.height-1;
    uint8_t *b = videoBuffer;
    uint8_t *endBuffer = &videoBuffer[videoStreamBufferLen];
    while (b < endBuffer) {
      // Loop doing RLE until end *or* we have more data and no RLE command could use up more than we have left
      uint8_t *nearlyEndOfBuffer = videoStreamRemaining ? &videoBuffer[videoStreamBufferLen-260] : endBuffer;
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
      if (videoStreamRemaining) {
        uint32_t amountToShift = (uint32_t)(b-videoBuffer) & ~7ul; // aligned to the nearest word (STM32 f_read fails otherwise!)
        uint32_t leftInStream = videoStreamBufferLen - amountToShift;
        memmove(videoBuffer, &videoBuffer[amountToShift], leftInStream); // move data back
        b -= amountToShift;
        videoStreamBufferLen = leftInStream;
        uint32_t bufferRemaining = sizeof(videoBuffer)-leftInStream;
        uint32_t len = videoStreamRemaining;
        if (len>bufferRemaining) len=bufferRemaining;
        //jsiConsolePrintf("=========== READ %d (%d)\n", leftInStream, leftInStream&3);
        f_read(&videoFile, &videoBuffer[leftInStream], len, &actual);
        videoStreamRemaining -= len;
        videoStreamBufferLen += len;
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
    jswrap_pb_videoStop();
  }
  // get IDs for next stream
  videoStreamId = *(uint16_t*)&videoBuffer[videoStreamBufferLen-6]; // +0 = '01'/'00' stream index?
  videoStreamLen = *(uint32_t*)&videoBuffer[videoStreamBufferLen-4]; // +0 = '01'/'00' stream index?
  if (videoStreamId==AVI_STREAM_VIDEO) {
    // the next frame is a video frame, so ensure we parse it at the right time
    // if the frame is audio we want to parse it ASAP
    videoNextFrameTime += videoFrameTime;
  }
}

void graphicsInternalFlip() {
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_pb_init"
}*/
void jswrap_pb_init() {
  // Initialise audio
  STM32_I2S_Init();
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
  bool busy = videoLoaded!=0;
  if (jshGetSystemTime() >= videoNextFrameTime)
    jswrap_pb_videoFrame();
  return busy;
}

/*
AUDIO CODEC INITIALISATION CODE
--------------------------------

var es = I2C1;
es.setup({sda:B9, scl:B8});
function es8388_write_reg(r,d) {
  es.writeTo(0x10, [r,d]);
}
function es8388_read_reg(r) {
 es.writeTo(0x10,r);
 return es.readFrom(0x10,1)[0];
}

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

function es8388_spkvol_set(volume){
  if (volume > 33)
      volume = 33;
  es8388_write_reg(0x2E, volume); // 46 LOUT1 (33 = max)
  es8388_write_reg(0x2F, volume); // 47 ROUT1
  es8388_write_reg(0x30, volume); // 48 LOUT2 (33 = max)
  es8388_write_reg(0x31, volume); // 49 ROUT2
}

function es8388_reset() {
  console.log("Resetting ES8388");
  es8388_write_reg(0x00, 0x80); // Reset the ES8388 control registers
  es8388_write_reg(0x00, 0x00);
}

function es8388_init() {
  console.log("Initialising ES8388");
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
  es8388_write_reg(0x26, 0x00);
  es8388_write_reg(0x27, 0xB8); // LDAC to Lout (0xB8 = -15dB, 0x90 = 0dB)
  es8388_write_reg(0x28, 0x38);
  es8388_write_reg(0x29, 0x38);
  es8388_write_reg(0x2A, 0xB8); // RDAC to Rout (0xB8 = -15dB, 0x90 = 0dB)
  es8388_spkvol_set(0x1E);      // Set volume: 0x1E = 0dB
  es8388_write_reg(0x02, 0xAA); // power up DEM and STM
  // Doc above also has notes on suspend/etc
}

es8388_reset();
setTimeout(_=>{
  es8388_init();
  Pip.videoStart("test-chirp.avi",{debug:1});
}, 1000);
*/