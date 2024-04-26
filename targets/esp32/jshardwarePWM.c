/*
 * This file is designed to support PWM functions in Espruino,
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
#include "jsutils.h"
 
#include "jshardwarePWM.h"
#include "driver/ledc.h"

#include <stdio.h>

#define PWMFreqDefault 5000
#define PWMPinEmpty 111
#define PWMTimerDefault 3

int getTimerIndex(Pin pin,int freq){
  int i;
  for(i = 0; i < PWMFreqMax; i++){
  if(PWMFreqChannels[i].pin == pin) return i;  
  }
  return -1;  
}
int getFreeTimer(Pin pin,int freq){
  int i;
  i = getTimerIndex(pin,freq);
  if(i >= 0) return i;
  for(i = 0; i < PWMFreqMax; i++){
  if(PWMFreqChannels[i].pin == PWMPinEmpty) return i;
  }
  return -1;
}

int getChannelIndex(Pin pin){
  int i;
  for(i = 0; i < PWMMax; i++){
    if(PWMChannels[i].pin == pin) return i;
  }
  return -1;
}
int getFreeChannel(pin){
  int i;
  i = getChannelIndex(pin);
  if(i >= 0) return i;
  for(i = 0; i < PWMMax; i++){
  if(PWMChannels[i].pin == PWMPinEmpty) return i;
  }
  return -1;
}

void timerConfig(int freq,int timer){
  ledc_timer_config_t PWM_timer = {        
#if ESP_IDF_VERSION_5
    .duty_resolution = LEDC_TIMER_10_BIT,
#else
    .bit_num = LEDC_TIMER_10_BIT,//set timer counter bit number        
#endif    
#if CONFIG_IDF_TARGET_ESP32
    .freq_hz = freq,//set frequency of pwm        
    .speed_mode = LEDC_HIGH_SPEED_MODE,//timer mode,        
    .timer_num = timer//timer index
  };
#elif CONFIG_IDF_TARGET_ESP32S3
    .freq_hz = freq,//set frequency of pwm        
    .speed_mode = LEDC_LOW_SPEED_MODE,//timer mode,        
    .timer_num = timer//timer index
  };
#else
	#error Not an ESP32 or ESP32-S3
#endif
  ledc_timer_config(&PWM_timer);
}

void channelConfig(int timer, int channel, int value, Pin pin){
#if CONFIG_IDF_TARGET_ESP32
  ledc_channel_config_t PWM_channel = {        
    .channel = channel,//set LEDC channel 0        
    .duty = value,//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)        
    .gpio_num = pin,//GPIO number        
    .intr_type = LEDC_INTR_DISABLE,//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.        
    .speed_mode = LEDC_HIGH_SPEED_MODE,//set LEDC mode, from ledc_mode_t
    .timer_sel = timer
  };
#elif CONFIG_IDF_TARGET_ESP32S3
  ledc_channel_config_t PWM_channel = {        
    .channel = channel,//set LEDC channel 0        
    .duty = value,//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)        
    .gpio_num = pin,//GPIO number        
    .intr_type = LEDC_INTR_DISABLE,//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.        
    .speed_mode = LEDC_LOW_SPEED_MODE,//set LEDC mode, from ledc_mode_t
    .timer_sel = timer
  };
#else
	#error Not an ESP32 or ESP32-S3
#endif
  ledc_channel_config(&PWM_channel);
}

void PWMInit(){
  int i;
  timerConfig(PWMFreqDefault,PWMTimerDefault);
  for(i = 0; i < PWMMax; i++) PWMChannels[i].pin = PWMPinEmpty;
  for(i = 0; i < PWMFreqMax; i++) PWMFreqChannels[i].pin = PWMPinEmpty;
}

void writePWM(Pin pin,uint16_t value,int freq){
  int channel; int timer;
  if(freq == PWMFreqDefault || freq == 0){
    channel = getFreeChannel(pin);
    if(channel < 0){jsError("no PWM channel available anymore");}
    else{
    PWMChannels[channel].pin = pin;
    channelConfig(PWMTimerDefault,channel,value,pin);
  }
  }
  else{
  timer = getFreeTimer(pin,freq);
  if(timer < 0){jsError("no PWM channel available anymore");}
  else{
    PWMFreqChannels[timer].pin = pin;
    PWMFreqChannels[timer].freq = freq;
    timerConfig(freq,timer);
    channelConfig(timer,PWMMax + timer,value,pin);
  }
  }  
}

void setPWM(Pin pin,uint16_t value){
#if CONFIG_IDF_TARGET_ESP32
  int channel = getChannelIndex(pin);
  if(channel < 0) jsError("pin not assigned to pwm");
  else ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, value);  
#elif CONFIG_IDF_TARGET_ESP32S3
	// No DAC implemented on ESP32S3	
#else
	#error Not an ESP32 or ESP32-S3
#endif    
}



