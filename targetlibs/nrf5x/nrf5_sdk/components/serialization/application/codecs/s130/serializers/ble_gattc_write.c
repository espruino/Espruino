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
#include "app_util.h"

uint32_t ble_gattc_write_req_enc(uint16_t                               conn_handle,
                                 ble_gattc_write_params_t const * const p_write_params,
                                 uint8_t * const                        p_buf,
                                 uint32_t *                             p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 4, *p_buf_len);

    p_buf[index++] = SD_BLE_GATTC_WRITE;
    index         += uint16_encode(conn_handle, &p_buf[index]);
    p_buf[index++] = (p_write_params == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    if (p_write_params != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 9, *p_buf_len);
        p_buf[index++] = p_write_params->write_op;
        p_buf[index++] = p_write_params->flags;
        index         += uint16_encode(p_write_params->handle, &p_buf[index]);
        index         += uint16_encode(p_write_params->offset, &p_buf[index]);
        index         += uint16_encode(p_write_params->len, &p_buf[index]);

        SER_ERROR_CHECK(p_write_params->len <= BLE_GATTC_WRITE_P_VALUE_LEN_MAX,
                        NRF_ERROR_INVALID_PARAM);

        p_buf[index++] = (p_write_params->p_value == NULL) ?
                         SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

        if (p_write_params->p_value != NULL)
        {
            SER_ASSERT_LENGTH_LEQ(index + p_write_params->len, *p_buf_len);
            memcpy(&p_buf[index], p_write_params->p_value, p_write_params->len);
            index += p_write_params->len;
        }
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gattc_write_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GATTC_WRITE, p_result_code);
}
