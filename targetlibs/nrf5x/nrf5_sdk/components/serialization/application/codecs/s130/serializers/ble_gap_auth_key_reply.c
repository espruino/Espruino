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

#include "ble_gap_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap.h"
#include "app_util.h"


uint32_t ble_gap_auth_key_reply_req_enc(uint16_t              conn_handle,
                                        uint8_t               key_type,
                                        uint8_t const * const p_key,
                                        uint8_t * const       p_buf,
                                        uint32_t * const      p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t buf_len  = *p_buf_len;
    uint8_t  opcode   = SD_BLE_GAP_AUTH_KEY_REPLY;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  key_len;

    err_code = uint8_t_enc(&opcode, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&conn_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&key_type, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    switch (key_type)
    {
        case BLE_GAP_AUTH_KEY_TYPE_NONE:
            key_len = 0;
            break;

        case BLE_GAP_AUTH_KEY_TYPE_PASSKEY:
            key_len = 6;
            break;

        case BLE_GAP_AUTH_KEY_TYPE_OOB:
            key_len = 16;
            break;

        default:
            return NRF_ERROR_INVALID_PARAM;
    }

    err_code = buf_enc(p_key, key_len, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_gap_auth_key_reply_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_AUTH_KEY_REPLY, p_result_code);
}
