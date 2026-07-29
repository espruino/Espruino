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

  bool buttonState = false;
#ifdef BTN1_PININDEX
  buttonState = jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE;
#endif

  // start Espruino
  jsiInit(!buttonState); // pressing button on boot skips autoload from flash
  rp2EarlyLog("RP2 boot: jsiInit ok\r\n");

  // Start RP2040 USB after interpreter init so regular idle/task servicing is
  // immediately available during enumeration.
  rp2UsbInitNow();

  // main interpreter loop
  while (1) {
    jsiLoop();
  }

  return 0;
}
