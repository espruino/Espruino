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
 * - convert audio to 16kHz mono:
 * - re-encode as 320x240 RLE at 12fps
 * ffmpeg -i ND-PB_Bootup01-v01_Waudio.mp4 -ar 16000 -ac 1 -vf crop=1440:1080:0:0 -vcodec msrle -s 320x240 -r 12 bootup-320x240-RLE-12fps.avi
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
#ifndef LINUX
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "lcd_fsmc.h"
#endif

#include "avi.h"

#define AUDIO_BUFFER_SIZE 1200 // 16 bit 16kHz -> 1152
#define VIDEO_BUFFER_SIZE 40960

uint8_t videoBuffer[VIDEO_BUFFER_SIZE] __attribute__ ((aligned (8)));
// Can't go in 64k CCM RAM because no DMA is allowed to CCM!
// Maybe if we modified DMA to read to a buffer first?

int videoBufferLen;
uint16_t videoStreamId;
uint32_t videoStreamLen;        // length of current stream
uint32_t videoStreamRemaining;  // length left to read in current stream
uint32_t videoStreamBufferLen;  // length of stream in current buffer

uint8_t i2sbuf1[AUDIO_BUFFER_SIZE];
uint8_t i2sbuf2[AUDIO_BUFFER_SIZE];
uint8_t i2sbuf3[AUDIO_BUFFER_SIZE];
uint8_t i2sbuf4[AUDIO_BUFFER_SIZE];

volatile uint8_t i2splaybuf; // currently playing buffer number
volatile uint8_t i2ssavebuf; // currently writing to buffer number
uint8_t* i2sbuf[4]; // list of buffers

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

void DMA1_Stream4_IRQHandler(void) {
  if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4)==SET) {
    DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);

    i2splaybuf = (i2splaybuf+1) & 3;
    if(DMA1_Stream4->CR&(1<<19))
      DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t)i2sbuf[i2splaybuf], DMA_Memory_0);
    else
      DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t)i2sbuf[i2splaybuf], DMA_Memory_1);
    }
}

void I2S_SetSampleRate(int hz) {
  #define prescalerCount 7
  const uint16_t prescalers[prescalerCount][5]={
    {8000 ,256,5,12,1},
    {11020,429,4,19,0},
    {16000,213,2,13,0},
    {22050,429,4, 9,1},
    {32000,213,2, 6,1},
    {44100,271,2, 6,0},
    {48000,258,3, 3,1}
  };
  RCC_PLLI2SCmd(DISABLE);
  for(int i=0;i<prescalerCount;i++) {
    if(hz==prescalers[i][0]) {
      RCC_PLLI2SConfig((uint32_t)prescalers[i][1],(uint32_t)prescalers[i][2]);
      RCC->CR |= RCC_CR_PLLI2SON;
      while((RCC->CR&RCC_CR_PLLI2SRDY)==0);
      uint32_t i2spr;
      i2spr = prescalers[i][3]<<0;    // I2S Linear prescaler
      i2spr |= prescalers[i][4]<<8;   // Odd factor for the prescaler
      i2spr |= SPI_I2SPR_MCKOE; // master clock out
      SPI2->I2SPR=i2spr;
      return;
    }
  }
  jsiConsolePrintf("Samplerate %d not supported\n");
}

