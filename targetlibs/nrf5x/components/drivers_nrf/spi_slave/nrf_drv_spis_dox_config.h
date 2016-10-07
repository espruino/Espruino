/**
 *
 * @defgroup nrf_drv_spis_config SPI Slave driver configuration
 * @{
 * @ingroup nrf_drv_spis
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS_ENABLED

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
#define SPIS_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Mode *
 *  Following options are avaiable:
 * - 0 - MODE_0
 * - 1 - MODE_1
 * - 2 - MODE_2
 * - 3 - MODE_3
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS_DEFAULT_MODE


/** @brief SPIS default bit order *
 *  Following options are avaiable:
 * - 0 - MSB first
 * - 1 - LSB first
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS_DEFAULT_BIT_ORDER


/** @brief SPIS default DEF character *
 *  Minimum value: 0
 *  Maximum value: 255
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS_DEFAULT_DEF


/** @brief SPIS default ORC character *
 *  Minimum value: 0
 *  Maximum value: 255
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS_DEFAULT_ORC


/** @brief Enable SPIS0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS0_ENABLED


/** @brief Enable SPIS1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS1_ENABLED


/** @brief Enable SPIS2 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SPIS2_ENABLED



/** @} */
