/**
 *
 * @defgroup hci_slip_config SLIP protocol implementation used by HCI configuration
 * @{
 * @ingroup hci_slip
 */
/** @brief Enabling HCI transport module. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_SLIP_ENABLED

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
#define HCI_UART_BAUDRATE


/** @brief Hardware Flow Control *
 *  Following options are avaiable:
 * - 0 - Disabled
 * - 1 - Enabled
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_UART_FLOW_CONTROL


/** @brief UART RX pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_UART_RX_PIN


/** @brief UART TX pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_UART_TX_PIN


/** @brief UART RTS pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_UART_RTS_PIN


/** @brief UART CTS pin *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define HCI_UART_CTS_PIN



/** @} */
