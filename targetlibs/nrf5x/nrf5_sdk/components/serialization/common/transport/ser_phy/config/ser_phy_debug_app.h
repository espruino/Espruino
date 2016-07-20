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

#ifndef SER_PHY_DEBUG_APP_H__
#define SER_PHY_DEBUG_APP_H__

#ifndef SER_PHY_DEBUG_APP_ENABLE

#define DEBUG_EVT_SPI_MASTER_RAW_REQUEST(data)
#define DEBUG_EVT_SPI_MASTER_RAW_READY(data)
#define DEBUG_EVT_SPI_MASTER_RAW_XFER_DONE(data)
#define DEBUG_EVT_SPI_MASTER_RAW_API_CALL(data)
#define DEBUG_EVT_SPI_MASTER_RAW_READY_EDGE(data)
#define DEBUG_EVT_SPI_MASTER_RAW_REQUEST_EDGE(data)
#define DEBUG_EVT_SPI_MASTER_PHY_TX_PKT_SENT(data)
#define DEBUG_EVT_SPI_MASTER_PHY_RX_PKT_DROPPED(data)
#define DEBUG_EVT_SPI_MASTER_PHY_RX_PKT_RECEIVED(data)
#define DEBUG_EVT_SPI_MASTER_PHY_BUF_REQUEST(data)

#define DEBUG_EVT_SPI_MASTER_RAW_XFER_GUARDED(data)
#define DEBUG_EVT_SPI_MASTER_RAW_XFER_PASSED(data)
#define DEBUG_EVT_SPI_MASTER_RAW_XFER_ABORTED(data)
#define DEBUG_EVT_SPI_MASTER_RAW_XFER_RESTARTED(data)

#else
#include <stdint.h>

//Low level hardware events
typedef enum
{
    SPI_MASTER_RAW_READY,
    SPI_MASTER_RAW_REQUEST,
    SPI_MASTER_RAW_XFER_DONE,
    SPI_MASTER_RAW_API_CALL,
    SPI_MASTER_RAW_READY_EDGE,
    SPI_MASTER_RAW_REQUEST_EDGE,
    SPI_MASTER_RAW_XFER_STARTED,
    SPI_MASTER_RAW_XFER_GUARDED,
    SPI_MASTER_RAW_XFER_PASSED,
    SPI_MASTER_RAW_XFER_ABORTED,
    SPI_MASTER_RAW_XFER_RESTARTED,
    SPI_MASTER_PHY_TX_PKT_SENT,
    SPI_MASTER_PHY_BUF_REQUEST,
    SPI_MASTER_PHY_RX_PKT_RECEIVED,
    SPI_MASTER_PHY_RX_PKT_DROPPED,
    SPI_MASTER_EVT_MAX
} spi_master_raw_evt_type_t;


//Low level hardware event definition
typedef struct
{
    spi_master_raw_evt_type_t  evt;
    uint32_t                   data;
} spi_master_raw_evt_t;

typedef void (*spi_master_raw_callback_t)(spi_master_raw_evt_t event);

void debug_init(spi_master_raw_callback_t spi_master_raw_evt_callback);

void debug_evt(spi_master_raw_evt_type_t evt, uint32_t data);


#define DEBUG_EVT(evt, data)    \
do {                            \
    debug_evt(evt, data);       \
} while(0);


#define DEBUG_EVT_SPI_MASTER_RAW_REQUEST(data)          \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_REQUEST, data);            \
} while (0);


#define DEBUG_EVT_SPI_MASTER_RAW_READY(data)            \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_READY, data);              \
} while (0);


#define DEBUG_EVT_SPI_MASTER_RAW_XFER_DONE(data)        \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_XFER_DONE, data);          \
} while (0);


#define DEBUG_EVT_SPI_MASTER_RAW_API_CALL(data)         \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_API_CALL, data);           \
} while (0);


#define DEBUG_EVT_SPI_MASTER_RAW_READY_EDGE(data)       \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_READY_EDGE, data);         \
} while (0);


#define DEBUG_EVT_SPI_MASTER_RAW_REQUEST_EDGE(data)     \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_REQUEST_EDGE, data);       \
} while (0);


#define DEBUG_EVT_SPI_MASTER_PHY_TX_PKT_SENT(data)      \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_PHY_TX_PKT_SENT, data);        \
} while (0);


#define DEBUG_EVT_SPI_MASTER_PHY_RX_PKT_DROPPED(data)   \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_PHY_RX_PKT_DROPPED, data);     \
} while (0);


#define DEBUG_EVT_SPI_MASTER_PHY_RX_PKT_RECEIVED(data)  \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_PHY_RX_PKT_RECEIVED, data);    \
} while (0);


#define DEBUG_EVT_SPI_MASTER_PHY_BUF_REQUEST(data)      \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_PHY_BUF_REQUEST, data);        \
} while (0);

#define DEBUG_EVT_SPI_MASTER_RAW_XFER_GUARDED(data)     \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_XFER_GUARDED, data);       \
} while (0);

#define DEBUG_EVT_SPI_MASTER_RAW_XFER_PASSED(data)      \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_XFER_PASSED, data);        \
} while (0);

#define DEBUG_EVT_SPI_MASTER_RAW_XFER_ABORTED(data)     \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_XFER_ABORTED, data);       \
} while (0);

#define DEBUG_EVT_SPI_MASTER_RAW_XFER_RESTARTED(data)   \
do {                                                    \
    DEBUG_EVT(SPI_MASTER_RAW_XFER_RESTARTED, data);     \
} while (0);



#endif

#endif //SER_PHY_DEBUG_APP_H__
