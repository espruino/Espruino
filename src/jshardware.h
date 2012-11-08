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
#include "jsdevices.h"

void jshInit();
void jshKill();
void jshIdle(); // stuff to do on idle

bool jshIsUSBSERIALConnected(); // is the serial device connected?

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime();
/// Convert a time in Milliseconds to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms);
/// Convert ticks to a time in Milliseconds
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time);



/// Given a string, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromString(const char *s);
/// Given a var, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromVar(JsVar *pinv);
bool jshPinInput(int pin);
JsVarFloat jshPinAnalog(int pin);
void jshPinOutput(int pin, bool value);
void jshPinAnalogOutput(int pin, JsVarFloat value);
void jshPinPulse(int pin, bool value, JsVarFloat time);
void jshPinWatch(int pin, bool shouldWatch);
bool jshIsEventForPin(IOEvent *event, int pin);
void jshUSARTSetup(IOEventFlags device, int baudRate);
/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device);

/// Save contents of JsVars into Flash
void jshSaveToFlash();
/// Load contents of JsVars from Flash
void jshLoadFromFlash();
/// Returns true if flash contains something useful
bool jshFlashContainsCode();

/// Enter simple sleep mode (can be woken up by interrupts)
void jshSleep();

// ---------------------------------------------- LOW LEVEL
#ifdef ARM
// ----------------------------------------------------------------------------
//                                                                      SYSTICK
// On SYSTick interrupt, call this
void jshDoSysTick();
#ifdef USB
// Kick the USB SysTick watchdog - we need this to see if we have disconnected or not
void jshKickUSBWatchdog();
#endif

#endif // ARM

#endif /* JSHARDWARE_H_ */
