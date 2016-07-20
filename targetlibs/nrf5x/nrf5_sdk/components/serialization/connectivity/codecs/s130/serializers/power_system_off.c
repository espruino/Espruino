/*
 * Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "nrf_soc_conn.h"
#include "ble_serialization.h"

uint32_t power_system_off_req_dec(uint8_t const * const p_buf,
                                  uint16_t              packet_len)
{
    SER_ASSERT_NOT_NULL(p_buf);

    uint32_t index = 0;
    SER_ASSERT_LENGTH_LEQ(1, packet_len);
    SER_ASSERT(p_buf[index] == SD_POWER_SYSTEM_OFF, NRF_ERROR_INVALID_PARAM);
    index++;

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    return NRF_SUCCESS;
}
