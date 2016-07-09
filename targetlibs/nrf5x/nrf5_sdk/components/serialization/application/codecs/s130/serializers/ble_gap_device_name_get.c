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
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_gap_device_name_get_req_enc(uint8_t const * const  p_dev_name,
                                         uint16_t const * const p_len,
                                         uint8_t * const        p_buf,
                                         uint32_t * const       p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 2, *p_buf_len);

    p_buf[index++] = SD_BLE_GAP_DEVICE_NAME_GET;
    p_buf[index++] = (p_len != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_len != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 2, *p_buf_len);
        index += uint16_encode(*p_len, &p_buf[index]);
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
    p_buf[index++] = (p_dev_name != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_device_name_get_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint8_t * const       p_dev_name,
                                         uint16_t * const      p_dev_name_len,
                                         uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);
    uint32_t index       = 0;
    uint32_t status_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                           SD_BLE_GAP_DEVICE_NAME_GET,
                                                           p_result_code);

    if (status_code != NRF_SUCCESS)
    {
        return status_code;
    }

    if (*p_result_code != NRF_SUCCESS)
    {

        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    SER_ASSERT_NOT_NULL(p_dev_name_len);
    status_code = len16data_dec(p_buf, packet_len, &index, (uint8_t * *)&p_dev_name, p_dev_name_len);

    if (status_code != NRF_SUCCESS)
    {
        return status_code;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return status_code;
}
