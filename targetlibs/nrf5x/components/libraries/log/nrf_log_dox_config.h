/**
 *
 * @defgroup nrf_log_config Logging configuration
 * @{
 * @ingroup nrf_log
 */
/** @brief Logging module for nRF5 SDK *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_ENABLED

/** @brief If enabled then ANSI escape code for colors is prefixed to every string *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_USES_COLORS

/** @brief ANSI escape code prefix. *
 *  Following options are avaiable:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_COLOR_DEFAULT


/** @brief ANSI escape code prefix. *
 *  Following options are avaiable:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_ERROR_COLOR


/** @brief ANSI escape code prefix. *
 *  Following options are avaiable:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_WARNING_COLOR



/** @brief Default Severity level *
 *  Following options are avaiable:
 * - 0 - Off
 * - 1 - Error
 * - 2 - Warning
 * - 3 - Info
 * - 4 - Debug
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_DEFAULT_LEVEL


/** @brief Enable deffered logger.
 *
 * Log data is buffered and can be processed in idle.
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_DEFERRED

/** @brief Size of the buffer for logs in words.
 *
 * Must be power of 2
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_DEFERRED_BUFSIZE



/** @brief Enable timestamping
 *
 * Function for getting the timestamp is provided by the user
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define NRF_LOG_USES_TIMESTAMP



/** @} */
