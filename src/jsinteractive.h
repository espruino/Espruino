/*
 * jsinteractive.h
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef JSINTERACTIVE_H_
#define JSINTERACTIVE_H_

#include "jsparse.h"

void jsiInit();
void jsiKill();

void jsiLoop();

JsParse *jsiGetParser();

#endif /* JSINTERACTIVE_H_ */
