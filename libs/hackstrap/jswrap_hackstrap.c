/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Pixl.js (http://www.espruino.com/Pixl.js)
 * ----------------------------------------------------------------------------
 */

#include <jswrap_hackstrap.h>
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jsnative.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jstimer.h"
#include "jswrap_promise.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrf5x_utils.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h" // for self-test
#include "jsi2c.h" // touchscreen

#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"

#define LCD_SPI EV_SPI1
#define GPS_UART EV_SERIAL1

uint8_t nmeaCount = 0; // how many characters of NMEA data do we have?
char nmea[82]; //  82 is the max for NMEA
char nmeaLine[82]; // A line of received NMEA data

/*
a=[0x000000,0x000033,0x000066,0x000099,0x0000cc,0x0000ff,0x003300,0x003333,0x003366,0x003399,0x0033cc,0x0033ff,0x006600,0x006633,0x006666,0x006699,0x0066cc,0x0066ff,0x009900,0x009933,0x009966,0x009999,0x0099cc,0x0099ff,0x00cc00,0x00cc33,0x00cc66,0x00cc99,0x00cccc,0x00ccff,0x00ff00,0x00ff33,0x00ff66,0x00ff99,0x00ffcc,0x00ffff,0x330000,0x330033,0x330066,0x330099,0x3300cc,0x3300ff,0x333300,0x333333,0x333366,0x333399,0x3333cc,0x3333ff,0x336600,0x336633,0x336666,0x336699,0x3366cc,0x3366ff,0x339900,0x339933,0x339966,0x339999,0x3399cc,0x3399ff,0x33cc00,0x33cc33,0x33cc66,0x33cc99,0x33cccc,0x33ccff,0x33ff00,0x33ff33,0x33ff66,0x33ff99,0x33ffcc,0x33ffff,0x660000,0x660033,0x660066,0x660099,0x6600cc,0x6600ff,0x663300,0x663333,0x663366,0x663399,0x6633cc,0x6633ff,0x666600,0x666633,0x666666,0x666699,0x6666cc,0x6666ff,0x669900,0x669933,0x669966,0x669999,0x6699cc,0x6699ff,0x66cc00,0x66cc33,0x66cc66,0x66cc99,0x66cccc,0x66ccff,0x66ff00,0x66ff33,0x66ff66,0x66ff99,0x66ffcc,0x66ffff,0x990000,0x990033,0x990066,0x990099,0x9900cc,0x9900ff,0x993300,0x993333,0x993366,0x993399,0x9933cc,0x9933ff,0x996600,0x996633,0x996666,0x996699,0x9966cc,0x9966ff,0x999900,0x999933,0x999966,0x999999,0x9999cc,0x9999ff,0x99cc00,0x99cc33,0x99cc66,0x99cc99,0x99cccc,0x99ccff,0x99ff00,0x99ff33,0x99ff66,0x99ff99,0x99ffcc,0x99ffff,0xcc0000,0xcc0033,0xcc0066,0xcc0099,0xcc00cc,0xcc00ff,0xcc3300,0xcc3333,0xcc3366,0xcc3399,0xcc33cc,0xcc33ff,0xcc6600,0xcc6633,0xcc6666,0xcc6699,0xcc66cc,0xcc66ff,0xcc9900,0xcc9933,0xcc9966,0xcc9999,0xcc99cc,0xcc99ff,0xcccc00,0xcccc33,0xcccc66,0xcccc99,0xcccccc,0xccccff,0xccff00,0xccff33,0xccff66,0xccff99,0xccffcc,0xccffff,0xff0000,0xff0033,0xff0066,0xff0099,0xff00cc,0xff00ff,0xff3300,0xff3333,0xff3366,0xff3399,0xff33cc,0xff33ff,0xff6600,0xff6633,0xff6666,0xff6699,0xff66cc,0xff66ff,0xff9900,0xff9933,0xff9966,0xff9999,0xff99cc,0xff99ff,0xffcc00,0xffcc33,0xffcc66,0xffcc99,0xffcccc,0xffccff,0xffff00,0xffff33,0xffff66,0xffff99,0xffffcc,0xffffff];
while(a.length<255)a.push(0);
a[255]=0xFFFFFF;
// 12 bit
print(a.map(function(rgb) {
  r = rgb>>16;
  g = (rgb>>8)&0xFF;
  b = rgb&0xFF;
  return ((r&0xF0)<<4) | (g&0xF0) | (b>>4);
}).map(x=>"0x"+x.toString(16)).join(","))
// 16 bit
 print(a.map(function(rgb) {
  r = rgb>>16;
  g = (rgb>>8)&0xFF;
  b = rgb&0xFF;
  return ((r&0xF8)<<8) | ((g&0xFc)<<3) | (b>>3);
}).map(x=>"0x"+x.toString(16)).join(","))
 */
// 16 bit
//const uint16_t PALETTE[256] = { 0x0,0x6,0xc,0x13,0x19,0x1f,0x180,0x186,0x18c,0x193,0x199,0x19f,0x320,0x326,0x32c,0x333,0x339,0x33f,0x4c0,0x4c6,0x4cc,0x4d3,0x4d9,0x4df,0x660,0x666,0x66c,0x673,0x679,0x67f,0x7e0,0x7e6,0x7ec,0x7f3,0x7f9,0x7ff,0x3000,0x3006,0x300c,0x3013,0x3019,0x301f,0x3180,0x3186,0x318c,0x3193,0x3199,0x319f,0x3320,0x3326,0x332c,0x3333,0x3339,0x333f,0x34c0,0x34c6,0x34cc,0x34d3,0x34d9,0x34df,0x3660,0x3666,0x366c,0x3673,0x3679,0x367f,0x37e0,0x37e6,0x37ec,0x37f3,0x37f9,0x37ff,0x6000,0x6006,0x600c,0x6013,0x6019,0x601f,0x6180,0x6186,0x618c,0x6193,0x6199,0x619f,0x6320,0x6326,0x632c,0x6333,0x6339,0x633f,0x64c0,0x64c6,0x64cc,0x64d3,0x64d9,0x64df,0x6660,0x6666,0x666c,0x6673,0x6679,0x667f,0x67e0,0x67e6,0x67ec,0x67f3,0x67f9,0x67ff,0x9800,0x9806,0x980c,0x9813,0x9819,0x981f,0x9980,0x9986,0x998c,0x9993,0x9999,0x999f,0x9b20,0x9b26,0x9b2c,0x9b33,0x9b39,0x9b3f,0x9cc0,0x9cc6,0x9ccc,0x9cd3,0x9cd9,0x9cdf,0x9e60,0x9e66,0x9e6c,0x9e73,0x9e79,0x9e7f,0x9fe0,0x9fe6,0x9fec,0x9ff3,0x9ff9,0x9fff,0xc800,0xc806,0xc80c,0xc813,0xc819,0xc81f,0xc980,0xc986,0xc98c,0xc993,0xc999,0xc99f,0xcb20,0xcb26,0xcb2c,0xcb33,0xcb39,0xcb3f,0xccc0,0xccc6,0xcccc,0xccd3,0xccd9,0xccdf,0xce60,0xce66,0xce6c,0xce73,0xce79,0xce7f,0xcfe0,0xcfe6,0xcfec,0xcff3,0xcff9,0xcfff,0xf800,0xf806,0xf80c,0xf813,0xf819,0xf81f,0xf980,0xf986,0xf98c,0xf993,0xf999,0xf99f,0xfb20,0xfb26,0xfb2c,0xfb33,0xfb39,0xfb3f,0xfcc0,0xfcc6,0xfccc,0xfcd3,0xfcd9,0xfcdf,0xfe60,0xfe66,0xfe6c,0xfe73,0xfe79,0xfe7f,0xffe0,0xffe6,0xffec,0xfff3,0xfff9,0xffff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xffff };
// 12 bit
const uint16_t PALETTE[256] = { 0x0,0x3,0x6,0x9,0xc,0xf,0x30,0x33,0x36,0x39,0x3c,0x3f,0x60,0x63,0x66,0x69,0x6c,0x6f,0x90,0x93,0x96,0x99,0x9c,0x9f,0xc0,0xc3,0xc6,0xc9,0xcc,0xcf,0xf0,0xf3,0xf6,0xf9,0xfc,0xff,0x300,0x303,0x306,0x309,0x30c,0x30f,0x330,0x333,0x336,0x339,0x33c,0x33f,0x360,0x363,0x366,0x369,0x36c,0x36f,0x390,0x393,0x396,0x399,0x39c,0x39f,0x3c0,0x3c3,0x3c6,0x3c9,0x3cc,0x3cf,0x3f0,0x3f3,0x3f6,0x3f9,0x3fc,0x3ff,0x600,0x603,0x606,0x609,0x60c,0x60f,0x630,0x633,0x636,0x639,0x63c,0x63f,0x660,0x663,0x666,0x669,0x66c,0x66f,0x690,0x693,0x696,0x699,0x69c,0x69f,0x6c0,0x6c3,0x6c6,0x6c9,0x6cc,0x6cf,0x6f0,0x6f3,0x6f6,0x6f9,0x6fc,0x6ff,0x900,0x903,0x906,0x909,0x90c,0x90f,0x930,0x933,0x936,0x939,0x93c,0x93f,0x960,0x963,0x966,0x969,0x96c,0x96f,0x990,0x993,0x996,0x999,0x99c,0x99f,0x9c0,0x9c3,0x9c6,0x9c9,0x9cc,0x9cf,0x9f0,0x9f3,0x9f6,0x9f9,0x9fc,0x9ff,0xc00,0xc03,0xc06,0xc09,0xc0c,0xc0f,0xc30,0xc33,0xc36,0xc39,0xc3c,0xc3f,0xc60,0xc63,0xc66,0xc69,0xc6c,0xc6f,0xc90,0xc93,0xc96,0xc99,0xc9c,0xc9f,0xcc0,0xcc3,0xcc6,0xcc9,0xccc,0xccf,0xcf0,0xcf3,0xcf6,0xcf9,0xcfc,0xcff,0xf00,0xf03,0xf06,0xf09,0xf0c,0xf0f,0xf30,0xf33,0xf36,0xf39,0xf3c,0xf3f,0xf60,0xf63,0xf66,0xf69,0xf6c,0xf6f,0xf90,0xf93,0xf96,0xf99,0xf9c,0xf9f,0xfc0,0xfc3,0xfc6,0xfc9,0xfcc,0xfcf,0xff0,0xff3,0xff6,0xff9,0xffc,0xfff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfff };

#define CMDINDEX_CMD   0
#define CMDINDEX_DELAY 1
#define CMDINDEX_DATALEN  2
static const char ST7735_INIT_CODE[] = {
  // CMD,DELAY,DATA_LEN,D0,D1,D2...
  // SWRESET Software reset - but we have hardware reset
  // 0x01, 20, 0,
  // SLPOUT Leave sleep mode
  0x11, 100, 0,
  // FRMCTR1 , FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
  0xB1, 0, 3,  /*data*/0x01, 0x2C, 0x2D ,
  0xB2, 0, 3,  /*data*/0x01, 0x2C, 0x2D ,
  // FRMCTR3 Frame Rate configureation -- partial mode
  0xB3, 0, 6, /*data*/0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D ,
  // INVCTR Display inversion (no inversion)
   0xB4, 0, 1, /*data*/0x07 ,
  // PWCTR1 Power control -4.6V, Auto mode
   0xC0, 0, 3,  /*data*/0xA2, 0x02, 0x84,
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
   0xC1, 0, 1,  /*data*/0xC5,
  // PWCTR3 Power control , opamp current smal, boost frequency
   0xC2, 0, 2,  /*data*/0x0A, 0x00 ,
  // PWCTR4 Power control , BLK/2, opamp current small and medium low
   0xC3, 0, 2,  /*data*/0x8A, 0x2A,
  // PWRCTR5 , VMCTR1 Power control
   0xC4, 0, 2,  /*data*/0x8A, 0xEE,
   0xC5, 0, 1,  /*data*/0x0E ,
  // INVOFF Don't invert display
   0x20, 0, 0,
  // MADCTL row address/col address, bottom to top refesh (10.1.27)
   0x36, 0, 1, /*data*/0xC8,
  // COLMOD, Color mode 12 bit
   0x3A, 0, 1, /*data*/0x03,
  // GMCTRP1 Gamma correction
   0xE0, 0, 16, /*data*/0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 ,
  // GMCTRP2 Gamma Polarity correction
   0xE1, 0, 16, /*data*/0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 ,
  // DISPON Display on
   0x29, 10, 0,
  // NORON Normal on
   0x13, 10, 0,
  // End
   0, 0, 255/*DATA_LEN = 255 => END*/
};
void lcd_cmd(int cmd, int dataLen, const char *data) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  jshSPISend(LCD_SPI, cmd);
  if (dataLen) {
    jshPinSetValue(LCD_SPI_DC, 1); // data
    while (dataLen) {
      jshSPISend(LCD_SPI, *(data++));
      dataLen--;
    }
  }
  jshPinSetValue(LCD_SPI_CS, 1);
}
void lcd_init() {
  // Send initialization commands to ST7735
  const char *cmd = ST7735_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcd_cmd(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }
}

