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
#include "cond_field_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"


uint32_t ble_gap_address_set_req_dec(uint8_t const * const    p_buf,
                                     uint32_t                 packet_len,
                                     uint8_t *                p_addr_cycle_mode,
                                     ble_gap_addr_t * * const pp_addr)
{
    uint32_t index = SER_CMD_DATA_POS;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_addr_cycle_mode);
    SER_ASSERT_NOT_NULL(pp_addr);
    SER_ASSERT_NOT_NULL(*pp_addr);
    
    err_code = uint8_t_dec(p_buf, packet_len, &index, p_addr_cycle_mode);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_addr, ble_gap_addr_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_gap_address_set_rsp_enc(uint32_t         return_code,
                                     uint8_t * const  p_buf,
                                     uint32_t * const p_buf_len)
{
    uint32_t index = 0;

    return op_status_enc(SD_BLE_GAP_ADDRESS_SET, return_code, p_buf, p_buf_len, &index);
}
