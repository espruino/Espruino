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
#ifndef LINUX
#include "lcd_fsmc.h"
#endif

#include "avi.h"

#define AUDIO_BUFFER_SIZE 5120
#define VIDEO_BUFFER_SIZE 20480

uint8_t videoBuffer[VIDEO_BUFFER_SIZE] __attribute__ ((aligned (8)));
int videoBufferLen;
uint16_t videoStreamId;
uint32_t videoStreamLen;        // length of current stream
uint32_t videoStreamRemaining;  // length left to read in current stream
uint32_t videoStreamBufferLen;  // length of stream in current buffer

JsSysTime videoFrameTime;
JsSysTime videoNextFrameTime;
bool videoLoaded = false;
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
  *br = fread(buff, 1, btr, *fp);
  return *br!=0;
}
#endif

/*JSON{
    "type": "class",
    "class" : "Pip"
}
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "Pip",
  "name" : "videoStart",
  "generate" : "jswrap_pb_videoStart",
  "params" : [
      ["fn","JsVar","Filename"],
      ["options","JsVar","Options"]
  ]
}
*/
void jswrap_pb_videoStart(JsVar *fn, JsVar *options) {

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
      jsiConsolePrintf("AVI read %d %d %c%c%c%c\n", sizeof(videoBuffer), actual, videoBuffer[0],videoBuffer[1],videoBuffer[2],videoBuffer[3]);
      if (aviLoad(videoBuffer, actual, &videoInfo)) {
        videoStreamId = *(uint16_t*)&videoBuffer[videoInfo.videoOffset+2]; // +0 = '01'/'00' stream index?
        videoStreamLen = *(uint32_t*)&videoBuffer[videoInfo.videoOffset+4]; // +0 = '01'/'00' stream index?
        f_lseek(&videoFile, videoInfo.videoOffset+8); // go back to start of video data
        videoFrameTime = jshGetTimeFromMilliseconds(videoInfo.usPerFrame/1000.0);
        videoNextFrameTime = jshGetSystemTime() + videoFrameTime;

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
      f_close(&videoFile);
      videoLoaded = false;
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

  //jsiConsolePrintf("Stream 0x%04x, %d\n", videoStreamId, videoStreamLen);
  videoStreamRemaining = 0;
  videoStreamBufferLen = videoStreamLen+8; // 8 bytes contains info for next stream
  if (videoStreamLen > sizeof(videoBuffer)) {
    videoStreamRemaining = videoStreamBufferLen-sizeof(videoBuffer);
    videoStreamBufferLen = sizeof(videoBuffer);
  }
  size_t actual = 0;
  f_read(&videoFile, videoBuffer,videoStreamBufferLen, &actual);
  if (videoStreamId==AVI_STREAM_AUDIO) {
    if (videoStreamRemaining) {
      jsiConsolePrintf("Audio stream too big");
      jswrap_pb_videoStop();
    }
  } else if (videoStreamId==AVI_STREAM_VIDEO) {
    lcdFSMC_blitStart(&graphicsInternal, 0,0,videoInfo.width,videoInfo.height);
    int x=0, y=videoInfo.height--;
    uint8_t *b = videoBuffer;
    while (b < &videoBuffer[videoStreamBufferLen]) {
      // check if we need to refill our buffer
      if (videoStreamRemaining && (b+256 >= &videoBuffer[videoStreamBufferLen])) {
        uint32_t amountToShift = (b-videoBuffer) & ~3; // aligned to the nearest word (STM32 f_read fails otherwise!)
        uint32_t leftInStream = videoStreamBufferLen - amountToShift;
        memmove(videoBuffer, &videoBuffer[amountToShift], leftInStream); // move data back
        b -= amountToShift; // FIXME - this is broken
        videoStreamBufferLen = leftInStream;
        uint32_t bufferRemaining = sizeof(videoBuffer)-leftInStream;
        uint32_t len = videoStreamRemaining;
        if (len>bufferRemaining) len=bufferRemaining;
        f_read(&videoFile, &videoBuffer[leftInStream],len, &actual);
        videoStreamRemaining -= len;
        videoStreamBufferLen += len;
      }
      // ok, next RLE byte!
      uint8_t num = *(b++);
      if (num==0)  { // RLE code is 0 -> escape code for commands
        uint8_t cmd = *(b++);
        if (cmd==0) { // 0 = EOL
          x=0;
          y--;
          lcdFSMC_setCursor(&graphicsInternal, x,y);
        } else if (cmd==1) {
          //jsiConsolePrintf("end of bitmap!\n");
        } else if (cmd==2) { // 2 = DELTA
          x += *(b++);
          y -= *(b++);
          lcdFSMC_setCursor(&graphicsInternal, x,y);
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
    lcdFSMC_blitEnd();
    videoNextFrameTime += videoFrameTime;
    //jswrap_pb_videoStop(); // first frame only
  } else {
    // unknown stream - assume end and stop!
    jswrap_pb_videoStop();
  }
  // get IDs for next stream
  videoStreamId = *(uint16_t*)&videoBuffer[videoStreamBufferLen-6]; // +0 = '01'/'00' stream index?
  videoStreamLen = *(uint32_t*)&videoBuffer[videoStreamBufferLen-4]; // +0 = '01'/'00' stream index?
}

void graphicsInternalFlip() {
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_pb_init"
}*/
void jswrap_pb_init() {
  // TODO: Audio/other init
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
