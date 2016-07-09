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

#ifndef SER_PHY_DEBUG_COMM_H__
#define SER_PHY_DEBUG_COMM_H__

#ifndef SER_PHY_HCI_DEBUG_ENABLE

// empty definitions here
#define DEBUG_EVT_HCI_PHY_EVT_TX_PKT_SENT(data)
#define DEBUG_EVT_HCI_PHY_EVT_BUF_REQUEST(data)
#define DEBUG_EVT_HCI_PHY_EVT_RX_PKT_RECEIVED(data)
#define DEBUG_EVT_HCI_PHY_EVT_RX_PKT_DROPPED(data)
#define DEBUG_EVT_HCI_PHY_EVT_TX_ERROR(data)
#define DEBUG_EVT_SLIP_PACKET_TX(data)
#define DEBUG_EVT_SLIP_ACK_TX(data)
#define DEBUG_EVT_SLIP_PACKET_TXED(data)
#define DEBUG_EVT_SLIP_ACK_TXED(data)
#define DEBUG_EVT_SLIP_PACKET_RXED(data)
#define DEBUG_EVT_SLIP_ACK_RXED(data)
#define DEBUG_EVT_SLIP_ERR_RXED(data)
#define DEBUG_EVT_TIMEOUT(data)
#define DEBUG_HCI_RETX(data)
#define DEBUG_EVT_MAIN_BUSY(data)
#define DEBUG_EVT_TX_REQ(data)

#else
#include <stdint.h>

//Low level hardware events
typedef enum
{
    HCI_PHY_EVT_TX_PKT_SENT,
    HCI_PHY_EVT_BUF_REQUEST,
    HCI_PHY_EVT_RX_PKT_RECEIVED,
    HCI_PHY_EVT_RX_PKT_DROPPED,
    HCI_PHY_EVT_TX_ERROR,
    HCI_SLIP_EVT_PACKET_TX,
    HCI_SLIP_EVT_ACK_TX,
    HCI_SLIP_EVT_PACKET_TXED,
    HCI_SLIP_EVT_ACK_TXED,
    HCI_SLIP_EVT_PACKET_RXED,
    HCI_SLIP_EVT_ACK_RXED,
    HCI_SLIP_EVT_ERR_RXED,
    HCI_TIMER_EVT_TIMEOUT,
    HCI_RETX,
    HCI_MAIN_BUSY,
    HCI_TX_REQ,
    HCI_PHY_EVT_MAX
} hci_dbg_evt_type_t;


//Low level hardware event definition
typedef struct
{
    hci_dbg_evt_type_t   evt;
    uint32_t              data;
} hci_dbg_evt_t;

typedef void (*hci_dbg_event_handler_t)(hci_dbg_evt_t event);

void debug_init(hci_dbg_event_handler_t evt_callback);

void debug_evt(hci_dbg_evt_type_t evt, uint32_t data);


#define DEBUG_EVT(event_type, data)    \
do {                            \
    debug_evt(event_type, data);       \
} while(0);


#define DEBUG_EVT_HCI_PHY_EVT_TX_PKT_SENT(data)      \
do {                                                 \
    DEBUG_EVT(HCI_PHY_EVT_TX_PKT_SENT, data);        \
} while (0);


#define DEBUG_EVT_HCI_PHY_EVT_BUF_REQUEST(data)       \
do {                                                  \
    DEBUG_EVT(HCI_PHY_EVT_BUF_REQUEST, data);         \
} while (0);


#define DEBUG_EVT_HCI_PHY_EVT_RX_PKT_RECEIVED(data)   \
do {                                                  \
    DEBUG_EVT(HCI_PHY_EVT_RX_PKT_RECEIVED, data);     \
} while (0);


#define DEBUG_EVT_HCI_PHY_EVT_RX_PKT_DROPPED(data)    \
do {                                                  \
    DEBUG_EVT(HCI_PHY_EVT_RX_PKT_DROPPED, data);      \
} while (0);

#define DEBUG_EVT_HCI_PHY_EVT_TX_ERROR(data)          \
do {                                                  \
    DEBUG_EVT(HCI_PHY_EVT_TX_ERROR, data);            \
} while (0);

#define DEBUG_EVT_SLIP_PACKET_TX(data)                \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_PACKET_TX, data);          \
} while (0);

#define DEBUG_EVT_SLIP_ACK_TX(data)                   \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_ACK_TX, data);             \
} while (0);

#define DEBUG_EVT_SLIP_PACKET_TXED(data)              \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_PACKET_TXED, data);        \
} while (0);

#define DEBUG_EVT_SLIP_ACK_TXED(data)                 \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_ACK_TXED, data);           \
} while (0);

#define DEBUG_EVT_SLIP_PACKET_RXED(data)              \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_PACKET_RXED, data);        \
} while (0);

#define DEBUG_EVT_SLIP_ACK_RXED(data)                 \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_ACK_RXED, data);           \
} while (0);

#define DEBUG_EVT_SLIP_ERR_RXED(data)                 \
do {                                                  \
    DEBUG_EVT(HCI_SLIP_EVT_ERR_RXED, data);           \
} while (0);

#define DEBUG_EVT_TIMEOUT(data)                       \
do {                                                  \
    DEBUG_EVT(HCI_TIMER_EVT_TIMEOUT, data);           \
} while (0);

#define DEBUG_HCI_RETX(data)                          \
do {                                                  \
    DEBUG_EVT(HCI_RETX, data);                        \
} while (0);

#define DEBUG_EVT_MAIN_BUSY(data)                     \
do {                                                  \
    DEBUG_EVT(HCI_MAIN_BUSY, data);                   \
} while (0);

#define DEBUG_EVT_TX_REQ(data)                        \
do {                                                  \
    DEBUG_EVT(HCI_TX_REQ, data);                      \
} while (0);

#endif  // SER_PHY_HCI_DEBUG_ENABLE

#endif  // SER_PHY_DEBUG_COMM_H__
