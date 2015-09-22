/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 * @addtogroup nrf_spi_master SPI master HAL and driver
 * @ingroup    nrf_drivers
 * @brief      SPI master APIs.
 */

#ifndef NRF_DRV_SPI_H__
#define NRF_DRV_SPI_H__

#include "nordic_common.h"
#include "nrf_drv_config.h"
#include "nrf_spi.h"
#include "nrf_spim.h"
#include "sdk_errors.h"

#if defined(NRF52)
    #define NRF_DRV_SPI_PERIPHERAL(id)           \
        (CONCAT_3(SPI, id, _USE_EASY_DMA) == 1 ? \
            (void *)CONCAT_2(NRF_SPIM, id)       \
          : (void *)CONCAT_2(NRF_SPI, id))
    #define SPI2_IRQ            SPIM2_SPIS2_SPI2_IRQn
    #define SPI2_IRQ_HANDLER    SPIM2_SPIS2_SPI2_IRQHandler
#else
    #define NRF_DRV_SPI_PERIPHERAL(id)  (void *)CONCAT_2(NRF_SPI, id)
#endif
#define SPI0_IRQ            SPI0_TWI0_IRQn
#define SPI0_IRQ_HANDLER    SPI0_TWI0_IRQHandler
#define SPI1_IRQ            SPI1_TWI1_IRQn
#define SPI1_IRQ_HANDLER    SPI1_TWI1_IRQHandler


/**
 * @defgroup nrf_drv_spi_master SPI master driver
 * @{
 * @ingroup  nrf_spi_master
 *
 * @brief    Multi-instance SPI master driver.
 */

/**
 * @brief SPI master driver instance data structure.
 */
typedef struct
{
    void *    p_registers;  ///< Pointer to structure with SPI/SPIM peripheral instance registers.
    IRQn_Type irq;          ///< SPI/SPIM peripheral instance IRQ number.
    uint8_t   drv_inst_idx; ///< Driver instance index.
    bool      use_easy_dma; ///< True when peripheral with EasyDMA (i.e. SPIM) shall be used.
} nrf_drv_spi_t;

/**
 * @brief Macro for creating SPI master driver instance.
 */
#define NRF_DRV_SPI_INSTANCE(id)                        \
{                                                       \
    .p_registers  = NRF_DRV_SPI_PERIPHERAL(id),         \
    .irq          = CONCAT_3(SPI, id, _IRQ),            \
    .drv_inst_idx = CONCAT_3(SPI, id, _INSTANCE_INDEX), \
    .use_easy_dma = CONCAT_3(SPI, id, _USE_EASY_DMA)    \
}

/**
 * @brief This value may be provided instead of a pin number for signals MOSI,
 *        MISO and Slave Select to specify that given signal is not used and
 *        therefore it does not need to be connected to a pin.
 */
#define NRF_DRV_SPI_PIN_NOT_USED  0xFF

/**
 * @brief SPI data rates.
 */
typedef enum
{
    NRF_DRV_SPI_FREQ_125K = NRF_SPI_FREQ_125K, ///< 125 kbps.
    NRF_DRV_SPI_FREQ_250K = NRF_SPI_FREQ_250K, ///< 250 kbps.
    NRF_DRV_SPI_FREQ_500K = NRF_SPI_FREQ_500K, ///< 500 kbps.
    NRF_DRV_SPI_FREQ_1M   = NRF_SPI_FREQ_1M,   ///< 1 Mbps.
    NRF_DRV_SPI_FREQ_2M   = NRF_SPI_FREQ_2M,   ///< 2 Mbps.
    NRF_DRV_SPI_FREQ_4M   = NRF_SPI_FREQ_4M,   ///< 4 Mbps.
    NRF_DRV_SPI_FREQ_8M   = NRF_SPI_FREQ_8M    ///< 8 Mbps.
} nrf_drv_spi_frequency_t;

/**
 * @brief SPI modes.
 */
typedef enum
{
    NRF_DRV_SPI_MODE_0 = NRF_SPI_MODE_0, ///< SCK active high, sample on leading edge of clock.
    NRF_DRV_SPI_MODE_1 = NRF_SPI_MODE_1, ///< SCK active high, sample on trailing edge of clock.
    NRF_DRV_SPI_MODE_2 = NRF_SPI_MODE_2, ///< SCK active low, sample on leading edge of clock.
    NRF_DRV_SPI_MODE_3 = NRF_SPI_MODE_3  ///< SCK active low, sample on trailing edge of clock.
} nrf_drv_spi_mode_t;

/**
 * @brief SPI bit orders.
 */
typedef enum
{
    NRF_DRV_SPI_BIT_ORDER_MSB_FIRST = NRF_SPI_BIT_ORDER_MSB_FIRST, ///< Most significant bit shifted out first.
    NRF_DRV_SPI_BIT_ORDER_LSB_FIRST = NRF_SPI_BIT_ORDER_LSB_FIRST  ///< Least significant bit shifted out first.
} nrf_drv_spi_bit_order_t;