void I2S_DMAInit(int sampleCount) {
  DMA_DeInit(DMA1_Stream4);
  while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}

  DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_FEIF4|DMA_IT_DMEIF4|DMA_IT_TEIF4|DMA_IT_HTIF4|DMA_IT_TCIF4);
  DMA_InitTypeDef  DMA_InitStructure;
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)i2sbuf[1];
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = sampleCount;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);

  DMA_DoubleBufferModeConfig(DMA1_Stream4,(uint32_t)i2sbuf[2],DMA_Memory_0);
  DMA_DoubleBufferModeCmd(DMA1_Stream4,ENABLE);

  DMA_ITConfig(DMA1_Stream4,DMA_IT_TC,ENABLE);
}

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
      if (aviLoad(videoBuffer, actual, &videoInfo, debugInfo)) {
        videoStreamId = *(uint16_t*)&videoBuffer[videoInfo.videoOffset+2]; // +0 = '01'/'00' stream index?
        videoStreamLen = *(uint32_t*)&videoBuffer[videoInfo.videoOffset+4]; // +0 = '01'/'00' stream index?
        f_lseek(&videoFile, videoInfo.videoOffset+8); // go back to start of video data
        videoFrameTime = jshGetTimeFromMilliseconds(videoInfo.usPerFrame/1000.0);
        videoNextFrameTime = jshGetSystemTime() + videoFrameTime;
#ifndef LINUX
        // Set up Audio
        if (videoInfo.audioBufferSize) { // IF we have audio
          I2S_SetSampleRate(videoInfo.audioSampleRate);
          I2S_DMAInit(videoInfo.audioBufferSize >> 1); // 16 bit
          i2splaybuf=0;
          i2ssavebuf=0;
          DMA_Cmd(DMA1_Stream4, ENABLE); // wait until first audio data??
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
#ifndef LINUX
    DMA_Cmd(DMA1_Stream4, DISABLE); // Stop I2S
#endif
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
  if (debugInfo) jsiConsolePrintf("Stream 0x%04x, %d\n", videoStreamId, videoStreamLen);
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
      i2ssavebuf = (i2ssavebuf+1) & 3;
      // should we wait if we're overwriting a buffer we're reading from?
      int l = videoStreamBufferLen;
      if (l > AUDIO_BUFFER_SIZE) {
        jsiConsolePrintf("Audio stream too big for audioBuffer (%d)\n", l);
        l = AUDIO_BUFFER_SIZE;
      }
      memcpy(i2sbuf[i2ssavebuf], videoBuffer, videoStreamBufferLen);
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
        uint32_t amountToShift = (b-videoBuffer) & ~7; // aligned to the nearest word (STM32 f_read fails otherwise!)
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
    videoNextFrameTime += videoFrameTime;
    if (debugInfo) {
      //jswrap_pb_videoStop(); // first frame only
      JsSysTime tEnd = jshGetSystemTime();
      jsiConsolePrintf("%dms\n", (int)jshGetMillisecondsFromTime(tEnd-tStart));
    }
  } else {
    // unknown stream - assume end and stop!
    //jsiConsolePrintf("Unknown stream ID");
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
  // Initialise audio
  i2sbuf[0] = i2sbuf1;
  i2sbuf[1] = i2sbuf2;
  i2sbuf[2] = i2sbuf3;
  i2sbuf[3] = i2sbuf4;
#ifndef LINUX
  I2S_InitTypeDef I2S_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_SPI2); // PB12,AF5  I2S_LRCK
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); // PB13,AF5  I2S_SCLK
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);   // PC3 ,AF5  I2S_DACDATA
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_SPI2);   // PC6 ,AF5  I2S_MCK
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource2,GPIO_AF_SPI3); // PC2 ,AF6  I2S_ADCDATA (AF6 apparently?)

  I2S_InitStructure.I2S_Mode=I2S_Mode_MasterTx;
  I2S_InitStructure.I2S_Standard=I2S_Standard_Phillips;
  I2S_InitStructure.I2S_DataFormat=I2S_DataFormat_16bextended;
  I2S_InitStructure.I2S_MCLKOutput=I2S_MCLKOutput_Disable;
  I2S_InitStructure.I2S_AudioFreq=I2S_AudioFreq_Default;
  I2S_InitStructure.I2S_CPOL=I2S_CPOL_Low;
  I2S_Init(SPI2,&I2S_InitStructure);

  SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);
  I2S_Cmd(SPI2,ENABLE);

  NVIC_InitTypeDef   NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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
  bool busy = videoLoaded!=0;
  if (jshGetSystemTime() >= videoNextFrameTime)
    jswrap_pb_videoFrame();
  return busy;
}
