
/* Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of other
 * contributors to this software may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * 4. This software must only be used in a processor manufactured by Nordic
 * Semiconductor ASA, or in a processor manufactured by a third party that
 * is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ser_phy.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "app_util.h"
#include "app_uart.h"
#include "app_error.h"

#ifdef SER_CONNECTIVITY
#include "ser_phy_config_conn_nrf51.h"
#else
#include "ser_phy_config_app_nrf51.h"
#endif /* SER_CONNECTIVITY */

#include "boards.h"

#ifdef NRF51
#define SER_UART_IRQ UART0_IRQn
#elif defined NRF52
#define SER_UART_IRQ UARTE0_UART0_IRQn
#endif /* NRF51 */

static uint8_t * mp_tx_stream;                         /**< Pointer to Tx data */
static uint16_t  m_tx_stream_length;                   /**< Length of Tx data including SER_PHY
                                                        *   header */
static uint16_t  m_tx_stream_index;                    /**< Byte index in Tx data */
static uint8_t   m_tx_length_buf[SER_PHY_HEADER_SIZE]; /**< Buffer for header of Tx packet */

static uint8_t * mp_rx_stream;                         /**< Pointer to Rx buffer */
static uint16_t  m_rx_stream_length;                   /**< Length of Rx data including SER_PHY
                                                        *   header*/
static uint16_t  m_rx_stream_index;                    /**< Byte index in Rx data */
static uint8_t   m_rx_length_buf[SER_PHY_HEADER_SIZE]; /**< Buffer for header of Rx packet */
static uint8_t   m_rx_drop_buf[1];                     /**< 1-byte buffer used to trash incoming
                                                        *   data */
static uint8_t   m_rx_byte;                            /**< Rx byte passed from low-level driver */

static ser_phy_events_handler_t m_ser_phy_event_handler; /**< Event handler for upper layer */
static ser_phy_evt_t            m_ser_phy_rx_event;      /**< Rx event for upper layer
                                                          *   notification */
static ser_phy_evt_t            m_ser_phy_tx_event;      /**< Tx event for upper layer
                                                          *   notification */

static bool m_other_side_active = false; /* Flag indicating that the other side is running */

/**
 *@breif UART configuration structure, values are defined in SER_PHY config files:
 *  ser_phy_config_conn_nrf51.h for connectivity and ser_phy_config_app_nrf51.h for application.
 */
static const app_uart_comm_params_t comm_params =
{
    .rx_pin_no  = SER_PHY_UART_RX,
    .tx_pin_no  = SER_PHY_UART_TX,
    .rts_pin_no = SER_PHY_UART_RTS,
    .cts_pin_no = SER_PHY_UART_CTS,
    //Below values are defined in ser_config.h common for application and connectivity
    .flow_control = SER_PHY_UART_FLOW_CTRL,
    .use_parity   = SER_PHY_UART_PARITY,
    .baud_rate    = SER_PHY_UART_BAUDRATE
};

/** FUNCTION DECLARATIONS */

static __INLINE void callback_ser_phy_event(ser_phy_evt_t event);
static __INLINE void callback_packet_received(void);
static __INLINE void callback_packet_sent(void);
static __INLINE void callback_mem_request(void);
static __INLINE void callback_hw_error(uint32_t error_src);

static void ser_phy_uart_tx(void);
static void ser_phy_uart_rx(uint8_t rx_byte);
static void ser_phy_uart_evt_callback(app_uart_evt_t * uart_evt);


/** STATIC FUNCTION DEFINITIONS */

/**
 *@brief Callback for calling ser phy event handler to notify higher layer with appropriate event.
 * Handler is called only wen it was previously registered.
 */
static __INLINE void callback_ser_phy_event(ser_phy_evt_t event)
{
    if (m_ser_phy_event_handler)
    {
        m_ser_phy_event_handler(event);
    }
    return;
}

/**
 *@brief Callback for notifying upper layer that either a packet was succesfully received or it was
 * dropped.
 */