/*JSON{
  "type": "class",
  "class" : "Strap",
  "ifdef" : "HACKSTRAP"
}
Class containing utility functions for the [HackStrap Smart Watch](http://www.espruino.com/HackStrap)
*/


/*JSON{
  "type" : "variable",
  "name" : "VIBRATE",
  "generate_full" : "VIBRATE_PIN",
  "ifdef" : "HACKSTRAP",
  "return" : ["pin",""]
}
The HackStrap's vibration motor.
*/

#define ACCEL_POLL_INTERVAL 100 // in msec
/// Internal I2C used for Accelerometer/Pressure
JshI2CInfo internalI2C;
/// Is I2C busy? if so we'll skip one reading in our interrupt so we don't overlap
bool i2cBusy;
/// Promise when pressure is requested
JsVar *promisePressure;
/// counter that counts up if watch has stayed face up or down
unsigned char faceUpCounter;
/// Was the watch face-up? we use this when firing events
bool wasFaceUp;
/// time since LCD contents were last modified
volatile unsigned char flipCounter;
/// Is LCD power automatic? If true this is the number of ms for the timeout, if false it's 0
int lcdPowerTimeout = 100;
/// Is the LCD on?
bool lcdPowerOn;
/// accelerometer data
int accx,accy,accz,accdiff;
/// data on how watch was tapped
unsigned char tapInfo;

