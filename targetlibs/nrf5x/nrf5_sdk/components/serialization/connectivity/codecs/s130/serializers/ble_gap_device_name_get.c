/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gap_conn.h"
#include "ble_serialization.h"
#include "nrf_error.h"

uint32_t ble_gap_device_name_get_req_dec(uint8_t const * const p_buf,
                                         uint32_t              buf_len,
                                         uint8_t * *           pp_name,
                                         uint16_t * *          pp_name_len)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(pp_name_len);
    SER_ASSERT_NOT_NULL(*pp_name_len);

    SER_ASSERT_LENGTH_LEQ(index + 1, buf_len);

    index++;

    if (p_buf[index] != SER_FIELD_PRESENT && p_buf[index] != SER_FIELD_NOT_PRESENT)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        err_code = uint16_t_dec(p_buf, buf_len, &index, *pp_name_len);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }
    else
    {
        *pp_name_len = NULL;
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, buf_len);

    if (p_buf[index] != SER_FIELD_PRESENT && p_buf[index] != SER_FIELD_NOT_PRESENT)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (p_buf[index] == SER_FIELD_NOT_PRESENT)
    {
        *pp_name = NULL;
    }

    return NRF_SUCCESS;
}


uint32_t ble_gap_device_name_get_rsp_enc(uint32_t              return_code,
                                         uint8_t const * const p_dev_name,
                                         uint16_t              dev_name_len,
                                         uint8_t * const       p_buf,
                                         uint32_t * const      p_buflen)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buflen);

    uint32_t buflen   = *p_buflen;
    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_DEVICE_NAME_GET, return_code,
                                                        p_buf, p_buflen);
    uint32_t index = *p_buflen;

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    err_code = len16data_enc(p_dev_name, dev_name_len, p_buf, buflen, &index);

    *p_buflen = index;

    return err_code;
}
