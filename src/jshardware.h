/*
 * jshardware.h
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef JSHARDWARE_H_
#define JSHARDWARE_H_

#include "jsutils.h"
#include "jsvar.h"

void jshInit();
void jshKill();

/// Receive a byte, or return -1 if there isn't one
int jshRX();
/// Transmit a byte
void jshTX(char data);
/// Transmit a string
void jshTXStr(const char *str);

typedef long long JsSysTime;
/// Get the system time (in ticks)
JsSysTime jshGetSystemTime();
/// Convert a time in Milliseconds to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms);
/// Convert ticks to a time in Milliseconds
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time);


// IO Events - these happen when a pin changes
typedef struct IOEvent {
  JsSysTime time; // time event occurred
  unsigned char channel; // 'channel'
} IOEvent;

void jshPushIOEvent(JsSysTime time, unsigned char channel);
bool jshPopIOEvent(IOEvent *result); ///< returns true on success


/// Given a var, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromVar(JsVar *pinv);
bool jshPinInput(int pin);
JsVarFloat jshPinAnalog(int pin);
void jshPinOutput(int pin, bool value);
void jshPinPulse(int pin, bool value, JsVarFloat time);
void jshPinWatch(int pin, bool shouldWatch);
bool jshIsEventForPin(IOEvent *event, int pin);

/// Save contents of JsVars into Flash
void jshSaveToFlash();
/// Load contents of JsVars from Flash
void jshLoadFromFlash();
/// Returns true if flash contains something useful
bool jshFlashContainsCode();

#endif /* JSHARDWARE_H_ */
