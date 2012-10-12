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

bool jsiHasTimers(); // are there timers still left to run?
JsParse *jsiGetParser();

/// Change the console to a new location
void jsiSetConsoleDevice(IOEventFlags device);
/// Transmit a byte
void jsiConsolePrintChar(char data);
/// Transmit a string
void jsiConsolePrint(const char *str);
/// Transmit an integer
void jsiConsolePrintInt(int d);
/// Transmit a position in the lexer (for reporting errors)
void jsiConsolePrintPosition(struct JsLex *lex, int tokenPos);

// Sets a busy indicator, if one is set
void jsiSetBusy(bool isBusy);

#endif /* JSINTERACTIVE_H_ */
