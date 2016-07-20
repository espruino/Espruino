
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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "nrf_error.h"
#include "nrf_gpio.h"
#include "app_uart.h"
#include "ser_phy_hci.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_soc.h"

#ifdef SER_CONNECTIVITY
#include "ser_phy_config_conn_nrf51.h"
#else
#include "ser_phy_config_app_nrf51.h"
#endif /* SER_CONNECTIVITY */

#include "ser_config.h"
#define APP_SLIP_END     0xC0 /**< SLIP code for identifying the beginning and end of a packet frame.. */
#define APP_SLIP_ESC     0xDB /**< SLIP escape code. This code is used to specify that the following character is specially encoded. */
#define APP_SLIP_ESC_END 0xDC /**< SLIP special code. When this code follows 0xDB, this character is interpreted as payload data 0xC0.. */
#define APP_SLIP_ESC_ESC 0xDD /**< SLIP special code. When this code follows 0xDB, this character is interpreted as payload data 0xDB. */

#define HDR_SIZE 4
#define CRC_SIZE 2
#define PKT_SIZE (SER_HAL_TRANSPORT_MAX_PKT_SIZE + HDR_SIZE + CRC_SIZE)

static const app_uart_comm_params_t comm_params =
{
    .rx_pin_no  = SER_PHY_UART_RX,
    .tx_pin_no  = SER_PHY_UART_TX,
    .rts_pin_no = SER_PHY_UART_RTS,
    .cts_pin_no = SER_PHY_UART_CTS,
    // Below values are defined in ser_config.h common for application and connectivity
    .flow_control = SER_PHY_UART_FLOW_CTRL,
    .use_parity   = SER_PHY_UART_PARITY,
    .baud_rate    = SER_PHY_UART_BAUDRATE
};

static uint8_t m_small_buffer[HDR_SIZE];
static uint8_t m_big_buffer[PKT_SIZE];

static uint8_t * mp_small_buffer = NULL;
static uint8_t * mp_big_buffer   = NULL;
static uint8_t * mp_buffer       = NULL;

static ser_phy_hci_pkt_params_t m_header;
static ser_phy_hci_pkt_params_t m_payload;
static ser_phy_hci_pkt_params_t m_crc;
static ser_phy_hci_pkt_params_t m_header_pending;
static ser_phy_hci_pkt_params_t m_payload_pending;
static ser_phy_hci_pkt_params_t m_crc_pending;

static ser_phy_hci_slip_evt_t           m_ser_phy_hci_slip_event;
static ser_phy_hci_slip_event_handler_t m_ser_phy_hci_slip_event_handler; /**< Event handler for upper layer */

static bool    m_other_side_active = false; /**< Flag indicating that the other side is running */
static uint8_t m_rx_byte;                   /**< Rx byte passed from low-level driver */

static bool m_rx_escape = false;
static bool m_tx_escape = false;

static bool m_tx_busy = false; /**< Flag indicating that currently some transmission is ongoing */

static uint32_t                   m_tx_index;
static uint32_t                   m_rx_index;
static ser_phy_hci_pkt_params_t * mp_data = NULL;

/* Function declarations */
static uint32_t ser_phy_hci_tx_byte(void);
static bool     slip_decode(uint8_t * p_received_byte);
static void     ser_phi_hci_rx_byte(uint8_t rx_byte);
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

__STATIC_INLINE void callback_hw_error(uint32_t error_src)
{
    m_ser_phy_hci_slip_event.evt_type = SER_PHY_HCI_SLIP_EVT_HW_ERROR;

    // Pass error source to upper layer
    m_ser_phy_hci_slip_event.evt_params.hw_error.error_code = error_src;
    m_ser_phy_hci_slip_event_handler(&m_ser_phy_hci_slip_event);
}


__STATIC_INLINE void slip_encode(void)
{
    switch (mp_data->p_buffer[m_tx_index])
    {
        case APP_SLIP_END:
            m_tx_escape = true;
            (void)app_uart_put(APP_SLIP_ESC);
            break;

        case APP_SLIP_ESC:
            m_tx_escape = true;
            (void)app_uart_put(APP_SLIP_ESC);
            break;

        default:
            (void)app_uart_put(mp_data->p_buffer[m_tx_index]);
            m_tx_index++;
            break;
    }
}


