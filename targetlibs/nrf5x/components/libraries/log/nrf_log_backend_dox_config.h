/**
 *
 * @defgroup nrf_log_backend_config Logging sink configuration
 * @{
 * @ingroup nrf_log_backend
 */
/** @brief Buffer for storing single output string
 *
 * Logger backend RAM usage is determined by this value.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_MAX_STRING_LENGTH


/** @brief Number of digits for timestamp
 *
 * If higher resolution timestamp source is used it might be needed to increase that
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_TIMESTAMP_DIGITS


/** @brief If enabled data is printed over UART *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_USES_UART

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
#define NRF_LOG_BACKEND_SERIAL_UART_BAUDRATE


/** @brief UART TX pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_UART_TX_PIN


/** @brief UART RX pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_UART_RX_PIN


/** @brief UART RTS pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_UART_RTS_PIN


/** @brief UART CTS pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_UART_CTS_PIN


/** @brief Hardware Flow Control *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - Enabled
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_UART_FLOW_CONTROL


/** @brief UART instance used *
 *  Following options are avaiable:
 * - 0
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_UART_INSTANCE



/** @brief If enabled data is printed using RTT *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_BACKEND_SERIAL_USES_RTT


/** @} */
