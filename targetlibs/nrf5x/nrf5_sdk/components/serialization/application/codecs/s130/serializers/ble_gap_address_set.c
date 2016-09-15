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
#include "ble_gap.h"
#include "app_util.h"


uint32_t ble_gap_address_set_req_enc(uint8_t                      addr_cycle_mode,
                                     ble_gap_addr_t const * const p_addr,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 1 + 1, *p_buf_len);
    
    p_buf[index++] = SD_BLE_GAP_ADDRESS_SET;
    
    err_code = uint8_t_enc(&addr_cycle_mode, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    p_buf[index++] = (p_addr == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    if (p_addr != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 1 + BLE_GAP_ADDR_LEN, *p_buf_len);
        p_buf[index++] = p_addr->addr_type;
        memcpy(&p_buf[index], p_addr->addr, BLE_GAP_ADDR_LEN);
        index += BLE_GAP_ADDR_LEN;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_address_set_rsp_dec(uint8_t const * const p_buf,
                                     uint32_t              packet_len,
                                     uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_ADDRESS_SET, p_result_code);
}
