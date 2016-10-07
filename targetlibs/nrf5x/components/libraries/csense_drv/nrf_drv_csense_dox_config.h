/**
 *
 * @defgroup nrf_drv_csense_config Capacitive sensor module configuration
 * @{
 * @ingroup nrf_drv_csense
 */
/** @brief Enabled Capacitive touch sensor driver. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_DRV_CSENSE_ENABLED

/** @brief First TIMER instance used by the driver (except nRF51) *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER0_FOR_CSENSE


/** @brief Second TIMER instance used by the driver (except nRF51) *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define TIMER1_FOR_CSENSE


/** @brief Single measurement period.
 *
 * Time of single measurement can be calculated as T = (1/2)*MEASUREMENT_PERIOD*(1/f_OSC) where f_OSC = I_SOURCE / (2C*(VUP-VDOWN) ). I_SOURCE, VUP and VDOWN are values used to initialize COMP and C is capacitance of used pad.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEASUREMENT_PERIOD



/** @} */
