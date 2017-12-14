/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Task, queue and timer specific exposed components.
 * ----------------------------------------------------------------------------
 */
 /* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */
#include <stdio.h>
#include "jswrap_rtos.h"
#include "jsparse.h"
#include "rtosutil.h"

/*JSON{
  "type"    : "class",
  "class"   : "Queue"
}
A class to support some simple Queue handling for RTOS queues
*/
/*JSON{
  "type"     : "constructor",
  "class"    : "Queue",
  "name"     : "Queue",
  "generate" : "jswrap_Queue_constructor",
  "params"   : [ ["queueName", "JsVar", "Name of the queue"] ],
  "return"   : ["JsVar","A Queue object"]
}
Creates a Queue Object
*/
JsVar *jswrap_Queue_constructor(JsVar *queueName){
  int idx; char name[20];
  JsVar *queue = jspNewObject(0, "Queue");
  if (!queue) return 0;
  name[jsvGetString(queueName, name, sizeof(name))] = '\0';
  idx = queue_indexByName(name);
  jsvObjectSetChildAndUnLock(queue, "index", jsvNewFromInteger(idx));
  return queue;
}
/*JSON{
 "type"     : "method",
 "class"    : "Queue",
 "name"     : "read",
 "generate" : "jswrap_Queue_read"
}
reads one character from queue, if available
*/
void jswrap_Queue_read(JsVar *parent) {
  char data;
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  queue_read(jsvGetInteger(idx));
  jsvUnLock(idx);
  return;
}
/*JSON{
 "type"     : "method",
 "class"    : "Queue",
 "name"     : "writeChar",
 "params"   : [ ["char", "JsVar", "char to be send"] ],
 "generate" : "jswrap_Queue_writeChar"
}
Writes one character to queue
*/
void jswrap_Queue_writeChar(JsVar *parent,char c){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  queue_writeChar(idx,c);
}
/*JSON{
 "type"     : "method",
 "class"    : "Queue",
 "name"     : "log",
 "generate" : "jswrap_Queue_log"
}
logs list of queues
*/
void jswrap_Queue_log(JsVar *parent) {
  queue_list();
  return;
}

/*JSON{
  "type"    : "class",
  "class"   : "Task"
}
A class to support some simple Task handling for RTOS tasks
*/
/*JSON{
  "type"     : "constructor",
  "class"    : "Task",
  "name"     : "Task",
  "generate" : "jswrap_Task_constructor",
  "params"   : [ ["taskName", "JsVar", "Name of the task"] ],
  "return"   : ["JsVar","A Task object"]
}
Creates a Task Object
*/
JsVar *jswrap_Task_constructor(JsVar *taskName){
  int idx; char name[20];
  JsVar *task = jspNewObject(0, "Task");
  if (!task) return 0;
  name[jsvGetString(taskName, name, sizeof(name))] = '\0';
  idx = task_indexByName(name);
  jsvObjectSetChildAndUnLock(task, "index", jsvNewFromInteger(idx));
  return task;
}
/*JSON{
 "type"     : "method",
 "class"    : "Task",
 "name"     : "suspend",
 "generate" : "jswrap_Task_suspend"
}
Suspend task, be careful not to suspend Espruino task itself
*/
void jswrap_Task_suspend(JsVar *parent){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  task_Suspend(jsvGetInteger(idx));
  return;
}
/*JSON{
 "type"     : "method",
 "class"    : "Task",
 "name"     : "resume",
 "generate" : "jswrap_Task_resume"
}
Resumes a suspended task
*/
void jswrap_Task_resume(JsVar *parent){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  task_Resume(jsvGetInteger(idx));
  return;
}
/*JSON{
 "type"     : "method",
 "class"    : "Task",
 "name"     : "getCurrent",
 "generate" : "jswrap_Task_getCurrent",
 "return"   : ["JsVar","Name of current task"]
}
returns name of actual task
*/
JsVar *jswrap_Task_getCurrent(JsVar *parent){
  return jsvNewFromString(task_getCurrentName());
}
/*JSON{
 "type"     : "method",
 "class"    : "Task",
 "name"     : "notify",
 "generate" : "jswrap_Task_notify"
}
Sends a binary notify to task
*/
void jswrap_Task_notify(JsVar *parent){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  task_notify(jsvGetInteger(idx));
}
/*JSON{
 "type"     : "method",
 "class"    : "Task",
 "name"     : "log",
 "generate" : "jswrap_Task_log"
}
logs list of tasks
*/
void jswrap_Task_log(JsVar *parent) {
  task_list();
  return;
}

/*JSON{
  "type"	: "class",
  "class"	: "Timer"
}
A class to handle Timer on base of ESP32 Timer
*/
/*JSON{
  "type"     : "constructor",
  "class"    : "Timer",
  "name"     : "Timer",
  "generate" : "jswrap_Timer_constructor",
  "params"   : [ ["timerName", "JsVar", "Timer Name"],
                 ["group", "int", "Timer group"],
				 ["index", "int", "Timer index"],
                 ["isrIndex", "int", "isr (0 = Espruino, 1 = test)"]  ],
  "return"   : ["JsVar","A Timer Object"]
}
Creates a Timer Object
*/
JsVar *jswrap_Timer_constructor(JsVar *timerName,int group, int index, int isrIndex){
  int idx; char name[20];
  JsVar *timer = jspNewObject(0, "Timer");
  if(!timer) return 0;
  name[jsvGetString(timerName, name, sizeof(name))] = '\0';
  idx = timer_Init(name,group,index,isrIndex);
  jsvObjectSetChildAndUnLock(timer, "index", jsvNewFromInteger(idx));
  return timer;
}
/*JSON{
  "type"     : "method",
  "class"    : "Timer",
  "name"     : "start",
  "params"   : [["duration","int","duration of timmer in micro secs"]],
  "generate" : "jswrap_Timer_start"
}
Starts a timer
*/
void jswrap_Timer_start(JsVar *parent, int duration){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  timer_Start(jsvGetInteger(idx),duration);
}
/*JSON{
  "type"     : "method",
  "class"    : "Timer",
  "name"     : "reschedule",
  "params"   : [["duration","int","duration of timmer in micro secs"]],
  "generate" : "jswrap_Timer_reschedule"
}
Reschedules a timer, needs to be started at least once
*/
void jswrap_Timer_reschedule(JsVar *parent, int duration){
  JsVar *idx = jsvObjectGetChild(parent,"index",1);
  timer_Reschedule(jsvGetInteger(idx),duration);
}
/*JSON{
 "type"     : "method",
 "class"    : "Timer",
 "name"     : "log",
 "generate" : "jswrap_Timer_log"
}
logs list of timers
*/
void jswrap_Timer_log(JsVar *parent) {
  timer_List();
  return;
}
