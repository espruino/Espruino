/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "ble_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_uuid_encode_req_enc(ble_uuid_t const * const p_uuid,
                                 uint8_t const * const    p_uuid_le_len,
                                 uint8_t const * const    p_uuid_le,
                                 uint8_t * const          p_buf,
                                 uint32_t * const         p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(1 + 1, *p_buf_len);

    p_buf[index++] = SD_BLE_UUID_ENCODE;

    p_buf[index++] = (p_uuid != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_uuid != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 3, *p_buf_len);
        index         += uint16_encode(p_uuid->uuid, &p_buf[index]);
        p_buf[index++] = p_uuid->type;
    }

    SER_ASSERT_LENGTH_LEQ(index + 2, *p_buf_len);
    p_buf[index++] = (p_uuid_le_len == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;
    p_buf[index++] = (p_uuid_le == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_uuid_encode_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint8_t * const       p_uuid_le_len,
                                 uint8_t * const       p_uuid_le,
                                 uint32_t * const      p_result_code)
{
    uint32_t index = 0;
    uint32_t error_code;

    error_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                 SD_BLE_UUID_ENCODE, p_result_code);

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, packet_len);
    uint8_t uuid_le_len = p_buf[index++];

    if (p_uuid_le_len != NULL)
    {
        if (p_uuid_le != NULL)
        {
            SER_ASSERT_LENGTH_LEQ(index + uuid_le_len, packet_len);
            memcpy(p_uuid_le, &(p_buf[index]), uuid_le_len);
            index += uuid_le_len;
        }
        *p_uuid_le_len = uuid_le_len;
    }


    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}
