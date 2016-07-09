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

#include <string.h>
#include "ble_conn.h"
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"


uint32_t ble_enable_req_dec(uint8_t const * const         p_buf,
                            uint32_t                      packet_len,
                            ble_enable_params_t * * const pp_ble_enable_params)
{
    uint32_t index = SER_CMD_DATA_POS;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_ble_enable_params);
    SER_ASSERT_NOT_NULL(*pp_ble_enable_params);
    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_ble_enable_params, ble_enable_params_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_enable_rsp_enc(uint32_t         return_code,
                            uint8_t * const  p_buf,
                            uint32_t * const p_buf_len)
{
    uint32_t index = 0;

    return op_status_enc(SD_BLE_ENABLE, return_code, p_buf, p_buf_len, &index);
}
