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

#include "ble_gap_app.h"
#include <stdlib.h>
#include <string.h>
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_gap_rssi_get_req_enc(uint16_t             conn_handle,
                                  int8_t const * const p_rssi,
                                  uint8_t  * const     p_buf,
                                  uint32_t * const     p_buf_len)
{
    uint32_t index     = 0;
    uint32_t err_code  = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;
    SER_ASSERT_LENGTH_LEQ(index + 1, total_len);

    p_buf[index++] = SD_BLE_GAP_RSSI_GET;

    err_code = uint16_t_enc(&conn_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_rssi, p_buf, total_len, &index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_gap_rssi_get_rsp_dec(uint8_t const * const p_buf,
                                  uint32_t              packet_len,
                                  int8_t * const        p_rssi,
                                  uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                                             &index,
                                                             packet_len,
                                                             SD_BLE_GAP_RSSI_GET,
                                                             p_result_code);

    if (decode_result != NRF_SUCCESS)
    {
        return decode_result;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    decode_result = uint8_t_dec(p_buf, packet_len, &index, (uint8_t *) p_rssi);
    SER_ASSERT(decode_result == NRF_SUCCESS, decode_result);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return decode_result;
}
