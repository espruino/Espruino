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
#include <soc/rmt_struct.h>
#include <driver/gpio.h>
#include <soc/gpio_sig_map.h>
#include <esp_intr.h>
#include <driver/rmt.h>

#if ESP_IDF_VERSION_MAJOR>=4
#else
#include <soc/dport_reg.h>
#endif

#include "esp32_neopixel.h"

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
static rmtPulsePair neopixel_bits[2]= {
  {{PULSE_T0H,1,PULSE_T0L,0}},
  {{PULSE_T1H,1,PULSE_T1L,0}}
};

#if ESP_IDF_VERSION_MAJOR>=4

int neopixelConfiguredGPIO = -1;

/**
 * @brief Convert RGB data to RMT format.
 *
 * @note For WS2812, R,G,B each contains 256 different choices (i.e. uint8_t)
 *
 * @param[in] src: source data, to converted to RMT format
 * @param[in] dest: place where to store the convert result
 * @param[in] src_size: size of source data
 * @param[in] wanted_num: number of RMT items that want to get
 * @param[out] translated_size: number of source data that got converted
 * @param[out] item_num: number of RMT items which are converted from source data
 */
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
            // MSB first
            pdest->val =  neopixel_bits[(*psrc>>i) & 1].val;
            pdest++;
        }
        num+=8;
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
#endif

void neopixel_copy(){
  unsigned int i, j, offset, offset2, len, bit;
  offset = neopixel_bufpart * MAX_PULSES;
  neopixel_bufpart = (neopixel_bufpart+1)%BUFFERS;
  offset2 = neopixel_bufpart * MAX_PULSES;
  len = neopixel_len - neopixel_pos;
  if (len > (MAX_PULSES / 8)) len = (MAX_PULSES / 8);
  if (!len) {
//    for (i = 0; i < MAX_PULSES; i++)
//      RMTMEM.chan[RMTCHANNEL].data32[i + offset].val = 0;
    return;
  }
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
  return;
}
void neopixel_handleInterrupt(void *arg){
  portBASE_TYPE taskAwoken = 0;
  if (RMT.int_st.ch0_tx_thr_event) {
    neopixel_copy();
    RMT.int_clr.ch0_tx_thr_event = 1;
  }
  else if (RMT.int_st.ch0_tx_end && neopixel_sem) {
    xSemaphoreGiveFromISR(neopixel_sem, &taskAwoken);
    RMT.int_clr.ch0_tx_end = 1;
  }
  return;
}

void neopixel_init(int gpioNum){
#if ESP_IDF_VERSION_MAJOR>=4
  if (neopixelConfiguredGPIO != gpioNum) {
    if (neopixelConfiguredGPIO)
      rmt_driver_uninstall(RMTCHANNEL);
    neopixelConfiguredGPIO = gpioNum;
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpioNum, RMTCHANNEL);
    // set counter clock to 40MHz
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
    rmt_translator_init(config.channel, ws2812_rmt_adapter);
  }
#else
  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN);
  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST);

//  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpioNum], 2);
//  gpio_matrix_out(gpioNum, RMT_SIG_OUT0_IDX + RMTCHANNEL, 0, 0);
//  gpio_set_direction(gpioNum, GPIO_MODE_OUTPUT);
  rmt_set_pin((rmt_channel_t)RMTCHANNEL, RMT_MODE_TX, (gpio_num_t)gpioNum);

  neopixel_initRMTChannel(RMTCHANNEL);

  RMT.tx_lim_ch[RMTCHANNEL].limit = MAX_PULSES;
  RMT.int_ena.ch0_tx_thr_event = 1;
  RMT.int_ena.ch0_tx_end = 1;
  esp_intr_alloc(ETS_RMT_INTR_SOURCE, ESP_INTR_FLAG_INTRDISABLED, neopixel_handleInterrupt, NULL, &rmt_intr_handle);
#endif
}

bool esp32_neopixelWrite(Pin pin,unsigned char *rgbData, size_t rgbSize){
  if(rgbSize) {
    neopixel_init(pin);
#if ESP_IDF_VERSION_MAJOR>=4
    if (rmt_write_sample(RMTCHANNEL, rgbData, rgbSize, true) != ESP_OK)
      printf("transmit RMT samples failed\n");
    int timeout_ms = 100;
    return rmt_wait_tx_done(RMTCHANNEL, pdMS_TO_TICKS(timeout_ms));
#else
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
    rmt_intr_handle=0;
#endif
  }
  return true;
}
