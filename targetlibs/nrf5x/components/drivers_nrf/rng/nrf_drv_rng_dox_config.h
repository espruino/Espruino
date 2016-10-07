/**
 *
 * @defgroup nrf_drv_rng_config RNG peripheral driver configuration
 * @{
 * @ingroup nrf_drv_rng
 */
/** @brief Enable RNG driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define RNG_ENABLED

/** @brief Error correction *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define RNG_CONFIG_ERROR_CORRECTION


/** @brief Pool size *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define RNG_CONFIG_POOL_SIZE


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
#define RNG_CONFIG_IRQ_PRIORITY



/** @} */
