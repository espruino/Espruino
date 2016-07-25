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

#include "ble_gap_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gap_ppcp_get_req_dec(uint8_t const * const           p_buf,
                                  uint16_t                        packet_len,
                                  ble_gap_conn_params_t * * const pp_conn_params)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_conn_params);
    SER_ASSERT_NOT_NULL(*pp_conn_params);
    SER_ASSERT_LENGTH_LEQ(2, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_GAP_PPCP_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_conn_params = NULL;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gap_ppcp_get_rsp_enc(uint32_t                            return_code,
                                  uint8_t * const                     p_buf,
                                  uint32_t * const                    p_buf_len,
                                  ble_gap_conn_params_t const * const p_conn_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_PPCP_GET, return_code,
                                                        p_buf, p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    SER_ASSERT_NOT_NULL(p_conn_params);
    uint32_t index = *p_buf_len;

    SER_ASSERT_LENGTH_LEQ(index + 8, total_len);

    index += uint16_encode(p_conn_params->min_conn_interval, &p_buf[index]);
    index += uint16_encode(p_conn_params->max_conn_interval, &p_buf[index]);
    index += uint16_encode(p_conn_params->slave_latency, &p_buf[index]);
    index += uint16_encode(p_conn_params->conn_sup_timeout, &p_buf[index]);

    *p_buf_len = index;

    return NRF_SUCCESS;
}
