/**
 *
 * @defgroup app_simple_timer_config Simple application timer functionality configuration
 * @{
 * @ingroup app_simple_timer
 */
/** @brief Enabling simple_timer module *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SIMPLE_TIMER_ENABLED

/** @brief Timer frequency if in Timer mode *
 *  Following options are avaiable:
 * - 0 - 16 MHz
 * - 1 - 8 MHz
 * - 2 - 4 MHz
 * - 3 - 2 MHz
 * - 4 - 1 MHz
 * - 5 - 500 kHz
 * - 6 - 250 kHz
 * - 7 - 125 kHz
 * - 8 - 62.5 kHz
 * - 9 - 31.25 kHz
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SIMPLE_TIMER_CONFIG_FREQUENCY


/** @brief TIMER instance used *
 *  Following options are avaiable:
 * - 0 - 0
 * - 1 - 1
 * - 2 - 2
 * - 3 - 3 (nRF52 family only)
 * - 4 - 4 (nRF52 family only)
 *
 * @note This is an NRF_CONFIG macro.
 */
#define SIMPLE_TIMER_CONFIG_INSTANCE



/** @} */
