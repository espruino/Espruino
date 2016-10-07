/**
 *
 * @defgroup nrf_drv_lpcomp_config LPCOMP peripheral driver configuration
 * @{
 * @ingroup nrf_drv_lpcomp
 */
/** @brief Enable LPCOMP driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define LPCOMP_ENABLED

/** @brief Reference voltage *
 *  Following options are avaiable:
 * - 0 - Supply 1/8
 * - 1 - Supply 2/8
 * - 2 - Supply 3/8
 * - 3 - Supply 4/8
 * - 4 - Supply 5/8
 * - 5 - Supply 6/8
 * - 6 - Supply 7/8
 * - 8 - Supply 1/16 (nRF52)
 * - 9 - Supply 3/16 (nRF52)
 * - 10 - Supply 5/16 (nRF52)
 * - 11 - Supply 7/16 (nRF52)
 * - 12 - Supply 9/16 (nRF52)
 * - 13 - Supply 11/16 (nRF52)
 * - 14 - Supply 13/16 (nRF52)
 * - 15 - Supply 15/16 (nRF52)
 * - 7 - External Ref 0
 * - 65543 - External Ref 1
 *
 * @note This is an NRF_CONFIG macro.
 */
#define LPCOMP_CONFIG_REFERENCE


/** @brief Detection *
 *  Following options are avaiable:
 * - 0 - Crossing
 * - 1 - Up
 * - 2 - Down
 *
 * @note This is an NRF_CONFIG macro.
 */
#define LPCOMP_CONFIG_DETECTION


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
#define LPCOMP_CONFIG_INPUT


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
#define LPCOMP_CONFIG_IRQ_PRIORITY



/** @} */
