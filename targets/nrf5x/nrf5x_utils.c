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
#include "app_uart.h"
#include "nrf_error.h"
#include "nrf_nvmc.h"

unsigned int nrf_utils_get_baud_enum(int baud) {
  switch (baud) {
    case 1200: return UART_BAUDRATE_BAUDRATE_Baud1200;
    case 2400: return UART_BAUDRATE_BAUDRATE_Baud2400;
    case 4800: return UART_BAUDRATE_BAUDRATE_Baud4800;
    case 9600: return UART_BAUDRATE_BAUDRATE_Baud9600;
    case 14400: return UART_BAUDRATE_BAUDRATE_Baud14400;
    case 19200: return UART_BAUDRATE_BAUDRATE_Baud19200;
    case 28800: return UART_BAUDRATE_BAUDRATE_Baud28800;
    case 38400: return UART_BAUDRATE_BAUDRATE_Baud38400;
    case 57600: return UART_BAUDRATE_BAUDRATE_Baud57600;
    case 76800: return UART_BAUDRATE_BAUDRATE_Baud76800;
    case 115200: return UART_BAUDRATE_BAUDRATE_Baud115200;
    case 230400: return UART_BAUDRATE_BAUDRATE_Baud230400;
    case 250000: return UART_BAUDRATE_BAUDRATE_Baud250000;
    case 460800: return UART_BAUDRATE_BAUDRATE_Baud460800;
    case 921600: return UART_BAUDRATE_BAUDRATE_Baud921600;
    case 1000000: return UART_BAUDRATE_BAUDRATE_Baud1M;
    default: return 0; // error
  }
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
