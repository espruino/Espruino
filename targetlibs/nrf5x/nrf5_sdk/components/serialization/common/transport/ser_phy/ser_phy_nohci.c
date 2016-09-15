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

/**@file
 *
 * @defgroup ser_phy_spi_phy_driver_slave ser_phy_nrf51_spi_slave.c
 * @{
 * @ingroup ser_phy_spi_phy_driver_slave
 *
 * @brief SPI_RAW PHY slave driver.
 */

#include <stddef.h>
#include <string.h>

#include "app_error.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "app_mailbox.h"
#include "ser_phy.h"
#include "ser_phy_hci.h"
#include "crc16.h"
#include "nrf_soc.h"

#include "ser_phy_debug_comm.h"

static bool m_flag_nohci_init = false;
static bool m_flag_expect_ack;
static bool m_flag_buffer_reqested = false;

static uint16_t  m_rx_packet_length;
static uint8_t * m_p_rx_packet;

static uint16_t  m_rx_pending_packet_length;
static uint8_t * m_p_rx_pending_packet;

static uint16_t  m_rx_allocated_packet_length;
static uint8_t * m_p_rx_allocated_packet;

static uint8_t * m_p_tx_packet = NULL;
static uint16_t  m_tx_packet_length;

static ser_phy_events_handler_t m_ser_phy_callback = NULL;

#define PKT_HDR_SIZE 4 /**< Packet header size in number of bytes. */
#define PKT_CRC_SIZE 2 /**< Packet CRC size in number of bytes. */

static void ser_phy_nohci_assert(bool cond)
{
    APP_ERROR_CHECK_BOOL(cond);
}


static void ser_phy_event_callback(ser_phy_evt_t event)
{
    if (m_ser_phy_callback)
    {
        m_ser_phy_callback(event);
    }
}


static void memory_request_callback(uint16_t size)
{
    ser_phy_evt_t event;

    DEBUG_EVT_HCI_PHY_EVT_BUF_REQUEST(0);

    event.evt_type                               = SER_PHY_EVT_RX_BUF_REQUEST;
    event.evt_params.rx_buf_request.num_of_bytes = size;
    ser_phy_event_callback(event);
}


static void packet_received_callback(uint8_t * pBuffer, uint16_t size)
{
    ser_phy_evt_t event;

    DEBUG_EVT_HCI_PHY_EVT_RX_PKT_RECEIVED(0);

    event.evt_type = SER_PHY_EVT_RX_PKT_RECEIVED;
    event.evt_params.rx_pkt_received.num_of_bytes = size;
    event.evt_params.rx_pkt_received.p_buffer     = pBuffer;
    ser_phy_event_callback(event);
}


static void packet_dropped_callback(void)
{
    ser_phy_evt_t event;

    DEBUG_EVT_HCI_PHY_EVT_RX_PKT_DROPPED(0);

    event.evt_type = SER_PHY_EVT_RX_PKT_DROPPED;
    ser_phy_event_callback(event);
}


static void packet_transmitted_callback(void)
{
    ser_phy_evt_t event;

    DEBUG_EVT_HCI_PHY_EVT_TX_PKT_SENT(0);

    event.evt_type = SER_PHY_EVT_TX_PKT_SENT;
    ser_phy_event_callback(event);
}


static void hci_slip_event_handler(ser_phy_hci_slip_evt_t * p_event)
{
    if ( p_event->evt_type == SER_PHY_HCI_SLIP_EVT_PKT_SENT )
    {
        DEBUG_EVT_SLIP_PACKET_TXED(0);

        if (!m_flag_expect_ack)
        {
            m_p_tx_packet = NULL;
            packet_transmitted_callback();
        }
        else
        {
            ser_phy_nohci_assert(false); // packet was send as a ACK packet, callback should be with ACK_SENT
        }

    }
    else if ( p_event->evt_type == SER_PHY_HCI_SLIP_EVT_ACK_SENT )
    {
        DEBUG_EVT_SLIP_ACK_TXED(0);

        if (m_flag_expect_ack)
        {
            m_p_tx_packet = NULL;
            packet_transmitted_callback();
        }
        else
        {
            ser_phy_nohci_assert(false); // packet was send as a normal packet, callback should be with PKT_SENT
        }

    }
    else if ( p_event->evt_type == SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED )
    {
        CRITICAL_REGION_ENTER();

        if (m_p_rx_packet == NULL)
        {
            m_p_rx_packet                = p_event->evt_params.received_pkt.p_buffer;
            m_rx_packet_length           = p_event->evt_params.received_pkt.num_of_bytes;
            m_p_rx_allocated_packet      = m_p_rx_packet;
            m_rx_allocated_packet_length = m_rx_packet_length;
            m_flag_buffer_reqested       = true;
            memory_request_callback(m_rx_allocated_packet_length);
        }
        else if (m_p_rx_pending_packet == NULL)
        {
            m_p_rx_pending_packet      = p_event->evt_params.received_pkt.p_buffer;
            m_rx_pending_packet_length = p_event->evt_params.received_pkt.num_of_bytes;
        }
        else
        {
            // both buffers are not released; this is fault
            ser_phy_nohci_assert(false);
        }
        CRITICAL_REGION_EXIT();
    }
    else
    {
        // no other callbacks are expected
        ser_phy_nohci_assert(false);
    }
}


