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


#include "ble_gatts_app.h"
#include <stdlib.h>
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap.h"
#include "app_util.h"
#include "cond_field_serialization.h"
#include "ble_gatts_struct_serialization.h"


uint32_t ble_gatts_descriptor_add_req_enc(uint16_t                       char_handle,
                                          ble_gatts_attr_t const * const p_attr,
                                          uint16_t * const               p_handle,
                                          uint8_t * const                p_buf,
                                          uint32_t * const               p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(1, *p_buf_len);
    p_buf[index++] = SD_BLE_GATTS_DESCRIPTOR_ADD;

    uint32_t err_code = uint16_t_enc(&char_handle, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_attr,
                              p_buf,
                              *p_buf_len,
                              &index,
                              ble_gatts_attr_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
    p_buf[index++] = (p_handle != NULL) ? SER_FIELD_PRESENT :
                     SER_FIELD_NOT_PRESENT;

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_gatts_descriptor_add_rsp_dec(uint8_t const * const p_buf,
                                          uint32_t              packet_len,
                                          uint16_t * const      p_handle,
                                          uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                                             &index,
                                                             packet_len,
                                                             SD_BLE_GATTS_DESCRIPTOR_ADD,
                                                             p_result_code);

    if (decode_result != NRF_SUCCESS)
    {
        return decode_result;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    SER_ASSERT_LENGTH_LEQ(index + 2, packet_len);
    SER_ASSERT_NOT_NULL(p_handle);

    uint16_dec(p_buf, packet_len, &index, p_handle);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}
