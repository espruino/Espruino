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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "ser_sd_transport.h"
#include "ser_hal_transport.h"
#include "nrf_error.h"
#include "app_error.h"
#include "ble_serialization.h"

#include "ser_app_power_system_off.h"

#include "app_util.h"

#ifdef ENABLE_DEBUG_LOG_SUPPORT
#include "app_trace.h"
#define APPL_LOG                        app_trace_log             /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */
#else
#define APPL_LOG(...)
#endif //ENABLE_DEBUG_LOG_SUPPORT

/** SoftDevice event handler. */
static ser_sd_transport_evt_handler_t m_evt_handler = NULL;

/** 'One time' handler called in task context while waiting for response to scheduled command. */
static ser_sd_transport_rsp_wait_handler_t m_ot_rsp_wait_handler = NULL;

/** Handler called in task context while waiting for response to scheduled command. */
static ser_sd_transport_rsp_wait_handler_t m_os_rsp_wait_handler = NULL;

/** Handler called in serial peripheral interrupt context when response is received. */
static ser_sd_transport_rsp_set_handler_t m_os_rsp_set_handler = NULL;

/** Handler called when hal_transport notifies that packet reception has started. */
static ser_sd_transport_rx_notification_handler_t m_rx_notify_handler = NULL;

/** User decoder handler for expected response packet. */
static ser_sd_transport_rsp_handler_t m_rsp_dec_handler = NULL;

/** Flag indicated whether module is waiting for response packet. */
static volatile bool m_rsp_wait = false;

/** SoftDevice call return value decoded by user decoder handler. */
static uint32_t m_return_value;

/**@brief Function for handling the rx packets comming from hal_transport.
 *
 * @details
 *  This function is called in serial peripheral interrupt context. Response packets are handled in
 *  this context. Events are passed to the application and it is up to application in which context
 *  they are handled.
 *
 * @param[in]   p_data   Pointer to received data.
 * @param[in]   length   Size of data.
 */
static void ser_sd_transport_rx_packet_handler(uint8_t * p_data, uint16_t length)
{
    if (p_data && (length >= SER_PKT_TYPE_SIZE))
    {
        const uint8_t packet_type = p_data[SER_PKT_TYPE_POS];
        p_data += SER_PKT_TYPE_SIZE;
        length -= SER_PKT_TYPE_SIZE;

        switch (packet_type)
        {
            case SER_PKT_TYPE_RESP:
            case SER_PKT_TYPE_DTM_RESP:

                if (m_rsp_wait)
                {
                    m_return_value = m_rsp_dec_handler(p_data, length);
                    (void)ser_sd_transport_rx_free(p_data);

                    /* Reset response flag - cmd_write function is pending on it.*/
                    m_rsp_wait = false;

                    /* If os handler is set, signal os that response has arrived.*/
                    if (m_os_rsp_set_handler)
                    {
                        m_os_rsp_set_handler();
                    }
                }
                else
                {
                    /* Unexpected packet. */
                    (void)ser_sd_transport_rx_free(p_data);
                    APP_ERROR_HANDLER(packet_type);
                }
                break;

            case SER_PKT_TYPE_EVT:
                /* It is ensured during opening that handler is not NULL. No check needed. */
                APPL_LOG("\r\n[EVT_ID]: 0x%X \r\n", uint16_decode(&p_data[SER_EVT_ID_POS])); // p_data points to EVT_ID
                m_evt_handler(p_data, length);
                break;

            default:
                (void)ser_sd_transport_rx_free(p_data);
                APP_ERROR_HANDLER(packet_type);
                break;
        }
    }
}

/**@brief Function for handling the event from hal_transport.
 *
 * @param[in]   event   Event from hal_transport.
 */
