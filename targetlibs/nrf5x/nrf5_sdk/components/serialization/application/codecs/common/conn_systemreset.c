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
 
#include <stdint.h>
#include "ble_serialization.h"
#include "ser_hal_transport.h"
#include "ser_sd_transport.h"


uint32_t conn_systemreset(void)
{
    uint32_t err_code = NRF_SUCCESS;
    uint8_t * p_tx_buf = NULL;
    uint32_t tx_buf_len = 0;

    err_code = ser_hal_transport_tx_pkt_alloc(&p_tx_buf, (uint16_t *)&tx_buf_len);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    SER_ASSERT_LENGTH_LEQ(SER_PKT_TYPE_SIZE, tx_buf_len);
    p_tx_buf[SER_PKT_TYPE_POS] = SER_PKT_TYPE_RESET_CMD;
    tx_buf_len = SER_PKT_TYPE_SIZE;

    err_code = ser_sd_transport_cmd_write(p_tx_buf, tx_buf_len, NULL);
    if (err_code != NRF_SUCCESS)
    {
        err_code = NRF_ERROR_INTERNAL;
    }

    return err_code;
}
