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
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"

uint32_t ble_gap_encrypt_req_dec(uint8_t        const * const	p_buf,
                                 uint16_t                    	packet_len,
                                 uint16_t 					  *	const p_conn_handle,
                                 ble_gap_master_id_t ** const	pp_master_id,
                                 ble_gap_enc_info_t  ** const	pp_enc_info)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_enc_info);
    SER_ASSERT_NOT_NULL(pp_master_id);
    SER_ASSERT_NOT_NULL(*pp_enc_info);
    SER_ASSERT_NOT_NULL(*pp_master_id);

    uint32_t index    = SER_CMD_DATA_POS;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(2, packet_len - index);
    uint16_dec(p_buf, packet_len, &index, p_conn_handle);
    err_code = cond_field_dec(p_buf,
                              packet_len,
                              &index,
                              (void **)pp_master_id,
                              ble_gap_master_id_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              packet_len,
                              &index,
                              (void **)pp_enc_info,
                              ble_gap_enc_info_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_encrypt_rsp_enc(uint32_t          return_code,
                                 uint8_t   * const p_buf,
                                 uint32_t  * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_ENCRYPT, return_code, p_buf, p_buf_len);
}
