#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jsvar.h"
#include "jswrapper.h"

int main(void) {

  // basic hardware init
  jshInit();
  jswHWInit();
  jsvInit(JSVAR_CACHE_SIZE);

  // start Espruino
  jsiInit(true);
  rp2040EarlyLog("RP2040 boot: jsiInit ok\r\n");

  // Start RP2040 USB after interpreter init so regular idle/task servicing is
  // immediately available during enumeration.
  rp2040UsbInitNow();

  // main interpreter loop
  while (1) {
    jsiLoop();
  }

  return 0;
}
