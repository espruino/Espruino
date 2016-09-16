/**
 *
 * @defgroup nrf_drv_qdec_config QDEC peripheral driver configuration
 * @{
 * @ingroup nrf_drv_qdec
 */
/** @brief Enable QDEC driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_ENABLED

/** @brief Report period *
 *  Following options are avaiable:
 * - 0 - 10 Samples
 * - 1 - 40 Samples
 * - 2 - 80 Samples
 * - 3 - 120 Samples
 * - 4 - 160 Samples
 * - 5 - 200 Samples
 * - 6 - 240 Samples
 * - 7 - 280 Samples
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_REPORTPER


/** @brief Sample period *
 *  Following options are avaiable:
 * - 0 - 128 us
 * - 1 - 256 us
 * - 2 - 512 us
 * - 3 - 1024 us
 * - 4 - 2048 us
 * - 5 - 4096 us
 * - 6 - 8192 us
 * - 7 - 16384 us
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_SAMPLEPER


/** @brief A pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_PIO_A


/** @brief B pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_PIO_B


/** @brief LED pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_PIO_LED


/** @brief LED pre *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_LEDPRE


/** @brief LED polarity *
 *  Following options are avaiable:
 * - 0 - Active low
 * - 1 - Active high
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_LEDPOL


/** @brief Debouncing enable *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_DBFEN


/** @brief Sample ready interrupt enable *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define QDEC_CONFIG_SAMPLE_INTEN


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
#define QDEC_CONFIG_IRQ_PRIORITY



/** @} */
