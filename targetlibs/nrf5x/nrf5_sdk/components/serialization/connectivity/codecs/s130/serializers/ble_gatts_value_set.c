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
#include "cond_field_serialization.h"
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_value_set_req_dec(uint8_t const * const       p_buf,
                                     uint16_t                    packet_len,
                                     uint16_t *                  p_conn_handle,
                                     uint16_t *                  p_handle,
                                     ble_gatts_value_t * * const pp_value)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_handle);
    SER_ASSERT_NOT_NULL(pp_value);

    uint32_t index = SER_CMD_DATA_POS;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(4, packet_len - index); //make sure that payload length is at least 4 bytes
    uint16_dec(p_buf, packet_len, &index, p_conn_handle);
    uint16_dec(p_buf, packet_len, &index, p_handle);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_value, ble_gatts_value_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_gatts_value_set_rsp_enc(uint32_t            return_code,
                                     uint8_t * const     p_buff,
                                     uint32_t * const    p_buff_len,
                                     ble_gatts_value_t * p_value)
{
    SER_ASSERT_NOT_NULL(p_buff);
    SER_ASSERT_NOT_NULL(p_buff_len);

    uint32_t buf_len = *p_buff_len;
    uint32_t index   = 0;
    uint32_t err_code;

    err_code = op_status_enc(SD_BLE_GATTS_VALUE_SET, return_code, p_buff, p_buff_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }
    SER_ASSERT_NOT_NULL(p_value);
    err_code = ble_gatts_value_t_enc(p_value, p_buff, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buff_len = index; //update change made by op_status_enc
    
    return err_code;
}
