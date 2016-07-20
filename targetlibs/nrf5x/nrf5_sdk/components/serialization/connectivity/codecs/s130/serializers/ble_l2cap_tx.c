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

#include <stdlib.h>
#include <string.h>
#include "ble_l2cap_conn.h"
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_l2cap_tx_req_dec(uint8_t const * const        p_buf,
                              uint32_t const               buf_len,
                              uint16_t *                   p_conn_handle,
                              ble_l2cap_header_t * * const pp_l2cap_header,
                              uint8_t const * *            pp_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_l2cap_header);
    SER_ASSERT_NOT_NULL(*pp_l2cap_header);
    SER_ASSERT_NOT_NULL(pp_data);
    //SER_ASSERT_NOT_NULL(*pp_data);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t index    = SER_CMD_DATA_POS;

    err_code = uint16_t_dec(p_buf, buf_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_l2cap_header,
                              ble_l2cap_header_t_dec);

    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (*pp_l2cap_header != NULL)
    {
        *pp_data = p_buf + index + 1;
        index   += 1 + (*pp_l2cap_header)->len;
    }
    else
    {
        *pp_data = NULL;
        index++;
    }

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}

uint32_t ble_l2cap_tx_rsp_enc(uint32_t         return_code,
                              uint8_t * const  p_buf,
                              uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_L2CAP_TX, return_code,
                                           p_buf, p_buf_len);
}
