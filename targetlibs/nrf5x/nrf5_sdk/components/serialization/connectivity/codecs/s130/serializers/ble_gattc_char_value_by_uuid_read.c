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

#include "ble_gattc_conn.h"
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "cond_field_serialization.h"
#include "ble_types.h"
#include <string.h>

uint32_t ble_gattc_char_value_by_uuid_read_req_dec(
    uint8_t const * const p_buf,
    uint16_t              buf_len,
    uint16_t * const      p_conn_handle,
    ble_uuid_t * * const  pp_uuid,
    ble_gattc_handle_range_t * * const
    pp_handle_range)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_uuid);
    SER_ASSERT_NOT_NULL(*pp_uuid);
    SER_ASSERT_NOT_NULL(pp_handle_range);
    SER_ASSERT_NOT_NULL(*pp_handle_range);

    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2, buf_len);
    SER_ASSERT(p_buf[index] == SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ, NRF_ERROR_INVALID_DATA);
    index++;

    uint16_dec(p_buf, buf_len, &index, p_conn_handle);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_uuid, ble_uuid_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_handle_range,
                              ble_gattc_handle_range_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}


uint32_t ble_gattc_char_value_by_uuid_read_rsp_enc(uint32_t         return_code,
                                                   uint8_t * const  p_buf,
                                                   uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ, return_code,
                                           p_buf, p_buf_len);
}

