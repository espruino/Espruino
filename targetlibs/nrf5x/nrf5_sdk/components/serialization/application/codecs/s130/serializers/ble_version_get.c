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

#include "ble_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_version_get_req_enc(ble_version_t const * const p_version,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 2, *p_buf_len);

    p_buf[index++] = SD_BLE_VERSION_GET;

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);

    p_buf[index++] = (p_version == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}

uint32_t ble_version_get_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_version_t *       p_version,
                                 uint32_t * const      p_result_code)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t status_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                           SD_BLE_VERSION_GET,
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
    uint8_dec(p_buf, packet_len, &index, &(p_version->version_number));
    uint16_dec(p_buf, packet_len, &index, &(p_version->company_id));
    uint16_dec(p_buf, packet_len, &index, &p_version->subversion_number);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return status_code;
}


