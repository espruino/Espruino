/*
  Test.h - Test library for Wiring - implementation
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// include this library's description file

#define CPLUSPLUS
extern "C" {
#include "jshardware.h"
#include "jsinteractive.h"
}

#include "Arduino.h"
#include "HardwareSerial.h"


void setup(void) {
  jshInit();
  Serial.println("jshInit...");
  bool buttonState = false;
  buttonState = false;
#ifdef BTN_PININDEX
  jshPinInput(BTN_PININDEX) == BTN_ONSTATE;
#endif
  Serial.println("jsiInit...");
  jsiInit(!buttonState); // pressing USER button skips autoload
  Serial.println("Init done.");
}

void loop(void) {
  jsiLoop();
}

