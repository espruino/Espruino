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

#ifndef SER_CONFIG_H__
#define SER_CONFIG_H__

#include <stdint.h>

#include "nrf.h"

/***********************************************************************************************//**
 * General parameters configuration.
 **************************************************************************************************/

/** Value used as error code on SoftDevice stack dump. Can be used to identify stack location on
 *  stack unwind.*/
#define SER_SD_ERROR_CODE    (uint32_t)(0xDEADBEEF)

/** Value used as error code indicating warning - unusual situation but not critical so system
 *  should NOT be reseted. */
#define SER_WARNING_CODE     (uint32_t)(0xBADDCAFE)

/***********************************************************************************************//**
 * HAL Transport layer configuration.
 **************************************************************************************************/

/** Max packets size in serialization HAL Transport layer (packets before adding PHY header i.e.
 *  packet length). */
#define SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE    (uint32_t)(384)
#define SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE    (uint32_t)(384)

#define SER_HAL_TRANSPORT_MAX_PKT_SIZE ((SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE) >= \
                                        (SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE)    \
                                        ?                                               \
                                        (SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE) :  \
                                        (SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE))
#ifdef SER_CONNECTIVITY
    #define SER_HAL_TRANSPORT_TX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE
    #define SER_HAL_TRANSPORT_RX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE

#else /* APPLICATION SIDE */
    #define SER_HAL_TRANSPORT_TX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE
    #define SER_HAL_TRANSPORT_RX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE
#endif /* SER_CONNECTIVITY */


/***********************************************************************************************//**
 * SER_PHY layer configuration.
 **************************************************************************************************/

#define SER_PHY_HEADER_SIZE             2

#define SER_PHY_SPI_FREQUENCY           NRF_DRV_SPI_FREQ_1M

/** Max transfer unit for SPI MASTER and SPI SLAVE. */
#define SER_PHY_SPI_MTU_SIZE            255

/** UART transmission parameters */
#define SER_PHY_UART_FLOW_CTRL          APP_UART_FLOW_CONTROL_ENABLED
#define SER_PHY_UART_PARITY             true
#define SER_PHY_UART_BAUDRATE           UART_BAUDRATE_BAUDRATE_Baud1M

/** Find UART baudrate value based on chosen register setting. */
#if (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud1200)
    #define SER_PHY_UART_BAUDRATE_VAL 1200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud2400)
    #define SER_PHY_UART_BAUDRATE_VAL 2400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud4800)
    #define SER_PHY_UART_BAUDRATE_VAL 4800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud9600)
    #define SER_PHY_UART_BAUDRATE_VAL 9600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud14400)
    #define SER_PHY_UART_BAUDRATE_VAL 14400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud19200)
    #define SER_PHY_UART_BAUDRATE_VAL 19200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud28800)
    #define SER_PHY_UART_BAUDRATE_VAL 28800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud38400)
    #define SER_PHY_UART_BAUDRATE_VAL 38400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud57600)
    #define SER_PHY_UART_BAUDRATE_VAL 57600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud76800)
    #define SER_PHY_UART_BAUDRATE_VAL 76800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud115200)
    #define SER_PHY_UART_BAUDRATE_VAL 115200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud230400)
    #define SER_PHY_UART_BAUDRATE_VAL 230400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud250000)
    #define SER_PHY_UART_BAUDRATE_VAL 250000uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud460800)
    #define SER_PHY_UART_BAUDRATE_VAL 460800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud921600)
    #define SER_PHY_UART_BAUDRATE_VAL 921600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud1M)
    #define SER_PHY_UART_BAUDRATE_VAL 1000000uL
#endif /* SER_PHY_UART_BAUDRATE */

/** Configuration timeouts of connectivity MCU */
#define CONN_CHIP_RESET_TIME            50      /**< The time to keep the reset line to the nRF51822 low (in milliseconds). */
#define CONN_CHIP_WAKEUP_TIME           500     /**< The time for nRF51822 to reset and become ready to receive serialized commands (in milliseconds). */

#define SER_MAX_CONNECTIONS 8

#endif /* SER_CONFIG_H__ */
