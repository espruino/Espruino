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

#include <string.h>

#include "ble_gatts_conn.h"
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_gatts_include_add_req_dec(uint8_t const * const p_buf,
                                       uint16_t              packet_len,
                                       uint16_t * const      p_service_handle,
                                       uint16_t * const      p_inc_srvc_handle,
                                       uint16_t * * const    pp_include_handle)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_service_handle);
    SER_ASSERT_NOT_NULL(p_inc_srvc_handle);
    SER_ASSERT_NOT_NULL(pp_include_handle);
    SER_ASSERT_LENGTH_EQ(6, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_GATTS_INCLUDE_ADD, NRF_ERROR_INVALID_PARAM);
    index++;

    uint16_dec(p_buf, packet_len, &index, p_service_handle);
    uint16_dec(p_buf, packet_len, &index, p_inc_srvc_handle);

    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_include_handle = NULL;
    }
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gatts_include_add_rsp_enc(uint32_t               return_code,
                                       uint8_t * const        p_buf,
                                       uint32_t * const       p_buf_len,
                                       uint16_t const * const p_include_handle)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t buf_len = *p_buf_len;
    uint32_t index   = 0;
    uint32_t err_code;

    err_code = op_status_enc(SD_BLE_GATTS_INCLUDE_ADD, return_code, p_buf, p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }
    SER_ASSERT_NOT_NULL(p_include_handle);
    err_code = uint16_t_enc(p_include_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index; //update change made by op_status_enc
    return err_code;
}
