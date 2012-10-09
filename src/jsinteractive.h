/*
 * jsinteractive.h
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef JSINTERACTIVE_H_
#define JSINTERACTIVE_H_

#include "jsparse.h"

/// autoLoad = do we load the current state if it exists?
void jsiInit(bool autoLoad);
void jsiKill();

void jsiLoop();

bool jsiHasTimers(); // are there timers still left to run?
JsParse *jsiGetParser();

#endif /* JSINTERACTIVE_H_ */
