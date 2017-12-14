/*
 * This file is designed to support Analog functions in Espruino,
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
#include "jshardwareAnalog.h"
#include "driver/adc.h"
#include "driver/dac.h"

#include <stdio.h>

#define adc_channel_max 8

adc_atten_t adc_channel[8];

adc1_channel_t pinToAdcChannel(Pin pin){
  adc1_channel_t channel;
  switch(pin){
    case 36: channel = ADC1_CHANNEL_0; break;
    case 37: channel = ADC1_CHANNEL_1; break;
    case 38: channel = ADC1_CHANNEL_2; break;
    case 39: channel = ADC1_CHANNEL_3; break;
    case 32: channel = ADC1_CHANNEL_4; break;
    case 33: channel = ADC1_CHANNEL_5; break;
    case 34: channel = ADC1_CHANNEL_6; break;
    case 35: channel = ADC1_CHANNEL_7; break;
    default: channel = -1; break;
  }
  return channel;
}
adc_atten_t rangeToAdcAtten(int range){
  adc_atten_t atten;
  switch (range){
	case 1000: atten = ADC_ATTEN_0db; break;
	case 1340: atten = ADC_ATTEN_2_5db; break;
	case 2000: atten = ADC_ATTEN_6db; break;
	case 3600: atten = ADC_ATTEN_11db; break;
	default: atten = ADC_ATTEN_11db; break;
  }
  return atten;
}
int pinToAdcChannelIdx(Pin pin){
  int idx;
  switch(pin){
	case 36: idx = 0;break;
	case 37: idx = 1;break;
	case 38: idx = 2;break;
	case 39: idx = 3;break;
	case 32: idx = 4;break;
	case 33: idx = 5;break;
	case 34: idx = 6;break;
	case 35: idx = 7;break;
	default: idx = -1; break;
  }
  return idx;
}

dac_channel_t pinToDacChannel(Pin pin){
  dac_channel_t channel;
  switch(pin){
    case 25: channel = DAC_CHANNEL_1; break;
    case 26: channel = DAC_CHANNEL_2; break;
    default: channel = -1; break;
  }
  return channel;
}

void ADCReset(){
  initADC(1);
}
void initADC(int ADCgroup){
  switch(ADCgroup){
	case 1:
	  adc1_config_width(ADC_WIDTH_12Bit);
	  for(int i = 0; i < adc_channel_max; i++){ adc_channel[i] = ADC_ATTEN_11db; }
	  break;
	case 2:
	  jsExceptionHere(JSET_ERROR, "not implemented\n");
	  break;
	case 3:
	  jsExceptionHere(JSET_ERROR, "not implemented\n");
	  break;
	default:
	  jsExceptionHere(JSET_ERROR, "out of range\n");
	break;
  }
}

void rangeADC(Pin pin,int range){
  int idx,atten;
  idx = pinToAdcChannelIdx(pin);
  printf("idx:%d\n",idx);
  if(idx >= 0){
	adc_channel[idx] = rangeToAdcAtten(range);
	printf("Atten:%d \n",adc_channel[idx]);
  }
}

int readADC(Pin pin){
  adc1_channel_t channel; int value;
  channel = pinToAdcChannel(pin);
  adc1_config_channel_atten(channel,adc_channel[pinToAdcChannelIdx(pin)]);
  if(channel >= 0) {
	value = adc1_get_voltage(channel);
	return value;
  }
  else return -1;  
}

void writeDAC(Pin pin,uint8_t value){
  dac_channel_t channel;
  if(value > 255){
	jsExceptionHere(JSET_ERROR, "not implemented, only 8 bit supported\n");
	return;
  }
  channel = pinToDacChannel(pin);
  if(channel >= 0) dac_out_voltage(channel, value);
}



