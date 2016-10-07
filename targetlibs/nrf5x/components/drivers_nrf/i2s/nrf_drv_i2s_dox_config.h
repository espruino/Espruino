/**
 *
 * @defgroup nrf_drv_i2s_config I2S peripheral driver configuration
 * @{
 * @ingroup nrf_drv_i2s
 */
/** @brief Enable I2S driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_ENABLED

/** @brief SCK pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_SCK_PIN


/** @brief LRCK pin *
 *  Minimum value: 1
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_LRCK_PIN


/** @brief MCK pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_MCK_PIN


/** @brief SDOUT pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_SDOUT_PIN


/** @brief SDIN pin *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_SDIN_PIN


/** @brief Mode *
 *  Following options are avaiable:
 * - 0 - Master
 * - 1 - Slave
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_MASTER


/** @brief Format *
 *  Following options are avaiable:
 * - 0 - I2S
 * - 1 - Aligned
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_FORMAT


/** @brief Alignment *
 *  Following options are avaiable:
 * - 0 - Left
 * - 1 - Right
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_ALIGN


/** @brief Sample width (bits) *
 *  Following options are avaiable:
 * - 0 - 8
 * - 1 - 16
 * - 2 - 24
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_SWIDTH


/** @brief Channels *
 *  Following options are avaiable:
 * - 0 - Stereo
 * - 1 - Left
 * - 2 - Right
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_CHANNELS


/** @brief MCK behavior *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 2147483648 - 32MHz/2
 * - 1342177280 - 32MHz/3
 * - 1073741824 - 32MHz/4
 * - 805306368 - 32MHz/5
 * - 671088640 - 32MHz/6
 * - 536870912 - 32MHz/8
 * - 402653184 - 32MHz/10
 * - 369098752 - 32MHz/11
 * - 285212672 - 32MHz/15
 * - 268435456 - 32MHz/16
 * - 201326592 - 32MHz/21
 * - 184549376 - 32MHz/23
 * - 142606336 - 32MHz/30
 * - 138412032 - 32MHz/31
 * - 134217728 - 32MHz/32
 * - 100663296 - 32MHz/42
 * - 68157440 - 32MHz/63
 * - 34340864 - 32MHz/125
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_MCK_SETUP


/** @brief MCK/LRCK ratio *
 *  Following options are avaiable:
 * - 0 - 32x
 * - 1 - 48x
 * - 2 - 64x
 * - 3 - 96x
 * - 4 - 128x
 * - 5 - 192x
 * - 6 - 256x
 * - 7 - 384x
 * - 8 - 512x
 *
 * @note This is an NRF_CONFIG macro.
 */
#define I2S_CONFIG_RATIO


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
#define I2S_CONFIG_IRQ_PRIORITY



/** @} */
