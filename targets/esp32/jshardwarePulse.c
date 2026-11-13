/*
 * This file is designed to support Pulse functions in Espruino,
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
#include "jshardwarePulse.h"

#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/rmt_tx.h>

RMTChannel RMTChannels[RMTChannelMax];

#define RMTPinEmpty 111

static rmt_channel_handle_t rmt_channels[RMTChannelMax];

int getRMTIndex(Pin pin){
  for(int i = 0; i < RMTChannelMax; i++){
    if(RMTChannels[i].pin == pin) return i;
  }
  return -1;
}

int getFreeRMT(Pin pin){
  for(int i = 0; i < RMTChannelMax; i++){
    if(RMTChannels[i].pin == RMTPinEmpty){
      RMTChannels[i].pin = pin;
      return i;
    }
  }
  return -1;
}

void RMTReset(){
  for(int i = 0; i < RMTChannelMax; i++){
    if(RMTChannels[i].pin != RMTPinEmpty){


      if(rmt_channels[i]){
        rmt_disable(rmt_channels[i]);
        rmt_del_channel(rmt_channels[i]);
        rmt_channels[i] = NULL;
      }


      RMTChannels[i].pin = RMTPinEmpty;
    }
  }
}


void RMTInit(){
  for(int i = 0; i < RMTChannelMax; i++){
    RMTChannels[i].pin = RMTPinEmpty;

    rmt_channels[i] = NULL;

  }
}

int RMTInitChannel(Pin pin, bool pulsePolarity){

  int i = getFreeRMT(pin);

  if(i < 0)
    return -1;


  rmt_tx_channel_config_t tx_config = {
      .gpio_num = pin,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 1000000,   // 1 MHz -> 1 tick = 1 µs
      .mem_block_symbols = 64,
      .trans_queue_depth = 4,
  };

  if(rmt_new_tx_channel(&tx_config, &rmt_channels[i]) != ESP_OK)
    return -1;

  rmt_enable(rmt_channels[i]);


  return i;
}


void sendPulse(Pin pin, bool pulsePolarity, int duration){

  int i = getRMTIndex(pin);

  if(i < 0)
    i = RMTInitChannel(pin, pulsePolarity);

  if(i < 0){
    printf("all RMT channels in use\n");
    return;
  }


  /* duration is µs (1MHz clock) */

  rmt_symbol_word_t symbol;

  if(pulsePolarity){
    symbol.level0 = 0;
    symbol.duration0 = duration;
    symbol.level1 = 1;
    symbol.duration1 = 10;
  } else {
    symbol.level0 = 1;
    symbol.duration0 = duration;
    symbol.level1 = 0;
    symbol.duration1 = 10;
  }

  rmt_transmit_config_t tx_config = {
      .loop_count = 0
  };

  rmt_transmit(
      rmt_channels[i],
      NULL,
      &symbol,
      sizeof(symbol),
      &tx_config
  );

  rmt_tx_wait_all_done(rmt_channels[i], portMAX_DELAY);

}