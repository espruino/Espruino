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
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_gap_auth_key_reply_req_dec(uint8_t const * const p_buf,
                                        uint16_t              packet_len,
                                        uint16_t *            p_conn_handle,
                                        uint8_t *             p_key_type,
                                        uint8_t * * const     pp_key)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_key_type);

    uint32_t index    = SER_CMD_DATA_POS;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  key_len;

    SER_ASSERT_LENGTH_LEQ(2, packet_len - index);
    uint16_dec(p_buf, packet_len, &index, p_conn_handle);

    SER_ASSERT_LENGTH_LEQ(1, packet_len - index);
    uint8_dec(p_buf, packet_len, &index, p_key_type);

    switch (*p_key_type)
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

    err_code = buf_dec(p_buf, packet_len, &index, pp_key, key_len, key_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    return err_code;
}

uint32_t ble_gap_auth_key_reply_rsp_enc(uint32_t         return_code,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_AUTH_KEY_REPLY, return_code, p_buf, p_buf_len);
}
