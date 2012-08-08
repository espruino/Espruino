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

/// Given a var, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromVar(JsVar *pinv);
bool jshPinInput(int pin);
void jshPinOutput(int pin, bool value);
void jshPinPulse(int pin, bool value, JsVarFloat time);

#endif /* JSHARDWARE_H_ */
