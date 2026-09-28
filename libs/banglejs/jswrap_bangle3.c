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
#include "lcd_memlcd.h"

#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/gpio.h>

#include "neopixel_bitbang.h"

#include "jswrap_bangle.h" // for jswrap_banglejs_touchHandler
#include "lcd_memlcd.h" // for lcdMemLCD_callWhenIdle
#include "graphics.h" // for error screen
#include "banglejs3_py32/src/const.h"
#include "banglejs3_py32/src/swd.h"
#include "banglejs3_py32/py32_firmware.h"

// ------------------------------------------------------ from jshardware.c
extern Pin eventFlagsToPin[ESPR_EXTI_COUNT];
extern const struct device *jshToZephyrPort(JsvPinInfoPort port);
// ----------------------------------------------------------------------------
#define PY32_OUT_SHIFT 4
/// bottom 4 bits are buttons, higher bits are outputs
unsigned short sxValues = (PY32_OUT_DEFAULTS << PY32_OUT_SHIFT);
volatile bool inPY32Update = false;

// Simple write to PY32 - we bit-bang this as setting up SPI for 1 byte takes too long and doesn't work in an IRQ
void jshPY32Transfer(uint8_t *buf, int count) {
  // assume display SPI is currently disabled as we only enable it for LCD flip
  //pm_device_action_run(spi1_dev, PM_DEVICE_ACTION_SUSPEND);
  const JshPinInfo *mosi = &pinInfo[LCD_SPI_MOSI];
  const JshPinInfo *miso = &pinInfo[LCD_SPI_MISO];
  const JshPinInfo *sck = &pinInfo[LCD_SPI_SCK];
  const struct device *mosiport = jshToZephyrPort(mosi->port);
  const struct device *misoport = jshToZephyrPort(miso->port);
  const struct device *sckport = jshToZephyrPort(sck->port);
  gpio_pin_set_raw(sckport, sck->pin, 0);
  jshPinSetValue(LCD_SPI_CS, 0);
  // FIXME can delay less if we know the IRQ line as asserted as the chip is awake
  for (volatile int i=0;i<1000;i++); // delay as PY32 must wake (we can't use k_usleep as we could be in an IRQ here)
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

// Send state to PY32. Return 4th buffer byte (version if cmd=PY32_INITIALISE, 0 otherwise)
int jshPY32Update(PY32Command cmd, int data) {
  if (inPY32Update) {
    jsiConsolePrintf("inPY32Update (%d %d)\n", cmd, data);
    return 0;
  }
  inPY32Update = true;
  uint8_t buf[4] = {cmd,0,0,0};
  int bufLen = 3;
  buf[0] = cmd;
  if (cmd == PY32_CMD_SET_OUTPUT) {
    PY32OutputState pyOutputState = sxValues>>PY32_OUT_SHIFT; // current output values
    buf[1] = pyOutputState&255;
    buf[2] = pyOutputState>>8;
  } else if (cmd==PY32_CMD_DISPLAY) {
    buf[1] = data;
  } else {
    if (cmd==PY32_CMD_INITIALISE) {
      // We use this to do a bigger SPI read that normal and get version info
      // and we expect the CMD to be set to PY32_CMD_NONE below
      bufLen = 4;
    }
    buf[0] = PY32_CMD_NONE;
  }
  jshPY32Transfer(buf, bufLen);
  uint8_t pyButtonState = buf[0];
  //jsiConsolePrintf("B %d %d %d %d\n", buf[0],buf[1],buf[2],buf[3]);
  #if PY32_OUT_SHIFT!=4
  #error PY32_OUT_SHIFT=4
  #endif
  uint16_t lastState = sxValues;
  sxValues = (sxValues&~15) | (pyButtonState&15);
  PY32InputState inputState = pyButtonState>>4;
  if (inputState & PY32_IN_TOUCH_IRQ)
    jswrap_banglejs_touchHandler(0,0); // touch handler IRQ
  if (inputState & PY32_REDRAW_REQUEST)
    graphicsSetModified(&graphicsInternal,0,0,LCD_WIDTH-1,LCD_HEIGHT-1); // PY32 wants a redraw
  uint16_t changed = lastState ^ sxValues;
  //jsiConsolePrintf("I %02x %02x %02x  %x %x upd(%d,%d)\n", buf[0],buf[1],buf[2],sxValues, changed, cmd, data);
  if (changed & 15) {
    for (int i=0;i<ESPR_EXTI_COUNT;i++)
      if (((changed&1) && eventFlagsToPin[i]==BTN1_PININDEX) ||
          ((changed&2) && eventFlagsToPin[i]==BTN2_PININDEX) ||
          ((changed&4) && eventFlagsToPin[i]==BTN3_PININDEX) ||
          ((changed&8) && eventFlagsToPin[i]==BTN4_PININDEX)) {
        jshPushIOWatchEvent(EV_EXTI0+i);
      }
  }
  inPY32Update = false;
  return buf[3];
}

void jshVirtualPinInitialise() {
  sxValues    = 0;
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  // Check we're not being called while LCD is updating - if we are, wait
  if (lcdMemLCD_isBusy()) {
    //jsiConsolePrintf("jshVirtualPinSetValue busy\n");
    int timeout = 10000000;
    while (lcdMemLCD_isBusy() && --timeout);
    if (timeout==0)
      return jsiConsolePrintf("jshVirtualPinSetValue(%d,%d) timeout\n",pin,state);
    jshDelayMicroseconds(100); // give PY32 time to finish
  }
  int p = pinInfo[pin].pin;
  unsigned short oldsxValues = sxValues;
  if (!IS_PIN_A_BUTTON(pin)) { // buttons read only
    if (state) sxValues |= 1<<p;
    else sxValues &= ~(1<<p);
    if (oldsxValues != sxValues) {
      // set status flags for PY32 only if changed
      jshPY32Update(PY32_CMD_SET_OUTPUT, 128+pin);
    }
  }
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
  if (inPY32Update) return;
  jshPY32Update(PY32_CMD_NONE, 2); // no set
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
  jshDelayMicroseconds(1000); // wait for pins to settle
  jshPY32Update(PY32_CMD_NONE, 0); // dummy write
  IOEventFlags channel = jshPinWatch(LCD_SPI_IRQ, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, jshVirtualPinIRQHandler);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_banglejs3_init"
}*/
void jswrap_banglejs3_init() {
 /* int version = jshPY32Update(PY32_CMD_INITIALISE, 0); // update current status (and clear IRQ line)
  jsiConsolePrintf("LCD firmware 0x%02x\n", version);
  if (version != py32_firmware_version) {
    jsiConsolePrintf("LCD firmware needs update to 0x%02x\n", py32_firmware_version);
  }*/
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_banglejs3_idle"
}*/
bool jswrap_banglejs3_idle() {
  /* if for some reason the IRQ is low while in idle (it should normally
  be handled by IRQ, handle it here */
  if (!jshPinGetValue(LCD_SPI_IRQ))
    lcdMemLCD_callWhenIdle(jshVirtualPinIRQWorker);

  return false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "lcdUpdateFirmware",
    "generate" : "jswrap_banglejs_lcdUpdateFirmware",
    "ifdef" : "BANGLEJS3"
}
Reflash the firmware on the LCD controller
*/
void jswrap_banglejs_lcdUpdateFirmware() {
  if (py32_firmware_len&3) {
    jsWarn("Firmware not multiple of 4 bytes");
    return;
  }
  // SWD test
  jshPinSetValue(LCD_SPI_CS, 0); // CS will wake device up
  jshDelayMicroseconds(100); // wait wakeup
  swdInit(); // DAP ID 0x0bc11477
  swdHalt();
  jshPinSetValue(LCD_SPI_CS, 1); // disable CS
  swdPY32FlashWriteInit();
  jsiConsolePrintf("Erase\n");
  swdPY32FlashErase();
  jsiConsolePrintf("Write\n");
  swdPY32FlashWrite(0x08000000, (uint32_t*)&py32_firmware, py32_firmware_len);
  jsiConsolePrintf("Read\n");
  swdReadMem(0x08000000);

  swdSoftReset();
  swdKill();
  jsiConsolePrintf("Wait for reboot\n");
  jshDelayMicroseconds(1000000); // wait 1s for reboot
  jshPY32Update(PY32_CMD_NONE, 0); // dummy write
  // Force a redraw next time around idle loop
  graphicsSetModified(&graphicsInternal,0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
  jsiConsolePrintf("Done\n");

  /* pyocd
0000301 I DP IDR = 0x0bc11477 (v1 MINDP rev0) [dap]
0000323 I AHB-AP#0 IDR = 0x04770031 (AHB-AP var3 rev0) [discovery]
0000337 I AHB-AP#0 Class 0x1 ROM table #0 @ 0xe00ff000 (designer=43b:Arm part=4c0) [rom_table]
0000348 I [0]<e000e000:SCS v6-M class=14 designer=43b:Arm part=008> [rom_table]
0000354 I [1]<e0001000:DWT v6-M class=14 designer=43b:Arm part=00a> [rom_table]
0000360 I [2]<e0002000:BPU v6-M class=14 designer=43b:Arm part=00b> [rom_table]
0000365 I debugvar 'DbgMCU_APB_Fz1' = 0x0 (0) [pack_target]
0000365 I debugvar 'DbgMCU_APB_Fz2' = 0x0 (0) [pack_target]
0000365 I debugvar 'DbgMCU_CR' = 0x2 (2) [pack_target]
0000389 I CPU core #0: Cortex-M0+ r0p1, v6.0-M architecture [cortex_m]
  */
}