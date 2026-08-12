/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Bangle.js 3 (http://www.espruino.com/Bangle.js)
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jswrap_bangle3.h"

#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/gpio.h>

#include "neopixel_bitbang.h"

#include "jswrap_bangle.h" // for jswrap_banglejs_touchHandler
#include "lcd_memlcd.h" // for lcdMemLCD_callWhenIdle
#include "graphics.h" // for error screen
#include "banglejs3_py32/src/const.h"

// ------------------------------------------------------ from jshardware.c
extern Pin eventFlagsToPin[ESPR_EXTI_COUNT];
extern const struct device *jshToZephyrPort(JsvPinInfoPort port);
// ----------------------------------------------------------------------------
#define PY32_OUT_SHIFT 4
/// bottom 4 bits are buttons, higher bits are outputs
unsigned short sxValues = (PY32_OUT_DEFAULTS << PY32_OUT_SHIFT);


// Simple write to PY32 - we bit-bang this as setting up SPI for 1 byte takes too long and doesn't work in an IRQ
void jshPY32Transfer(uint8_t *buf, int count) {
  // assume display SPI is currently disabled as we only enable it for LCD flip
  //pm_device_action_run(spi1_dev, PM_DEVICE_ACTION_SUSPEND);

  jshPinSetValue(LCD_SPI_CS, 0);
  const JshPinInfo *mosi = &pinInfo[LCD_SPI_MOSI];
  const JshPinInfo *miso = &pinInfo[LCD_SPI_MISO];
  const JshPinInfo *sck = &pinInfo[LCD_SPI_SCK];
  const struct device *mosiport = jshToZephyrPort(mosi->port);
  const struct device *misoport = jshToZephyrPort(miso->port);
  const struct device *sckport = jshToZephyrPort(sck->port);
  for (volatile int i=0;i<1000;i++); // delay (we can't use k_usleep as we could be in an IRQ here)
  for (unsigned int i=0;i<count;i++) {
    int data = buf[i], rxdata = 0;
    int bit;
    for (bit=7;bit>=0;bit--) {
      gpio_pin_set_raw(mosiport, mosi->pin, (data>>bit)&1);
      gpio_pin_set_raw(sckport, sck->pin, 1);
      rxdata |= gpio_pin_get_raw(misoport, miso->pin) ? (1<<bit) : 0;
      gpio_pin_set_raw(sckport, sck->pin, 0);
    }
    buf[i] = rxdata;
  }
  jshPinSetValue(LCD_SPI_CS, 1);
}

// Send state to PY32
void jshPY32Update(bool setOutput) {
  uint8_t buf[3];
  if (setOutput) {
    PY32OutputState pyOutputState = sxValues>>PY32_OUT_SHIFT; // current output values
    buf[0] = PY32_CMD_SET_OUTPUT;
    buf[1] = pyOutputState&255;
    buf[2] = pyOutputState>>8;
  } else {
    buf[0] = PY32_CMD_NONE;
    buf[1] = 0;
    buf[2] = 0;
  }
  jshPY32Transfer(buf, sizeof(buf));
  uint8_t pyButtonState = buf[0];
  //jsiConsolePrintf("B %d %d %d\n", buf[0],buf[1],buf[2]);
  #if PY32_OUT_SHIFT!=4
  #error PY32_OUT_SHIFT=4
  #endif

  sxValues = (sxValues&~15) | (pyButtonState&15);
  if (pyButtonState & 16) jswrap_banglejs_touchHandler(0,0); // touch handler IRQ
}

void jshVirtualPinInitialise() {
  sxValues    = 0;
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  int p = pinInfo[pin].pin;
  if (!IS_PIN_A_BUTTON(pin)) { // buttons read only
    if (state) sxValues |= 1<<p;
    else sxValues &= ~(1<<p);
  }
  // set status flags for PY32
  jshPY32Update(true);
}

bool jshVirtualPinGetValue(Pin pin) {
  int p = pinInfo[pin].pin;
  return ((sxValues >> p) & 1) != 0;
}

void jshVirtualPinSetState(Pin pin, JshPinState state) {
}

JshPinState jshVirtualPinGetState(Pin pin) {
  return (IS_PIN_A_LED(pin) ? JSHPINSTATE_GPIO_OUT : JSHPINSTATE_GPIO_IN) | (jshVirtualPinGetValue(pin)?JSHPINSTATE_PIN_IS_ON:0);
}

/// called when we're sure the LCD SPI interface is idle!
void jshVirtualPinIRQWorker() {
  static volatile bool alreadyInWorker = false;
  if (alreadyInWorker) return;
  alreadyInWorker = true;
  uint16_t lastState = sxValues;
  jshPY32Update(false); // no set
  uint16_t changed = lastState ^ sxValues;
  /*if (changed & 16)
    jsiConsolePrintf("Touch IRQ");*/ // FIXME we do this in jshPY32Update at the moment
  if (changed & 15) {
    for (int i=0;i<ESPR_EXTI_COUNT;i++)
      if (((changed&1) && eventFlagsToPin[i]==BTN1_PININDEX) ||
          ((changed&2) && eventFlagsToPin[i]==BTN2_PININDEX) ||
          ((changed&4) && eventFlagsToPin[i]==BTN3_PININDEX) ||
          ((changed&8) && eventFlagsToPin[i]==BTN4_PININDEX)) {
        jshPushIOWatchEvent(EV_EXTI0+i);
      }
  }
  alreadyInWorker = false;
}

void jshVirtualPinIRQHandler(bool state, IOEventFlags flags) {
  if (!state) {
    // IRQ low, so something ready
    lcdMemLCD_callWhenIdle(jshVirtualPinIRQWorker);
  }
}

// ----------------------------------------------------------------------------

/* Override Zephyr's weak fatal error handler with something to write to the LCD */
void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf) {
    // name the error
    char buf[20];
  const char *reason_str = "UNKNOWN";
  switch (reason) {
  case K_ERR_CPU_EXCEPTION: reason_str = "CPU FAULT"; break;
  case K_ERR_SPURIOUS_IRQ: reason_str = "SPURIOUS IRQ"; break;
  case K_ERR_STACK_CHK_FAIL: reason_str = "STACK OVERFLOW"; break;
  case K_ERR_KERNEL_PANIC: reason_str = "KERNEL PANIC"; break;
  case K_ERR_KERNEL_OOPS: reason_str = "KERNEL OOPS"; break;
  // K_ERR_ARCH_START = 16
  case 0x12: reason_str = "MEM INST"; break;
  case 0x19: reason_str = "BUS FAULT"; break;
  case 0x1A: reason_str = "USAGE FAULT"; break;
  default:
      strcpy(buf, "UNKNOWN 0x");
      itostr(reason, &buf[10], 16);
      reason_str = buf;
      break;
  }

  // crash screen
  JsGraphics *gfx = &graphicsInternal;
  graphicsStructResetState(gfx);
  graphicsClear(gfx);
  int y=60;
  graphicsDrawString(gfx,60,y+=10,"Espruino "JS_VERSION);
  graphicsDrawString(gfx,60,y+=10,ESPR_STRINGIFY(GIT_COMMIT));
  graphicsDrawString(gfx,60,y+=20,reason_str);
  /* Exception Stack Frame (PC and LR registers) */
  if (esf != NULL) {
    strcpy(buf, "0x");
    itostr_extra(esf->basic.pc, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=20,"PC");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.lr, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"LR");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.ip, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"IP");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.r0, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"r0");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.r1, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"r1");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.r2, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"r2");
    graphicsDrawString(gfx,80,y,buf);
    itostr_extra(esf->basic.r3, &buf[2], false, 16);
    graphicsDrawString(gfx,60,y+=10,"r3");
    graphicsDrawString(gfx,80,y,buf);
  }

  //jshPinSetValue(LCD_BL, 1); // enable backlight
  // Output to LCD (Direct SW Mode)
  unsigned char *pixels = lcdMemLCD_getRowPtr(0)-1; // ignore row header
  jshPY32Transfer(pixels, LCD_HEIGHT + ((LCD_WIDTH*LCD_HEIGHT*6) >> 3));

  /* 4. Choose recovery path: Halt or Reset */
  #if defined(CONFIG_REBOOT)
      // Optionally wait 5 seconds so user can read screen, then reboot
      k_busy_wait(5000000);
      sys_reboot(SYS_REBOOT_COLD);
  #else
      // Or halt CPU execution indefinitely
      for (;;) {
          __NOP();
      }
  #endif
}

