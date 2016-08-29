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
#include "ble_gap_conn.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gap_disconnect_req_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint16_t * const      p_conn_handle,
                                    uint8_t * const       p_hci_status)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_hci_status);

    uint8_t  opcode;
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, packet_len, &index, &opcode);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    SER_ASSERT(opcode == SD_BLE_GAP_DISCONNECT, NRF_ERROR_INVALID_PARAM);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, p_hci_status);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}

uint32_t ble_gap_disconnect_rsp_enc(uint32_t         return_code,
                                    uint8_t * const  p_buf,
                                    uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_DISCONNECT, return_code, p_buf, p_buf_len);
}
