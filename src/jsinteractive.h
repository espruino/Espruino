/*
 * jsinteractive.h
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef JSINTERACTIVE_H_
#define JSINTERACTIVE_H_

#include "jsparse.h"
#include "jshardware.h"

/// autoLoad = do we load the current state if it exists?
void jsiInit(bool autoLoad);
void jsiKill();

void jsiLoop();
/// Tries to get rid of some memory (by clearing command history). Returns true if it got rid of something, false if it didn't.
bool jsiFreeMoreMemory();

bool jsiHasTimers(); // are there timers still left to run?
JsParse *jsiGetParser();

/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device);
/// Transmit a byte
void jsiConsolePrintChar(char data);
void jsiConsolePrintCharEscaped(char data); ///< transmit a byte, but escape it
/// Transmit a string
void jsiConsolePrint(const char *str);
void jsiConsolePrintEscaped(const char *str); ///< transmit a string, but escape it
/// Transmit an integer
void jsiConsolePrintInt(int d);
/// Transmit a position in the lexer (for reporting errors)
void jsiConsolePrintPosition(struct JsLex *lex, int tokenPos);

/// Shows a busy indicator, if one is set up
void jsiSetBusy(bool isBusy);
/// Shows a sleep indicator, if one is set up
void jsiSetSleep(bool isSleep);


#endif /* JSINTERACTIVE_H_ */
