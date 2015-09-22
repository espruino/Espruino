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

#include <string.h>
#include <stdint.h>
#include "app_error.h"
#include "app_scheduler.h"
#include "ble_conn.h"
#include "ble_serialization.h"
#include "ser_config.h"
#include "ser_hal_transport.h"
#include "ser_conn_event_encoder.h"


void ser_conn_ble_event_encoder(void * p_event_data, uint16_t event_size)
{
    if (NULL == p_event_data)
    {
        APP_ERROR_CHECK(NRF_ERROR_NULL);
    }
    UNUSED_PARAMETER(event_size);

    uint32_t    err_code   = NRF_SUCCESS;
    uint8_t *   p_tx_buf   = NULL;
    uint32_t    tx_buf_len = 0;
    ble_evt_t * p_ble_evt  = (ble_evt_t *)p_event_data;

    /* Allocate a memory buffer from HAL Transport layer for transmitting an event.
     * Loop until a buffer is available. */
    do
    {
        err_code = ser_hal_transport_tx_pkt_alloc(&p_tx_buf, (uint16_t *)&tx_buf_len);
    }
    while (err_code == NRF_ERROR_NO_MEM);
    APP_ERROR_CHECK(err_code);

    /* Create a new packet. */
    p_tx_buf[SER_PKT_TYPE_POS] = SER_PKT_TYPE_EVT;
    tx_buf_len                -= SER_PKT_TYPE_SIZE;

    /* Pass a memory for an event (opcode + data) and encode it. */
    err_code = ble_event_enc(p_ble_evt, 0, &p_tx_buf[SER_PKT_OP_CODE_POS], &tx_buf_len);

    if (NRF_ERROR_NOT_SUPPORTED != err_code)
    {
        APP_ERROR_CHECK(err_code);
        tx_buf_len += SER_PKT_TYPE_SIZE;
        err_code    = ser_hal_transport_tx_pkt_send(p_tx_buf, (uint16_t)tx_buf_len);
        APP_ERROR_CHECK(err_code);
        /* TX buffer is going to be freed automatically in the HAL Transport layer.
         * Scheduler must be paused because this function returns before a packet is physically sent
         * by transport layer. This can cause start processing of a next event from the application
         * scheduler queue. In result the next event reserves the TX buffer before the current
         * packet is sent. If in meantime a command arrives a command response cannot be sent in
         * result. Pausing the scheduler temporary prevents processing a next event. */
        app_sched_pause();
    }
    else
    {
        /* Event was NOT encoded, therefore the buffer is freed immediately. */
        err_code = ser_hal_transport_tx_pkt_free(p_tx_buf);
        APP_ERROR_CHECK(err_code);
        APP_ERROR_CHECK(SER_WARNING_CODE);
    }
}

