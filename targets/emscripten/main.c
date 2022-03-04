
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jswrap_json.h"

#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrapper.h"
#include "jswrap_bangle.h"
#include "jswrapper.h"
#ifdef LCD_CONTROLLER_LPM013M126
#include "lcd_memlcd.h"
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif

void *STACK_BASE; ///< used for jsuGetFreeStack on Linux
extern int timeToSleep;


void jsInit() {
  int i;
  STACK_BASE = (void*)&i; // used for jsuGetFreeStack on Linux

  jshInit();
  jswHWInit();
  jsvInit(0);
  jsiInit(true);
}

int jsIdle() {
   timeToSleep = -1;
   jsiLoop();
   return timeToSleep;
}

void jsSendPinWatchEvent(int pin) {
  extern IOEventFlags jshGetEventFlagsForPin(Pin pin);
  IOEventFlags ev = jshGetEventFlagsForPin(pin);
  if (ev!=EV_NONE)
    jshPushIOWatchEvent(ev);
}

void jsSendTouchEvent(int tx, int ty, int pts, int gesture) {
#ifdef TOUCH_DEVICE
  touchHandlerInternal(tx, ty, pts, gesture);
#endif
}

void jsKill() {
  jsiKill();
  jsvKill();
  jshKill();
}
