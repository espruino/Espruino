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
#ifndef RADIO_TEST_H
#define RADIO_TEST_H

#include <stdint.h>


void radio_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel);
void radio_modulated_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel);
void radio_rx_carrier(uint8_t mode, uint8_t channel);
void radio_tx_sweep_start(uint8_t txpower, uint8_t mode, uint8_t channel_start, uint8_t channel_end, uint8_t delayms);
void radio_rx_sweep_start(uint8_t mode, uint8_t channel_start, uint8_t channel_end, uint8_t delayms);
void radio_sweep_end(void);

#endif
