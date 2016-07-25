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

#include "ble_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_uuid_encode_req_dec(uint8_t const * const p_buf,
                                 uint16_t              packet_len,
                                 ble_uuid_t * * const  pp_uuid,
                                 uint8_t * * const     pp_uuid_le_len,
                                 uint8_t * * const     pp_uuid_le)
{
    uint32_t index = SER_CMD_HEADER_SIZE;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_uuid);
    SER_ASSERT_NOT_NULL(pp_uuid_le_len);
    SER_ASSERT_NOT_NULL(pp_uuid_le);
    SER_ASSERT_NOT_NULL(*pp_uuid);
    SER_ASSERT_NOT_NULL(*pp_uuid_le_len);
    SER_ASSERT_NOT_NULL(*pp_uuid_le);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 3, packet_len);

    if (p_buf[index] == SER_FIELD_NOT_PRESENT)
    {
        index++;
        *pp_uuid = NULL;
    }
    else if (p_buf[index] == SER_FIELD_PRESENT)
    {
        SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 6, packet_len);
        index++;
        uint16_dec(p_buf, packet_len, &index, &((*pp_uuid)->uuid));
        (*pp_uuid)->type = p_buf[index++];
    }
    else
    {
        return NRF_ERROR_INVALID_DATA;
    }

    if (p_buf[index] == SER_FIELD_NOT_PRESENT)
    {
        *pp_uuid_le_len = NULL;
    }
    else if (p_buf[index] != SER_FIELD_PRESENT)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    index++;

    if (p_buf[index] == SER_FIELD_NOT_PRESENT)
    {
        *pp_uuid_le = NULL;
    }
    else if (p_buf[index] != SER_FIELD_PRESENT)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    SER_ASSERT_LENGTH_EQ(index + 1, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_uuid_encode_rsp_enc(uint32_t              return_code,
                                 uint8_t * const       p_buf,
                                 uint32_t * const      p_buf_len,
                                 uint8_t               uuid_le_len,
                                 uint8_t const * const p_uuid_le)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_UUID_ENCODE, return_code,
                                                        p_buf, p_buf_len);
    uint32_t index = *p_buf_len;

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, total_len);
    p_buf[index++] = uuid_le_len;

    if (p_uuid_le != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + uuid_le_len, total_len);
        memcpy(p_buf + index, p_uuid_le, uuid_le_len);
        index += uuid_le_len;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}
