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

#include <jswrap_pixljs.h>
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jsnative.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jstimer.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf5x_utils.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h" // for self-test

#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"

const Pin PIXL_IO_PINS[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

/*JSON{
    "type": "class",
    "class" : "Pixl"
}
Class containing utility functions for
[Pixl.js](http://www.espruino.com/Pixl.js)
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "getBatteryPercentage",
    "generate" : "jswrap_espruino_getBattery",
    "return" : ["int", "A percentage between 0 and 100" ]
}
DEPRECATED - Please use `E.getBattery()` instead.

Return an approximate battery percentage remaining based on a normal CR2032
battery (2.8 - 2.2v)
*/
JsVarInt jswrap_pixljs_getBattery() {
  JsVarFloat v = jshReadVRef();
  int pc = (v-2.2)*100/0.6;
  if (pc>100) pc=100;
  if (pc<0) pc=0;
  return pc;
}




/*JSON{
  "type" : "variable",
  "name" : "SDA",
  "generate_full" : "JSH_PORTA_OFFSET + 4",
  "ifdef" : "PIXLJS",
  "return" : ["pin",""]
}
The pin marked SDA on the Arduino pin footprint. This is connected directly to
pin A4.
*/
/*JSON{
  "type" : "variable",
  "name" : "SCL",
  "generate_full" : "JSH_PORTA_OFFSET + 5",
  "ifdef" : "PIXLJS",
  "return" : ["pin",""]
}
The pin marked SDA on the Arduino pin footprint. This is connected directly to
pin A5.
*/


void lcd_wr(int data) {
  int bit;
  for (bit=7;bit>=0;bit--) {
    // TODO: we could push this faster by accessing IO directly
    jshPinSetValue(LCD_SPI_MOSI, (data>>bit)&1 );
    jshPinSetValue(LCD_SPI_SCK, 1 );
    jshPinSetValue(LCD_SPI_SCK, 0 );
  }
}

void lcd_flip_gfx(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!

  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar,"buffer",0);
  if (!buf) return;
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (!bPtr || bLen<128*8) return;

  int xcoord = gfx->data.modMinX&~7;
  int xlen = gfx->data.modMaxX+1-xcoord;

  jshPinSetValue(LCD_SPI_CS,0);
  for (int y=0;y<8;y++) {
    // skip any lines that don't need updating
    int ycoord = y*8;
    if (ycoord > gfx->data.modMaxY ||
        ycoord+7 < gfx->data.modMinY) continue;
    // Send only what we need
    jshPinSetValue(LCD_SPI_DC,0);
    lcd_wr(0xB0|y/* page */);
    lcd_wr(0x00|(xcoord&15)/* x lower*/);
    lcd_wr(0x10|(xcoord>>4)/* x upper*/);
    jshPinSetValue(LCD_SPI_DC,1);

    char *px = &bPtr[y*128 + (xcoord>>3)];
    for (int x=0;x<xlen;x++) {
      int bit = 128>>(x&7);
      lcd_wr(
          ((px[  0]&bit)?1:0) |
          ((px[ 16]&bit)?2:0) |
          ((px[ 32]&bit)?4:0) |
          ((px[ 48]&bit)?8:0) |
          ((px[ 64]&bit)?16:0) |
          ((px[ 80]&bit)?32:0) |
          ((px[ 96]&bit)?64:0) |
          ((px[112]&bit)?128:0));
      if ((x&7) == 7) px++;
    }
  }
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
    gfx.data.modMaxX = 127;
    gfx.data.modMaxY = 63;
  }
  lcd_flip_gfx(&gfx);
  graphicsSetVar(&gfx);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "setContrast",
    "generate" : "jswrap_pixljs_setContrast",
    "params" : [
      ["c","float","Contrast between 0 and 1"]
    ]
}
Set the LCD's contrast
*/
void jswrap_pixljs_setContrast(JsVarFloat c) {
  if (c<0) c=0;
  if (c>1) c=1;
  jshPinSetValue(LCD_SPI_CS,0);
  jshPinSetValue(LCD_SPI_DC,0);
  lcd_wr(0x81);
  lcd_wr((int)(63*c));
  //lcd_wr(0x20|div); div = 0..7
  jshPinSetValue(LCD_SPI_CS,1);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "setLCDPower",
    "generate" : "jswrap_pixljs_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ]
}
This function can be used to turn Pixl.js's LCD off or on.

