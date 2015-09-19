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

#include "nrf_utils.h"

 // Configure the low frequency clock to use the external 32.768 kHz crystal as a source. Start this clock.
void lfclk_config_and_start()
{
  // Select the preferred clock source.
  NRF_CLOCK->LFCLKSRC =
      (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
  
  // Trigger the LFCLKSTART task.
  NRF_CLOCK->TASKS_LFCLKSTART = (1UL);

  while (NRF_CLOCK->EVENTS_LFCLKSTARTED != (1UL))
  {
    // Wait for the LFCLK to start...
  }
  
  // Clear the event.
  NRF_CLOCK->EVENTS_LFCLKSTARTED = (0UL);
    
  while (((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) != ((CLOCK_LFCLKSTAT_STATE_Running << CLOCK_LFCLKSTAT_STATE_Pos) & CLOCK_LFCLKSTAT_STATE_Msk)))
  {
    // Do nothing...
  }
}

// Configure the RTC to default settings (ticks every 1/32768 seconds) and then start it.
void rtc1_config_and_start()
{
  NRF_RTC1->PRESCALER = (0UL);
  //NRF_RTC1->EVTENSET = (0x02);
  NRF_RTC1->TASKS_START = (1UL);
}

void cnfg_leds_outputs()
{
  nrf_gpio_cfg_output(17);
  nrf_gpio_cfg_output(18);
  nrf_gpio_cfg_output(19);
  nrf_gpio_cfg_output(20);
}

uint8_t nrf_utils_get_random_number()
{
  
  NRF_RNG->CONFIG = 0x00000001; // Use the bias generator.
  NRF_RNG->TASKS_START = 1;

  while (NRF_RNG->EVENTS_VALRDY != 1)
  {
    // Do nothing.
  }

  NRF_RNG -> EVENTS_VALRDY = 0;

  uint8_t rand_num = (uint8_t) NRF_RNG->VALUE;

  NRF_RNG -> TASKS_STOP = 1;

  return rand_num;

}