__STATIC_INLINE bool check_pending_tx()
{
    bool tx_continue = false;

    if (m_header_pending.p_buffer != NULL)
    {
        m_header  = m_header_pending;
        m_payload = m_payload_pending;
        m_crc     = m_crc_pending;

        m_header_pending.p_buffer      = NULL;
        m_header_pending.num_of_bytes  = 0;
        m_payload_pending.p_buffer     = NULL;
        m_payload_pending.num_of_bytes = 0;
        m_crc_pending.p_buffer         = NULL;
        m_crc_pending.num_of_bytes     = 0;

        m_tx_index  = 0; // may be also done in ser_phy_hci_tx_byte???
        tx_continue = true;

        /* Start sending pending packet */
        (void)ser_phy_hci_tx_byte();
    }

    return tx_continue;
}


static uint32_t ser_phy_hci_tx_byte()
{
    /* Flags informing about actually transmited part of packet*/
    static bool header  = false;
    static bool payload = false;
    static bool crc     = false;

    static bool ack_end    = false;
    static bool packet_end = false;

    if (ack_end == true)
    {
        ack_end   = false;
        m_tx_busy = check_pending_tx();
        /* Report end of ACK transmission*/
        m_ser_phy_hci_slip_event.evt_type = SER_PHY_HCI_SLIP_EVT_ACK_SENT;
        m_ser_phy_hci_slip_event_handler(&m_ser_phy_hci_slip_event);

    }
    else if (packet_end == true)
    {
        packet_end = false;
        m_tx_busy  = check_pending_tx();
        /* Report end of packet transmission*/
        m_ser_phy_hci_slip_event.evt_type = SER_PHY_HCI_SLIP_EVT_PKT_SENT;
        m_ser_phy_hci_slip_event_handler(&m_ser_phy_hci_slip_event);

    }
    else if ((m_tx_index == 0) && !header && !payload && !crc)
    {
        /* Beginning of packet - sent 0xC0*/
        header  = true;
        mp_data = &m_header;
        (void)app_uart_put(APP_SLIP_END);
    }
    else if ((m_tx_index == mp_data->num_of_bytes) && crc == true)
    {
        /* End of packet - sent 0xC0*/
        (void)app_uart_put(APP_SLIP_END);

        m_crc.p_buffer = NULL;
        crc            = false;
        m_tx_index     = 0;
        packet_end     = true;
    }
    else if ((m_tx_index == mp_data->num_of_bytes) && header == true)
    {
        /* End of header transmission*/
        m_tx_index = 0;

        if (m_payload.p_buffer != NULL)
        {
            header  = false;
            payload = true;
            mp_data = &m_payload;

            /* Handle every character in buffer accordingly to SLIP protocol*/
            slip_encode();
        }
        else
        {
            /* End of ACK - sent 0xC0*/
            (void)app_uart_put(APP_SLIP_END);

            header  = false;
            ack_end = true;
        }
    }
    else if ((m_tx_index == mp_data->num_of_bytes) && payload == true)
    {
        /* End of payload transmission*/
        m_tx_index = 0;

        if (m_crc.p_buffer != NULL)
        {
            m_payload.p_buffer = NULL;
            payload            = false;
            crc                = true;
            mp_data            = &m_crc;

            /* Handle every character in buffer accordingly to SLIP protocol*/
            slip_encode();
        }
        /* CRC is not used for this packet -> finish packet transmission */
        else
        {
            /* End of packet - send 0xC0*/
            (void)app_uart_put(APP_SLIP_END);

            m_payload.p_buffer = NULL;
            payload            = false;
            packet_end         = true;
        }
    }
    else if (m_tx_escape == false)
    {
        /* Handle every character in buffer accordingly to SLIP protocol*/
        slip_encode();
    }
    else if (m_tx_escape == true)
    {
        /* Send SLIP special code*/
        m_tx_escape = false;

        if (mp_data->p_buffer[m_tx_index] == APP_SLIP_END)
        {
            (void)app_uart_put(APP_SLIP_ESC_END);
            m_tx_index++;
        }
        else
        {
            (void)app_uart_put(APP_SLIP_ESC_ESC);
            m_tx_index++;
        }
    }

    return NRF_SUCCESS;
}


