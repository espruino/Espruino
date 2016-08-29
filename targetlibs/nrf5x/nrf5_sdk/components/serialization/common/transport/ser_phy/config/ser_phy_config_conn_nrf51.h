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

#ifndef SER_PHY_CONFIG_CONN_NRF51_H__
#define SER_PHY_CONFIG_CONN_NRF51_H__

#include "boards.h"
#include "ser_config.h"

/***********************************************************************************************//**
 * SER_PHY layer configuration.
 **************************************************************************************************/
#define SER_PHY_SPI_PPI_RDY_CH                  0
#define SER_PHY_SPI_GPIOTE_RDY_CH               0

#define SER_PHY_SPI_SLAVE_INSTANCE              1

#define SER_PHY_SPI_SLAVE_REQ_PIN               SER_CON_SPIS_REQ_PIN
#define SER_PHY_SPI_SLAVE_RDY_PIN               SER_CON_SPIS_RDY_PIN
#define SER_PHY_SPI_SLAVE_SCK_PIN               SER_CON_SPIS_SCK_PIN
#define SER_PHY_SPI_SLAVE_MISO_PIN              SER_CON_SPIS_MISO_PIN
#define SER_PHY_SPI_SLAVE_MOSI_PIN              SER_CON_SPIS_MOSI_PIN
#define SER_PHY_SPI_SLAVE_SS_PIN                SER_CON_SPIS_CSN_PIN

/* UART configuration */
#define UART_IRQ_PRIORITY                       APP_IRQ_PRIORITY_LOW

#define SER_PHY_UART_RX                         SER_CON_RX_PIN
#define SER_PHY_UART_TX                         SER_CON_TX_PIN
#define SER_PHY_UART_CTS                        SER_CON_CTS_PIN
#define SER_PHY_UART_RTS                        SER_CON_RTS_PIN

#endif //SER_PHY_CONFIG_CONN_NRF51_H__