/* ser_phy API function */
void ser_phy_interrupts_enable(void)
{

    NVIC_EnableIRQ(UART0_IRQn);
    return;
}


/* ser_phy API function */
void ser_phy_interrupts_disable(void)
{
    NVIC_DisableIRQ(UART0_IRQn);
    return;
}


/* ser_phy API function */
uint32_t ser_phy_rx_buf_set(uint8_t * p_buffer)
{
    uint32_t status = NRF_SUCCESS;

    if (m_flag_buffer_reqested)
    {
        m_flag_buffer_reqested = false;

        if (p_buffer)
        {
            memcpy(p_buffer, m_p_rx_allocated_packet, m_rx_allocated_packet_length);
            packet_received_callback(p_buffer, m_rx_allocated_packet_length);
        }
        else
        {
            packet_dropped_callback();
        }

        CRITICAL_REGION_ENTER();

        if (m_p_rx_allocated_packet == m_p_rx_packet && (m_p_rx_pending_packet == NULL))
        {
            // packet is copied and there is no pending packet
            (void) ser_phy_hci_slip_rx_buf_free(m_p_rx_packet);
            m_p_rx_packet           = NULL;
            m_p_rx_allocated_packet = NULL;
        }
        else if (m_p_rx_allocated_packet == m_p_rx_packet && (m_p_rx_pending_packet != NULL))
        {
            // there is a pending packet - request memory for it
            m_p_rx_allocated_packet      = m_p_rx_pending_packet;
            m_rx_allocated_packet_length = m_rx_pending_packet_length;
            m_flag_buffer_reqested       = true;
        }
        else if (m_p_rx_allocated_packet == m_p_rx_pending_packet )
        {
            // the pending packet was serviced - release both
            m_p_rx_allocated_packet = NULL;
            (void) ser_phy_hci_slip_rx_buf_free(m_p_rx_packet);
            m_p_rx_packet = NULL;
            (void) ser_phy_hci_slip_rx_buf_free(m_p_rx_pending_packet);
            m_p_rx_pending_packet = NULL;
        }
        else
        {
            // no other calls are expected
            ser_phy_nohci_assert(false);
        }
        CRITICAL_REGION_EXIT();

        // request memory for a pending
        if (m_p_rx_allocated_packet)
        {
            memory_request_callback(m_rx_allocated_packet_length);
        }
    }
    else
    {
        status = NRF_ERROR_BUSY;
    }
    return status;
}


/* ser_phy API function */
uint32_t ser_phy_tx_pkt_send(const uint8_t * p_buffer, uint16_t num_of_bytes)
{
    uint32_t status = NRF_SUCCESS;
    uint32_t err_code;

    if ( p_buffer == NULL || num_of_bytes == 0)
    {
        return NRF_ERROR_NULL;
    }

    if ( m_p_tx_packet == NULL)
    {
        m_tx_packet_length = num_of_bytes;
        m_p_tx_packet      = (uint8_t *)p_buffer;

        if (m_tx_packet_length <= PKT_HDR_SIZE + PKT_CRC_SIZE)
        {
            ser_phy_hci_pkt_params_t pkt; // all packets smaller than 6 goes as ACK

            m_flag_expect_ack = true;
            pkt.p_buffer      = (uint8_t *)m_p_tx_packet;
            pkt.num_of_bytes  = m_tx_packet_length;
            DEBUG_EVT_SLIP_ACK_TX(0);
            err_code = ser_phy_hci_slip_tx_pkt_send(&pkt, NULL, NULL); // this will look like ACK for slip
            ser_phy_nohci_assert(err_code == NRF_SUCCESS);
        }
        else
        {
            ser_phy_hci_pkt_params_t header;  // this is fake header - just first 4 bytes
            ser_phy_hci_pkt_params_t crc;     // this is fake header - just last 2 bytes
            ser_phy_hci_pkt_params_t payload; // this is fake payload - all except for header and crc

            m_flag_expect_ack    = false;
            header.p_buffer      = (uint8_t *)m_p_tx_packet;
            header.num_of_bytes  = PKT_HDR_SIZE;
            crc.p_buffer         = (uint8_t *)m_p_tx_packet + m_tx_packet_length - PKT_CRC_SIZE;
            crc.num_of_bytes     = PKT_CRC_SIZE;
            payload.p_buffer     = (uint8_t *)m_p_tx_packet + PKT_HDR_SIZE;
            payload.num_of_bytes = m_tx_packet_length - PKT_HDR_SIZE - PKT_CRC_SIZE;
            DEBUG_EVT_SLIP_PACKET_TX(0);
            err_code = ser_phy_hci_slip_tx_pkt_send(&header, &payload, &crc); // this will look like normal packet for slip
            ser_phy_nohci_assert(err_code == NRF_SUCCESS);
        }
    }
    else
    {
        status = NRF_ERROR_BUSY;
    }

    return status;
}


/* ser_phy API function */
uint32_t ser_phy_open(ser_phy_events_handler_t events_handler)
{
    uint32_t err_code;

    if (m_flag_nohci_init)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (events_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }
    err_code = ser_phy_hci_slip_open(hci_slip_event_handler);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    m_ser_phy_callback = events_handler;
    m_flag_nohci_init  = true;
    return NRF_SUCCESS;
}


/* ser_phy API function */
void ser_phy_close(void)
{
    m_ser_phy_callback = NULL;
    ser_phy_hci_slip_close();
    m_flag_nohci_init = false;
}


