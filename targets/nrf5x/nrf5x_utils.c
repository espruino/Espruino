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
#include "nrf_delay.h"
#include "nrf_temp.h"
#include "app_uart.h"
#include "nrf_error.h"
#include "nrf_nvmc.h"

#define LED1 17
#define LED2 18
#define LED3 19
#define LED4 20

#define FLASH_PAGE_SIZE NRF_FICR->CODEPAGESIZE
#define NUMBER_OF_FLASH_PAGES NRF_FICR->CODESIZE

void nrf_utils_write_flash_address(uint32_t addr, uint32_t val)
{
  nrf_nvmc_write_word(addr, val);
}

void nrf_utils_write_flash_addresses(uint32_t addr, const uint32_t * src, uint32_t len)
{
	nrf_nvmc_write_words(addr, src, len); // Is there a bug here??? see nvmc.h header file it says len is bytes?
}

bool nrf_utils_get_page(uint32_t addr, uint32_t * page_address, uint32_t * page_size)
{
  if (addr < (uint32_t) 0 || addr > (uint32_t) (FLASH_PAGE_SIZE * NUMBER_OF_FLASH_PAGES))
  {
	  return false;
  }
  *page_address = (uint32_t) floor(addr / FLASH_PAGE_SIZE);
  *page_size = (uint32_t) FLASH_PAGE_SIZE;
  return true;
}

void nrf_utils_erase_flash_page(uint32_t addr)
{
	uint32_t * page_address;
	uint32_t * page_size;
	nrf_utils_get_page(addr, page_address, page_size);
	nrf_nvmc_page_erase((uint32_t) page_address);
}

void nrf_utils_read_flash_addresses(void *buf, uint32_t addr, uint32_t len)
{
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

  uint32_t i;
  for(i = 0; i < len; i++)
  {
	((uint32_t *) buf)[i] = *((uint32_t *) (addr + i));
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
  }
}

void nrf_utils_cnfg_leds_as_outputs() 
{
  nrf_gpio_cfg_output(LED1);
  nrf_gpio_cfg_output(LED2);
  nrf_gpio_cfg_output(LED3);
  nrf_gpio_cfg_output(LED4);

  // Turn all the LEDS off. 1 means off on the nRF52DK...
  nrf_gpio_pin_set(LED1);
  nrf_gpio_pin_set(LED2);
  nrf_gpio_pin_set(LED3);
  nrf_gpio_pin_set(LED4);
}

void nrf_utils_delay_us(uint32_t microsec) 
{
  nrf_delay_us(microsec);
}

void nrf_utils_gpio_pin_set(uint32_t pin) 
{
  nrf_gpio_pin_set(pin);
}

void nrf_utils_gpio_pin_clear(uint32_t pin) 
{
  nrf_gpio_pin_clear(pin);
}

uint32_t nrf_utils_gpio_pin_read(uint32_t pin) 
{
  nrf_gpio_pin_read(pin);
}

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source. Start this clock.
void nrf_utils_lfclk_config_and_start()
{

  // Select the preferred clock source.
  NRF_CLOCK->LFCLKSRC = (uint32_t) ((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
  
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
void nrf_utils_rtc1_config_and_start()
{
  NRF_RTC1->PRESCALER = (0UL);
  NRF_RTC1->TASKS_START = (1UL);
}

uint32_t nrf_utils_get_system_time(void) 
{
  return (uint32_t) NRF_RTC1->COUNTER;
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

uint32_t nrf_utils_read_temperature(void) {
  
  nrf_temp_init();

  int32_t volatile nrf_temp;

  NRF_TEMP->TASKS_START = 1;
  while (NRF_TEMP->EVENTS_DATARDY == 0)
  {
    // Do nothing...
  }
  NRF_TEMP->EVENTS_DATARDY = 0;

  nrf_temp = (nrf_temp_read() / 4);

  NRF_TEMP->TASKS_STOP = 1;

  return (uint32_t) nrf_temp;

}

void nrf_utils_app_uart_put(uint8_t character) {
	while (app_uart_put(character) != NRF_SUCCESS);
}
