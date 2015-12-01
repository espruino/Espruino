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
#include "app_uart.h"
#include "nrf_error.h"
#include "nrf_nvmc.h"

#define NRF_UTILS_FLASH_PAGE_SIZE NRF_FICR -> CODEPAGESIZE
#define NRF_UTILS_NUMBER_OF_PAGES NRF_FICR -> CODESIZE
#define NRF_UTILS_FLASH_SIZE (NRF_UTILS_FLASH_PAGE_SIZE * NRF_UTILS_NUMBER_OF_PAGES)

bool nrf_utils_get_page(uint32_t addr, uint32_t * page_address, uint32_t * page_size)
{
  if (addr > (NRF_UTILS_FLASH_SIZE))
  {
	  return false;
  }
  *page_address = (uint32_t) (floor(addr / NRF_UTILS_FLASH_PAGE_SIZE) * NRF_UTILS_FLASH_PAGE_SIZE);
  *page_size = NRF_UTILS_FLASH_PAGE_SIZE;
  return true;
}

void nrf_utils_erase_flash_page(uint32_t addr)
{
  nrf_nvmc_page_erase(addr);
}

void nrf_utils_read_flash_bytes(uint8_t * buf, uint32_t addr, uint32_t len)
{
  if (addr > (NRF_UTILS_FLASH_SIZE))
  {
  	return;
  }

  if (addr + len > (NRF_UTILS_FLASH_SIZE))
  {
	len = (NRF_UTILS_FLASH_SIZE) - addr;
  }

  while (NRF_NVMC -> READY == NVMC_READY_READY_Busy);

  uint32_t data = *(uint32_t *) (addr & (~3UL));
  while (len-- > 0)
  {
    if (addr & (3UL) == 0)
    {
      data = *(uint32_t *) addr;
    }
    *(buf++) = ((uint8_t *) &data)[(addr++ & (3UL))];
  }
}

void nrf_utils_write_flash_bytes(uint32_t addr, uint8_t * buf, uint32_t len)
{
  nrf_nvmc_write_bytes(addr, buf, len);
}

uint32_t nrf_utils_gpio_pin_get_state(uint32_t pin)
{
	/*uint32_t pin_register;
	pin_register = NRF_GPIO->PIN_CNF[pin];*/
    return (uint32_t) 0;
}

void nrf_utils_delay_us(uint32_t microsec) 
{
  nrf_delay_us(microsec);
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

int nrf_utils_get_device_id(uint8_t * device_id, int maxChars)
{
	uint32_t deviceID[2];
	deviceID[0] = NRF_FICR->DEVICEID[0];
	deviceID[1] = NRF_FICR->DEVICEID[1];

	uint8_t * temp = (uint8_t *) &deviceID[0];

	uint32_t i;
	for (i = 0; i < maxChars || i < 8; i++)
	{
	  *device_id = *temp;
	  device_id++;
	  temp++;
	}

	return i;
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

void nrf_utils_app_uart_put(uint8_t character) {
	while (app_uart_put(character) != NRF_SUCCESS);
}

void print_string_to_terminal(uint8_t * debug_string, uint32_t len)
{
  int i;
  for (i = 0; i < len; i++)
  {
	  nrf_utils_app_uart_put((char) debug_string[i]);
  }
}
