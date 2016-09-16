/**
 *
 * @defgroup nrf_drv_timer_config TIMER periperal driver configuration
 * @{
 * @ingroup nrf_drv_timer
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER_ENABLED

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
#define TIMER_DEFAULT_CONFIG_FREQUENCY


/** @brief Timer mode or operation *
 *  Following options are avaiable:
 * - 0 - Timer
 * - 1 - Counter
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER_DEFAULT_CONFIG_MODE


/** @brief Timer counter bit width *
 *  Following options are avaiable:
 * - 0 - 16 bit
 * - 1 - 8 bit
 * - 2 - 24 bit
 * - 3 - 32 bit
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER_DEFAULT_CONFIG_BIT_WIDTH


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
#define TIMER_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Enable TIMER0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER0_ENABLED


/** @brief Enable TIMER1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER1_ENABLED


/** @brief Enable TIMER2 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER2_ENABLED


/** @brief Enable TIMER3 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER3_ENABLED


/** @brief Enable TIMER4 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER4_ENABLED



/** @} */