static __INLINE void callback_packet_received(void)
{
    if (mp_rx_stream == m_rx_drop_buf)
    {
        m_ser_phy_rx_event.evt_type = SER_PHY_EVT_RX_PKT_DROPPED;
    }
    else
    {
        m_ser_phy_rx_event.evt_type = SER_PHY_EVT_RX_PKT_RECEIVED;
        m_ser_phy_rx_event.evt_params.rx_pkt_received.num_of_bytes =
            m_rx_stream_index - SER_PHY_HEADER_SIZE;
        m_ser_phy_rx_event.evt_params.rx_pkt_received.p_buffer =
            mp_rx_stream;
    }

    mp_rx_stream       = NULL;
    m_rx_stream_length = 0;
    m_rx_stream_index  = 0;

    callback_ser_phy_event(m_ser_phy_rx_event);
}

/**
 *@brief Callback for notifying upper layer that a packet was succesfully transmitted
 */
static __INLINE void callback_packet_sent(void)
{
    mp_tx_stream       = NULL;
    m_tx_stream_length = 0;
    m_tx_stream_index  = 0;

    m_ser_phy_tx_event.evt_type = SER_PHY_EVT_TX_PKT_SENT;

    callback_ser_phy_event(m_ser_phy_tx_event);
}

/**
 *@brief Callback for requesting from upper layer memory for an incomming packet.
 */
static __INLINE void callback_mem_request(void)
{
    m_rx_stream_length          = uint16_decode(m_rx_length_buf) + SER_PHY_HEADER_SIZE;
    m_ser_phy_rx_event.evt_type = SER_PHY_EVT_RX_BUF_REQUEST;
    m_ser_phy_rx_event.evt_params.rx_buf_request.num_of_bytes =
        m_rx_stream_length - SER_PHY_HEADER_SIZE;

    callback_ser_phy_event(m_ser_phy_rx_event);
}

/**
 *@brief Callback for notifying upper layer of a HW error that occured.
 */
static __INLINE void callback_hw_error(uint32_t error_src)
{
    mp_rx_stream                = NULL;
    m_rx_stream_length          = 0;
    m_rx_stream_index           = 0;
    m_ser_phy_rx_event.evt_type = SER_PHY_EVT_HW_ERROR;

    //Pass error source to upper layer
    m_ser_phy_rx_event.evt_params.hw_error.error_code = error_src;
    callback_ser_phy_event(m_ser_phy_rx_event);
}

/**
 *@brief Function for handling Tx procedure.
 */
static void ser_phy_uart_tx(void)
{
    if (mp_tx_stream != NULL)
    {
        bool     tx_done_flag = false;       /**< Local flag for indicating that TX is completed */
        uint32_t err_code     = NRF_SUCCESS; /**< Error code for storing result of app_uart_put */

        //Blocking TXRDY interrupt is done to avoid interrupting when this procedure is
        //triggered from main context
        NRF_UART0->INTENCLR = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos);

        //Notify upper layer if whole packet has been transmitted
        if (m_tx_stream_index == m_tx_stream_length)
        {
            callback_packet_sent();
            tx_done_flag = true;
        }
        //First transmit 2 bytes of packet length
        else if (m_tx_stream_index < SER_PHY_HEADER_SIZE)
        {
            err_code = app_uart_put(m_tx_length_buf[m_tx_stream_index]);
        }
        //Then transmit payload
        else if (m_tx_stream_index < m_tx_stream_length)
        {
            err_code = app_uart_put(mp_tx_stream[m_tx_stream_index - SER_PHY_HEADER_SIZE]);
        }

        //Increment index only if byte was sent without errors
        if ((err_code == NRF_SUCCESS) && !tx_done_flag)
        {
            m_tx_stream_index++;
        }

        //Unblock TXRDY interrupts
        NRF_UART0->INTENSET = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos);
    }

}

/**
 *@brief Function for handling Rx procedure.
 */
static void ser_phy_uart_rx(uint8_t rx_byte)
{

    if (mp_rx_stream == NULL )
    {
        //Receive length value and request rx buffer from higher layer
        if (m_rx_stream_index < SER_PHY_HEADER_SIZE)
        {
            m_rx_length_buf[m_rx_stream_index++] = rx_byte;

            if (m_rx_stream_index == SER_PHY_HEADER_SIZE)
            {
                //Block RXRDY interrupts at this point to not handle incoming bytes until upper
                //layer provides memory for payload
                NRF_UART0->INTENCLR = (UART_INTENCLR_RXDRDY_Clear << UART_INTENCLR_RXDRDY_Pos);

                //Request rx buffer from upper layer
                callback_mem_request();
            }
        }
    }
    else if (m_rx_stream_index < m_rx_stream_length)
    {
        //Receive or drop payload
        if (mp_rx_stream == m_rx_drop_buf)
        {
            //Drop incoming data to the one-element drop buffer
            *mp_rx_stream = rx_byte;
            m_rx_stream_index++;
        }
        else
        {
            mp_rx_stream[m_rx_stream_index - SER_PHY_HEADER_SIZE] = rx_byte;
            m_rx_stream_index++;
        }
    }

    //Process RX packet, notify higher layer
    if (m_rx_stream_index == m_rx_stream_length)
    {
        callback_packet_received();
    }
}

