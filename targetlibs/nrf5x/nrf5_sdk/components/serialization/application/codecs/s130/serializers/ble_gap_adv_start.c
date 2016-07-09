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

#include "ble_gap_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "ble_gap.h"
#include "app_util.h"

uint32_t ble_gap_adv_start_req_enc(ble_gap_adv_params_t const * const p_adv_params,
                                   uint8_t * const                    p_buf,
                                   uint32_t * const                   p_buf_len)
{
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    SER_ASSERT_LENGTH_LEQ(index + 2, total_len);
    p_buf[index++] = SD_BLE_GAP_ADV_START;
    p_buf[index++] = (p_adv_params == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    if (p_adv_params != NULL)
    {
        err_code = uint8_t_enc(&(p_adv_params->type), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_enc(p_adv_params->p_peer_addr, p_buf, total_len, &index, ble_gap_addr_enc);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint8_t_enc(&(p_adv_params->fp), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_enc(p_adv_params->p_whitelist, p_buf, total_len, &index, ble_gap_whitelist_t_enc);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint16_t_enc(&(p_adv_params->interval), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        
        err_code = uint16_t_enc(&(p_adv_params->timeout), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = ble_gap_adv_ch_mask_t_enc(&(p_adv_params->channel_mask), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_gap_adv_start_rsp_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_ADV_START, p_result_code);
}
