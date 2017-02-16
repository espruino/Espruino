/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Task, queue and timer specific exposed components.
 * ----------------------------------------------------------------------------
 */
 
#ifndef TARGETS_JSWRAP_RTOS_H_
#define TARGETS_JSWRAP_RTOS_H_

#include "jsvar.h"

//===== Queue Library
JsVar *jswrap_Queue_constructor(JsVar *queueName);
void jswrap_Queue_read(JsVar *parent);
void jswrap_Queue_writeChar(JsVar *parent,char c);
void jswrap_Queue_log(JsVar *parent);

// ==== Task handling
JsVar *jswrap_Task_constructor(JsVar *taskName);
void jswrap_Task_suspend(JsVar *parent);
void jswrap_Task_resume(JsVar *parent);
JsVar *jswrap_Task_getCurrent(JsVar *parent);
void jswrap_Task_notify(JsVar *taskName);
void jswrap_Task_log(JsVar *parent);

// ==== Timer handling
JsVar *jswrap_Timer_constructor(JsVar *timerName,int group, int index, int isrIndex);
void jswrap_Timer_start(JsVar *parent, int duration);
void jswrap_Timer_reschedule(JsVar *parent, int duration);
void jswrap_Timer_log(JsVar *parent);

#endif /* TARGETS_JSWRAP_RTOS_H_ */
