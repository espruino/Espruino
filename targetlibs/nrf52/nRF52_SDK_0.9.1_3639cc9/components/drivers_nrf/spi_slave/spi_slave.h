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
 * @defgroup lib_driver_spi_slave Software controlled SPI Slave driver.
 * @{
 * @ingroup  nrf_drivers
 *
 * @brief    Software controlled SPI Slave driver.
 */

#ifndef SPI_SLAVE_H__
#define SPI_SLAVE_H__

#include <stdint.h>
#include "nrf_error.h"

/**@brief SPI transaction bit order definitions. */
typedef enum
{
    SPIM_LSB_FIRST,                         /**< Least significant bit shifted out first. */
    SPIM_MSB_FIRST                          /**< Most significant bit shifted out first. */
} spi_slave_endian_t;

/**@brief SPI mode definitions for clock polarity and phase. */
typedef enum
{
    SPI_MODE_0,                             /**< (CPOL = 0, CPHA = 0). */
    SPI_MODE_1,                             /**< (CPOL = 0, CPHA = 1). */
    SPI_MODE_2,                             /**< (CPOL = 1, CPHA = 0). */
    SPI_MODE_3                              /**< (CPOL = 1, CPHA = 1). */
} spi_slave_mode_t;

/**@brief SPI peripheral device configuration data. */
typedef struct 
{
    uint32_t           pin_miso;            /**< SPI MISO pin. */
    uint32_t           pin_mosi;            /**< SPI MOSI pin. */
    uint32_t           pin_sck;             /**< SPI SCK pin. */
    uint32_t           pin_csn;             /**< SPI CSN pin. */
    spi_slave_mode_t   mode;                /**< SPI mode. */
    spi_slave_endian_t bit_order;           /**< SPI transaction bit order. */    
    uint8_t            def_tx_character;    /**< Device configuration mode default character (DEF). Character clocked out in case of an ignored transaction. */    
    uint8_t            orc_tx_character;    /**< Device configuration mode over-read character. Character clocked out after an over-read of the transmit buffer. */        
} spi_slave_config_t;

/**@brief Event callback function event definitions. */
typedef enum
{
    SPI_SLAVE_BUFFERS_SET_DONE,             /**< Memory buffer set event. Memory buffers have been set successfully to the SPI slave device and SPI transactions can be done. */
    SPI_SLAVE_XFER_DONE,                    /**< SPI transaction event. SPI transaction has been completed. */  
    SPI_SLAVE_EVT_TYPE_MAX                  /**< Enumeration upper bound. */      
} spi_slave_evt_type_t;

/**@brief Struct containing event context from the SPI slave driver. */
typedef struct
{
    spi_slave_evt_type_t evt_type;          /**< Type of event. */    
    uint32_t             rx_amount;         /**< Number of bytes received in last transaction (parameter is only valid upon @ref SPI_SLAVE_XFER_DONE event). */
    uint32_t             tx_amount;         /**< Number of bytes transmitted in last transaction (parameter is only valid upon @ref SPI_SLAVE_XFER_DONE event). */    
} spi_slave_evt_t;

/**@brief SPI slave event callback function type.
 *
 * @param[in] event                 SPI slave driver event.  
 */
typedef void (*spi_slave_event_handler_t)(spi_slave_evt_t event);

/**@brief Function for registering a handler for SPI slave driver event.
 *
 * @note Multiple registration requests will overwrite any possible existing registration. 
 *
 * @param[in] event_handler         The function to be called by the SPI slave driver upon event.
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation success. NULL handler registered.    
 */
uint32_t spi_slave_evt_handler_register(spi_slave_event_handler_t event_handler);

/**@brief Function for initializing the SPI slave device.
 *
 * @param[in] p_spi_slave_config    SPI peripheral device configuration data.
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.    
 * @retval NRF_ERROR_INVALID_PARAM  Operation failure. Invalid parameter supplied.
 */
uint32_t spi_slave_init(const spi_slave_config_t * p_spi_slave_config);

/**@brief Function for preparing the SPI slave device ready for a single SPI transaction.
 * 
 * Function prepares the SPI slave device to be ready for a single SPI transaction. It configures 
 * the SPI slave device to use the memory, supplied with the function call, in SPI transactions. 
 * 
 * The @ref spi_slave_event_handler_t will be called with appropriate event @ref 
 * spi_slave_evt_type_t when either the memory buffer configuration or SPI transaction has been 
 * completed.
 *
 * @note The callback function @ref spi_slave_event_handler_t can be called before returning from 
 * this function, since it is called from the SPI slave interrupt context.
 *
 * @note This function can be called from the callback function @ref spi_slave_event_handler_t 
 * context.
 *
 * @note Client application must call this function after every @ref SPI_SLAVE_XFER_DONE event if it 
 * wants the SPI slave driver to be ready for possible new SPI transaction. 
 *
 * @param[in] p_tx_buf              Pointer to the TX buffer.
 * @param[in] p_rx_buf              Pointer to the RX buffer.
 * @param[in] tx_buf_length         Length of the TX buffer in bytes.
 * @param[in] rx_buf_length         Length of the RX buffer in bytes. 
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.   
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. SPI slave device in incorrect state.
 * @retval NRF_ERROR_INTERNAL       Operation failure. Internal error ocurred.
 */
uint32_t spi_slave_buffers_set(uint8_t * p_tx_buf, 
                               uint8_t * p_rx_buf, 
                               uint8_t   tx_buf_length, 
                               uint8_t   rx_buf_length);

/**@brief Function for changing defult pull-up configuration for CSN pin.
 *
 * In the default configuration, pull-up on the CSN pin is disabled.
 * If alternate configuration is required this function can be called before spi_slave_init()
 * to change default settings for CSN pin.
 *
 * @param[in] alternate_config  Pull-up configuration for the CSN pin.
 *
 * @retval NRF_SUCCESS  Operation success.
*/
uint32_t spi_slave_set_cs_pull_up_config(uint32_t alternate_config);

/**@brief Function for changing defult drive for MISO pin.
 *
 * In the default configuration, pin drive for the MISO pin is set to S0S1.
 * If alternate configuration is required this function can be called before spi_slave_init()
 * to change default settings for MISO pin.
 *
 * @param[in] alternate_config  Drive configuration for the MISO pin.
 *
 * @retval NRF_SUCCESS  Operation success.
*/
uint32_t spi_slave_set_drive_config(uint32_t alternate_config);

#endif // SPI_SLAVE_H__

/** @} */
