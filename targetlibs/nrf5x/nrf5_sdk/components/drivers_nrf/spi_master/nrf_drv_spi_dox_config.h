/**
 *
 * @defgroup nrf_drv_spi_config SPI/SPIM peripheral driver configuration
 * @{
 * @ingroup nrf_drv_spi
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_ENABLED

/** @brief Enables logging in the module. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_CONFIG_LOG_ENABLED

/** @brief Default Severity level *
 *  Following options are avaiable:
 * - 0 - Off
 * - 1 - Error
 * - 2 - Warning
 * - 3 - Info
 * - 4 - Debug
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_CONFIG_LOG_LEVEL


/** @brief ANSI escape code prefix. *
 *  Following options are avaiable:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_CONFIG_INFO_COLOR


/** @brief ANSI escape code prefix. *
 *  Following options are avaiable:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_CONFIG_DEBUG_COLOR



/** @brief Interrupt priority
 *
 * Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
 *
 *  Following options are avaiable:
 * - 0 - 0 (highest)
 * - 1 - 1
 * - 2 - 2
 * - 3 - 3
 * - 4 - 4 (except nRF51 family)
 * - 5 - 5 (except nRF51 family)
 * - 6 - 6 (except nRF51 family)
 * - 7 - 7 (except nRF51 family)
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Enable SPI0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI0_ENABLED

/** @brief Use EasyDMA *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI0_USE_EASY_DMA



/** @brief Enable SPI1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI1_ENABLED

/** @brief Use EasyDMA *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI1_USE_EASY_DMA



/** @brief Enable SPI2 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI2_ENABLED

/** @brief Use EasyDMA *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPI2_USE_EASY_DMA




/** @} */
