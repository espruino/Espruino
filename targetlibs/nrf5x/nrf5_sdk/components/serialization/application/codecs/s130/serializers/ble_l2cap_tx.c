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
#include "ble_l2cap_app.h"
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gap.h"
#include "app_util.h"
#include "cond_field_serialization.h"

uint32_t ble_l2cap_tx_req_enc(uint16_t                         conn_handle,
                              ble_l2cap_header_t const * const p_l2cap_header,
                              uint8_t const * const            p_data,
                              uint8_t * const                  p_buf,
                              uint32_t * const                 p_buf_len)
{
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(1, *p_buf_len);
    p_buf[index++] = SD_BLE_L2CAP_TX;

    err_code = uint16_t_enc(&conn_handle, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_l2cap_header, p_buf, *p_buf_len, &index, ble_l2cap_header_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_l2cap_header != NULL)
    {
        err_code = buf_enc(p_data, p_l2cap_header->len, p_buf, *p_buf_len, &index);
    }
    else
    {
        err_code = buf_enc(NULL, 0, p_buf, *p_buf_len, &index);
    }

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_l2cap_tx_rsp_dec(uint8_t const * const p_buf,
                              uint32_t              packet_len,
                              uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_L2CAP_TX, p_result_code);
}
