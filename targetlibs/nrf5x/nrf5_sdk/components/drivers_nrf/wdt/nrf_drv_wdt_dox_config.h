/**
 *
 * @defgroup nrf_drv_wdt_config WDT peripheral driver configuration
 * @{
 * @ingroup nrf_drv_wdt
 */
/** @brief Enable WDT driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define WDT_ENABLED

/** @brief WDT behavior in CPU SLEEP or HALT mode *
 *  Following options are avaiable:
 * - 1 - Run in SLEEP, Pause in HALT
 * - 8 - Pause in SLEEP, Run in HALT
 * - 9 - Run in SLEEP and HALT
 * - 0 - Pause in SLEEP and HALT
 *
 * @note This is an NRF_CONFIG macro.
 */
#define WDT_CONFIG_BEHAVIOUR


/** @brief Reload value *
 *  Minimum value: 15
 *  Maximum value: 4294967295
 *
 * @note This is an NRF_CONFIG macro.
 */
#define WDT_CONFIG_RELOAD_VALUE


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
#define WDT_CONFIG_IRQ_PRIORITY



/** @} */
