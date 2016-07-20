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

#include "ble_gattc_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_gattc_attr_info_discover_req_enc(uint16_t                               conn_handle,
                                              ble_gattc_handle_range_t const * const p_handle_range,
                                              uint8_t * const                        p_buf,
                                              uint32_t *                             p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code;
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 4, *p_buf_len);

    p_buf[index++] = SD_BLE_GATTC_ATTR_INFO_DISCOVER;
    index         += uint16_encode(conn_handle, &p_buf[index]);
    
    err_code = cond_field_enc(p_handle_range, p_buf, *p_buf_len, &index, ble_gattc_handle_range_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gattc_attr_info_discover_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GATTC_ATTR_INFO_DISCOVER, p_result_code);
}
