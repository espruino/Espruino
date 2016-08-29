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

#include "ble_gattc_app.h"
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "cond_field_serialization.h"
#include "ble_types.h"
#include <string.h>

uint32_t ble_gattc_char_value_by_uuid_read_req_enc
    (uint16_t                              conn_handle,
    ble_uuid_t const * const               p_uuid,
    ble_gattc_handle_range_t const * const p_handle_range,
    uint8_t * const                        p_buf,
    uint32_t * const                       p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t err_code;
    uint32_t index   = 0;
    uint32_t buf_len = *p_buf_len;
    uint8_t  opcode  = SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ;

    err_code = uint8_t_enc(&opcode, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&conn_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_uuid, p_buf, buf_len, &index, ble_uuid_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_handle_range, p_buf, buf_len, &index, ble_gattc_handle_range_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_gattc_char_value_by_uuid_read_rsp_dec(uint8_t const * const p_buf,
                                                   uint32_t              packet_len,
                                                   uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf,
                               packet_len,
                               SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ,
                               p_result_code);
}
