/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#ifndef DTM_UART_PARAMS_H__
#define DTM_UART_PARAMS_H__

#include <stdbool.h>
#include <stdint.h>

/** @ingroup ble_dtm_app
 */


/**@brief Enumeration of supported baud rates. */ 
typedef enum
{
    UART_BAUD_RATE_1200,     /**< Baud rate 1200. */
    UART_BAUD_RATE_2400,     /**< Baud rate 2400. */
    UART_BAUD_RATE_4800,     /**< Baud rate 4800. */
    UART_BAUD_RATE_9600,     /**< Baud rate 9600. */
    UART_BAUD_RATE_14400,    /**< Baud rate 14400. */
    UART_BAUD_RATE_19200,    /**< Baud rate 19200. */
    UART_BAUD_RATE_28800,    /**< Baud rate 28800. */
    UART_BAUD_RATE_38400,    /**< Baud rate 38400. */
    UART_BAUD_RATE_57600,    /**< Baud rate 57600. */
    UART_BAUD_RATE_76800,    /**< Baud rate 76800. */
    UART_BAUD_RATE_115200,   /**< Baud rate 115200. */
    UART_BAUD_RATE_230400,   /**< Baud rate 230400. */
    UART_BAUD_RATE_250000,   /**< Baud rate 250000. */
    UART_BAUD_RATE_460800,   /**< Baud rate 460800. */
    UART_BAUD_RATE_921600,   /**< Baud rate 921600. */
    UART_BAUD_RATE_1000000,  /**< Baud rate 1000000. */
    UART_BAUD_RATE_MAX       /**< Enumeration upper bound. */
} app_uart_stream_baud_rate_t;

/**@brief UART communication structure holding configuration settings for the peripheral.
 */
typedef struct
{
    uint8_t                     rx_pin_no;      /**< RX pin number. */
    uint8_t                     tx_pin_no;      /**< TX pin number. */
    uint8_t                     rts_pin_no;     /**< RTS pin number, only used if flow control is enabled. */
    uint8_t                     cts_pin_no;     /**< CTS pin number, only used if flow control is enabled. */
    bool                        use_parity;     /**< Even parity if TRUE, no parity if FALSE. */
    app_uart_stream_baud_rate_t baud_rate;      /**< Baud rate configuration. */
} app_uart_stream_comm_params_t;

#endif // DTM_UART_PARAMS_H__