* With the LCD off, Pixl.js draws around 0.1mA
* With the LCD on, Pixl.js draws around 0.25mA
*/
void jswrap_pixljs_setLCDPower(bool isOn) {
  jshPinSetValue(LCD_SPI_CS,0);
  jshPinSetValue(LCD_SPI_DC,0);
  if (isOn) {
    lcd_wr(0xA4); // cancel pixel on
    lcd_wr(0xAF); // display on
  } else {
    lcd_wr(0xAE); // display off
    lcd_wr(0xA5); // all pixels on
  }
  jshPinSetValue(LCD_SPI_CS,1);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "lcdw",
    "generate" : "jswrap_pixljs_lcdw",
    "params" : [
      ["c","int",""]
    ]
}
Writes a command directly to the ST7567 LCD controller
*/
void jswrap_pixljs_lcdw(JsVarInt c) {
  jshPinSetValue(LCD_SPI_CS,0);
  jshPinSetValue(LCD_SPI_DC,0);
  lcd_wr(c);
  jshPinSetValue(LCD_SPI_CS,1);
}

static bool selftest_check_pin(Pin pin) {
  unsigned int i;
  bool ok = true;
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 1);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced low\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
    if (PIXL_IO_PINS[i]!=pin)
      jshPinOutput(PIXL_IO_PINS[i], 0);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p shorted low\n", pin);
    ok = false;
  }

  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 0);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLDOWN);
  if (jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced high\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
     if (PIXL_IO_PINS[i]!=pin)
       jshPinOutput(PIXL_IO_PINS[i], 1);
   if (jshPinGetValue(pin)) {
     jsiConsolePrintf("Pin %p shorted high\n", pin);
     ok = false;
   }
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
  return ok;
}

