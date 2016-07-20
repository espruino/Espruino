/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gatts_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gatts.h"
#include "app_util.h"
#include "cond_field_serialization.h"
#include "ble_gatts_struct_serialization.h"

uint32_t ble_gatts_characteristic_add_req_enc(
    uint16_t                               service_handle,
    ble_gatts_char_md_t const * const      p_char_md,
    ble_gatts_attr_t const * const         p_attr_char_value,
    ble_gatts_char_handles_t const * const p_handles,
    uint8_t * const                        p_buf,
    uint32_t * const                       p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  opcode   = SD_BLE_GATTS_CHARACTERISTIC_ADD;
    uint32_t buf_len  = *p_buf_len;

    err_code = uint8_t_enc(&opcode, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&service_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_char_md, p_buf, buf_len, &index, ble_gatts_char_md_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_attr_char_value, p_buf, buf_len, &index, ble_gatts_attr_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_handles, p_buf, buf_len, &index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_gatts_characteristic_add_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              buf_len,
                                              uint16_t * * const    pp_handles,
                                              uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t index = 0;

    uint32_t err_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, buf_len,
                                                        SD_BLE_GATTS_CHARACTERISTIC_ADD,
                                                        p_result_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, buf_len);
        return NRF_SUCCESS;
    }

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              &index,
                              (void * *)pp_handles,
                              ble_gatts_char_handles_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}
