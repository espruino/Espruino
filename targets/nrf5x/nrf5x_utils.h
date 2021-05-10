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

/**@file
 *
 * @brief Utility functions specific to nRF5x devices to be used by jshardware.c, libs and others...
 *
 */
#ifndef NRF5X_UTILS_H__
#define NRF5X_UTILS_H__

#include <stdint.h>
#include "nrf_gpio.h"

unsigned int nrf_utils_get_baud_enum(int baud);

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source & start.
void nrf_utils_lfclk_config_and_start(void);

unsigned int nrf_utils_cap_sense(int capSenseTxPin, int capSenseRxPin);

/// Ensure UICR flags are set correctly for the current device
void nrf_configure_uicr_flags(void);

/// Because Nordic's library functions don't inline on NRF52840!
#ifdef NRF_P1
#define NRF_GPIO_PIN_SET_FAST(PIN) if((PIN)<P0_PIN_NUM) NRF_P0->OUTSET=1<<(PIN); else NRF_P1->OUTSET=1<<((PIN)-P0_PIN_NUM);
#define NRF_GPIO_PIN_CLEAR_FAST(PIN) if((PIN)<P0_PIN_NUM) NRF_P0->OUTCLR=1<<(PIN); else NRF_P1->OUTCLR=1<<((PIN)-P0_PIN_NUM);
#define NRF_GPIO_PIN_WRITE_FAST(PIN,V) if((PIN)<P0_PIN_NUM) { if (V) NRF_P0->OUTSET=1<<(PIN); else NRF_P0->OUTCLR=1<<(PIN); } else { if (V) NRF_P1->OUTSET=1<<(PIN); else NRF_P1->OUTCLR=1<<(PIN); }
#define NRF_GPIO_PIN_READ_FAST(PIN) (((PIN)<P0_PIN_NUM) ? (NRF_P0->IN >> (PIN))&1 : (NRF_P0->IN >> (PIN)) &1 )
#else
#define NRF_GPIO_PIN_SET_FAST(PIN) NRF_P0->OUTSET=1<<(PIN);
#define NRF_GPIO_PIN_CLEAR_FAST(PIN) NRF_P0->OUTCLR=1<<(PIN);
#define NRF_GPIO_PIN_WRITE_FAST(PIN,V) if (V) NRF_P0->OUTSET=1<<(PIN); else NRF_P0->OUTCLR=1<<(PIN);
#define NRF_GPIO_PIN_READ_FAST(PIN) ((NRF_P0->IN >> (PIN))&1)
#endif

#endif // NRF5X_UTILS_H__

/** @} */
