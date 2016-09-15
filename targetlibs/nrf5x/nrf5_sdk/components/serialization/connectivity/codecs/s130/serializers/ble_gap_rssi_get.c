/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gap_rssi_get_req_dec(uint8_t const * const p_buf,
                                  uint16_t              packet_len,
                                  uint16_t *            p_conn_handle,
                                  int8_t * * const      pp_rssi)
{
    uint32_t index = SER_CMD_DATA_POS;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_rssi);
    SER_ASSERT_NOT_NULL(*pp_rssi);
    SER_ASSERT_LENGTH_LEQ(3, packet_len);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void **) pp_rssi, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_gap_rssi_get_rsp_enc(uint32_t         return_code,
                                  uint8_t * const  p_buf,
                                  uint32_t * const p_buf_len,
                                  int8_t           rssi)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_RSSI_GET, return_code,
                                                        p_buf, p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    uint32_t index = *p_buf_len;

    err_code = uint8_t_enc((uint8_t *) &rssi, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
