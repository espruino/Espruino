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
#if ESP_IDF_VERSION_MAJOR >= 5
  #include "driver/rmt_tx.h"
  #include "driver/rmt_encoder.h"
  #include "hal/rmt_types.h"
  #include "esp_intr_alloc.h"
  static rmt_channel_handle_t neopixel_tx_chan = NULL;
  static rmt_encoder_handle_t neopixel_copy_encoder = NULL;
  static gpio_num_t neopixel_gpio_num = GPIO_NUM_NC;
#else
  #include <soc/rmt_struct.h>
  #include <soc/gpio_sig_map.h>
  #include "esp_intr.h"
  #include <driver/rmt.h>
  #if ESP_IDF_VERSION_MAJOR >= 4
  #else
    #include <soc/dport_reg.h>
  #endif
#endif

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
#if ESP_IDF_VERSION_MAJOR < 5
typedef union {
  struct {
    uint32_t duration0:15;
    uint32_t level0:1;
    uint32_t duration1:15;
    uint32_t level1:1;
  };
  uint32_t val;
} rmtPulsePair;

static uint8_t *neopixel_buffer = NULL;
static unsigned int neopixel_pos, neopixel_len, neopixel_bufpart;
static xSemaphoreHandle neopixel_sem = NULL;
static intr_handle_t rmt_intr_handle = NULL;
static rmtPulsePair neopixel_bits[2] = {
  {{PULSE_T0H,1,PULSE_T0L,0}},
  {{PULSE_T1H,1,PULSE_T1L,0}}
};
#endif

int neopixelConfiguredGPIO = -1;

#if ESP_IDF_VERSION_MAJOR >= 5

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

#endif  // ESP_IDF_VERSION_MAJOR >= 5

#if ESP_IDF_VERSION_MAJOR < 5

#if ESP_IDF_VERSION_MAJOR == 4
static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                        size_t wanted_num, size_t *translated_size, size_t *item_num) {
  if (src == NULL || dest == NULL) {
    *translated_size = 0;
    *item_num = 0;
    return;
  }
  size_t size = 0;
  size_t num = 0;
  uint8_t *psrc = (uint8_t *)src;
  rmt_item32_t *pdest = dest;
  while (size < src_size && num < wanted_num) {
    for (int i = 7; i >= 0; i--) {
      pdest->val = neopixel_bits[(*psrc >> i) & 1].val;
      pdest++;
    }
    num += 8;
    size++;
    psrc++;
  }
  *translated_size = size;
  *item_num = num;
}

#else 

void neopixel_initRMTChannel(int rmtChannel){
  RMT.apb_conf.fifo_mask = 1;  //enable memory access, instead of FIFO mode.
  RMT.apb_conf.mem_tx_wrap_en = 1; //wrap around when hitting end of buffer
  RMT.conf_ch[rmtChannel].conf0.div_cnt = DIVIDER;
  RMT.conf_ch[rmtChannel].conf0.mem_size = BUFFERS;//1;
  RMT.conf_ch[rmtChannel].conf0.carrier_en = 0;
  RMT.conf_ch[rmtChannel].conf0.carrier_out_lv = 1;
  RMT.conf_ch[rmtChannel].conf0.mem_pd = 0;

  RMT.conf_ch[rmtChannel].conf1.rx_en = 0;
  RMT.conf_ch[rmtChannel].conf1.mem_owner = 0;
  RMT.conf_ch[rmtChannel].conf1.tx_conti_mode = 0;    //loop back mode.
  RMT.conf_ch[rmtChannel].conf1.ref_always_on = 1;    // use apb clock: 80M
  RMT.conf_ch[rmtChannel].conf1.idle_out_en = 1;
  RMT.conf_ch[rmtChannel].conf1.idle_out_lv = 0;
}

#endif // ESP_IDF_VERSION_MAJOR == 4

void neopixel_copy() {
  unsigned int i, j, offset, offset2, len, bit;
  offset = neopixel_bufpart * MAX_PULSES;
  neopixel_bufpart = (neopixel_bufpart+1)%BUFFERS;
  offset2 = neopixel_bufpart * MAX_PULSES;
  len = neopixel_len - neopixel_pos;
  if (len > (MAX_PULSES / 8)) len = (MAX_PULSES / 8);
  if (!len) return;
  for (i = 0; i < len; i++) {
    bit = neopixel_buffer[i + neopixel_pos];
    for (j = 0; j < 8; j++, bit <<= 1) {
      RMTMEM.chan[RMTCHANNEL].data32[j + i * 8 + offset].val = neopixel_bits[(bit >> 7) & 0x01].val;
    }
    if (i + neopixel_pos == neopixel_len - 1)
      RMTMEM.chan[RMTCHANNEL].data32[7 + i * 8 + offset].duration1 = PULSE_TRS;
  }
  for (i *= 8; i < MAX_PULSES; i++) RMTMEM.chan[RMTCHANNEL].data32[i + offset].val = 0;
  RMTMEM.chan[RMTCHANNEL].data32[offset2].val = 0;
  neopixel_pos += len;
}

