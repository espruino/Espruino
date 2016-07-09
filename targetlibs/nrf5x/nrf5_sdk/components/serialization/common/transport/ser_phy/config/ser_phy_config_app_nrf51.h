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

#ifndef SER_CONFIG_APP_HAL_NRF51_H__
#define SER_CONFIG_APP_HAL_NRF51_H__

#include "boards.h"
#include "ser_config.h"

#if defined(SPI_MASTER_0_ENABLE)
#define SER_PHY_SPI_MASTER SPI_MASTER_0
#endif
#if defined(SPI_MASTER_1_ENABLE)
#define SER_PHY_SPI_MASTER SPI_MASTER_1
#endif
#if defined(SPI_MASTER_2_ENABLE)
#define SER_PHY_SPI_MASTER SPI_MASTER_2
#endif

#if (defined(SPI0_ENABLED) && (SPI0_ENABLED == 1)) || defined(SPI_MASTER_0_ENABLE)

#define SER_PHY_SPI_MASTER_INSTANCE             NRF_DRV_SPI_INSTANCE(0)
#define SER_PHY_SPI_MASTER_PIN_SCK              SER_APP_SPIM0_SCK_PIN
#define SER_PHY_SPI_MASTER_PIN_MISO             SER_APP_SPIM0_MISO_PIN
#define SER_PHY_SPI_MASTER_PIN_MOSI             SER_APP_SPIM0_MOSI_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_SELECT     SER_APP_SPIM0_SS_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_REQUEST    SER_APP_SPIM0_REQ_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_READY      SER_APP_SPIM0_RDY_PIN

#elif (defined(SPI1_ENABLED) && (SPI1_ENABLED == 1)) || defined(SPI_MASTER_1_ENABLE)

#define SER_PHY_SPI_MASTER_INSTANCE             NRF_DRV_SPI_INSTANCE(1)
#define SER_PHY_SPI_MASTER_PIN_SCK              SER_APP_SPIM1_SCK_PIN
#define SER_PHY_SPI_MASTER_PIN_MISO             SER_APP_SPIM1_MISO_PIN
#define SER_PHY_SPI_MASTER_PIN_MOSI             SER_APP_SPIM1_MOSI_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_SELECT     SER_APP_SPIM1_SS_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_REQUEST    SER_APP_SPIM1_REQ_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_READY      SER_APP_SPIM1_RDY_PIN

#elif (defined(SPI2_ENABLED) && (SPI2_ENABLED == 1)) || defined(SPI_MASTER_2_ENABLE)

#define SER_PHY_SPI_MASTER_INSTANCE             NRF_DRV_SPI_INSTANCE(2)
#define SER_PHY_SPI_MASTER_PIN_SCK              SER_APP_SPIM2_SCK_PIN
#define SER_PHY_SPI_MASTER_PIN_MISO             SER_APP_SPIM2_MISO_PIN
#define SER_PHY_SPI_MASTER_PIN_MOSI             SER_APP_SPIM2_MOSI_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_SELECT     SER_APP_SPIM2_SS_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_REQUEST    SER_APP_SPIM2_REQ_PIN
#define SER_PHY_SPI_MASTER_PIN_SLAVE_READY      SER_APP_SPIM2_RDY_PIN

#endif

#define CONN_CHIP_RESET_PIN_NO                  SER_CONN_CHIP_RESET_PIN /**< Pin used for reseting the nRF51822. */

/* UART configuration */
#define UART_IRQ_PRIORITY                       APP_IRQ_PRIORITY_MID
#define SER_PHY_UART_RX                         SER_APP_RX_PIN
#define SER_PHY_UART_TX                         SER_APP_TX_PIN
#define SER_PHY_UART_CTS                        SER_APP_CTS_PIN
#define SER_PHY_UART_RTS                        SER_APP_RTS_PIN

#endif //SER_CONFIG_APP_HAL_NRF51_H__