uint32_t ser_phy_hci_slip_tx_pkt_send(const ser_phy_hci_pkt_params_t * p_header,
                                      const ser_phy_hci_pkt_params_t * p_payload,
                                      const ser_phy_hci_pkt_params_t * p_crc)
{
    /* Block TXRDY interrupts at this point*/
    // NRF_UART0->INTENCLR = (UART_INTENCLR_TXDRDY_Clear << UART_INTENCLR_TXDRDY_Pos);
    CRITICAL_REGION_ENTER();

    if (p_header == NULL)
    {
        return NRF_ERROR_NULL;
    }

    /* Check if no tx is ongoing */
    if (!m_tx_busy)
    {
        m_header = *p_header;

        if (p_payload != NULL)
        {
            m_payload = *p_payload;
        }

        if (p_crc != NULL)
        {
            m_crc = *p_crc;
        }
    }
    /* Tx is ongoing, schedule transmission as pending */
    else
    {
        if (p_crc != NULL)
        {
            m_crc_pending = *p_crc;
        }

        if (p_payload != NULL)
        {
            m_payload_pending = *p_payload;
        }

        m_header_pending = *p_header;
    }

    /* Start packet transmission only if no other tx is ongoing */
    if (!m_tx_busy)
    {
        m_tx_busy = true;
        (void)ser_phy_hci_tx_byte();
    }

    /* Enable TXRDY interrupts at this point*/
    // NRF_UART0->INTENSET = (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos);
    CRITICAL_REGION_EXIT();
    return NRF_SUCCESS;
}


/* Function returns false when last byte in packet is detected.*/
static bool slip_decode(uint8_t * p_received_byte)
{
    switch (*p_received_byte)
    {
        case APP_SLIP_END:
            return false;

        case APP_SLIP_ESC:
            m_rx_escape = true;
            break;

        case APP_SLIP_ESC_END:

            if (m_rx_escape == true)
            {
                m_rx_escape      = false;
                *p_received_byte = APP_SLIP_END;
            }
            break;

        case APP_SLIP_ESC_ESC:

            if (m_rx_escape == true)
            {
                m_rx_escape      = false;
                *p_received_byte = APP_SLIP_ESC;
            }
            break;

        /* Normal character - decoding not needed*/
        default:
            break;
    }

    return true;
}


static void ser_phi_hci_rx_byte(uint8_t rx_byte)
{
    static bool rx_sync         = false;
    uint8_t     received_byte   = rx_byte;
    static bool big_buff_in_use = false;

    /* Test received byte for SLIP packet start: 0xC0*/
    if (!rx_sync)
    {
        if (received_byte == APP_SLIP_END)
        {
            m_rx_index = 0;
            rx_sync    = true;
        }
        return;
    }

    /* Additional check needed in case rx_sync flag was set by end of previous packet*/
    if ((m_rx_index) == 0 && (received_byte == APP_SLIP_END))
    {
        return;
    }

    /* Check if small (ACK) buffer is available*/
    if ((mp_small_buffer != NULL) && (big_buff_in_use == false))
    {
        if (m_rx_index == 0)
        {
            mp_buffer = mp_small_buffer;
        }

        /* Check if switch between small and big buffer is needed*/
        if (m_rx_index == sizeof (m_small_buffer) /*NEW!!!*/ && received_byte != APP_SLIP_END)
        {
            /* Check if big (PKT) buffer is available*/
            if (mp_big_buffer != NULL)
            {
                /* Switch to big buffer*/
                memcpy(m_big_buffer, m_small_buffer, sizeof (m_small_buffer));
                mp_buffer = m_big_buffer;
            }
            else
            {
                /* Small buffer is too small and big buffer not available - cannot continue reception*/
                rx_sync = false;
                return;
            }
        }

        /* Check if big buffer is full */
        if ((m_rx_index >= PKT_SIZE) && (received_byte != APP_SLIP_END))
        {
            /* Do not notify upper layer - the packet is too big and cannot be handled by slip */
            rx_sync = false;
            return;
        }

        /* Decode byte. Will return false when it is 0xC0 - end of packet*/
        if (slip_decode(&received_byte))
        {
            /* Write Rx byte only if it is not escape char */
            if (!m_rx_escape)
            {
                mp_buffer[m_rx_index++] = received_byte;
            }
        }
        else
        {
            /* Reset pointers to signalise buffers are locked waiting for upper layer */
            if (mp_buffer == mp_small_buffer)
            {
                mp_small_buffer = NULL;
            }
            else
            {
                mp_big_buffer = NULL;
            }
            /* Report packet reception end*/
            m_ser_phy_hci_slip_event.evt_type =
                SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED;
            m_ser_phy_hci_slip_event.evt_params.received_pkt.p_buffer     = mp_buffer;
            m_ser_phy_hci_slip_event.evt_params.received_pkt.num_of_bytes = m_rx_index;
            m_ser_phy_hci_slip_event_handler(&m_ser_phy_hci_slip_event);

            rx_sync = false;
        }
    }
    else if (mp_big_buffer != NULL)
    {
        big_buff_in_use = true;
        mp_buffer       = mp_big_buffer;

        /* Check if big buffer is full */
        if ((m_rx_index >= PKT_SIZE) && (received_byte != APP_SLIP_END))
        {
            /* Do not notify upper layer - the packet is too big and cannot be handled by slip */
            rx_sync = false;
            return;
        }

        /* Decode byte*/
        if (slip_decode(&received_byte))
        {
            /* Write Rx byte only if it is not escape char */
            if (!m_rx_escape)
            {
                mp_buffer[m_rx_index++] = received_byte;
            }
        }
        else
        {
            /* Report packet reception end*/
            m_ser_phy_hci_slip_event.evt_type =
                SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED;
            m_ser_phy_hci_slip_event.evt_params.received_pkt.p_buffer     = mp_buffer;
            m_ser_phy_hci_slip_event.evt_params.received_pkt.num_of_bytes = m_rx_index;
            m_ser_phy_hci_slip_event_handler(&m_ser_phy_hci_slip_event);

            rx_sync         = false;
            mp_big_buffer   = NULL;
            big_buff_in_use = false;
        }
    }
    else
    {
        /* Both buffers are not available - cannot continue reception*/
        rx_sync = false;
        return;
    }
}


