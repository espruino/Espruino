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
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */
 
#include "rom/uart.h"
#include "rtosutil.h"

#include <stdio.h>
#include <string.h>

// implementation of simple queue oriented commands. see header file for more info.
void queues_init(){
  int i;
  for(i = 0; i < queueMax; i++){ RTOSqueues[i].name = NULL; RTOSqueues[i].handle = NULL;}
}
int queue_indexByName(char *queueName){
  int i;
  for(i = 0; i < queueMax; i++){
	if(RTOSqueues[i].handle != 0){
	  if(strcmp(queueName,RTOSqueues[i].name) == 0){
        return i;
	  }
	}
  }
  return -1;
}
QueueHandle_t queue_handleByName(char *queueName){
  int idx;
  idx = queue_indexByName(queueName);
  return RTOSqueues[idx].handle;
}
int queue_init(char *queueName,int length,int sizeOfEntry){
  int i;
  for(i = 0; i < queueMax; i++){
	if(NULL == RTOSqueues[i].handle){
	  RTOSqueues[i].name = queueName;
	  RTOSqueues[i].handle = xQueueCreate(length,sizeOfEntry);
	  return i;
	}
  }
  return -1;
}
void queue_list(){
  int i;
  for(i = 0; i < queueMax; i++){
	printf("queue %s : %d\n",RTOSqueues[i].name,RTOSqueues[i].handle);
  }
}
char *queue_read(int idx){
  char data;
  if(xQueueReceive(RTOSqueues[idx].handle,&data,0)){
    return data;
  } 
  return NULL;
}
void queue_writeChar(int idx,char c){
  if(!xQueueSend(RTOSqueues[idx].handle,&c,1)){
    printf("SerialTaskOverflow\n");
  }
}

// implementation of simple task oriented commands. see header file for more info.
void tasks_init(){
  int i;
  for(i = 0; i < taskMax; i++){ RTOStasks[i].name = NULL; RTOStasks[i].handle = NULL;}
}
int task_indexByName(char *taskName){
  int i;
  for(i = 0; i < taskMax; i++){
	if(RTOStasks[i].handle != 0){
	  if(strcmp(taskName,RTOStasks[i].name) == 0){
        return i;
	  }
	}
  }
  return -1;
}
TaskHandle_t task_handleByName(char *taskName){
  int idx;
  idx = task_indexByName(taskName);
  return RTOStasks[idx].handle;
}
int *task_getCurrentIndex(){
  int i;TaskHandle_t handle;
  handle = xTaskGetCurrentTaskHandle();
  for(i = 0; i < taskMax; i++){
	if(RTOStasks[i].handle == handle){
	  return i;
	}
  }
  return NULL;
}
char *task_getCurrentName(){
  int i;TaskHandle_t handle;
  handle = xTaskGetCurrentTaskHandle();
  for(i = 0; i < taskMax; i++){
    if(RTOStasks[i].handle == handle){
	  return RTOStasks[i].name;
    }
  }
  return NULL;
}
int task_init(TaskFunction_t taskCode, char *taskName,unsigned short stackDepth,UBaseType_t priority,BaseType_t coreId){
  int i;
  for(i = 0; i < taskMax; i++){
	if(NULL == RTOStasks[i].handle){
	  RTOStasks[i].name = taskName;
	  xTaskCreatePinnedToCore(taskCode,taskName,stackDepth,NULL,priority,&RTOStasks[i].handle,coreId);
	  return i;
	}
  }
  return -1;
}
void task_list(){
  int i;
  for(i = 0; i < taskMax;i++){
	printf("task %s : %d\n",RTOStasks[i].name,RTOStasks[i].handle);
  }
}
void task_Suspend(int idx){
  vTaskSuspend(RTOStasks[idx].handle);
}
void task_Resume(int idx){
  vTaskResume(RTOStasks[idx].handle);
}