// ----------------------------------------------------------------------------

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setRGB",
    "generate" : "jswrap_banglejs_setRGB",
    "params" : [
      ["red","int","red channel (0..255)"],
      ["green","int","green channel (0..255)"],
      ["blue","int","blue channel (0..255)"]
    ],
    "ifdef" : "BANGLEJS3"
}
This function can be used to change Bangle.js's RGB indicator light.

Using to `Bangle.setRGB(0,0,0)` or just using `Bangle.setRGB()` will
turn the LED off.
*/
void jswrap_banglejs_setRGB(int r, int g, int b) {
  if (!r && !g && !b) {
    jshPinSetValue(MISC_PIN_RGB_DATA, 0);
    jshPinSetValue(MISC_PIN_RGB_EN, 0);
    return;
  }
  jshPinSetValue(MISC_PIN_RGB_EN, 1);
  jshDelayMicroseconds(100);


  // just use software bit-banging - it's only 24 bits and using a HW SPI seems like a waste
  uint8_t rgbData[] = { r,g,b };
  neopixelWrite_bitbang(MISC_PIN_RGB_DATA, rgbData, 3);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "enableUART",
    "generate" : "jswrap_banglejs_enableUART",
    "params" : [
      ["en","bool","true to enable"]
    ],
    "ifdef" : "BANGLEJS3"
}
Enables the UART on the charging port of the Bangle.js
*/
void jswrap_banglejs_enableUART(bool en) {
  if (en) {
    //jshPinSetValue(MISC_PIN_AUX_SWAP, 1); // FIXME
    pm_device_action_run(DEVICE_DT_GET(DT_NODELABEL(uart20)), PM_DEVICE_ACTION_RESUME);
  } else {
    //jshPinSetValue(MISC_PIN_AUX_SWAP, 0); // FIXME
    pm_device_action_run(DEVICE_DT_GET(DT_NODELABEL(uart20)), PM_DEVICE_ACTION_SUSPEND);
  }
}


/*JSON{
  "type" : "hwinit",
  "generate" : "jswrap_banglejs3_hwinit"
}*/
void jswrap_banglejs3_hwinit() {
  // LCD controller
  jshPinSetValue(LCD_SPI_CS, 1);
  jshPinSetState(LCD_SPI_CS, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(LCD_SPI_IRQ, JSHPINSTATE_GPIO_IN_PULLUP);
  jshDelayMicroseconds(100); // wait for pins to settle
  jshPY32Update(false); // update current status (and clear IRQ line)
  IOEventFlags channel = jshPinWatch(LCD_SPI_IRQ, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, jshVirtualPinIRQHandler);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_banglejs3_idle"
}*/
bool jswrap_banglejs3_idle() {
  /* if for some reason the IRQ is low while in idle (it should normally
  be handled by IRQ, handle it here */
  /*if (!jshPinGetValue(LCD_SPI_IRQ))
    lcdMemLCD_callWhenIdle(jshVirtualPinIRQWorker);*/
  // FIXME: we can't do this here because the py32 wants to wait until it has finished updating the LCD before re-initialising SPI
  return false;
}