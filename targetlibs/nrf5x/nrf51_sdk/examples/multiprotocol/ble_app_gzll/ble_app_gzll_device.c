/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @{
 * @ingroup ble_app_gzll_gazell_part
 */

#include "ble_app_gzll_device.h"
#include <stdint.h>
#include "nordic_common.h"
#include "app_error.h"
#include "nrf_gzll.h"
#include "ble_app_gzll_ui.h"
#include "bsp.h"

#define MAX_TX_ATTEMPTS          100                   /**< Maximum number of Transmit attempts.*/
#define DUMMY_PACKET             0x80                  /**< First payload.*/
#define NB_TX_WITH_SAME_PAYLOAD  3                     /**< Numbers of times each packets is sent (this avoid changing the payload too quickly and allows to have a visible LED pattern on the receiver side).*/
#define PAYLOAD_SIZE             8                     /**< Size of the payload to send over Gazell.*/

#define GET_GAZELL_ERROR()\
    do {\
        nrf_gzll_error_code_t ERR_CODE = nrf_gzll_get_error_code();\
        UNUSED_VARIABLE(ERR_CODE);\
        APP_ERROR_HANDLER(false);\
    } while (0)

static uint8_t m_gzll_packet[8];


/**@brief Function for starting Gazell functionality.
 */
void gzll_app_start(void)
{
    bool gzll_call_ok;

    gzll_call_ok  = nrf_gzll_init(NRF_GZLL_MODE_DEVICE);
    if (!gzll_call_ok)
    {
        GET_GAZELL_ERROR();
    }
    
    gzll_call_ok = nrf_gzll_set_max_tx_attempts(MAX_TX_ATTEMPTS);
    if (!gzll_call_ok)
    {
        GET_GAZELL_ERROR();
    }
    
    gzll_call_ok = nrf_gzll_enable();
    if (!gzll_call_ok)
    {
        GET_GAZELL_ERROR();
    }

    // Add a packet to the TX FIFO to start the data transfer. 
    // From here on more data will be added through the Gazell callbacks
    m_gzll_packet[0] = DUMMY_PACKET;
    gzll_call_ok   = nrf_gzll_add_packet_to_tx_fifo(0, m_gzll_packet, PAYLOAD_SIZE);
    if (!gzll_call_ok)
    {
        GET_GAZELL_ERROR();
    }
}


void gzll_app_stop()
{
    // Disable gazell.
    nrf_gzll_disable();
    
    // Wait for Gazell to shut down
    while (nrf_gzll_is_enabled())
    {
        // Do nothing.
    }
    
    // Clean up after Gazell.
    NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_DisableIRQ(TIMER2_IRQn);   
}


/**@brief Callback function for Gazell Transmit Success. Transmits next packet.
 */
void nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
    bool       push_ok;
    static int cpt = 0;
    uint8_t    dummy[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];
    uint32_t   dummy_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
    uint32_t err_code;

    // If an ACK was received, we send another packet. 
    err_code = bsp_indication_set(BSP_INDICATE_SENT_OK);
    APP_ERROR_CHECK(err_code);

    if (tx_info.payload_received_in_ack)
    {
        // if data received attached to the ack, pop them.
        push_ok = nrf_gzll_fetch_packet_from_rx_fifo(pipe, dummy, &dummy_length);
        if (!push_ok)
        {
            GET_GAZELL_ERROR();
        }
    }

    cpt++;
    if (cpt > NB_TX_WITH_SAME_PAYLOAD)
    {
        cpt = 0;

        m_gzll_packet[0] = ~(m_gzll_packet[0]);
        if (m_gzll_packet[0] == DUMMY_PACKET)
        {
            m_gzll_packet[0] = 0x00;
        }

        m_gzll_packet[0] <<= 1;
        if (m_gzll_packet[0] == 0)
        {
            m_gzll_packet[0]++;
        }

        m_gzll_packet[0] = ~(m_gzll_packet[0]);
    }

    push_ok = nrf_gzll_add_packet_to_tx_fifo(0, m_gzll_packet, PAYLOAD_SIZE);
    if (!push_ok)
    {
        GET_GAZELL_ERROR();
    }
}


/**@brief Callback function for Gazell Transmit fail. Resends the current packet.
 */
void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
    bool     push_ok;
    uint8_t  dummy[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];
    uint32_t dummy_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
    uint32_t err_code;

    // If the transmission failed, send a new packet.
    err_code = bsp_indication_set(BSP_INDICATE_SEND_ERROR);
    APP_ERROR_CHECK(err_code);

    if (tx_info.payload_received_in_ack)
    {
        // if data received attached to the ack, pop them.
        push_ok = nrf_gzll_fetch_packet_from_rx_fifo(pipe, dummy, &dummy_length);
        if (!push_ok)
        {
            GET_GAZELL_ERROR();
        }
    }

    push_ok = nrf_gzll_add_packet_to_tx_fifo(0, m_gzll_packet, PAYLOAD_SIZE);
    if (!push_ok)
    {
        GET_GAZELL_ERROR();
    }
}


/**@brief Callback function for Gazell Receive Data Ready. Flushes the receive's FIFO.
 */
void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info)
{
    // We dont expect to receive any data in return, but if it happens we flush the RX fifo.
    bool gzll_call_ok = nrf_gzll_flush_rx_fifo(pipe);
    if (!gzll_call_ok)
    {
        GET_GAZELL_ERROR();
    }
}

/**@brief Callback function for Gazell Disabled - Not needed in this example.
 */
void nrf_gzll_disabled()
{
}

/** 
 * @}
 */
