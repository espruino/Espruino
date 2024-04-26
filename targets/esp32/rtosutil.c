#/*
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

#include <stdio.h>
#include <string.h>

#include "jsinteractive.h"
#include "jstimer.h"

#if ESP_IDF_VERSION_5
#include "soc/uart_reg.h"
#include "esp_private/esp_clk.h"
#endif 
#include "rom/uart.h"
#include "rtosutil.h"

#include "soc/timer_group_struct.h"
#include "driver/timer.h"



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
  if(RTOSqueues[i].name == NULL) return;
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
  if(RTOStasks[i].name == NULL) return;
  printf("task %s : %d\n",RTOStasks[i].name,RTOStasks[i].handle);
  }
}
void task_Suspend(int idx){
  vTaskSuspend(RTOStasks[idx].handle);
}
void task_Resume(int idx){
  vTaskResume(RTOStasks[idx].handle);
}
void task_notify(int idx){
  xTaskNotifyGive(RTOStasks[idx].handle);
}
void taskWaitNotify(){
  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
}

#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP    TIMER_GROUP_0     /*!< Test on timer group 0 */
#define TIMER_DIVIDER  80               /*!< Hardware timer clock divider */
#if ESP_IDF_VERSION_4 || ESP_IDF_VERSION_5
#define TIMER_FINE_ADJ   (1.4*(esp_clk_apb_freq() / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
#define TIMER_TX_UPDATE(TIMER_N) TIMERG0.hw_timer[TIMER_N].update.tx_update = 1
#define TIMER_ALARM_EN(TIMER_N) TIMERG0.hw_timer[TIMER_N].config.tx_alarm_en = 1;
#define TIMER_0_INT_CLR() TIMERG0.int_clr_timers.t0_int_clr = 1
#define TIMER_1_INT_CLR() TIMERG0.int_clr_timers.t1_int_clr = 1
#else
#define TIMER_FINE_ADJ   (1.4*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
#define TIMER_TX_UPDATE(TIMER_N) TIMERG0.hw_timer[TIMER_N].update = 1
#define TIMER_ALARM_EN(TIMER_N) TIMERG0.hw_timer[TIMER_N].config.alarm_en = 1
#define TIMER_0_INT_CLR() TIMERG0.int_clr_timers.t0 = 1;
#define TIMER_1_INT_CLR() TIMERG0.int_clr_timers.t1 = 1;
#endif

BaseType_t xHigherPriorityTaskWoken = pdFALSE;

void IRAM_ATTR espruino_isr(void *para){
  int idx = (int) para;
  if(idx == 0){
    TIMER_TX_UPDATE(TIMER_0);
    TIMER_0_INT_CLR();
  }
  else{
    TIMER_TX_UPDATE(TIMER_1);
    TIMER_1_INT_CLR();
  }
  jstUtilTimerInterruptHandler();
}
void IRAM_ATTR test_isr(void *para){
  int idx = (int) para;
  printf("x\n");
  if(idx == 0){
    TIMER_TX_UPDATE(TIMER_0);
    TIMER_0_INT_CLR();
  }
  else{
    TIMER_TX_UPDATE(TIMER_1);
    TIMER_1_INT_CLR();
  }
}


void timers_Init(){
  int i;
  for(i = 0; i < timerMax; i++){
  ESP32Timers[i].name = NULL;
  }
}
int timer_indexByName(char *timerName){
  int i;
  for(i = 0; i < timerMax; i++){
  if(ESP32Timers[i].name == NULL) return -1;
  if(strcmp(timerName,ESP32Timers[i].name) == 0){
    return i;
  }
  }
  return -1;
}
int timer_Init(char *timerName,int group,int index,int isr_idx){
  int i;
  for(i = 0; i < timerMax; i++){
  if(ESP32Timers[i].name == NULL){
    ESP32Timers[i].name = timerName;
    ESP32Timers[i].group = group;
    ESP32Timers[i].index = index;
      timer_config_t config;
      config.alarm_en = 1;
      config.auto_reload = 1;
      config.counter_dir = TIMER_COUNT_UP;
      config.divider = TIMER_DIVIDER;
      config.intr_type = TIMER_INTR_SEL;
      config.counter_en = TIMER_PAUSE;
      timer_init(group, index, &config);/*Configure timer*/
      timer_pause(group, index);/*Stop timer counter*/
      timer_set_counter_value(group, index, 0x00000000ULL);/*Load counter value */
      timer_enable_intr(group, index);
      if(isr_idx == 0){
      ESP32Timers[i].taskToNotifyIdx = task_indexByName("TimerTask");
        timer_isr_register(group, index, espruino_isr, (void*) i, ESP_INTR_FLAG_IRAM, NULL);
      }
      else{
      timer_isr_register(group, index, test_isr, (void*) i, ESP_INTR_FLAG_IRAM, NULL);  
      }
      return i;
  }
  }
  return -1;
}
void timer_Start(int idx,uint64_t duration){
  timer_enable_intr(ESP32Timers[idx].group, ESP32Timers[idx].index);
  timer_set_alarm_value(ESP32Timers[idx].group, ESP32Timers[idx].index, duration - TIMER_FINE_ADJ);
#if CONFIG_IDF_TARGET_ESP32
  TIMER_ALARM_EN(idx);
#elif CONFIG_IDF_TARGET_ESP32S3
  TIMERG0.hw_timer[idx].config.tn_alarm_en=1;
#else
  #error Not an ESP32 or ESP32-S3
#endif
  timer_start(ESP32Timers[idx].group, ESP32Timers[idx].index);
}
void timer_Reschedule(int idx,uint64_t duration){
  timer_set_alarm_value(ESP32Timers[idx].group, ESP32Timers[idx].index, duration - TIMER_FINE_ADJ);
#if CONFIG_IDF_TARGET_ESP32
    TIMER_ALARM_EN(idx);
#elif CONFIG_IDF_TARGET_ESP32S3
  TIMERG0.hw_timer[idx].config.tn_alarm_en=1;
#else
  #error Not an ESP32 or ESP32-S3
#endif
}
	
void timer_List(){
#if ESP_IDF_VERSION_5  
  printf("timer_List not implemented\n");
#else
  int i;
  for(i = 0; i < timerMax; i++){
  if(ESP32Timers[i].name == NULL){printf("timer %d free\n",i);}
  else {printf("timer %s : %d.%d\n",ESP32Timers[i].name,ESP32Timers[i].group,ESP32Timers[i].index);}
  }
#endif
}