static bool pixl_selfTest() {
  unsigned int timeout, i;
  JsVarFloat v;
  bool ok = true;

  // light up all LEDs white
  jshPinOutput(LED1_PININDEX, LED1_ONSTATE);
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);

  v = jshReadVRef();
  if (v<3.2 || v>3.4) {
    jsiConsolePrintf("VCC out of range 3.2-3.4 (%f)\n", v);
    ok = false;
  }

  if (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE)
    jsiConsolePrintf("Release BTN1\n");
  if (jshPinGetValue(BTN4_PININDEX)==BTN4_ONSTATE)
    jsiConsolePrintf("Release BTN4\n");
  timeout = 2000;
  while ((jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE ||
          jshPinGetValue(BTN4_PININDEX)==BTN4_ONSTATE) && timeout--)
    nrf_delay_ms(1);
  if (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN1 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN2_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN2 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN3_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN3 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN4_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN4 stuck down\n");
    ok = false;
  }
  nrf_delay_ms(100);
  jshPinInput(LED1_PININDEX);
  nrf_delay_ms(500);

  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED1_PININDEX);
  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.3 || v>0.65) {
    jsiConsolePrintf("Backlight OOR (%f)\n", v);
    ok = false;
  }

  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++) {
    if (!selftest_check_pin(PIXL_IO_PINS[i])) ok = false;
  }

  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
    jshPinSetState(PIXL_IO_PINS[i], JSHPINSTATE_GPIO_IN);

  if (jshHasEvents()) {
    jsiConsolePrintf("Have events - no BLE test\n");
  } else {
    uint32_t err_code;
    err_code = jsble_set_scanning(true, NULL);
    jsble_check_error(err_code);
    int timeout = 20;
    while (timeout-- && !jshHasEvents()) {
      nrf_delay_ms(100);
    }
    err_code = jsble_set_scanning(false, NULL);
    jsble_check_error(err_code);
    if (!jshHasEvents()) {
      jsiConsolePrintf("No BLE adverts found in 2s\n");
      ok = false;
    }
  }

  return ok;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_pixljs_init"
}*/
void jswrap_pixljs_init() {
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_DC,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinOutput(LCD_SPI_MOSI,0);
  jshPinOutput(LCD_SPI_RST,0);
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx,128,64,1);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_ARRAYBUFFER_MSB;
  gfx.graphicsVar = graphics;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVarInitial(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);
  // Set initial image
  const unsigned char PIXLJS_IMG[] = {
        251, 239, 135, 192, 0, 0, 31, 0, 0, 0, 125, 247, 195, 224, 0, 0, 15, 128, 0,
        0, 62, 251, 225, 240, 0, 0, 7, 192, 0, 0, 31, 125, 240, 248, 0, 0, 3, 224, 0,
        0, 15, 190, 248, 124, 0, 0, 1, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        224, 62, 0, 15, 128, 248, 124, 0, 0, 1, 240, 31, 0, 7, 192, 124, 62, 0, 0, 0,
        248, 15, 128, 3, 224, 62, 31, 0, 0, 0, 124, 7, 192, 1, 240, 31, 15, 128, 0,
        0, 62, 3, 224, 0, 248, 15, 135, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15,
        190, 248, 124, 0, 248, 1, 240, 0, 0, 7, 223, 124, 62, 0, 124, 0, 248, 1, 128,
        3, 239, 190, 31, 0, 62, 0, 124, 1, 224, 1, 247, 223, 15, 128, 31, 0, 62, 0,
        240, 0, 251, 239, 135, 192, 15, 128, 31, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 62, 0, 1, 240, 248, 15, 135, 223, 30, 31, 223, 0, 0, 248, 124, 7, 195, 239,
        143, 31, 255, 128, 0, 124, 62, 3, 225, 247, 199, 158, 55, 192, 0, 62, 31, 1,
        240, 251, 227, 199, 131, 224, 0, 31, 15, 128, 248, 125, 241, 227, 240, 0, 0,
        0, 0, 0, 0, 0, 0, 240, 254, 0, 0, 0, 0, 0, 0, 0, 0, 120, 31, 128, 0, 0, 0, 0,
        0, 0, 12, 60, 3, 192, 0, 0, 0, 0, 0, 0, 15, 30, 49, 224, 0, 0, 0, 0, 0, 0, 7,
        143, 63, 240, 0, 0, 0, 0, 0, 0, 1, 135, 143, 224, 0, 0, 0, 0, 0, 0, 0, 3, 192,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 224, 0, 0, 0, 0, 0, 0, 0, 0, 7, 224, 0, 0, 0, 0, 0,
        0, 0, 0, 3, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 240, 0, 63
  };

  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);
  // LCD init 2
  jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
  const unsigned char LCD_SPI_INIT_DATA[] = {
       //0xE2,  // soft reset
       0xA2,   // bias 1/9
       //0xA3,   // bias 1/7
       0xC8,   // reverse scan dir
       0x81,   // contrast control (next byte)
       35,     // actual contrast (0..63)
       0x25,   // regulation resistor ratio (0..7)
       0x2F,   // control power circuits - last 3 bits = VB/VR/VF
       0xF8, 1, // Set boost level to 5x - draws maybe 0.04mA more, but much better blacks
       0xA0,   // start at column 128
       0xAF    // disp on
  };
  for (unsigned int i=0;i<sizeof(LCD_SPI_INIT_DATA);i++)
    lcd_wr(LCD_SPI_INIT_DATA[i]);
  jshPinSetValue(LCD_SPI_CS,1);

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;

  JsVar *splashScreen = jsfReadFile(jsfNameFromString(".splash"),0,0);
  if (jsvIsString(splashScreen)) {
    if (jsvGetStringLength(splashScreen)) {
      graphicsSetVar(&gfx);
      jsvUnLock(jswrap_graphics_drawImage(graphics, splashScreen,0,0,0));
      graphicsGetFromVar(&gfx, graphics);
    }
  } else {
    if (firstStart) {
      // animate logo in
      for (int i=128;i>24;i-=4) {
        lcd_flip_gfx(&gfx);
        graphicsClear(&gfx);
        graphicsDrawImage1bpp(&gfx,i,15,81,34,PIXLJS_IMG);
      }
    } else {
      // if a standard reset, just display logo
      graphicsClear(&gfx);
      graphicsDrawImage1bpp(&gfx,24,15,81,34,PIXLJS_IMG);
    }
    jswrap_graphics_drawCString(&gfx,28,39,JS_VERSION);
    // Write MAC address in bottom right
    JsVar *addr = jswrap_ble_getAddress();
    char buf[20];
    jsvGetString(addr, buf, sizeof(buf));
    jsvUnLock(addr);
    jswrap_graphics_drawCString(&gfx,127-strlen(buf)*4,59,buf);
  }
  lcd_flip_gfx(&gfx);
  jsvUnLock(splashScreen);


  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || jshPinGetValue(BTN4_PININDEX) == BTN4_ONSTATE)) {
    // don't do it during a software reset - only first hardware reset
    jsiConsolePrintf("SELF TEST\n");
    if (pixl_selfTest()) jsiConsolePrintf("Test passed!\n");
  }

  // If the button is *still* pressed, remove all code from flash memory too!
  if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    jsfRemoveCodeFromFlash();
    jsiConsolePrintf("Removed saved code from Flash\n");
  }
  graphicsSetVar(&gfx);

  firstStart = false;
  jsvUnLock(graphics);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_pixljs_kill"
}*/
void jswrap_pixljs_kill() {

}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_pixljs_idle"
}*/
bool jswrap_pixljs_idle() {
  return false;
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "menu",
    "generate" : "gen_jswrap_E_showMenu",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ],
    "typescript" : "menu(menu: Menu): MenuInstance;"
}
Display a menu on Pixl.js's screen, and set up the buttons to navigate through
it.

DEPRECATED: Use `E.showMenu`
*/

