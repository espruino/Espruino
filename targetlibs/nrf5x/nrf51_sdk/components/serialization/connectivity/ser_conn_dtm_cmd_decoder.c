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

#include "app_util.h"
#include "ble_dtm_conn.h"
#include "ble_serialization.h"
#include "nrf_error.h"
#include "softdevice_handler.h"
#include "ser_conn_dtm_cmd_decoder.h"
#include "ser_hal_transport.h"

static bool                          m_is_ready_to_enter_dtm = false;
static app_uart_stream_comm_params_t m_comm_params           = { 0 };

uint32_t ser_conn_dtm_command_process(uint8_t * p_command, uint16_t command_len)
{
    SER_ASSERT_NOT_NULL(p_command);

    uint32_t  err_code   = NRF_SUCCESS;
    uint8_t * p_tx_buf   = NULL;
    uint32_t  tx_buf_len = 0;

    /* Allocate a memory buffer from HAL Transport layer for transmitting the Command Response.
     * Loop until a buffer is available. */
    do
    {
        err_code = ser_hal_transport_tx_pkt_alloc(&p_tx_buf, (uint16_t *)&tx_buf_len);
    }
    while (NRF_ERROR_NO_MEM == err_code);

    if (err_code == NRF_SUCCESS)
    {
        p_tx_buf[SER_PKT_TYPE_POS] = SER_PKT_TYPE_DTM_RESP;
        tx_buf_len                -= SER_PKT_TYPE_SIZE;

        err_code = ble_dtm_init_req_dec(p_command, command_len, &m_comm_params);

        if (NRF_SUCCESS == err_code)
        {
            err_code = ble_dtm_init_rsp_enc(NRF_SUCCESS,
                                            &p_tx_buf[SER_PKT_TYPE_SIZE],
                                            &tx_buf_len);

            if (err_code != NRF_SUCCESS)
            {
                return NRF_ERROR_INTERNAL;
            }

            tx_buf_len += SER_PKT_TYPE_SIZE;

            /* Set a flag that device is ready to enter DTM mode. */
            m_is_ready_to_enter_dtm = true;

            err_code = ser_hal_transport_tx_pkt_send(p_tx_buf, (uint16_t)tx_buf_len);
            if (err_code != NRF_SUCCESS)
            {
                err_code = NRF_ERROR_INTERNAL;
            }

            /* TX buffer is going to be freed automatically in the HAL Transport layer. */
        }
        else
        {
            err_code = NRF_ERROR_INTERNAL;
        }
    }

    return err_code;
}


void ser_conn_is_ready_to_enter_dtm(void)
{
    if (m_is_ready_to_enter_dtm)
    {
        /* Disable SoftDevice. */
        (void)sd_softdevice_disable();

        /* Close HAL Transport Layer. */
        ser_hal_transport_close();

        /* Start DTM mode. */
        (void)dtm_start(m_comm_params);
    }
}