typedef enum {
  JSS_NONE,
  JSS_LCD_ON = 1,
  JSS_LCD_OFF = 2,
  JSS_ACCEL_DATA = 4, // need to push xyz data to JS
  JSS_ACCEL_TAPPED = 8, // tap event detected
  JSS_GPS_DATA = 16, // we got a line of GPS data
} JsStrapTasks;
JsStrapTasks strapTasks;


void lcd_flip_spi_callback() {
  // just an empty stub - we'll just push data as fast as we can
}

void lcd_flip_gfx(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!

  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar,"buffer",0);
  if (!buf) return;
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (!bPtr || bLen<128*8) return;

  if (lcdPowerTimeout && !lcdPowerOn) {
    // LCD was turned off, turn it back on
    jswrap_hackstrap_setLCDPower(1);
  }
  flipCounter = 0;

  unsigned char buffer1[LCD_WIDTH*2]; // 16 bits per pixel
  unsigned char buffer2[LCD_WIDTH*2]; // 16 bits per pixel

  // use nearest 2 pixels as we're sending 12 bits
  gfx->data.modMinX = (gfx->data.modMinX)&~1;
  gfx->data.modMaxX = (gfx->data.modMaxX+2)&~1;
  int xlen = gfx->data.modMaxX - gfx->data.modMinX;

  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2A;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinX;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxX;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2B;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinY;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxY+1;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2C;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data

  for (int y=gfx->data.modMinY;y<=gfx->data.modMaxY;y++) {
    unsigned char *buffer = (y&1)?buffer1:buffer2;
    // skip any lines that don't need updating
    unsigned char *px = (unsigned char *)&bPtr[y*LCD_WIDTH + gfx->data.modMinX];
    unsigned char *bufPtr = (unsigned char*)buffer;
    for (int x=0;x<xlen;x+=2) {
      unsigned int a = PALETTE[*(px++)];
      unsigned int b = PALETTE[*(px++)];
      *(bufPtr++) = a>>4;
      *(bufPtr++) = (a<<4) | (b>>8);
      *(bufPtr++) = b;
    }
    size_t len = ((unsigned char*)bufPtr)-buffer;
    if (len>255) {
      // do biggest bit of buffer last so we can get on and work
      // out the next scanline while it's sent out
      jshSPISendMany(LCD_SPI, buffer, 0, len-254, NULL);
      jshSPISendMany(LCD_SPI, &buffer[len-254], 0, 254, lcd_flip_spi_callback);
    } else {
      jshSPISendMany(LCD_SPI, buffer, 0, len, lcd_flip_spi_callback);
    }
  }
  jshSPIWait(LCD_SPI);
  jshPinSetValue(LCD_SPI_CS,1);
  jsvUnLock(buf);
  // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}


