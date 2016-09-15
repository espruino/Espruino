/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include <stdio.h>
#include "ant_key_manager.h"
#include "ant_key_manager_config.h"
#include "ant_interface.h"
#include "nrf_assert.h"

static uint8_t m_ant_plus_network_key[] = ANT_PLUS_NETWORK_KEY;
static uint8_t m_ant_fs_network_key[]   = ANT_FS_NETWORK_KEY;

uint32_t ant_custom_key_set(uint8_t network_number, uint8_t * network_key)
{
    ASSERT(network_key != NULL);
    return sd_ant_network_address_set(network_number, network_key);
}

uint32_t ant_plus_key_set(uint8_t network_number)
{
    return sd_ant_network_address_set(network_number, m_ant_plus_network_key);
}

uint32_t ant_fs_key_set(uint8_t network_number)
{
    return sd_ant_network_address_set(network_number, m_ant_fs_network_key);
}
