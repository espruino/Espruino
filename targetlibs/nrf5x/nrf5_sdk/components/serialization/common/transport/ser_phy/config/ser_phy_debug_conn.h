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

#ifndef SER_PHY_DEBUG_CONN_H__
#define SER_PHY_DEBUG_CONN_H__

#ifndef SER_PHY_DEBUG_CONN_ENABLE

#define DEBUG_EVT_SPI_SLAVE_RAW_RX_XFER_DONE(data);

#define DEBUG_EVT_SPI_SLAVE_RAW_TX_XFER_DONE(data);

#define DEBUG_EVT_SPI_SLAVE_RAW_BUFFERS_SET(data);

#define DEBUG_EVT_SPI_SLAVE_RAW_REQ_SET(data);

#define DEBUG_EVT_SPI_SLAVE_RAW_REQ_CLEARED(data);

#define DEBUG_EVT_SPI_SLAVE_PHY_BUF_REQUEST(data);

#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_RECEIVED(data);

#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_DROPPED(data);

#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_SENT(data);

#else

#include <stdint.h>

// low level hardware event types
typedef enum
{
    SPI_SLAVE_RAW_BUFFERS_SET,
    SPI_SLAVE_RAW_RX_XFER_DONE,
    SPI_SLAVE_RAW_TX_XFER_DONE,
    SPI_SLAVE_RAW_REQ_SET,
    SPI_SLAVE_RAW_REQ_CLEARED,
    SPI_SLAVE_PHY_BUF_REQUEST,
    SPI_SLAVE_PHY_PKT_SENT,
    SPI_SLAVE_PHY_PKT_RECEIVED,
    SPI_SLAVE_PHY_PKT_DROPPED,
    SPI_SLAVE_RAW_EVT_TYPE_MAX
} spi_slave_raw_evt_type_t;

// low level hardware event definition
typedef struct
{
    spi_slave_raw_evt_type_t  evt_type;
    uint32_t                  data;
} spi_slave_raw_evt_t;

typedef void (*spi_slave_raw_callback_t)(spi_slave_raw_evt_t event);

void debug_init(spi_slave_raw_callback_t spi_slave_raw_evt_callback);

void debug_evt(spi_slave_raw_evt_type_t evt_type, uint32_t data);

#define DEBUG_EVT(evt, data)    \
do {                            \
    debug_evt(evt, data);       \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_RAW_RX_XFER_DONE(data)  \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_RAW_RX_XFER_DONE, data);    \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_RAW_TX_XFER_DONE(data)  \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_RAW_TX_XFER_DONE, data);    \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_RAW_BUFFERS_SET(data)   \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_RAW_BUFFERS_SET, data);     \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_RAW_REQ_SET(data)       \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_RAW_REQ_SET, data);         \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_RAW_REQ_CLEARED(data)   \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_RAW_REQ_CLEARED, data);     \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_PHY_BUF_REQUEST(data)   \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_PHY_BUF_REQUEST, data);     \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_RECEIVED(data)  \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_PHY_PKT_RECEIVED, data);    \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_DROPPED(data)   \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_PHY_PKT_DROPPED, data);     \
} while (0);


#define DEBUG_EVT_SPI_SLAVE_PHY_PKT_SENT(data)      \
do {                                                \
    DEBUG_EVT(SPI_SLAVE_PHY_PKT_SENT, data);        \
} while (0);


#endif

#endif //SER_PHY_DEBUG_CONN_H__
