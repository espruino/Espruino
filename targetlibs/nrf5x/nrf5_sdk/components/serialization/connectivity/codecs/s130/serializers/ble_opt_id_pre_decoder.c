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

#include "ble_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_opt_id_pre_dec(uint8_t const * const   p_buf,
                            uint16_t                packet_len,
                            uint32_t *  const       p_opt_id)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_opt_id);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 4, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_OPT_SET, NRF_ERROR_INVALID_PARAM);
    index++;

    err_code = uint32_t_dec(p_buf, packet_len, &index, p_opt_id);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}