/// Send buffer contents to the screen. Usually only the modified data will be output, but if all=true then the whole screen contents is sent
void lcd_flip(JsVar *parent, bool all) {
  JsGraphics gfx; 
  if (!graphicsGetFromVar(&gfx, parent)) return;
  if (all) {
    gfx.data.modMinX = 0;
    gfx.data.modMinY = 0;
    gfx.data.modMaxX = LCD_WIDTH-1;
    gfx.data.modMaxY = LCD_HEIGHT-1;
  }
  lcd_flip_gfx(&gfx);
  graphicsSetVar(&gfx);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setLCDPower",
    "generate" : "jswrap_hackstrap_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ]
}
This function can be used to turn HackStrap's LCD off or on.
*/
void jswrap_hackstrap_setLCDPower(bool isOn) {
  if (isOn) {
    lcd_cmd(0x11, 0, NULL); // SLPOUT
    jshPinOutput(LCD_BL,0); // backlight
  } else {
    lcd_cmd(0x10, 0, NULL); // SLPIN
    jshPinOutput(LCD_BL,1); // backlight
  }
  if (lcdPowerOn != isOn) {
    JsVar *strap =jsvObjectGetChild(execInfo.root, "Strap", 0);
    if (strap) {
      JsVar *v = jsvNewFromBool(isOn);
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"lcdPower", &v, 1);
      jsvUnLock(v);
    }
    jsvUnLock(strap);
  }
  lcdPowerOn = isOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setLCDTimeout",
    "generate" : "jswrap_hackstrap_setLCDTimeout",
    "params" : [
      ["isOn","float","The timeout of the display in seconds, or `0`/`undefined` to turn power saving off. Default is 10 seconds."]
    ]
}
This function can be used to turn HackStrap's LCD power saving on or off.

With power saving off, the display will remain in the state you set it with `Strap.setLCDPower`.