/**
 * @brief SPI master driver instance configuration structure.
 */
typedef struct
{
    uint8_t sck_pin;      ///< SCK pin number.
    uint8_t mosi_pin;     ///< MOSI pin number.
                          /**< Optional - use @ref NRF_DRV_SPI_PIN_NOT_USED
                           *   value instead if this signal is not needed. */
    uint8_t miso_pin;     ///< MISO pin number.
                          /**< Optional - use @ref NRF_DRV_SPI_PIN_NOT_USED
                           *   value instead if this signal is not needed. */
    uint8_t ss_pin;       ///< Slave Select pin number.
                          /**< Optional - use @ref NRF_DRV_SPI_PIN_NOT_USED
                           *   value instead if this signal is not needed. */
    uint8_t irq_priority; ///< Interrupt priority.
    uint8_t orc;          ///< Over-run character.
                          /**< Used when all bytes from TX buffer are sent,
                               but transfer continues due to RX. */
    nrf_drv_spi_frequency_t frequency; ///< SPI frequency.
    nrf_drv_spi_mode_t      mode;      ///< SPI mode.
    nrf_drv_spi_bit_order_t bit_order; ///< SPI bit order.
} nrf_drv_spi_config_t;

/**
 * @brief SPI master instance default configuration.
 */
#define NRF_DRV_SPI_DEFAULT_CONFIG(id)                       \
{                                                            \
    .sck_pin      = CONCAT_3(SPI, id, _CONFIG_SCK_PIN),      \
    .mosi_pin     = CONCAT_3(SPI, id, _CONFIG_MOSI_PIN),     \
    .miso_pin     = CONCAT_3(SPI, id, _CONFIG_MISO_PIN),     \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,                \
    .irq_priority = CONCAT_3(SPI, id, _CONFIG_IRQ_PRIORITY), \
    .orc          = 0xFF,                                    \
    .frequency    = NRF_DRV_SPI_FREQ_4M,                     \
    .mode         = NRF_DRV_SPI_MODE_0,                      \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
}

/**
 * @brief SPI master driver events, passed to the handler routine provided
 *        during initialization.
 */
typedef enum
{
    NRF_DRV_SPI_EVENT_DONE, ///< Transfer done.
} nrf_drv_spi_event_t;

/**
 * @brief SPI master driver event handler type.
 */
typedef void (*nrf_drv_spi_handler_t)(nrf_drv_spi_event_t event);


/**
 * @brief Function for initializing the SPI master driver instance.
 *
 * This function configures and enables specified peripheral.
 *
 * @param[in] p_instance Pointer to the instance structure.
 * @param[in] p_config   Pointer to the structure with initial configuration.
 *                       If NULL, default configuration will be used.
 * @param     handler    Event handler provided by the user. If NULL, transfers
 *                       will be performed in blocking mode.
 *
 * @retval NRF_SUCCESS             Initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE Driver was already initialized.
 */
ret_code_t nrf_drv_spi_init(nrf_drv_spi_t const * const p_instance,
                            nrf_drv_spi_config_t const * p_config,
                            nrf_drv_spi_handler_t handler);

/**
 * @brief Function for uninitializing the SPI master driver instance.
 *
 * @param[in] p_instance Pointer to the instance structure.
 */
void       nrf_drv_spi_uninit(nrf_drv_spi_t const * const p_instance);

/**
 * @brief Function for starting the SPI data transfer.
 *
 * If an event handler was provided in nrf_drv_spi_init() call, this function
 * returns immediately and the handler is called when the transfer is done.
 * Otherwise, the transfer is performed in blocking mode, i.e. this function
 * returns when the transfer is finished.
 *
 * @note Peripherals using EasyDMA (i.e. SPIM) require that the transfer buffers
 *       are placed in the Data RAM region. If they are not and SPIM instance is
 *       used, this function will fail with error code NRF_ERROR_INVALID_ADDR.
 *
 * @param[in] p_instance       Pointer to the instance structure.
 * @param[in] p_tx_buffer      Pointer to the transmit buffer. May be NULL,
 *                             if there is nothing to send.
 * @param     tx_buffer_length Length of the transmit buffer.
 * @param[in] p_rx_buffer      Pointer to the receive buffer. May be NULL,
 *                             if there is nothing to receive.
 * @param     rx_buffer_length Length of the receive buffer.
 *
 * @retval NRF_SUCCESS            Operation was successful.
 * @retval NRF_ERROR_BUSY         Previously started transfer has not finished
 *                                yet.
 * @retval NRF_ERROR_INVALID_ADDR Provided buffers are not placed in the Data
 *                                RAM region. See note.
 */
ret_code_t nrf_drv_spi_transfer(nrf_drv_spi_t const * const p_instance,
                                uint8_t const * p_tx_buffer,
                                uint8_t         tx_buffer_length,
                                uint8_t       * p_rx_buffer,
                                uint8_t         rx_buffer_length);

#endif // NRF_DRV_SPI_H__

/** @} */
