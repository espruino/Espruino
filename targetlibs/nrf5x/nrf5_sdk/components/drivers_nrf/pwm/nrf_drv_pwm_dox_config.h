/**
 *
 * @defgroup nrf_drv_pwm_config PWM peripheral driver configuration
 * @{
 * @ingroup nrf_drv_pwm
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_ENABLED

/** @brief Out0 pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_OUT0_PIN


/** @brief Out1 pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_OUT1_PIN


/** @brief Out2 pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_OUT2_PIN


/** @brief Out3 pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_OUT3_PIN


/** @brief Base clock *
 *  Following options are avaiable:
 * - 0 - 16 MHz
 * - 1 - 8 MHz
 * - 2 - 4 MHz
 * - 3 - 2 MHz
 * - 4 - 1 MHz
 * - 5 - 500 kHz
 * - 6 - 250 kHz
 * - 7 - 125 MHz
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_BASE_CLOCK


/** @brief Count mode *
 *  Following options are avaiable:
 * - 0 - Up
 * - 1 - Up and Down
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_COUNT_MODE


/** @brief Top value *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_TOP_VALUE


/** @brief Load mode *
 *  Following options are avaiable:
 * - 0 - Common
 * - 1 - Grouped
 * - 2 - Individual
 * - 3 - Waveform
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_LOAD_MODE


/** @brief Step mode *
 *  Following options are avaiable:
 * - 0 - Auto
 * - 1 - Triggered
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM_DEFAULT_CONFIG_STEP_MODE


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
#define PWM_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Enable PWM0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM0_ENABLED


/** @brief Enable PWM1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM1_ENABLED


/** @brief Enable PWM2 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PWM2_ENABLED



/** @} */
