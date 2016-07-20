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

#include <string.h>
#include "ble_gatts_conn.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_service_changed_req_dec(uint8_t const * const p_buf,
                                           uint32_t              packet_len,
                                           uint16_t *            p_conn_handle,
                                           uint16_t *            p_start_handle,
                                           uint16_t *            p_end_handle)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_start_handle);
    SER_ASSERT_NOT_NULL(p_end_handle);

    SER_ASSERT_LENGTH_EQ(SER_CMD_HEADER_SIZE + 6, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_GATTS_SERVICE_CHANGED, NRF_ERROR_INVALID_PARAM);
    index++;

    uint16_dec(p_buf, packet_len, &index, p_conn_handle);
    uint16_dec(p_buf, packet_len, &index, p_start_handle);
    uint16_dec(p_buf, packet_len, &index, p_end_handle);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gatts_service_changed_rsp_enc(uint32_t         return_code,
                                           uint8_t * const  p_buf,
                                           uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_SERVICE_CHANGED,
                                           return_code,
                                           p_buf,
                                           p_buf_len);
}
