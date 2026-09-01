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

#if ESP_IDF_VERSION_MAJOR >= 5
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/rmt_tx.h>
#else
#include "driver/rmt.h"
#endif

RMTChannel RMTChannels[RMTChannelMax];

#define RMTPinEmpty 111

#if ESP_IDF_VERSION_MAJOR < 5
rmt_item32_t items[1];
#else
static rmt_channel_handle_t rmt_channels[RMTChannelMax];
#endif

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

#if ESP_IDF_VERSION_MAJOR >= 5

      if(rmt_channels[i]){
        rmt_disable(rmt_channels[i]);
        rmt_del_channel(rmt_channels[i]);
        rmt_channels[i] = NULL;
      }

#else

      rmt_driver_uninstall(i);

#endif

      RMTChannels[i].pin = RMTPinEmpty;
    }
  }
}


void RMTInit(){
  for(int i = 0; i < RMTChannelMax; i++){
    RMTChannels[i].pin = RMTPinEmpty;

#if ESP_IDF_VERSION_MAJOR >= 5
    rmt_channels[i] = NULL;
#endif

  }
}

int RMTInitChannel(Pin pin, bool pulsePolarity){

  int i = getFreeRMT(pin);

  if(i < 0)
    return -1;

#if ESP_IDF_VERSION_MAJOR >= 5

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

#else

  rmt_config_t config;

  config.rmt_mode = RMT_MODE_TX;
  config.channel = i;
  config.gpio_num = pin;

  config.mem_block_num = 1;

  config.tx_config.loop_en = 0;
  config.tx_config.carrier_en = 0;
  config.tx_config.idle_output_en = 1;

  if(pulsePolarity)
    config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  else
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

  config.tx_config.carrier_duty_percent = 50;
  config.tx_config.carrier_freq_hz = 10000;
  config.tx_config.carrier_level = 1;

  config.clk_div = 80;

  rmt_config(&config);
  rmt_driver_install(config.channel, 0, 0);

#endif

  return i;
}

#if ESP_IDF_VERSION_MAJOR < 5

void setPulseLow(int duration){
  items[0].duration0 = duration;
  items[0].level0 = 0;
  items[0].duration1 = 10;
  items[0].level1 = 1;
}

void setPulseHigh(int duration){
  items[0].duration0 = duration;
  items[0].level0 = 1;
  items[0].duration1 = 10;
  items[0].level1 = 0;
}

#endif

void sendPulse(Pin pin, bool pulsePolarity, int duration){

  int i = getRMTIndex(pin);

  if(i < 0)
    i = RMTInitChannel(pin, pulsePolarity);

  if(i < 0){
    printf("all RMT channels in use\n");
    return;
  }

#if ESP_IDF_VERSION_MAJOR >= 5

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

#else

  if(pulsePolarity)
    setPulseLow(duration);
  else
    setPulseHigh(duration);

#if ESP_IDF_VERSION_MAJOR >= 4
  rmt_set_pin(i, RMT_MODE_TX, pin);
#else
  rmt_set_pin(i, RMT_MODE_TX, pin);
#endif

  rmt_write_items(i, items, 1, 1);

#endif
}