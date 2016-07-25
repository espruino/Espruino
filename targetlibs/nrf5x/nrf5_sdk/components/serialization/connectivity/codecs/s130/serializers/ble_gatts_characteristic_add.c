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

#include "ble_gatts_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_gatts_characteristic_add_req_dec(uint8_t const * const              p_buf,
                                              uint32_t                           buf_len,
                                              uint16_t *                         service_handle,
                                              ble_gatts_char_md_t * * const      pp_char_md,
                                              ble_gatts_attr_t * * const         pp_attr_char_value,
                                              ble_gatts_char_handles_t * * const pp_handles)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(service_handle);
    SER_ASSERT_NOT_NULL(pp_char_md);
    SER_ASSERT_NOT_NULL(pp_attr_char_value);
    SER_ASSERT_NOT_NULL(pp_handles);
    SER_ASSERT_NOT_NULL(*pp_char_md);
    SER_ASSERT_NOT_NULL(*pp_attr_char_value);
    SER_ASSERT_NOT_NULL(*pp_handles);

    uint32_t index = SER_CMD_DATA_POS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len);
    uint32_t err_code;
    SER_ASSERT_LENGTH_LEQ(2, buf_len - index);
    uint16_dec(p_buf, buf_len, &index, service_handle);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_char_md, ble_gatts_char_md_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              &index,
                              (void * *)pp_attr_char_value,
                              ble_gatts_attr_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_handles, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT(index == buf_len, NRF_ERROR_INVALID_LENGTH);

    return err_code;
}

uint32_t ble_gatts_characteristic_add_rsp_enc(uint32_t                               return_code,
                                              uint8_t * const                        p_buf,
                                              uint32_t * const                       p_buf_len,
                                              ble_gatts_char_handles_t const * const p_handles)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;
    SER_ASSERT_LENGTH_LEQ(1 + 4, total_len);

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_CHARACTERISTIC_ADD,
                                                        return_code,
                                                        p_buf,
                                                        p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (return_code == NRF_SUCCESS) /* Add value and it's length. */
    {
        err_code = cond_field_enc((void *)p_handles,
                                  p_buf,
                                  total_len,
                                  p_buf_len,
                                  ble_gatts_char_handles_enc);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }
    return err_code;
}