With power saving on, the display will turn on if a button is pressed, the watch is turned face up, or the screen is updated. It'll turn off automatically after the given timeout.
*/
void jswrap_hackstrap_setLCDTimeout(JsVarFloat timeout) {
  if (!isfinite(timeout)) lcdPowerTimeout=0;
  else lcdPowerTimeout = timeout*(1000.0/ACCEL_POLL_INTERVAL);
  if (lcdPowerTimeout<0) lcdPowerTimeout=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "isLCDOn",
    "generate" : "jswrap_hackstrap_isLCDOn",
    "return" : ["bool","Is the display on or not?"]
}
*/
bool jswrap_hackstrap_isLCDOn() {
  return lcdPowerOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "lcdWr",
    "generate" : "jswrap_hackstrap_lcdWr",
    "params" : [
      ["cmd","int",""],
      ["data","JsVar",""]
    ]
}
Writes a command directly to the ST7735 LCD controller
*/
void jswrap_hackstrap_lcdWr(JsVarInt cmd, JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  lcd_cmd(cmd, dLen, dPtr);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setGPSPower",
    "generate" : "jswrap_hackstrap_setGPSPower",
    "params" : [
      ["isOn","bool","True if the GPS should be on, false if not"]
    ]
}
Set the power to the GPS.
*/
void jswrap_hackstrap_setGPSPower(bool isOn) {
  if (isOn) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.baudRate = 9600;
    inf.pinRX = GPS_PIN_RX;
    inf.pinTX = GPS_PIN_TX;
    jshUSARTSetup(GPS_UART, &inf);
    jshPinOutput(GPS_PIN_EN,1); // GPS on
    nmeaCount = 0;
  } else {
    jshPinOutput(GPS_PIN_EN,0); // GPS off
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
}


// Holding down both buttons will reboot
void watchdogHandler() {
  //jshPinOutput(LED1_PININDEX, 1);
  // Handle watchdog
  if (!(jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN2_PININDEX)))
    jshKickWatchDog();
  // power on display if a button is pressed
  if (lcdPowerTimeout &&
      (jshPinGetValue(BTN1_PININDEX) || jshPinGetValue(BTN2_PININDEX) ||
       jshPinGetValue(BTN3_PININDEX))) {
    flipCounter = 0;
    if (!lcdPowerOn)
      strapTasks |= JSS_LCD_ON;
  }
  if (flipCounter<255) flipCounter++;

  if (lcdPowerTimeout && lcdPowerOn && flipCounter>=lcdPowerTimeout) {
    // 10 seconds of inactivity, turn off display
    strapTasks |= JSS_LCD_OFF;
  }


  if (i2cBusy) return;
  // poll KX023 accelerometer (no other way as IRQ line seems disconnected!)
  unsigned char buf[6];
  buf[0]=6;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 6, buf, true);
  int newx = (buf[1]<<8)|buf[0];
  int newy = (buf[3]<<8)|buf[2];
  int newz = (buf[5]<<8)|buf[4];
  if (newx&0x8000) newx-=0x10000;
  if (newy&0x8000) newy-=0x10000;
  if (newz&0x8000) newz-=0x10000;
  int dx = newx-accx;
  int dy = newy-accy;
  int dz = newz-accz;
  accx = newx;
  accy = newy;
  accz = newz;
  accdiff = dx*dx + dy*dy + dz*dz;
  strapTasks |= JSS_ACCEL_DATA;
  // read interrupt source data
  buf[0]=0x12;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 2, buf, true);
  // 0 -> 0x12 INS1 - tap event
  // 1 -> 0x13 INS2 - what kind of event
  int tapType = (buf[1]>>2)&3;
  if (tapType) {
    // report tap
    tapInfo = buf[0] | (tapType<<6);
    strapTasks |= JSS_ACCEL_TAPPED;
    // clear the IRQ flags
    buf[0]=0x17;
    jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  }

  //jshPinOutput(LED1_PININDEX, 0);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_hackstrap_init"
}*/
void jswrap_hackstrap_init() {
  jshPinOutput(GPS_PIN_EN,0); // GPS off
  jshPinOutput(VIBRATE_PIN,0); // vibrate off

  jshPinOutput(LCD_BL,0); // backlight on


  // LCD Init 1
  lcdPowerOn = true;
  jshPinOutput(LCD_SPI_CS,1);
  jshPinOutput(LCD_SPI_DC,1);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
  jshPinOutput(LCD_SPI_RST, 1);
  jshDelayMicroseconds(10000);
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
  gfx.graphicsVar = graphics;
  gfx.data.width = LCD_WIDTH;
  gfx.data.height = LCD_HEIGHT;
  gfx.data.bpp = LCD_BPP;

  //gfx.data.fontSize = JSGRAPHICS_FONTSIZE_6X8;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);


  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);

  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 8000000;
  inf.pinMOSI = LCD_SPI_MOSI;
  inf.pinSCK = LCD_SPI_SCK;
  jshSPISetup(LCD_SPI, &inf);

  lcd_init(); // Send initialization commands to ST7735

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;

  graphicsClear(&gfx);
  int h=6;
  jswrap_graphics_drawCString(&gfx,0,h*1," ____                 _ ");
  jswrap_graphics_drawCString(&gfx,0,h*2,"|  __|___ ___ ___ _ _|_|___ ___ ");
  jswrap_graphics_drawCString(&gfx,0,h*3,"|  __|_ -| . |  _| | | |   | . |");
  jswrap_graphics_drawCString(&gfx,0,h*4,"|____|___|  _|_| |___|_|_|_|___|");
  jswrap_graphics_drawCString(&gfx,0,h*5,"         |_| espruino.com");
  jswrap_graphics_drawCString(&gfx,0,h*6," "JS_VERSION" (c) 2019 G.Williams");
  // Write MAC address in bottom right
  JsVar *addr = jswrap_ble_getAddress();
  char buf[20];
  jsvGetString(addr, buf, sizeof(buf));
  jsvUnLock(addr);
  jswrap_graphics_drawCString(&gfx,(LCD_WIDTH-1)-strlen(buf)*6,h*8,buf);
  lcd_flip_gfx(&gfx);

