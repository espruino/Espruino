
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jswrap_json.h"

#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrapper.h"
#include "lcd_st7789_8bit.h"

void *STACK_BASE; ///< used for jsuGetFreeStack on Linux
extern int timeToSleep;


void jsInit() {
  int i;
  STACK_BASE = (void*)&i; // used for jsuGetFreeStack on Linux

  jshInit();
  jsvInit(0);
  jsiInit(true);
}

int jsIdle() {
   timeToSleep = -1;
   jsiLoop();
   return timeToSleep;
}

bool jsGfxChanged() {
  bool b = EMSCRIPTEN_GFX_CHANGED;
  EMSCRIPTEN_GFX_CHANGED = false;
  return b;
}
char *jsGfxGetPtr(int line) {
  if (EMSCRIPTEN_GFX_WIDESCREEN) {
    if (line<40 || line>=200)
      return 0;
  }
  line += EMSCRIPTEN_GFX_YSTART;
  if (EMSCRIPTEN_GFX_WIDESCREEN)
    line -= 40;
  if (line<0) line+=320;
  if (line>=320) line-=320;
  return &EMSCRIPTEN_GFX_BUFFER[line*240*2];
}
void jsSendPinWatchEvent(int pin) {
  extern IOEventFlags jshGetEventFlagsForPin(Pin pin);
  IOEventFlags ev = jshGetEventFlagsForPin(pin);
  if (ev!=EV_NONE)
    jshPushIOWatchEvent(ev);
}

void jsKill() {
  jsiKill();
  jsvKill();
  jshKill();
}
