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

#include "ble_gap_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gap_address_get_req_dec(uint8_t const * const    p_buf,
                                     uint16_t                 packet_len,
                                     ble_gap_addr_t * * const pp_address)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_address);
    SER_ASSERT_LENGTH_LEQ(1 + 1, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_GAP_ADDRESS_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_address = NULL;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gap_address_get_rsp_enc(uint32_t                     return_code,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len,
                                     ble_gap_addr_t const * const p_address)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_ADDRESS_GET, return_code,
                                                        p_buf, p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    uint32_t index = *p_buf_len;

    if (return_code != NRF_SUCCESS)
    {
        *p_buf_len = index;
        return NRF_SUCCESS;
    }

    SER_ASSERT_NOT_NULL(p_address);

    SER_ASSERT_LENGTH_LEQ(index + sizeof (ble_gap_addr_t), total_len);
    p_buf[index++] = p_address->addr_type;
    memcpy(&p_buf[index], &p_address->addr[0], BLE_GAP_ADDR_LEN);
    index += BLE_GAP_ADDR_LEN;

    *p_buf_len = index;

    return NRF_SUCCESS;
}
