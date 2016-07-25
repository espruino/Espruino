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

#include "ble_gap_app.h"
#include <stddef.h>
#include "nrf_error.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gap_ppcp_set_req_enc(ble_gap_conn_params_t const * const p_conn_params,
                                  uint8_t * const                     p_buf,
                                  uint32_t * const                    p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(1 + 1, *p_buf_len);

    p_buf[index++] = SD_BLE_GAP_PPCP_SET;
    p_buf[index++] = (p_conn_params != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_conn_params != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 8, *p_buf_len);
        index += uint16_encode(p_conn_params->min_conn_interval, &p_buf[index]);
        index += uint16_encode(p_conn_params->max_conn_interval, &p_buf[index]);
        index += uint16_encode(p_conn_params->slave_latency, &p_buf[index]);
        index += uint16_encode(p_conn_params->conn_sup_timeout, &p_buf[index]);
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_ppcp_set_rsp_dec(uint8_t const * const p_buf,
                                  uint32_t              packet_len,
                                  uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_PPCP_SET, p_result_code);
}
