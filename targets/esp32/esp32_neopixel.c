/*
This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 * adapted from source written by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 * and https://github.com/cashoefman/ESP32-C3-Rainbow-LED-Strip/blob/master/components/led_strip for C3 port
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * ESP32 specific exposed components for neopixel.
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>

// IDF version-specific includes
  #include "driver/rmt_tx.h"
  #include "driver/rmt_encoder.h"
  #include "hal/rmt_types.h"
  #include "esp_intr_alloc.h"
  static rmt_channel_handle_t neopixel_tx_chan = NULL;
  static rmt_encoder_handle_t neopixel_copy_encoder = NULL;
  static gpio_num_t neopixel_gpio_num = GPIO_NUM_NC;

#include "esp32_neopixel.h"
#include "jshardware.h"  // For jshGetPinFromVar

// Common constants
#define RMTCHANNEL  0
#define MAX_PULSES  64
#define BUFFERS         8
#define DIVIDER    4 /* Above 4, timings start to deviate*/
#define DURATION  12.5 /* minimum time of a single RMT duration */

#define WS2812_T0H_NS (350)
#define WS2812_T0L_NS (900)
#define WS2812_T1H_NS (900)
#define WS2812_T1L_NS (350)
#define WS2812_RESET_US (280)

#define PULSE_T0H  (  WS2812_T0H_NS / (DURATION * DIVIDER))
#define PULSE_T1H  (  WS2812_T1H_NS / (DURATION * DIVIDER))
#define PULSE_T0L  (  WS2812_T0L_NS / (DURATION * DIVIDER))
#define PULSE_T1L  (  WS2812_T1L_NS / (DURATION * DIVIDER))
#define PULSE_TRS  (50000 / (DURATION * DIVIDER))

// Legacy data (IDF < 5 only)

int neopixelConfiguredGPIO = -1;


extern void esp_rom_gpio_connect_out_signal(uint32_t gpio_num, uint32_t signal_idx, bool out_inv, bool oen_inv);

static esp_err_t neopixel_rmt_init_v5(gpio_num_t gpio_num) {
  if (neopixelConfiguredGPIO == (int)gpio_num) {
    if (neopixel_tx_chan != NULL && neopixel_gpio_num == gpio_num) {
      return ESP_OK;
    }
  }
  
  if (neopixel_tx_chan != NULL) {
    rmt_disable(neopixel_tx_chan);
    rmt_del_channel(neopixel_tx_chan);
    if (neopixel_copy_encoder) {
      rmt_del_encoder(neopixel_copy_encoder);
    }
    if (neopixel_gpio_num != GPIO_NUM_NC) {
      esp_rom_gpio_connect_out_signal((uint32_t)neopixel_gpio_num, 256, false, false);
    }
    neopixel_tx_chan = NULL;
    neopixel_copy_encoder = NULL;
    neopixel_gpio_num = GPIO_NUM_NC;
  }

  // Create TX channel (10MHz resolution)
  rmt_tx_channel_config_t tx_config = {
    .gpio_num = gpio_num,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 10000000,  // 10MHz = 100ns/tick
    .mem_block_symbols = 64,
    .trans_queue_depth = 4,
    .flags = {
      .invert_out = false,
      .with_dma = false,
    },
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &neopixel_tx_chan));
  ESP_ERROR_CHECK(rmt_enable(neopixel_tx_chan));

  // Copy encoder for raw symbols
  rmt_copy_encoder_config_t enc_config = {};
  ESP_ERROR_CHECK(rmt_new_copy_encoder(&enc_config, &neopixel_copy_encoder));
  
  neopixel_gpio_num = gpio_num;
  neopixelConfiguredGPIO = (int)gpio_num;
  rmt_tx_wait_all_done(neopixel_tx_chan, pdMS_TO_TICKS(100));
  return ESP_OK;
}

static bool neopixel_write_v5(gpio_num_t gpio_num, unsigned char *rgbData, size_t rgbSize) {
  if (neopixel_rmt_init_v5(gpio_num) != ESP_OK) {
    return false;
  }
  
  // Timings @ 10MHz (100ns/tick)
  float ns_per_tick = 100.0f;
  uint32_t t0h = (uint32_t)(WS2812_T0H_NS / ns_per_tick + 0.5f);  // 4
  uint32_t t0l = (uint32_t)(WS2812_T0L_NS / ns_per_tick + 0.5f);  // 9
  uint32_t t1h = (uint32_t)(WS2812_T1H_NS / ns_per_tick + 0.5f);  // 9
  uint32_t t1l = (uint32_t)(WS2812_T1L_NS / ns_per_tick + 0.5f);  // 4
  uint32_t tres = (uint32_t)(WS2812_RESET_US * 1000 / ns_per_tick + 0.5f); // 500us

  size_t num_pixels = rgbSize / 3;
  size_t num_symbols = num_pixels * 24 + 1;  // 24 bits/pixel + reset
  
  rmt_symbol_word_t *symbols = malloc(num_symbols * sizeof(rmt_symbol_word_t));
  if (!symbols) return false;

  size_t sym_idx = 0;
  
  // WS2812 GRB order
  for (size_t i = 0; i < num_pixels; i++) {
    uint8_t g = rgbData[i * 3 + 1];  // Green (MSB first)
    uint8_t r = rgbData[i * 3 + 0];  // Red
    uint8_t b = rgbData[i * 3 + 2];  // Blue
    
    uint8_t pixels[3] = {g, r, b};
    
    for (int c = 0; c < 3; c++) {
      for (int bit = 7; bit >= 0; bit--) {
        if (pixels[c] & (1 << bit)) {
          symbols[sym_idx++] = (rmt_symbol_word_t){
            .duration0 = t1h, .level0 = 1,
            .duration1 = t1l, .level1 = 0
          };
        } else {
          symbols[sym_idx++] = (rmt_symbol_word_t){
            .duration0 = t0h, .level0 = 1,
            .duration1 = t0l, .level1 = 0
          };
        }
      }
    }
  }
  
  // Reset (long low)
  symbols[sym_idx++] = (rmt_symbol_word_t){
    .duration0 = tres, .level0 = 0,
    .duration1 = 0,    .level1 = 0
  };

  // Transmit
  rmt_transmit_config_t tx_config = {.loop_count = 0};
  esp_err_t ret = rmt_transmit(neopixel_tx_chan, neopixel_copy_encoder, 
                              symbols, num_symbols * sizeof(rmt_symbol_word_t), &tx_config);
  free(symbols);
  
  if (ret != ESP_OK) return false;
  return true;
}



void neopixel_init(int gpioNum) {
  neopixel_gpio_num = (gpio_num_t)gpioNum;
}

bool esp32_neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
  if (!jshIsPinValid(pin)) {
    return false;
  }

  gpio_num_t gpio_num = (gpio_num_t)pin;
  
  return neopixel_write_v5(gpio_num, rgbData, rgbSize);
}
