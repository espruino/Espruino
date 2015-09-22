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

/**@file
 *
 * @defgroup spi_slave_example_main SPI slave example application.
 * @{
 * @ingroup  spi_slave_example
 *
 * @brief    SPI slave example application implementation.
 */

#ifndef SPI_SLAVE_EXAMPLE_H__
#define SPI_SLAVE_EXAMPLE_H__

#include <stdint.h>

/**@brief Function for initializing the SPI slave example.
 *
 * @retval NRF_SUCCESS  Operation success.
 */ 
uint32_t spi_slave_example_init(void);

#endif // SPI_SLAVE_EXAMPLE_H__

/** @} */
