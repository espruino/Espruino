/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#include "app_uart.h"
#include "nrf_drv_uart.h"
#include "nrf_assert.h"
#include "sdk_common.h"

static uint8_t tx_buffer[1];
static uint8_t rx_buffer[1];
static volatile bool rx_done;
static app_uart_event_handler_t   m_event_handler;            /**< Event handler function. */

void uart_event_handler(nrf_drv_uart_event_t * p_event, void* p_context)
{
    if (p_event->type == NRF_DRV_UART_EVT_RX_DONE)
    {
        app_uart_evt_t app_uart_event;
        app_uart_event.evt_type   = APP_UART_DATA;
        app_uart_event.data.value = p_event->data.rxtx.p_data[0];
        (void)nrf_drv_uart_rx(rx_buffer,1);
        rx_done = true;
        m_event_handler(&app_uart_event);
    }
    else if (p_event->type == NRF_DRV_UART_EVT_ERROR)
    {
        app_uart_evt_t app_uart_event;
        app_uart_event.evt_type                 = APP_UART_COMMUNICATION_ERROR;
        app_uart_event.data.error_communication = p_event->data.error.error_mask;
        (void)nrf_drv_uart_rx(rx_buffer,1);
        m_event_handler(&app_uart_event);
    }
    else if (p_event->type == NRF_DRV_UART_EVT_TX_DONE)
    {
       // Last byte from FIFO transmitted, notify the application.
       // Notify that new data is available if this was first byte put in the buffer.
       app_uart_evt_t app_uart_event;
       app_uart_event.evt_type = APP_UART_TX_EMPTY;
       m_event_handler(&app_uart_event);
    }
}

uint32_t app_uart_init(const app_uart_comm_params_t * p_comm_params,
                       app_uart_buffers_t           * p_buffers,
                       app_uart_event_handler_t       event_handler,
                       app_irq_priority_t             irq_priority)
{
    nrf_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
    config.baudrate = (nrf_uart_baudrate_t)p_comm_params->baud_rate;
    config.hwfc = (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_DISABLED) ?
            NRF_UART_HWFC_DISABLED : NRF_UART_HWFC_ENABLED;
    config.interrupt_priority = irq_priority;
    config.parity = p_comm_params->use_parity ? NRF_UART_PARITY_INCLUDED : NRF_UART_PARITY_EXCLUDED;
    config.pselcts = p_comm_params->cts_pin_no;
    config.pselrts = p_comm_params->rts_pin_no;
    config.pselrxd = p_comm_params->rx_pin_no;
    config.pseltxd = p_comm_params->tx_pin_no;

    m_event_handler = event_handler;

    rx_done = false;

    if (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_LOW_POWER)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    uint32_t err_code = nrf_drv_uart_init(&config, uart_event_handler);
    VERIFY_SUCCESS(err_code);

#ifdef NRF52
    if (!config.use_easy_dma)
#endif
    {
        nrf_drv_uart_rx_enable();
    }
    return nrf_drv_uart_rx(rx_buffer,1);
}


uint32_t app_uart_get(uint8_t * p_byte)
{
    ASSERT(p_byte);
    uint32_t err_code = NRF_SUCCESS;
    if (rx_done)
    {
        *p_byte = rx_buffer[0];
    }
    else
    {
        err_code = NRF_ERROR_NOT_FOUND;
    }
    return err_code;
}

uint32_t app_uart_put(uint8_t byte)
{
    tx_buffer[0] = byte;
    ret_code_t ret =  nrf_drv_uart_tx(tx_buffer,1);
    if (NRF_ERROR_BUSY == ret)
    {
        return NRF_ERROR_NO_MEM;
    }
    else if (ret != NRF_SUCCESS)
    {
        return NRF_ERROR_INTERNAL;
    }
    else
    {
        return NRF_SUCCESS;
    }
}

uint32_t app_uart_flush(void)
{
    return NRF_SUCCESS;
}

uint32_t app_uart_close(void)
{
    nrf_drv_uart_uninit();
    return NRF_SUCCESS;
}
