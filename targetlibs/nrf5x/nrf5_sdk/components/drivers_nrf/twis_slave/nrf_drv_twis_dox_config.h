/**
 *
 * @defgroup nrf_drv_twis_config TWIS peripheral driver configuration
 * @{
 * @ingroup nrf_drv_twis
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_ENABLED

/** @brief Address0 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_DEFAULT_CONFIG_ADDR0


/** @brief Address1 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_DEFAULT_CONFIG_ADDR1


/** @brief SCL pin pull configuration *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - Pull down
 * - 3 - Pull up
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_DEFAULT_CONFIG_SCL_PULL


/** @brief SDA pin pull configuration *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - Pull down
 * - 3 - Pull up
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_DEFAULT_CONFIG_SDA_PULL


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
#define TWIS_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Enable TWIS0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS0_ENABLED


/** @brief Enable TWIS1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS1_ENABLED


/** @brief Assume that any instance would be initialized only once
 *
 * Optimization flag. Registers used by TWIS are shared by other peripherals. Normally, during initialization driver tries to clear all registers to known state before doing the initialization itself. This gives initialization safe procedure, no matter when it would be called. If you activate TWIS only once and do never uninitialize it - set this flag to 1 what gives more optimal code.
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_ASSUME_INIT_AFTER_RESET_ONLY


/** @brief Remove support for synchronous mode
 *
 * Synchronous mode would be used in specific situations. And it uses some additional code and data memory to safely process state machine by polling it in status functions. If this functionality is not required it may be disabled to free some resources.
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWIS_NO_SYNC_MODE



/** @} */