/*
  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || jshPinGetValue(BTN4_PININDEX) == BTN4_ONSTATE)) {
    // don't do it during a software reset - only first hardware reset
    jsiConsolePrintf("SELF TEST\n");
    if (pixl_selfTest()) jsiConsolePrintf("Test passed!\n");
  }*/

  // If the button is *still* pressed, remove all code from flash memory too!
  /*if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    jsfRemoveCodeFromFlash();
    jsiConsolePrintf("Removed saved code from Flash\n");
  }*/
  graphicsSetVar(&gfx);

  firstStart = false;
  jsvUnLock(graphics);

  // Setup touchscreen I2C
  i2cBusy = true;
  jshI2CInitInfo(&internalI2C);
  internalI2C.bitrate = 0x7FFFFFFF;
  internalI2C.pinSDA = ACCEL_PIN_SDA;
  internalI2C.pinSCL = ACCEL_PIN_SCL;
  jshPinSetValue(internalI2C.pinSCL, 1);
  jshPinSetState(internalI2C.pinSCL,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(internalI2C.pinSDA, 1);
  jshPinSetState(internalI2C.pinSDA,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  // accelerometer init
  jswrap_hackstrap_accelWr(0x18,0x0a); // CNTL1 Off, 4g range, Wakeup
  jswrap_hackstrap_accelWr(0x19,0x80); // CNTL2 Software reset
  jshDelayMicroseconds(2000);
  /*jswrap_hackstrap_accelWr(0x1b,0x02); // ODCNTL - 50Hz acceleration output data rate, filteringlow-pass  ODR/9
  jswrap_hackstrap_accelWr(0x1a,0xb11011110); // CNTL3
  // 50Hz tilt
  // 50Hz directional tap
  // 50Hz general motion detection and the high-pass filtered outputs
  jswrap_hackstrap_accelWr(0x1c,0); // INC1 disabled
  jswrap_hackstrap_accelWr(0x1d,0); // INC2 disabled
  jswrap_hackstrap_accelWr(0x1e,0); // INC3 disabled
  jswrap_hackstrap_accelWr(0x1f,0); // INC4 disabled
  jswrap_hackstrap_accelWr(0x20,0); // INC5 disabled
  jswrap_hackstrap_accelWr(0x21,0); // INC6 disabled
  jswrap_hackstrap_accelWr(0x23,3); // WUFC wakeupi detect counter
  //jswrap_hackstrap_accelWr(0x24,3); // TDTRC Tap detect enable
  //jswrap_hackstrap_accelWr(0x25, 0x78); // TDTC Tap detect double tap
  //jswrap_hackstrap_accelWr(0x26, 0x20); // TTH Tap detect threshold high (0xCB recommended)
  //jswrap_hackstrap_accelWr(0x27, 0x10); // TTH Tap detect threshold low (0x1A recommended)
  jswrap_hackstrap_accelWr(0x30,1); // ATH low wakeup detect threshold
  jswrap_hackstrap_accelWr(0x35,0); // LP_CNTL no averaging of samples
  jswrap_hackstrap_accelWr(0x3e,0); // clear the buffer*/
  jswrap_hackstrap_accelWr(0x18,0b10001100);  // CNTL1 On, ODR/2(high res), 4g range, Wakeup, tap
  // pressure init
  buf[0]=0x06; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, (uint8_t)*buf, true); // SOFT_RST
  i2cBusy = false;

  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  jshEnableWatchDog(10); // 10 second watchdog
  JsSysTime t = jshGetTimeFromMilliseconds(ACCEL_POLL_INTERVAL);
  jstExecuteFn(watchdogHandler, NULL, jshGetSystemTime()+t, t);


}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_hackstrap_kill"
}*/
void jswrap_hackstrap_kill() {
  jstStopExecuteFn(watchdogHandler, 0);
  jsvUnLock(promisePressure);
  promisePressure = 0;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_hackstrap_idle"
}*/
bool jswrap_hackstrap_idle() {
  if (strapTasks == JSS_NONE) return false;
  JsVar *strap =jsvObjectGetChild(execInfo.root, "Strap", 0);
  if (strapTasks & JSS_LCD_OFF) jswrap_hackstrap_setLCDPower(0);
  if (strapTasks & JSS_LCD_ON) jswrap_hackstrap_setLCDPower(1);
  if (strapTasks & JSS_ACCEL_DATA) {
    if (strap && jsiObjectHasCallbacks(strap, JS_EVENT_PREFIX"accel")) {
      JsVar *o = jsvNewObject();
      if (o) {
        jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(accx/8192.0));
        jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(accy/8192.0));
        jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(accz/8192.0));
        jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(accx*accx + accy*accy + accz*accz)/8192.0));
        jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(sqrt(accdiff)/8192.0));
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"accel", &o, 1);
        jsvUnLock(o);
      }
    }
    bool faceUp = (accz<7000) && abs(accx)<4096 && abs(accy)<4096;
    if (faceUp!=wasFaceUp) {
      faceUpCounter=0;
      wasFaceUp = faceUp;
    }
    if (faceUpCounter<255) faceUpCounter++;
    if (faceUpCounter==2) {
      if (strap) {
        JsVar *v = jsvNewFromBool(faceUp);
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"faceUp", &v, 1);
        jsvUnLock(v);
      }
      if (lcdPowerTimeout && !lcdPowerOn) {
        // LCD was turned off, turn it back on
        jswrap_hackstrap_setLCDPower(1);
        flipCounter = 0;
      }
    }
  }
  if (strap && (strapTasks & JSS_ACCEL_TAPPED)) {
    JsVar *o = jsvNewObject();
    if (o) {
      const char *string="";
      if (tapInfo&1) string="front";
      if (tapInfo&2) string="back";
      if (tapInfo&4) string="bottom";
      if (tapInfo&8) string="top";
      if (tapInfo&16) string="right";
      if (tapInfo&32) string="left";
      int n = (tapInfo&0x80)?2:1;
      jsvObjectSetChildAndUnLock(o, "dir", jsvNewFromString(string));
      jsvObjectSetChildAndUnLock(o, "double", jsvNewFromBool(tapInfo&0x80));
      jsvObjectSetChildAndUnLock(o, "x", jsvNewFromInteger((tapInfo&16)?-n:(tapInfo&32)?n:0));
      jsvObjectSetChildAndUnLock(o, "y", jsvNewFromInteger((tapInfo&4)?-n:(tapInfo&8)?n:0));
      jsvObjectSetChildAndUnLock(o, "z", jsvNewFromInteger((tapInfo&1)?-n:(tapInfo&2)?n:0));
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"tap", &o, 1);
      jsvUnLock(o);
    }
  }
  if (strap && (strapTasks & JSS_GPS_DATA)) {
    JsVar *line = jsvNewFromString(nmeaLine);
    if (line) jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"GPS", &line, 1);
    jsvUnLock(line);
  }
  jsvUnLock(strap);
  strapTasks = JSS_NONE;
  return false;
}

