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

#include "ble_dtm_conn.h"
#include "dtm_uart.h"
#include "nrf_error.h"
#include "ble_serialization.h"

uint32_t ble_dtm_init_req_dec(uint8_t const * const p_buf, uint16_t packet_len,
                              app_uart_stream_comm_params_t * p_comm_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_comm_params);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, packet_len, &index, &p_comm_params->tx_pin_no);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &p_comm_params->rx_pin_no);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &p_comm_params->baud_rate);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT(index == packet_len, NRF_ERROR_INVALID_LENGTH);

    return err_code;
}

uint32_t ble_dtm_init_rsp_enc(uint32_t return_code, uint8_t * const p_buf,
                              uint32_t * const p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint32_t_enc(&return_code, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
