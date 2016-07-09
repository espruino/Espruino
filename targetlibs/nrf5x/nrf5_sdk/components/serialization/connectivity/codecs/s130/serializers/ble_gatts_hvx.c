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
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_util.h"

uint32_t ble_gatts_hvx_req_dec(uint8_t const * const            p_buf,
                               uint32_t                         packet_len,
                               uint16_t * const                 p_conn_handle,
                               ble_gatts_hvx_params_t * * const pp_hvx_params)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_hvx_params);
    SER_ASSERT_NOT_NULL(*pp_hvx_params);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2 + 1, packet_len);
    SER_ASSERT(p_buf[index] == SD_BLE_GATTS_HVX, NRF_ERROR_INVALID_PARAM);
    index++;

    uint16_dec(p_buf, packet_len, &index, p_conn_handle);

    uint32_t err_code = cond_field_dec(p_buf, packet_len,
                                       &index,
                                       (void * *)pp_hvx_params,
                                       ble_gatts_hvx_params_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}

uint32_t ble_gatts_hvx_rsp_enc(uint32_t               return_code,
                               uint8_t * const        p_buf,
                               uint32_t * const       p_buf_len,
                               uint16_t const * const p_bytes_written)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_HVX,
                                                        return_code,
                                                        p_buf,
                                                        p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    uint32_t index = *p_buf_len;

    err_code = cond_field_enc(p_bytes_written, p_buf, total_len, &index, uint16_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
