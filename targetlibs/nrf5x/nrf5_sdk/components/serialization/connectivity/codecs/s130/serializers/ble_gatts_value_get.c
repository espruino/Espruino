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

#include "ble_gatts_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_value_get_req_dec(uint8_t const * const       p_buf,
                                     uint16_t                    packet_len,
                                     uint16_t * const            p_conn_handle,
                                     uint16_t * const            p_handle,
                                     ble_gatts_value_t * * const pp_value)
{
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_handle);
    SER_ASSERT_NOT_NULL(pp_value);
    SER_ASSERT_NOT_NULL(*pp_value);
    SER_ASSERT_NOT_NULL((*pp_value)->p_value);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 5, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_GATTS_VALUE_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        err_code = uint16_t_dec(p_buf, packet_len, &index, &((*pp_value)->len));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint16_t_dec(p_buf, packet_len, &index, &((*pp_value)->offset));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        SER_ASSERT_LENGTH_LEQ(1, packet_len - index);
        if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
        {
            (*pp_value)->p_value = NULL;
        }
    }
    else
    {
        *pp_value = NULL;
    }

    return err_code;
}


uint32_t ble_gatts_value_get_rsp_enc(uint32_t                   return_code,
                                     uint8_t * const            p_buf,
                                     uint32_t * const           p_buf_len,
                                     ble_gatts_value_t * const  p_value)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_RSP_HEADER_SIZE, *p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_VALUE_GET,
                                                        return_code,
                                                        p_buf,
                                                        p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    uint32_t index = *p_buf_len;

    if (return_code == NRF_SUCCESS)
    {
        SER_ASSERT_NOT_NULL(p_value);

        err_code = ble_gatts_value_t_enc(p_value, p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    *p_buf_len = index;

    return err_code;
}
