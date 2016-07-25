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
#include "app_util.h"

uint32_t ble_gattc_char_values_read_req_dec(uint8_t const * const p_buf,
                                            uint16_t              packet_len,
                                            uint16_t * const      p_conn_handle,
                                            uint16_t * * const    pp_handles,
                                            uint16_t * const      p_handle_count)
{
    SER_ASSERT_NOT_NULL(p_buf);          //check if *p_buf is allocated
    SER_ASSERT_NOT_NULL(p_conn_handle);  //check if *p_conn_handle exist
    SER_ASSERT_NOT_NULL(pp_handles);     //check if *p_handles exist
    SER_ASSERT_NOT_NULL(*pp_handles);    //check if p_handles exist
    SER_ASSERT_NOT_NULL(p_handle_count); //check if *p_handle_count exist

    uint32_t index = SER_CMD_DATA_POS;
    uint32_t status_code;

    SER_ASSERT_LENGTH_LEQ(5, packet_len - index); //make sure that payload length is at least 5 bytes
    uint16_dec(p_buf, packet_len, &index, p_conn_handle);
    //decode handle table count with optional handle table
    status_code = count16_cond_data16_dec(p_buf, packet_len, &index, pp_handles, p_handle_count);
    SER_ASSERT(status_code == NRF_SUCCESS, status_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return status_code;
}


uint32_t ble_gattc_char_values_read_rsp_enc(uint32_t         return_code,
                                            uint8_t * const  p_buf,
                                            uint32_t * const p_buf_len)
{
    uint32_t index = 0;

    return op_status_enc(SD_BLE_GATTC_CHAR_VALUES_READ, return_code, p_buf, p_buf_len, &index);
}