static void ser_sd_transport_hal_handler(ser_hal_transport_evt_t event)
{
    switch (event.evt_type)
    {
    case SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED:
        ser_sd_transport_rx_packet_handler(event.evt_params.rx_pkt_received.p_buffer,
                                           event.evt_params.rx_pkt_received.num_of_bytes);
        break;
    case SER_HAL_TRANSP_EVT_RX_PKT_RECEIVING:
        if (m_rx_notify_handler)
        {
            m_rx_notify_handler();
        }
        break;
    case SER_HAL_TRANSP_EVT_TX_PKT_SENT:
        if(ser_app_power_system_off_get() == true)
        {
            ser_app_power_system_off_enter();
        }
        break;
    case SER_HAL_TRANSP_EVT_PHY_ERROR:

        if (m_rsp_wait)
        {
            m_return_value = NRF_ERROR_INTERNAL;

            /* Reset response flag - cmd_write function is pending on it.*/
            m_rsp_wait = false;

            /* If os handler is set, signal os that response has arrived.*/
            if (m_os_rsp_set_handler)
            {
                m_os_rsp_set_handler();
            }
        }
        break;
    default:
        break;
    }
}

uint32_t ser_sd_transport_open(ser_sd_transport_evt_handler_t             evt_handler,
                               ser_sd_transport_rsp_wait_handler_t        os_rsp_wait_handler,
                               ser_sd_transport_rsp_set_handler_t         os_rsp_set_handler,
                               ser_sd_transport_rx_notification_handler_t rx_notify_handler)
{
    m_os_rsp_wait_handler = os_rsp_wait_handler;
    m_os_rsp_set_handler  = os_rsp_set_handler;
    m_rx_notify_handler   = rx_notify_handler;
    m_ot_rsp_wait_handler = NULL;
    m_evt_handler         = evt_handler;

    if (evt_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return ser_hal_transport_open(ser_sd_transport_hal_handler);
}

uint32_t ser_sd_transport_close(void)
{
    m_evt_handler         = NULL;
    m_os_rsp_wait_handler = NULL;
    m_os_rsp_set_handler  = NULL;
    m_ot_rsp_wait_handler = NULL;

    ser_hal_transport_close();

    return NRF_SUCCESS;
}

uint32_t ser_sd_transport_ot_rsp_wait_handler_set(ser_sd_transport_rsp_wait_handler_t handler)
{
    m_ot_rsp_wait_handler = handler;

    return NRF_SUCCESS;
}

bool ser_sd_transport_is_busy(void)
{
    return m_rsp_wait;
}

uint32_t ser_sd_transport_tx_alloc(uint8_t * * pp_data, uint16_t * p_len)
{
    uint32_t err_code;

    if (m_rsp_wait)
    {
        err_code = NRF_ERROR_BUSY;
    }
    else
    {
        err_code = ser_hal_transport_tx_pkt_alloc(pp_data, p_len);
    }
    return err_code;
}

uint32_t ser_sd_transport_tx_free(uint8_t * p_data)
{
    return ser_hal_transport_tx_pkt_free(p_data);
}

uint32_t ser_sd_transport_rx_free(uint8_t * p_data)
{
    p_data -= SER_PKT_TYPE_SIZE;
    return ser_hal_transport_rx_pkt_free(p_data);
}

uint32_t ser_sd_transport_cmd_write(const uint8_t *                p_buffer,
                                    uint16_t                       length,
                                    ser_sd_transport_rsp_handler_t cmd_rsp_decode_callback)
{
    uint32_t err_code = NRF_SUCCESS;

    m_rsp_wait        = true;
    m_rsp_dec_handler = cmd_rsp_decode_callback;
    err_code          = ser_hal_transport_tx_pkt_send(p_buffer, length);
    APP_ERROR_CHECK(err_code);

    /* Execute callback for response decoding only if one was provided.*/
    if ((err_code == NRF_SUCCESS) && cmd_rsp_decode_callback)
    {
        if (m_ot_rsp_wait_handler)
        {
            m_ot_rsp_wait_handler();
            m_ot_rsp_wait_handler = NULL;
        }

        m_os_rsp_wait_handler();
        err_code = m_return_value;
    }
    else
    {
        m_rsp_wait = false;
    }
    APPL_LOG("\r\n[SD_CALL_ID]: 0x%X, err_code= 0x%X\r\n", p_buffer[1], err_code);
    return err_code;
}
