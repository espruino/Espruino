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
#include <stdlib.h>
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gap_address_get_req_enc(ble_gap_addr_t const * const p_address,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 1, *p_buf_len);

    p_buf[index++] = SD_BLE_GAP_ADDRESS_GET;
    p_buf[index++] = (p_address == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_address_get_rsp_dec(uint8_t const * const  p_buf,
                                     uint32_t               packet_len,
                                     ble_gap_addr_t * const p_address,
                                     uint32_t * const       p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                                             &index,
                                                             packet_len,
                                                             SD_BLE_GAP_ADDRESS_GET,
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

    SER_ASSERT_LENGTH_LEQ(index + 1 + BLE_GAP_ADDR_LEN, packet_len);
    SER_ASSERT_NOT_NULL(p_address);

    p_address->addr_type = p_buf[index++];
    memcpy(p_address->addr, &p_buf[index], BLE_GAP_ADDR_LEN);
    index += BLE_GAP_ADDR_LEN;

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    return NRF_SUCCESS;
}