void neopixel_handleInterrupt(void *arg) {
  portBASE_TYPE taskAwoken = 0;
#if CONFIG_IDF_TARGET_ESP32S3
  if (RMT.int_st.ch0_tx_thr_event_int_st) {
    neopixel_copy();
    RMT.int_clr.ch0_tx_thr_event_int_clr = 1;
  } else if (RMT.int_st.ch0_tx_end_int_st && neopixel_sem) {
    xSemaphoreGiveFromISR(neopixel_sem, &taskAwoken);
    RMT.int_clr.ch0_tx_end_int_clr = 1;
  }
#else
  if (RMT.int_st.ch0_tx_thr_event) {
    neopixel_copy();
    RMT.int_clr.ch0_tx_thr_event = 1;
  } else if (RMT.int_st.ch0_tx_end && neopixel_sem) {
    xSemaphoreGiveFromISR(neopixel_sem, &taskAwoken);
    RMT.int_clr.ch0_tx_end = 1;
  }
#endif
}

#endif  // ESP_IDF_VERSION_MAJOR < 5

void neopixel_init(int gpioNum) {
#if ESP_IDF_VERSION_MAJOR >= 5
  neopixel_gpio_num = (gpio_num_t)gpioNum;
#elif ESP_IDF_VERSION_MAJOR >= 4
  if (neopixelConfiguredGPIO != gpioNum) {
    if (neopixelConfiguredGPIO != -1) {
      rmt_driver_uninstall(RMTCHANNEL);
      gpio_matrix_out(neopixelConfiguredGPIO, SIG_GPIO_OUT_IDX, 0, 0);
    }
    neopixelConfiguredGPIO = gpioNum;
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpioNum, RMTCHANNEL);
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    uint32_t counter_clk_hz = 0;
    if (rmt_get_counter_clock(config.channel, &counter_clk_hz) != ESP_OK)
      printf("get rmt counter clock failed\n");
    float ratio = (float)counter_clk_hz / 1e9;
    neopixel_bits[0].duration0 = (uint32_t)(ratio * WS2812_T0H_NS);
    neopixel_bits[0].duration1 = (uint32_t)(ratio * WS2812_T0L_NS);
    neopixel_bits[1].duration0 = (uint32_t)(ratio * WS2812_T1H_NS);
    neopixel_bits[1].duration1 = (uint32_t)(ratio * WS2812_T1L_NS);
    // set ws2812 to rmt adapter
    #if ESP_IDF_VERSION_MAJOR < 5
    rmt_translator_init(config.channel, ws2812_rmt_adapter);
    #endif
  }
#else
  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN);
  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST);
  if (neopixelConfiguredGPIO != gpioNum) {
    if (neopixelConfiguredGPIO != -1) {
      gpio_matrix_out(neopixelConfiguredGPIO,SIG_GPIO_OUT_IDX,0,0);
    }
    neopixelConfiguredGPIO = gpioNum;
  }
  rmt_set_pin((rmt_channel_t)RMTCHANNEL, RMT_MODE_TX, (gpio_num_t)gpioNum);
  neopixel_initRMTChannel(RMTCHANNEL);
  RMT.tx_lim_ch[RMTCHANNEL].limit = MAX_PULSES;
  RMT.int_ena.ch0_tx_thr_event = 1;
  RMT.int_ena.ch0_tx_end = 1;
  esp_intr_alloc(ETS_RMT_INTR_SOURCE, ESP_INTR_FLAG_INTRDISABLED, neopixel_handleInterrupt, NULL, &rmt_intr_handle);
#endif
}

bool esp32_neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
  if (!jshIsPinValid(pin)) {
    return false;
  }

  gpio_num_t gpio_num = (gpio_num_t)pin;
  
#if ESP_IDF_VERSION_MAJOR >= 5
  return neopixel_write_v5(gpio_num, rgbData, rgbSize);
#elif ESP_IDF_VERSION_MAJOR >= 4
  if (rgbSize) {
    neopixel_init(gpio_num);
    if (rmt_write_sample(RMTCHANNEL, rgbData, rgbSize, true) != ESP_OK) {
      printf("transmit RMT samples failed\n");
      return false;
    }
    int timeout_ms = 100;
    return rmt_wait_tx_done(RMTCHANNEL, pdMS_TO_TICKS(timeout_ms));
  }
  return true;
#else
  if (rgbSize) {
    neopixel_init(gpio_num);
    neopixel_buffer = rgbData;
    neopixel_len = rgbSize;
    neopixel_pos = 0;
    neopixel_bufpart = 0;
    neopixel_sem = xSemaphoreCreateBinary();
    for(int i=0;i<BUFFERS-1;i++)
      if (neopixel_pos < neopixel_len)
        neopixel_copy();
    RMT.conf_ch[RMTCHANNEL].conf1.mem_rd_rst = 1;
    esp_intr_enable(rmt_intr_handle);
    RMT.conf_ch[RMTCHANNEL].conf1.tx_start = 1;
    xSemaphoreTake(neopixel_sem, portMAX_DELAY);
    vSemaphoreDelete(neopixel_sem);
    esp_intr_free(rmt_intr_handle);
    neopixel_sem = NULL;
    rmt_intr_handle = 0;
  }
  return true;
#endif
}
