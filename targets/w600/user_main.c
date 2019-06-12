#include "wm_include.h"
#include "wm_gpio.h"

#include "jshardware.h"
#include "jsvar.h"
#include "jsinteractive.h"

void UserMain(void){

  jshInit();
  jsvInit(0);
  jsiInit(true);

  while (1) {
    jsiLoop();
  }

}