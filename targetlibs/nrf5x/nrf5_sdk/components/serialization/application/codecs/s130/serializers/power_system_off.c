/*
 * Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "nrf_soc_app.h"
#include "ble_serialization.h"
#include "nrf_soc.h"


uint32_t power_system_off_req_enc(uint8_t * const p_buf, uint32_t * const p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(1, *p_buf_len);

    p_buf[index++] = SD_POWER_SYSTEM_OFF;

    *p_buf_len = index;

    return NRF_SUCCESS;
}
