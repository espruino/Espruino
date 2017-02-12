/*
 * This file is designed to support FREERTOS functions in Espruino,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Juergen Marsch 
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Task, queue and timer specific exposed components.
 * ----------------------------------------------------------------------------
 */
 
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


#define queueMax 10 //for queus we use an array of name/handle info. Core RTOS is not very helpful for this
struct RTOSqueue{ char *name;QueueHandle_t handle;};
struct RTOSqueue RTOSqueues[queueMax];

#define taskMax 10 //for tasks we use an array of name/handle info similiar to queue array
struct RTOStask{ char *name;TaskHandle_t handle;int rx;};
struct RTOStask RTOStasks[taskMax];

#define timerMax 2 //for Timer we use an array of timer relevant info
struct ESP32Timer{ char *name; int group; int index; uint64_t duration; int taskToNotifyIdx; };
struct ESP32Timer ESP32Timers[timerMax];

void queues_init(); //initializes array of queues
int queue_indexByName(char *queueName); //returns index of queue in queue array by name
QueueHandle_t queue_handleByName(char *queueName); //returns handle of queue by name
int queue_init(char *queueName,int length,int sizeOfEntry); //initializes a queue with name,length and size of each entry
char *queue_read(int idx); //reads one character from queue
void queue_writeChar(int idx,char c); //writes one char to queue
void queue_list(); //logs queue list

void tasks_init(); //initializes array of tasks
int task_indexByName(char *taskName); //returns index of task in task array by name
TaskHandle_t task_handleByName(char *taskName); //returns handle of task by name
int *task_getCurrentIndex(); //returns index of actual task
char *task_getCurrentName(); //returns name of actual task
int task_init(TaskFunction_t taskCode, char *taskName,unsigned short stackDepth,UBaseType_t priority,BaseType_t coreId);
//initializes a task, using nonstandard rtos api call xTaskCreatePinnedToCore
void task_list(); //lists all entrys of task array usinf printf
void task_Suspend(int idx); //suspends task given by index in task array
void task_Resume(int idx); //resumes task given by index in task array
void task_notify(int idx); //notify task given by index with binary
void taskWaitNotify(); //waits for notify

void timers_Init();
int timer_indexByName(char *timerName);
int timer_Init(char *timerName,int group,int index,int isr_idx);
void timer_Start(int idx,uint64_t duration);
void timer_Reschedule(int idx,uint64_t duration);
void timer_List();

void console_readToQueue(); //reads char from uart and writes to RTOS queue, not using interrupts (yet)