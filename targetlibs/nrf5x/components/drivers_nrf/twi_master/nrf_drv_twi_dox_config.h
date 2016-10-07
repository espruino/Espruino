/**
 *
 * @defgroup nrf_drv_twi_config TWI/TWIM peripheral driver configuration
 * @{
 * @ingroup nrf_drv_twi
 */
/** @brief  *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI_ENABLED

/** @brief Frequency *
 *  Following options are avaiable:
 * - 26738688 - 100k
 * - 67108864 - 250k
 * - 104857600 - 400k
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI_DEFAULT_CONFIG_FREQUENCY


/** @brief Enables bus clearing procedure during init *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI_DEFAULT_CONFIG_CLR_BUS_INIT


/** @brief Enables bus holding after uninit *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI_DEFAULT_CONFIG_HOLD_BUS_UNINIT


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
#define TWI_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Enable TWI0 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI0_ENABLED

/** @brief Use EasyDMA (if present) *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI0_USE_EASY_DMA



/** @brief Enable TWI1 instance *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI1_ENABLED

/** @brief Use EasyDMA (if present) *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TWI1_USE_EASY_DMA




/** @} */