/**
 *@brief Callback for processing events from low-level UART driver.
 */
static void ser_phy_uart_evt_callback(app_uart_evt_t * uart_evt)
{
    if (uart_evt == NULL)
    {
        return;
    }

    switch (uart_evt->evt_type)
    {
        case APP_UART_COMMUNICATION_ERROR:

            //Process error only if this is parity or overrun error.
            //Break and framing error is always present when app side is not active
            if (uart_evt->data.error_communication &
                (UART_ERRORSRC_PARITY_Msk | UART_ERRORSRC_OVERRUN_Msk))
            {
                callback_hw_error(uart_evt->data.error_communication);
            }
            break;

        case APP_UART_TX_EMPTY:
            ser_phy_uart_tx();
            break;

        case APP_UART_DATA:

            //After first reception disable pulldown - it was only needed before start
            //of the other side
            if (!m_other_side_active)
            {
                nrf_gpio_cfg_input(comm_params.rx_pin_no, NRF_GPIO_PIN_NOPULL);
                m_other_side_active = true;
            }

            m_rx_byte = uart_evt->data.value;
            ser_phy_uart_rx(m_rx_byte);
            break;

        default:
            APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
            break;
    }
}

/** API FUNCTIONS */

uint32_t ser_phy_open(ser_phy_events_handler_t events_handler)
{
    uint32_t err_code = NRF_SUCCESS;

    if (events_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }

    //Check if function was not called before
    if (m_ser_phy_event_handler != NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    //Configure UART and register handler
    //uart_evt_handler is used to handle events produced by low-level uart driver
    APP_UART_INIT(&comm_params, ser_phy_uart_evt_callback, UART_IRQ_PRIORITY, err_code);

//    //Pull down Rx pin until another side gets up to avoid receiving false bytes due to glitches
//    //on Rx line
//    nrf_gpio_cfg_input(comm_params.rx_pin_no, NRF_GPIO_PIN_PULLDOWN);

    m_ser_phy_event_handler = events_handler;

    //If intialization did not go alright return error
    if (err_code != NRF_SUCCESS)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return err_code;
}



uint32_t ser_phy_tx_pkt_send(const uint8_t * p_buffer, uint16_t num_of_bytes)
{
    if (p_buffer == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (num_of_bytes == 0)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    //Check if there is no ongoing transmission at the moment
    if ((mp_tx_stream == NULL) && (m_tx_stream_length == 0) && (m_tx_stream_index == 0))
    {
        (void) uint16_encode(num_of_bytes, m_tx_length_buf);
        mp_tx_stream       = (uint8_t *)p_buffer;
        m_tx_stream_length = num_of_bytes + SER_PHY_HEADER_SIZE;

        //Call tx procedure to start transmission of a packet
        ser_phy_uart_tx();
    }
    else
    {
        return NRF_ERROR_BUSY;
    }

    return NRF_SUCCESS;
}

uint32_t ser_phy_rx_buf_set(uint8_t * p_buffer)
{

    if (m_ser_phy_rx_event.evt_type != SER_PHY_EVT_RX_BUF_REQUEST)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_buffer != NULL)
    {
        mp_rx_stream = p_buffer;
    }
    else
    {
        mp_rx_stream = m_rx_drop_buf;
    }

    //Unblock RXRDY interrupts as higher layer has responded (with a valid or NULL pointer)
    NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos);

    return NRF_SUCCESS;
}

void ser_phy_close(void)
{
    m_ser_phy_event_handler = NULL;
    (void)app_uart_close();
}

void ser_phy_interrupts_enable(void)
{
    NVIC_EnableIRQ(SER_UART_IRQ);
}

void ser_phy_interrupts_disable(void)
{
    NVIC_DisableIRQ(SER_UART_IRQ);
}