/*JSON{
  "type" : "EV_SERIAL1",
  "generate" : "jswrap_hackstrap_gps_character"
}*/
bool jswrap_hackstrap_gps_character(char ch) {
  if (ch=='\r') return true; // we don't care
  // if too many chars, roll over since it's probably because we skipped a newline
  if (nmeaCount>=sizeof(nmea)) nmeaCount=0;
  nmea[nmeaCount++]=ch;
  if (ch!='\n') return true; // now handled
  // Now we have a line of GPS data...
/*  $GNRMC,161945.00,A,5139.11397,N,00116.07202,W,1.530,,190919,,,A*7E
    $GNVTG,,T,,M,1.530,N,2.834,K,A*37
    $GNGGA,161945.00,5139.11397,N,00116.07202,W,1,06,1.29,71.1,M,47.0,M,,*64
    $GNGSA,A,3,09,06,23,07,03,29,,,,,,,1.96,1.29,1.48*14
    $GPGSV,3,1,12,02,45,293,13,03,10,109,16,05,13,291,,06,56,213,25*73
    $GPGSV,3,2,12,07,39,155,18,09,76,074,33,16,08,059,,19,02,218,18*7E
    $GPGSV,3,3,12,23,40,066,23,26,08,033,18,29,07,342,20,30,14,180,*7F
    $GNGLL,5139.11397,N,00116.07202,W,161945.00,A,A*69 */
  // Let's just chuck it over into JS-land for now
  if (nmeaCount>1) {
    memcpy(nmeaLine, nmea, nmeaCount);
    nmeaLine[nmeaCount-1]=0; // just overwriting \n
  }
  nmeaCount = 0;
  strapTasks |= JSS_GPS_DATA;
  return true; // handled
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "accelWr",
    "generate" : "jswrap_hackstrap_accelWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ]
}
Writes a register on the KX023 Accelerometer
*/
void jswrap_hackstrap_accelWr(JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 2, buf, true);
  i2cBusy = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "accelRd",
    "generate" : "jswrap_hackstrap_accelRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""]
}
Reads a register from the KX023 Accelerometer
*/
int jswrap_hackstrap_accelRd(JsVarInt reg) {
  unsigned char buf[1];
  buf[0] = (unsigned char)reg;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  i2cBusy = false;
  return buf[0];
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "getPressure",
    "generate" : "jswrap_hackstrap_getPressure",
    "return" : ["JsVar","A promise that will be resolved with `{temperature, pressure, altitude}`"]
}
Read temperature, pressure and altitude data. A promise is returned
which will be resolved with `{temperature, pressure, altitude}`.

