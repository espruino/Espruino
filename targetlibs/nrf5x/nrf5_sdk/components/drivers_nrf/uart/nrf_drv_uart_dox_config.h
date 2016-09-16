/**
 *
 * @defgroup nrf_drv_uart_config UART/UARTE peripheral driver configuration
 * @{
 * @ingroup nrf_drv_uart
 */
/** @brief Enable UART driver *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_ENABLED

/** @brief Hardware Flow Control *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - Enabled
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_DEFAULT_CONFIG_HWFC


/** @brief Parity *
 *  Following options are avaiable:
 * - 0 - Excluded
 * - 14 - Included
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_DEFAULT_CONFIG_PARITY


/** @brief Default Baudrate *
 *  Following options are avaiable:
 * - 323584 - 1200 baud
 * - 643072 - 2400 baud
 * - 1290240 - 4800 baud
 * - 2576384 - 9600 baud
 * - 3862528 - 14400 baud
 * - 5152768 - 19200 baud
 * - 7716864 - 28800 baud
 * - 10289152 - 38400 baud
 * - 15400960 - 57600 baud
 * - 20615168 - 76800 baud
 * - 30801920 - 115200 baud (nRF52 family only)
 * - 30924800 - 115200 baud (nRF51 family only)
 * - 61865984 - 230400 baud
 * - 67108864 - 250000 baud
 * - 121634816 - 460800 baud
 * - 251658240 - 921600 baud
 * - 268435456 - 57600 baud
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_DEFAULT_CONFIG_BAUDRATE


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
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY


/** @brief Default setting for using EasyDMA *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART0_CONFIG_USE_EASY_DMA


/** @brief Driver supporting EasyDMA *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_EASY_DMA_SUPPORT


/** @brief Driver supporting Legacy mode *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define UART_LEGACY_SUPPORT



/** @} */
