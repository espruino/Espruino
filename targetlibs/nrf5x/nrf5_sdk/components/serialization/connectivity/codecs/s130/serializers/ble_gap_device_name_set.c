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
#include "app_util.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"

uint32_t ble_gap_device_name_set_req_dec(uint8_t const * const             p_buf,
                                         uint32_t                          packet_len,
                                         ble_gap_conn_sec_mode_t * * const pp_write_perm,
                                         uint8_t * * const                 pp_dev_name,
                                         uint16_t * const                  p_dev_name_len)
{
    SER_ASSERT_NOT_NULL(p_buf);          //check if *p_buf is allocated
    SER_ASSERT_NOT_NULL(pp_write_perm);  //check if *pp_write_perm exist
    SER_ASSERT_NOT_NULL(pp_dev_name);    //check if *pp_dev_name exist
    SER_ASSERT_NOT_NULL(p_dev_name_len); //check if *p_dev_name_len exist

    uint32_t index = SER_CMD_DATA_POS;
    uint32_t status_code;

    SER_ASSERT_LENGTH_LEQ(4, packet_len - index); //make sure that payload is at least 4 bytes

    //decode optional write permissions field
    status_code = cond_field_dec(p_buf,
                                 packet_len,
                                 &index,
                                 (void * *)pp_write_perm,
                                 ble_gap_conn_sec_mode_dec);
    SER_ASSERT(status_code == NRF_SUCCESS, status_code);

    //decode optional device name field
    status_code = len16data_dec(p_buf, packet_len, &index, pp_dev_name, p_dev_name_len);
    SER_ASSERT(status_code == NRF_SUCCESS, status_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    return status_code;
}


uint32_t ble_gap_device_name_set_rsp_enc(uint32_t         return_code,
                                         uint8_t * const  p_buf,
                                         uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_DEVICE_NAME_SET,
                                           return_code,
                                           p_buf,
                                           p_buf_len);
}
