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

uint32_t ble_version_get_req_dec(uint8_t const * const   p_buf,
                                 uint16_t                packet_len,
                                 ble_version_t * * const pp_version)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_version);
    SER_ASSERT_NOT_NULL(*pp_version);
    SER_ASSERT_LENGTH_EQ(SER_CMD_HEADER_SIZE + 1, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_VERSION_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_version = NULL;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_version_get_rsp_enc(uint32_t                    return_code,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len,
                                 ble_version_t const * const p_version)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_VERSION_GET, return_code,
                                                        p_buf, p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    SER_ASSERT_NOT_NULL(p_version);
    uint32_t index = *p_buf_len;

    SER_ASSERT_LENGTH_LEQ(index + 5, total_len);
    p_buf[index++] = p_version->version_number;
    index         += uint16_encode(p_version->company_id, &p_buf[index]);
    index         += uint16_encode(p_version->subversion_number, &p_buf[index]);

    *p_buf_len = index;

    return NRF_SUCCESS;
}
