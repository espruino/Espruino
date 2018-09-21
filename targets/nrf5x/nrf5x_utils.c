/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nrf5x_utils.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_uart.h"
#include "nrf_error.h"
#include "nrf_nvmc.h"
#include "nrf_timer.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_ppi.h"

#include "jsparse.h"

unsigned int nrf_utils_get_baud_enum(int baud) {
  // https://devzone.nordicsemi.com/f/nordic-q-a/391/uart-baudrate-register-values#post-id-1194
  uint64_t br = (((uint64_t)baud) * (1<<25)) / 125000;
  if (br > 0xFFFFFFFF) return 0;
  return ((uint32_t)br + 0x800) & 0xFFFFF000;
}

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source. Start this clock.
void nrf_utils_lfclk_config_and_start()
{
  // Select the preferred clock source.
  // NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
  NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos;

  // Start the 32 kHz clock, and wait for the start up to complete
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while(NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);

  /*
  // wait until the clock is running - Xtal only?
  while (((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) != ((CLOCK_LFCLKSTAT_STATE_Running << CLOCK_LFCLKSTAT_STATE_Pos) & CLOCK_LFCLKSTAT_STATE_Msk)))
  {
    // Do nothing...
  }*/

}


unsigned int nrf_utils_cap_sense(int capSenseTxPin, int capSenseRxPin) {
#ifdef NRF5DDD
  uint32_t err_code;

  nrf_drv_gpiote_in_config_t rxconfig = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
  nrf_drv_gpiote_in_init(capSenseRxPin, &rxconfig, 0);
  nrf_drv_gpiote_in_event_enable(capSenseRxPin, false /* no interrupt */);
  //

  nrf_drv_gpiote_out_config_t txconfig = GPIOTE_CONFIG_OUT_TASK_TOGGLE(true);
  nrf_drv_gpiote_out_init(capSenseTxPin, &txconfig);
  nrf_drv_gpiote_out_task_enable(capSenseTxPin);
  //;

  nrf_timer_mode_set(NRF_TIMER2, TIMER_MODE_MODE_Timer);
  nrf_timer_bit_width_set(NRF_TIMER2, NRF_TIMER_BIT_WIDTH_16);
  nrf_timer_frequency_set(NRF_TIMER2, NRF_TIMER_FREQ_16MHz);
  nrf_timer_cc_write(NRF_TIMER2, 0, 0);
  nrf_timer_cc_write(NRF_TIMER2, 1, 2047);
  nrf_timer_cc_write(NRF_TIMER2, 3, 4095);
  nrf_timer_shorts_enable(NRF_TIMER2, NRF_TIMER_SHORT_COMPARE3_CLEAR_MASK);

  nrf_ppi_channel_t ppi_channel1, ppi_channel2, ppi_channel3;

  // 1: When RX pin goes low->high, capture it in the timer
  err_code = nrf_drv_ppi_channel_alloc(&ppi_channel1);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_ppi_channel_assign(ppi_channel1,
                                        nrf_drv_gpiote_in_event_addr_get(capSenseRxPin),
                                        nrf_timer_task_address_get(NRF_TIMER2, NRF_TIMER_TASK_CAPTURE2));
  APP_ERROR_CHECK(err_code);

  // 2: When timer is at Compare0, toggle
  err_code = nrf_drv_ppi_channel_alloc(&ppi_channel2);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_ppi_channel_assign(ppi_channel2,
                                        nrf_timer_event_address_get(NRF_TIMER2, NRF_TIMER_EVENT_COMPARE0),
                                        nrf_drv_gpiote_out_task_addr_get(capSenseTxPin));
  APP_ERROR_CHECK(err_code);
  // 3: When timer is at Compare1, toggle
  err_code = nrf_drv_ppi_channel_alloc(&ppi_channel3);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_ppi_channel_assign(ppi_channel3,
                                        nrf_timer_event_address_get(NRF_TIMER2, NRF_TIMER_EVENT_COMPARE1),
                                        nrf_drv_gpiote_out_task_addr_get(capSenseTxPin));
  APP_ERROR_CHECK(err_code);

  // Enable both configured PPI channels
  err_code = nrf_drv_ppi_channel_enable(ppi_channel1);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_ppi_channel_enable(ppi_channel2);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_ppi_channel_enable(ppi_channel3);
  APP_ERROR_CHECK(err_code);

  // start timer

  NRF_TIMER2->CC[2] = 0;
  nrf_timer_task_trigger(NRF_TIMER2, NRF_TIMER_TASK_START);
  nrf_gpio_cfg_input(capSenseRxPin, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_output(capSenseTxPin);

  //
  unsigned int sum = 0;
  NRF_TIMER2->EVENTS_COMPARE[1] = 0;
  int i;
  for (i=0;i<50;i++) {
    unsigned int timeout = 100000;
    while (!NRF_TIMER2->EVENTS_COMPARE[1] && --timeout);
    NRF_TIMER2->EVENTS_COMPARE[1] = 0;
    sum += NRF_TIMER2->CC[2];
    if (jspIsInterrupted()) break;
  }

  nrf_gpio_cfg_input(capSenseTxPin, NRF_GPIO_PIN_NOPULL);

  nrf_timer_task_trigger(NRF_TIMER2, NRF_TIMER_TASK_STOP);
  nrf_timer_shorts_disable(NRF_TIMER2, NRF_TIMER_SHORT_COMPARE3_CLEAR_MASK);
  nrf_drv_ppi_channel_disable(ppi_channel3);
  nrf_drv_ppi_channel_disable(ppi_channel2);
  nrf_drv_ppi_channel_disable(ppi_channel1);
  nrf_drv_ppi_channel_free(ppi_channel3);
  nrf_drv_ppi_channel_free(ppi_channel2);
  nrf_drv_ppi_channel_free(ppi_channel1);

  nrf_drv_gpiote_in_uninit(capSenseTxPin);
  nrf_drv_gpiote_in_uninit(capSenseRxPin);

  return sum;
#else
  unsigned int sum = 0;

  int i;
  unsigned int mask = 1<<capSenseRxPin;

  nrf_gpio_cfg_input(capSenseRxPin, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_pin_clear(capSenseTxPin);
  nrf_gpio_cfg_output(capSenseTxPin);

  for (i=0;i<100;i++) {
    const unsigned int CTR_MAX = 100000;
    unsigned int ctr = CTR_MAX;
    nrf_gpio_pin_set(capSenseTxPin);
    while (!(NRF_GPIO->IN & mask) && ctr--);
    sum += CTR_MAX-ctr;
    nrf_gpio_pin_clear(capSenseTxPin);
    while (NRF_GPIO->IN & mask && ctr--);
    sum += CTR_MAX-ctr;
    if (jspIsInterrupted()) break;
  }
  nrf_gpio_cfg_input(capSenseTxPin, NRF_GPIO_PIN_NOPULL);

  return sum;
#endif
}