/*TYPESCRIPT
/**
 * Menu item that holds a boolean value.
 *\/
type MenuBooleanItem = {
  value: boolean;
  format?: (value: boolean) => string;
  onchange?: (value: boolean) => void;
};

/**
 * Menu item that holds a numerical value.
 *\/
type MenuNumberItem = {
  value: number;
  format?: (value: number) => string;
  onchange?: (value: number) => void;
  step?: number;
  min?: number;
  max?: number;
  wrap?: boolean;
};

/**
 * Options passed to a menu.
 *\/
type MenuOptions = {
  title?: string;
  back?: () => void;
  selected?: number;
  fontHeight?: number;
  x?: number;
  y?: number;
  x2?: number;
  y2?: number;
  cB?: number;
  cF?: number;
  cHB?: number;
  cHF?: number;
  predraw?: (g: Graphics) => void;
  preflip?: (g: Graphics, less: boolean, more: boolean) => void;
};

/**
 * Object containing data about a menu to pass to `E.showMenu`.
 *\/
type Menu = {
  ""?: MenuOptions;
  [key: string]:
    | MenuOptions
    | (() => void)
    | MenuBooleanItem
    | MenuNumberItem
    | { value: string; onchange?: () => void }
    | undefined;
};

/**
 * Menu instance.
 *\/
type MenuInstance = {
  draw: () => void;
  move: (n: number) => void;
  select: () => void;
};
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMenu",
    "generate_js" : "libs/js/pixljs/E_showMenu.min.js",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ],
    "typescript": [
      "showMenu(menu: Menu): MenuInstance;",
      "showMenu(): void;"
    ]
}
Display a menu on the screen, and set up the buttons to navigate through it.

Supply an object containing menu items. When an item is selected, the function
it references will be executed. For example:

```
var boolean = false;
var number = 50;
// First menu
var mainmenu = {
  "" : { "title" : "-- Main Menu --" },
  "Backlight On" : function() { LED1.set(); },
  "Backlight Off" : function() { LED1.reset(); },
  "Submenu" : function() { E.showMenu(submenu); },
  "A Boolean" : {
    value : boolean,
    format : v => v?"On":"Off",
    onchange : v => { boolean=v; }
  },
  "A Number" : {
    value : number,
    min:0,max:100,step:10,
    onchange : v => { number=v; }
  },
  "Exit" : function() { E.showMenu(); }, // remove the menu
};
// Submenu
var submenu = {
  "" : { title : "-- SubMenu --",
         back : function() { E.showMenu(mainmenu); } },
  "One" : undefined, // do nothing
  "Two" : undefined // do nothing
};
// Actually display the menu
E.showMenu(mainmenu);
```

The menu will stay onscreen and active until explicitly removed, which you can
do by calling `E.showMenu()` without arguments.

See http://www.espruino.com/graphical_menu for more detailed information.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMessage",
    "generate_js" : "libs/js/pixljs/E_showMessage.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["title","JsVar","(optional) a title for the message"]
    ],
    "ifdef" : "PIXLJS",
    "typescript" : "showMessage(message: string, title?: string): void;"
}

A utility function for displaying a full screen message on the screen.

Draws to the screen and returns immediately.

```
E.showMessage("These are\nLots of\nLines","My Title")
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showPrompt",
    "generate_js" : "libs/js/pixljs/E_showPrompt.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","(optional) an object of options (see below)"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "PIXLJS",
    "typescript" : [
      "showPrompt<T = boolean>(message: string, options?: { title?: string, buttons?: { [key: string]: T } }): Promise<T>;",
      "showPrompt(): void;"
    ]
}

Displays a full screen prompt on the screen, with the buttons requested (or
`Yes` and `No` for defaults).

When the button is pressed the promise is resolved with the requested values
(for the `Yes` and `No` defaults, `true` and `false` are returned).

```
E.showPrompt("Do you like fish?").then(function(v) {
  if (v) print("'Yes' chosen");
  else print("'No' chosen");
});
// Or
E.showPrompt("How many fish\ndo you like?",{
  title:"Fish",
  buttons : {"One":1,"Two":2,"Three":3}
}).then(function(v) {
  print("You like "+v+" fish");
});
```

To remove the prompt, call `E.showPrompt()` with no arguments.

The second `options` argument can contain:

```
{
  title: "Hello",                      // optional Title
  buttons : {"Ok":true,"Cancel":false} // list of button text & return value
}
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showAlert",
    "generate_js" : "libs/js/pixljs/E_showAlert.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","(optional) a title for the message"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "PIXLJS",
    "typescript" : "showAlert(message?: string, options?: string): Promise<void>;"
}

Displays a full screen prompt on the screen, with a single 'Ok' button.

When the button is pressed the promise is resolved.

```
E.showAlert("Hello").then(function() {
  print("Ok pressed");
});
// or
E.showAlert("These are\nLots of\nLines","My Title").then(function() {
  print("Ok pressed");
});
```

To remove the window, call `E.showAlert()` with no arguments.
*/
