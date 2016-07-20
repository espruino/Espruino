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

#include "nrf_soc.h"
#include "nrf_error.h"
#include "ble_serialization.h"
#include "cond_field_serialization.h"

uint32_t temp_get_req_enc(int32_t const * const p_temp,
                          uint8_t * const       p_buf,
                          uint32_t * const      p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    uint32_t total_len = *p_buf_len;

    SER_ASSERT_LENGTH_LEQ(1, total_len);
    p_buf[index++] = SD_TEMP_GET;

    err_code = cond_field_enc(p_temp, p_buf, total_len, &index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}

uint32_t temp_get_rsp_dec(uint8_t const * const p_buf,
                          uint32_t              packet_len,
                          uint32_t * const      p_result_code,
                          int32_t * const       p_temp)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);
    SER_ASSERT_NOT_NULL(p_temp);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                               &index,
                                               packet_len,
                                               SD_TEMP_GET,
                                               p_result_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (*p_result_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    err_code = uint32_t_dec(p_buf, packet_len, &index, p_temp);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
