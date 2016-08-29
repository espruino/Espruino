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

#include "ble_gattc_conn.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_descriptors_discover_req_dec(uint8_t const * const              p_buf,
                                                uint16_t                           packet_len,
                                                uint16_t * const                   p_conn_handle,
                                                ble_gattc_handle_range_t * * const pp_handle_range)
{
    uint32_t index = SER_CMD_DATA_POS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_handle_range);
    SER_ASSERT_NOT_NULL(*pp_handle_range);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2 + 1, packet_len);

    uint16_dec(p_buf, packet_len, &index, p_conn_handle);

    switch (p_buf[index++])
    {
        case SER_FIELD_NOT_PRESENT:
            *pp_handle_range = NULL;
            break;

        case SER_FIELD_PRESENT:
            SER_ASSERT_LENGTH_LEQ(index + 2 + 2, packet_len);
            uint16_dec(p_buf, packet_len, &index, &(*pp_handle_range)->start_handle);
            uint16_dec(p_buf, packet_len, &index, &(*pp_handle_range)->end_handle);
            break;

        default:
            return NRF_ERROR_INVALID_DATA;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gattc_descriptors_discover_rsp_enc(uint32_t         return_code,
                                                uint8_t * const  p_buf,
                                                uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTC_DESCRIPTORS_DISCOVER,
                                           return_code, p_buf, p_buf_len);
}
