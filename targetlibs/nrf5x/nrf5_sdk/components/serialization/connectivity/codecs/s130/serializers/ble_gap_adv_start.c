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
#include "cond_field_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_gap_adv_start_req_dec(uint8_t const * const          p_buf,
                                   uint32_t                       packet_len,
                                   ble_gap_adv_params_t * * const pp_adv_params)
{
    uint32_t index = 0, i = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_adv_params);
    SER_ASSERT_NOT_NULL(*pp_adv_params);
    SER_ASSERT_NOT_NULL((*pp_adv_params)->p_peer_addr);
    SER_ASSERT_NOT_NULL((*pp_adv_params)->p_whitelist);
    SER_ASSERT_NOT_NULL((*pp_adv_params)->p_whitelist->pp_addrs);
    SER_ASSERT_NOT_NULL((*pp_adv_params)->p_whitelist->pp_irks);

    for (i = 0; i < BLE_GAP_WHITELIST_ADDR_MAX_COUNT; i++)
    {
        SER_ASSERT_NOT_NULL((*pp_adv_params)->p_whitelist->pp_addrs[i]);
    }

    for (i = 0; i < BLE_GAP_WHITELIST_IRK_MAX_COUNT; i++)
    {
        SER_ASSERT_NOT_NULL((*pp_adv_params)->p_whitelist->pp_irks[i]);
    }

    /* Packet with variable length. */
    /* For now check: opcode + indicator showing if ble_gap_adv_params_t struct is present. */
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 1, packet_len);
    SER_ASSERT(p_buf[index] == SD_BLE_GAP_ADV_START, NRF_ERROR_INVALID_PARAM);
    index++;

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        err_code = uint8_t_dec(p_buf, packet_len, &index, &((*pp_adv_params)->type));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_dec(p_buf, packet_len, &index, (void **) &((*pp_adv_params)->p_peer_addr), ble_gap_addr_dec);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint8_t_dec(p_buf, packet_len, &index, &((*pp_adv_params)->fp));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_dec(p_buf, packet_len, &index, (void **) &((*pp_adv_params)->p_whitelist), ble_gap_whitelist_t_dec);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint16_t_dec(p_buf, packet_len, &index, &((*pp_adv_params)->interval));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        
        err_code = uint16_t_dec(p_buf, packet_len, &index, &((*pp_adv_params)->timeout));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = ble_gap_adv_ch_mask_t_dec(p_buf, packet_len, &index, &((*pp_adv_params)->channel_mask));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }
    else
    {
        *pp_adv_params = NULL;
    }
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}

uint32_t ble_gap_adv_start_rsp_enc(uint32_t         return_code,
                                   uint8_t * const  p_buf,
                                   uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_ADV_START, return_code, p_buf, p_buf_len);
}
