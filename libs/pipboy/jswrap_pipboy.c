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
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jsflags.h"
#include "jstimer.h"
#include "jswrap_fs.h"
#include "jswrap_file.h"

#include "avi.h"

#define AUDIO_BUFFER_SIZE 5120
#define VIDEO_BUFFER_SIZE 32768

uint8_t videoBuffer[VIDEO_BUFFER_SIZE];
uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

bool videoLoaded = false;
File_Handle videoFile;

#ifdef LINUX // Linux -> FatFS hacks
void f_close(File_Handle *f) {
  fclose(*f);
}
FRESULT f_read(File_Handle* fp, void* buff, uint32_t btr, uint32_t* br) {
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
  volatile unsigned short *LCD_RAM = ((volatile unsigned short *) 0x60080000); /* RS = 1 (D13 -> A18) */
  volatile unsigned short *LCD_REG = ((volatile unsigned short *) 0x60000000);

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
      aviLoad(videoBuffer, actual);
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

void jswrap_pb_videoFrame() {
  if (videoLoaded) {
   /*LCD_REG = 0x2A;
    *LCD_RAM = 0;
    *LCD_RAM = 0;
    *LCD_RAM = 0;
    *LCD_RAM = 239;
    *LCD_REG = 0x2B;
    *LCD_RAM = 0;
    *LCD_RAM = 0;
    *LCD_RAM = 319>>8;
    *LCD_RAM = 319&0xFF;
    *LCD_REG = 0x2C;
    size_t actual = 1;
    while (actual && !jspIsInterrupted()) {
      FRESULT res = f_read(&videoFile, (uint8_t*)buffer, sizeof(buffer), &actual);
      for (size_t i=0;i<(actual>>1);i++) {
        *LCD_RAM = buffer[i];
      }
    }
    if (!actual) {
      f_close(&videoFile);
      videoLoaded = false;
    }*/
  }
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
  jswrap_pb_videoFrame();
}
