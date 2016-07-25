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

#include "ble_gap_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"
#include "cond_field_serialization.h"
#include "ble_gap_struct_serialization.h"


uint32_t ble_gap_conn_sec_get_req_dec(uint8_t const * const        p_buf,
                                      uint32_t                     packet_len,
                                      uint16_t *                   p_conn_handle,
                                      ble_gap_conn_sec_t * * const pp_conn_sec)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);

    uint32_t index = 0;
    uint8_t  opcode;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, packet_len - index);
    uint8_dec(p_buf, packet_len, &index, &opcode);

    SER_ASSERT(opcode == SD_BLE_GAP_CONN_SEC_GET, NRF_ERROR_INVALID_PARAM);

    SER_ASSERT_LENGTH_LEQ(2, packet_len - index);
    uint16_dec(p_buf, packet_len, &index, p_conn_handle);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_conn_sec, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}

uint32_t ble_gap_conn_sec_get_rsp_enc(uint32_t                   return_code,
                                      ble_gap_conn_sec_t * const p_conn_sec,
                                      uint8_t * const            p_buf,
                                      uint32_t * const           p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t buflen   = *p_buf_len;
    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_CONN_SEC_GET, return_code,
                                                        p_buf, p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t index = *p_buf_len;

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    err_code = cond_field_enc(p_conn_sec, p_buf, buflen, &index, ble_gap_conn_sec_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
