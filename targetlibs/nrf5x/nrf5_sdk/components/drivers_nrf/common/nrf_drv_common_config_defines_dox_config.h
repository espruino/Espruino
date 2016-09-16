/**
 *
 * @defgroup nrf_drv_common_config_defines_config Software Component configuration
 * @{
 * @ingroup nrf_drv_common_config_defines
 */
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
#define NRF_DRV_IRQ_PRIORITY


/** @brief Interrupt priority
 *
 * Priorities 0,1,4,5 (nRF52) are reserved for SoftDevice
 *
 *  Following options are avaiable:
 * - 0 - 0 (highest)
 * - 1 - 1
 * - 2 - 2
 * - 3 - 3
 * - 4 - 4
 * - 5 - 5
 * - 6 - 6
 * - 7 - 7
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_DRV_IRQ_PRIORITY_8LEVELS


/** @brief Pin number *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_DRV_PIN


/** @} */
