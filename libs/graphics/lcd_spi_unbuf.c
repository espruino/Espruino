/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>, atc1441, MaBecker          
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Backend for drawing to SPI displays in unbuffered mode 
 * ----------------------------------------------------------------------------
 */

/*JSON{
  "type" : "class",
  "class" : "lcd_spi_unbuf"
}*/

#include "lcd_spi_unbuf.h"
#include "jsutils.h"
#include "jsinteractive.h"
#include "jswrap_graphics.h"
#include "jshardware.h"

int _pin_mosi;
int _pin_clk;
int _pin_cs;
int _pin_dc;
int _colstart;
int _rowstart;
IOEventFlags _device;

static void spi_cmd(const uint8_t cmd) 
{
  jshPinSetValue(_pin_dc, 0);
  jshSPISend(_device, cmd);
  jshPinSetValue(_pin_dc, 1);
}

static void spi_data(const uint8_t *data, int len) 
{
  if (len==0) return;
  jshSPISendMany(_device, data, NULL, len, NULL);
}

void jshLCD_SPI_UNBUFInitInfo(JshLCD_SPI_UNBUFInfo *inf) {
  inf->pinCS         = PIN_UNDEFINED;
  inf->pinDC         = PIN_UNDEFINED;
  inf->width         = 240;
  inf->height        = 320;
  inf->colstart        = 0;
  inf->rowstart        = 0;
}

bool jsspiPopulateOptionsInfo( JshLCD_SPI_UNBUFInfo *inf, JsVar *options){
  jshLCD_SPI_UNBUFInitInfo(inf);
  jsvConfigObject configs[] = {
    {"cs", JSV_PIN, &inf->pinCS},
    {"dc", JSV_PIN, &inf->pinDC},
    {"width", JSV_INTEGER , &inf->width},
    {"height", JSV_INTEGER , &inf->height},
    {"colstart", JSV_INTEGER , &inf->colstart},
    {"rowstart", JSV_INTEGER , &inf->rowstart},
  };  
  
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {	
    if ( inf->pinDC == PIN_UNDEFINED ) {
      return false;
    }
    return true;
  }  
  else { 
    return false;
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_spi_unbuf",
  "name" : "connect",
  "generate" : "jswrap_lcd_spi_unbuf_connect",
  "params" : [
    ["device","JsVar","The used SPI device"],
    ["options","JsVar","An Object containing extra information"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}*/
JsVar *jswrap_lcd_spi_unbuf_connect(JsVar *device, JsVar *options) { 
  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) {
    jsExceptionHere(JSET_ERROR,"creating new object Graphics");
    return NULL;  
  }

  JshLCD_SPI_UNBUFInfo inf;

  if (!jsspiPopulateOptionsInfo(&inf, options)) {
    jsExceptionHere(JSET_ERROR,"pins not supplied correctly");
    jsvUnLock(parent);
  	return NULL;
  }
  _pin_cs = inf.pinCS;
  _pin_dc = inf.pinDC;
  _colstart = inf.colstart;
  _rowstart = inf.rowstart;
  _device = jsiGetDeviceFromClass(device);
	
  if (!DEVICE_IS_SPI(_device)) { 
  	jsExceptionHere(JSET_ERROR,"Software SPI is not supported for now");
  	jsvUnLock(parent);
    return NULL;
  }  

  JsGraphics gfx;
  graphicsStructInit(&gfx,inf.width,inf.height,16);
  gfx.data.type = JSGRAPHICSTYPE_LCD_SPI_UNBUF;
  gfx.graphicsVar = parent;
  
  jshPinOutput(_pin_dc, 1);
  jshPinSetValue(_pin_dc, 1);

  jshPinOutput(_pin_cs, 1);
  jshPinSetValue(_pin_cs, 1);
  
  lcd_spi_unbuf_setCallbacks(&gfx);
  graphicsSetVar(&gfx);	
  return parent;
}

void disp_spi_transfer_addrwin(int x1, int y1, int x2, int y2) {
  unsigned char wd[4];
  x1 += _colstart;
  y1 += _rowstart;
  x2 += _colstart;
  y2 += _rowstart;
  spi_cmd(0x2A);
  wd[0] = x1>>8;
  wd[1] = x1 & 0xFF;
  wd[2] = x2>>8;
  wd[3] = x2 & 0xFF;
  spi_data((uint8_t *)&wd, 4);
  spi_cmd(0x2B);
  wd[0] = y1>>8;
  wd[1] = y1 & 0xFF;
  wd[2] = y2>>8;
  wd[3] = y2 & 0xFF;
  spi_data((uint8_t *)&wd, 4);
  spi_cmd(0x2C);
}

void disp_spi_transfer_color_many(uint16_t color, uint32_t len)
{
  uint16_t buffer_color[BUFFER];
  uint8_t idx = 0;
  uint32_t count = 0; 
  while (count < len) {
    buffer_color[idx] = (color>>8) | (color<<8);    
    idx++;  
    count++;  
    if (idx == BUFFER) {
      spi_data((uint8_t *)&buffer_color, idx*2);
      idx = 0;
    }
  }
  if (idx > 0) 
    spi_data((uint8_t *)&buffer_color, idx*2);
}

void lcd_spi_unbuf_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  uint16_t color =   (col>>8) | (col<<8); 
  jshPinSetValue(_pin_cs, 0);
  disp_spi_transfer_addrwin(x, y, x+1, y+1);  
  spi_data((uint8_t *)&color, 2);
  jshPinSetValue(_pin_cs, 1);
}

void lcd_spi_unbuf_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int pixels = (1+x2-x1)*(1+y2-y1);	
  jshPinSetValue(_pin_cs, 0);
  disp_spi_transfer_addrwin(x1, y1, x2, y2);
  disp_spi_transfer_color_many(col, pixels);
  jshPinSetValue(_pin_cs, 1);
}

void lcd_spi_unbuf_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcd_spi_unbuf_setPixel;
  gfx->fillRect = lcd_spi_unbuf_fillRect;
}