uint32_t ser_phy_hci_slip_rx_buf_free(uint8_t * p_buffer)
{
    uint32_t err_code = NRF_SUCCESS;

    if (p_buffer == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (p_buffer == m_small_buffer)
    {
        /* Free small buffer*/
        if (mp_small_buffer == NULL)
        {
            mp_small_buffer = m_small_buffer;
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }
    else if (p_buffer == m_big_buffer)
    {
        /* Free big buffer*/
        if (mp_big_buffer == NULL)
        {
            mp_big_buffer = m_big_buffer;
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }

    return err_code;
}


static void ser_phy_uart_evt_callback(app_uart_evt_t * uart_evt)
{
    if (uart_evt == NULL)
    {
        return;
    }

    switch (uart_evt->evt_type)
    {
        case APP_UART_COMMUNICATION_ERROR:

            // Process error only if this is parity or overrun error.
            // Break and framing error is always present when app side is not active
            if (uart_evt->data.error_communication &
                (UART_ERRORSRC_PARITY_Msk | UART_ERRORSRC_OVERRUN_Msk))
            {
                callback_hw_error(uart_evt->data.error_communication);
            }
            break;

        case APP_UART_TX_EMPTY:
            (void)ser_phy_hci_tx_byte();
            break;

        case APP_UART_DATA:

            // After first reception disable pulldown - it was only needed before start
            // of the other side
            if (!m_other_side_active)
            {
                m_other_side_active = true;
            }

            m_rx_byte = uart_evt->data.value;
            ser_phi_hci_rx_byte(m_rx_byte);
            break;

        default:
            APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
            break;
    }
}


uint32_t ser_phy_hci_slip_open(ser_phy_hci_slip_event_handler_t events_handler)
{
    uint32_t err_code;

    if (events_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Check if function was not called before
    if (m_ser_phy_hci_slip_event_handler != NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    // Configure UART and register handler
    // uart_evt_handler is used to handle events produced by low-level uart driver
    APP_UART_INIT(&comm_params, ser_phy_uart_evt_callback, UART_IRQ_PRIORITY, err_code);

    mp_small_buffer = m_small_buffer;
    mp_big_buffer   = m_big_buffer;

    m_ser_phy_hci_slip_event_handler = events_handler;

    return err_code;
}


void ser_phy_hci_slip_close(void)
{
    m_ser_phy_hci_slip_event_handler = NULL;
    (void)app_uart_close();
}

