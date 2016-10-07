/**
 *
 * @defgroup nrf_drv_comp_config COMP peripheral driver configuration
 * @{
 * @ingroup nrf_drv_comp
 */
/** @brief Enable COMP driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_ENABLED

/** @brief Reference voltage *
 *  Following options are avaiable:
 * - 0 - Internal 1.2V
 * - 1 - Internal 1.8V
 * - 2 - Internal 2.4V
 * - 4 - VDD
 * - 7 - ARef
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_REF


/** @brief Main mode *
 *  Following options are avaiable:
 * - 0 - Single ended
 * - 1 - Differential
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_MAIN_MODE


/** @brief Speed mode *
 *  Following options are avaiable:
 * - 0 - Low power
 * - 1 - Normal
 * - 2 - High speed
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_SPEED_MODE


/** @brief Hystheresis *
 *  Following options are avaiable:
 * - 0 - No
 * - 1 - 50mV
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_HYST


/** @brief Current Source *
 *  Following options are avaiable:
 * - 0 - Off
 * - 1 - 2.5 uA
 * - 2 - 5 uA
 * - 3 - 10 uA
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_ISOURCE


/** @brief Analog input *
 *  Following options are avaiable:
 * - 0
 * - 1
 * - 2
 * - 3
 * - 4
 * - 5
 * - 6
 * - 7
 *
 * @note This is an NRF_CONFIG macro.
 */
#define COMP_CONFIG_INPUT


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
#define COMP_CONFIG_IRQ_PRIORITY



/** @} */
