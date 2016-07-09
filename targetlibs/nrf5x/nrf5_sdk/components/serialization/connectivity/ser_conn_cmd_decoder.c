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
#include <string.h>
#include "nordic_common.h"
#include "app_error.h"
#include "ble_serialization.h"
#include "ser_config.h"
#include "conn_mw.h"
#include "ser_hal_transport.h"
#include "ser_conn_cmd_decoder.h"


uint32_t ser_conn_command_process(uint8_t * p_command, uint16_t command_len)
{
    SER_ASSERT_NOT_NULL(p_command);
    SER_ASSERT_LENGTH_LEQ(SER_OP_CODE_SIZE, command_len);

    uint32_t  err_code   = NRF_SUCCESS;
    uint8_t * p_tx_buf   = NULL;
    uint32_t  tx_buf_len = 0;
    uint8_t   opcode     = p_command[SER_CMD_OP_CODE_POS];
    uint32_t  index      = 0;

    /* Allocate a memory buffer from HAL Transport layer for transmitting the Command Response.
     * Loop until a buffer is available. */
    do
    {
        err_code = ser_hal_transport_tx_pkt_alloc(&p_tx_buf, (uint16_t *)&tx_buf_len);
    }
    while (NRF_ERROR_NO_MEM == err_code);

    if (NRF_SUCCESS == err_code)
    {
        /* Create a new response packet. */
        p_tx_buf[SER_PKT_TYPE_POS] = SER_PKT_TYPE_RESP;
        tx_buf_len                -= SER_PKT_TYPE_SIZE;

        /* Decode a request, pass a memory for a response command (opcode + data) and encode it. */
        err_code = conn_mw_handler
                       (p_command, command_len, &p_tx_buf[SER_PKT_OP_CODE_POS], &tx_buf_len);

        /* Command decoder not found. */
        if (NRF_ERROR_NOT_SUPPORTED == err_code)
        {
            APP_ERROR_CHECK(SER_WARNING_CODE);
            err_code = op_status_enc
                           (opcode, NRF_ERROR_NOT_SUPPORTED,
                           &p_tx_buf[SER_PKT_OP_CODE_POS], &tx_buf_len, &index);
            if (NRF_SUCCESS == err_code)
            {
                tx_buf_len += SER_PKT_TYPE_SIZE;
                err_code   = ser_hal_transport_tx_pkt_send(p_tx_buf, (uint16_t)tx_buf_len);
                /* TX buffer is going to be freed automatically in the HAL Transport layer. */
                if (NRF_SUCCESS != err_code)
                {
                    err_code = NRF_ERROR_INTERNAL;
                }
            }
            else
            {
                err_code = NRF_ERROR_INTERNAL;
            }
        }
        else if (NRF_SUCCESS == err_code) /* Send a response. */
        {
            tx_buf_len += SER_PKT_TYPE_SIZE;
            err_code    = ser_hal_transport_tx_pkt_send(p_tx_buf, (uint16_t)tx_buf_len);

            /* TX buffer is going to be freed automatically in the HAL Transport layer. */
            if (NRF_SUCCESS != err_code)
            {
                err_code = NRF_ERROR_INTERNAL;
            }
        }
        else
        {
            err_code = NRF_ERROR_INTERNAL;
        }
    }
    else
    {
        err_code = NRF_ERROR_INTERNAL;
    }

    return err_code;
}
