/**
 *
 * @defgroup nrf_drv_clock_config CLOCK peripheral driver configuration
 * @{
 * @ingroup nrf_drv_clock
 */
/** @brief Enable CLOCK driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define CLOCK_ENABLED

/** @brief HF XTAL Frequency *
 *  Following options are avaiable:
 * - 0 - Default (64 MHz)
 * - 255 - Default (16 MHz) (nRF51 family only)
 * - 0 - 32 MHz (nRF51 family only)
 *
 * @note This is an NRF_CONFIG macro.
 */
#define CLOCK_CONFIG_XTAL_FREQ


/** @brief LF Clock Source *
 *  Following options are avaiable:
 * - 0 - RC
 * - 1 - XTAL
 * - 2 - Synth
 *
 * @note This is an NRF_CONFIG macro.
 */
#define CLOCK_CONFIG_LF_SRC


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
#define CLOCK_CONFIG_IRQ_PRIORITY



/** @} */
