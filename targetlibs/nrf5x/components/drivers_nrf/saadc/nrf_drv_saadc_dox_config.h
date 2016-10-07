/**
 *
 * @defgroup nrf_drv_saadc_config SAADC peripheral driver configuration
 * @{
 * @ingroup nrf_drv_saadc
 */
/** @brief Enable SAADC driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SAADC_ENABLED

/** @brief Resolution *
 *  Following options are avaiable:
 * - 0 - 8 bit
 * - 1 - 10 bit
 * - 2 - 12 bit
 * - 3 - 14 bit
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SAADC_CONFIG_RESOLUTION


/** @brief Sample period *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - 2x
 * - 2 - 4x
 * - 3 - 8x
 * - 4 - 16x
 * - 5 - 32x
 * - 6 - 64x
 * - 7 - 128x
 * - 8 - 256x
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SAADC_CONFIG_OVERSAMPLE


/** @brief Enabling low power mode *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SAADC_CONFIG_LP_MODE


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
#define SAADC_CONFIG_IRQ_PRIORITY



/** @} */
