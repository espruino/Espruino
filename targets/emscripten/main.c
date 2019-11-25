
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jswrap_json.h"

#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrapper.h"
#include "lcd_emscripten.h"

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
char *jsGfxGetPtr() {
  return EMSCRIPTEN_GFX_BUFFER;
}

/*
  jsiKill();
  jsvGarbageCollect();
  jsvShowAllocated();
  jsvKill();
  jshKill();
*/
