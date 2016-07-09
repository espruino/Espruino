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


uint32_t ble_gap_device_name_set_req_enc(ble_gap_conn_sec_mode_t const * const p_write_perm,
                                         uint8_t const * const                 p_dev_name,
                                         uint16_t                              len,
                                         uint8_t * const                       p_buf,
                                         uint32_t * const                      p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(2, *p_buf_len);
    p_buf[index++] = SD_BLE_GAP_DEVICE_NAME_SET;

    p_buf[index++] = (p_write_perm != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_write_perm != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
        p_buf[index++] = (uint8_t) ((p_write_perm->sm) | (p_write_perm->lv << 4));
    }

    SER_ERROR_CHECK(len <= BLE_GAP_DEVNAME_MAX_LEN, NRF_ERROR_INVALID_PARAM);

    SER_ASSERT_LENGTH_LEQ(index + 3, *p_buf_len);
    index += uint16_encode(len, &p_buf[index]);

    p_buf[index++] = (p_dev_name != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_dev_name != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + len, *p_buf_len);
        memcpy(&p_buf[index], p_dev_name, len);
        index += len;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_device_name_set_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_DEVICE_NAME_SET, p_result_code);
}
