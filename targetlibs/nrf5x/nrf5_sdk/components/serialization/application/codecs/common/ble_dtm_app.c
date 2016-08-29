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
#include "app_error.h"
#include "ble_dtm_app.h"
#include "ble_serialization.h"
#include "nrf_error.h"
#include "ser_config.h"
#include "ser_hal_transport.h"
#include "ser_sd_transport.h"


static uint32_t dtm_init_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_dtm_init_rsp_dec(p_buffer, length, &result_code);
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t ble_dtm_init(app_uart_stream_comm_params_t * p_uart_comm_params)
{
    if (p_uart_comm_params == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    uint32_t err_code = NRF_SUCCESS;
    uint32_t index = 0;
    
    uint8_t * p_tx_buf = NULL;
    uint32_t tx_buf_len = 0;

    err_code = ser_hal_transport_tx_pkt_alloc(&p_tx_buf, (uint16_t *)&tx_buf_len);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    p_tx_buf[index++] = SER_PKT_TYPE_DTM_CMD;
    tx_buf_len -= SER_PKT_TYPE_SIZE;

    err_code = ble_dtm_init_req_enc(p_uart_comm_params, &(p_tx_buf[SER_PKT_TYPE_SIZE]), &tx_buf_len);
    if (err_code == NRF_SUCCESS)
    {
        tx_buf_len += SER_PKT_TYPE_SIZE;

        err_code = ser_sd_transport_cmd_write(p_tx_buf, tx_buf_len, dtm_init_rsp_dec);
        if (err_code != NRF_SUCCESS)
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
