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

#include "ble_gatts_conn.h"
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "ble_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_gatts_service_add_req_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint8_t * const       p_type,
                                       ble_uuid_t * * const  pp_uuid,
                                       uint16_t * * const    pp_handle)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_type);
    SER_ASSERT_NOT_NULL(pp_uuid);
    SER_ASSERT_NOT_NULL(*pp_uuid);
    SER_ASSERT_NOT_NULL(pp_handle);
    SER_ASSERT_NOT_NULL(*pp_handle);

    uint32_t index = SER_CMD_DATA_POS;

    uint32_t err_code;
    SER_ASSERT_LENGTH_LEQ(3, buf_len - index);
    err_code = uint8_t_dec(p_buf, buf_len, &index, p_type);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_uuid, ble_uuid_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_handle, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT(index == buf_len, NRF_ERROR_INVALID_LENGTH);

    return err_code;
}

uint32_t ble_gatts_service_add_rsp_enc(uint32_t               return_code,
                                       uint8_t * const        p_buf,
                                       uint32_t * const       p_buf_len,
                                       uint16_t const * const p_handle)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t buf_len = *p_buf_len;
    uint32_t index   = 0;
    uint32_t err_code;

    err_code = op_status_enc(SD_BLE_GATTS_SERVICE_ADD, return_code, p_buf, p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS; //this seems silly but it is not
    }
    SER_ASSERT_NOT_NULL(p_handle);
    err_code = uint16_t_enc(p_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index; //update change made by op_status_enc
    return err_code;
}
