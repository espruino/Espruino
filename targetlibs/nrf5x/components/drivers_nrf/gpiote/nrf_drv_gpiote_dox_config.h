/**
 *
 * @defgroup nrf_drv_gpiote_config GPIOTE peripheral driver configuration
 * @{
 * @ingroup nrf_drv_gpiote
 */
/** @brief Enable GPIOTE driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define GPIOTE_ENABLED

/** @brief Number of lower power input pins *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS


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
#define GPIOTE_CONFIG_IRQ_PRIORITY



/** @} */
