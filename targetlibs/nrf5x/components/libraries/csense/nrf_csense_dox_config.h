/**
 *
 * @defgroup nrf_csense_config nrf_csense module configuration
 * @{
 * @ingroup nrf_csense
 */
/** @brief Enabling CSENSE capcitive touch sensor module *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_ENABLED

/** @brief Minimal value of change to decide that pad was touched. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_PAD_HYSTERESIS


/** @brief Minimal value measured on pad to take its value while calculating step. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_PAD_DEVIATION


/** @brief Minimum normalized value on pad to take its value into account. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_MIN_PAD_VALUE


/** @brief Maximum number of pads used for one instance. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_MAX_PADS_NUMBER


/** @brief Maximum normalized value got from measurement. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_MAX_VALUE


/** @brief Output pin used by lower module.
 *
 * This is only used when running on NRF51.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_CSENSE_OUTPUT_PIN



/** @} */