Conversions take roughly 100ms.

```
Strap.getPressure().then(d=>{
  console.log(d);
  // {temperature, pressure, altitude}
});
```
*/
void jswrap_hackstrap_getPressure_callback() {
  JsVar *o = jsvNewObject();
  if (o) {
    i2cBusy = true;
    unsigned char buf[6];
    // ADC_CVT - 0b010 01 000  - pressure and temperature channel, OSR = 4096
    buf[0] = 0x48; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true);
    // wait 100ms
    jshDelayMicroseconds(100*1000); // we should really have a callback
    // READ_PT
    buf[0] = 0x10; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, PRESSURE_ADDR, 6, buf, true);
    int temperature = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (temperature&0x800000) temperature-=0x1000000;
    int pressure = (buf[3]<<16)|(buf[4]<<8)|buf[5];
    jsvObjectSetChildAndUnLock(o,"temperature", jsvNewFromFloat(temperature/100.0));
    jsvObjectSetChildAndUnLock(o,"pressure", jsvNewFromFloat(pressure/100.0));

    buf[0] = 0x31; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true); // READ_A
    jsi2cRead(&internalI2C, PRESSURE_ADDR, 3, buf, true);
    int altitude = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (altitude&0x800000) altitude-=0x1000000;
    jsvObjectSetChildAndUnLock(o,"altitude", jsvNewFromFloat(altitude/100.0));
    i2cBusy = false;

    jspromise_resolve(promisePressure, o);
  }
  jsvUnLock2(promisePressure,o);
  promisePressure = 0;
}

JsVar *jswrap_hackstrap_getPressure() {
  if (promisePressure) {
    jsExceptionHere(JSET_ERROR, "Conversion in progress");
    return 0;
  }
  promisePressure = jspromise_create();
  if (!promisePressure) return 0;

  jsiSetTimeout(jswrap_hackstrap_getPressure_callback, 100);
  return jsvLockAgain(promisePressure);
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "off",
    "generate" : "jswrap_hackstrap_off"
}
Turn HackStrap off. It can only be woken by pressing BTN1.
*/
void jswrap_hackstrap_off() {
  jsiKill();
  jsvKill();
  jshKill();
  jshPinOutput(GPS_PIN_EN,0); // GPS off
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  jshPinOutput(LCD_BL,1); // backlight off
  lcd_cmd(0x28, 0, NULL); // display off


  nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN1_PININDEX, NRF_GPIO_PIN_SENSE_LOW);
  sd_power_system_off();
}

/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "accel",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "HACKSTRAP"
}
Accelerometer data available with `{x,y,z,diff,mag}` object as a parameter

* `x` is X axis (left-right) in `g`
* `y` is Y axis (up-down) in `g`
* `z` is Z axis (in-out) in `g`
* `diff` is difference between this and the last reading in `g`
* `mag` is the magnitude of the acceleration in `g`
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "faceUp",
  "params" : [["up","bool","`true` if face-up"]],
  "ifdef" : "HACKSTRAP"
}
Has the watch been moved so that it is face-up, or not face up?
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "lcdPower",
  "params" : [["on","bool","`true` if screen is on"]],
  "ifdef" : "HACKSTRAP"
}
Has the screen been turned on or off? Can be used to stop tasks that are no longer useful if nothing is displayed.
*/
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "faceUp",
  "params" : [["data","JsVar","`{dir, double, x, y, z}`"]],
  "ifdef" : "HACKSTRAP"
}
If the watch is tapped, this event contains information on the way it was tapped.

`dir` reports the side of the watch that was tapped (not the direction it was tapped in).

```
{
  dir : "left/right/top/bottom/front/back",
  double : true/false // was this a double-tap?
  x : -2 .. 2, // the axis of the tap
  y : -2 .. 2, // the axis of the tap
  z : -2 .. 2 // the axis of the tap
```
 */

