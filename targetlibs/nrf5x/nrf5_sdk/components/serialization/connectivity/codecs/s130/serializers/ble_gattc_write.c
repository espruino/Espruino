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
#include "ble_gattc_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include <string.h>


uint32_t ble_gattc_write_req_dec(uint8_t const * const              p_buf,
                                 uint16_t                           packet_len,
                                 uint16_t * const                   p_conn_handle,
                                 ble_gattc_write_params_t * * const pp_write_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_write_params);
    SER_ASSERT_NOT_NULL(*pp_write_params);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t index    = 0;

    uint8_t op_code;
    err_code = uint8_t_dec(p_buf, packet_len, &index, &op_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    SER_ASSERT(op_code == SD_BLE_GATTC_WRITE, NRF_ERROR_INVALID_PARAM);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              packet_len,
                              &index,
                              (void * *)pp_write_params,
                              ble_gattc_write_params_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}

uint32_t ble_gattc_write_rsp_enc(uint32_t         return_code,
                                 uint8_t * const  p_buf,
                                 uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTC_WRITE, return_code, p_buf, p_buf_len);
}
