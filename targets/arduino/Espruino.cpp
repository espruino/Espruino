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

#include "Espruino.h"
#include "Arduino.h"
#include "HardwareSerial.h"


// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

Espruino::Espruino(void) {
}

void Espruino::init(void) {
  Serial.println("jshInit...");
  jshInit();
  bool buttonState = false;
  buttonState = jshPinInput(BTN_PININDEX) == BTN_ONSTATE;
  Serial.println("jsiInit...");
  jsiInit(!buttonState); // pressing USER button skips autoload
  Serial.println("Init done.");
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

void Espruino::loop(void) {
//  jsiLoop();
}

