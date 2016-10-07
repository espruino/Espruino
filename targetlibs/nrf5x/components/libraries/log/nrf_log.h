/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**@file
 *
 * @defgroup nrf_log Logger module
 * @{
 * @ingroup app_common
 *
 * @brief The nrf_log module interface.
 */

#ifndef NRF_LOG_H_
#define NRF_LOG_H_

#include "sdk_config.h"

/** @brief  Default module name prefix.
 *
 * The prefix can be defined in a module to override the default.
 */
#ifndef NRF_LOG_MODULE_NAME
    #define NRF_LOG_MODULE_NAME ""
#endif

/** @brief Severity level for the module.
 *
 * The severity level can be defined in a module to override the default.
 */
#ifndef NRF_LOG_LEVEL
    #define NRF_LOG_LEVEL NRF_LOG_DEFAULT_LEVEL
#endif

/** @brief  Color prefix of debug logs for the module.
 *
 * This color prefix can be defined in a module to override the default.
 */
#ifndef NRF_LOG_DEBUG_COLOR
    #define NRF_LOG_DEBUG_COLOR NRF_LOG_COLOR_DEFAULT
#endif

/** @brief  Color prefix of info logs for the module.
 *
 * This color prefix can be defined in a module to override the default.
 */
#ifndef NRF_LOG_INFO_COLOR
    #define NRF_LOG_INFO_COLOR NRF_LOG_COLOR_DEFAULT
#endif

#include "nrf_log_internal.h"

/** @def NRF_LOG_ERROR
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes error logs.
 */

/** @def NRF_LOG_WARNING
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes warning logs.
 */

/** @def NRF_LOG_INFO
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes info logs.
 */

/** @def NRF_LOG_DEBUG
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes debug logs.
 */

#define NRF_LOG_ERROR(...)                     NRF_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define NRF_LOG_WARNING(...)                   NRF_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define NRF_LOG_INFO(...)                      NRF_LOG_INTERNAL_INFO( __VA_ARGS__)
#define NRF_LOG_DEBUG(...)                     NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)

/**
 * @brief A macro for logging a formatted string without any prefix or timestamp.
 */
#define NRF_LOG_RAW_INFO(...)                  NRF_LOG_INTERNAL_RAW_INFO( __VA_ARGS__)

/** @def NRF_LOG_HEXDUMP_ERROR
 *  @brief Macro for logging raw bytes.
 *  @details It is compiled in only if @ref NRF_LOG_LEVEL includes error logs.
 *
 * @param p_data     Pointer to data.
 * @param len        Data length in bytes.
 */
/** @def NRF_LOG_HEXDUMP_WARNING
 *  @brief Macro for logging raw bytes.
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes warning logs.
 *
 * @param p_data     Pointer to data.
 * @param len        Data length in bytes.
 */
/** @def NRF_LOG_HEXDUMP_INFO
 *  @brief Macro for logging raw bytes.
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes info logs.
 *
 * @param p_data     Pointer to data.
 * @param len        Data length in bytes.
 */
/** @def NRF_LOG_HEXDUMP_DEBUG
 *  @brief Macro for logging raw bytes.
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes debug logs.
 *
 * @param p_data     Pointer to data.
 * @param len        Data length in bytes.
 */
#define NRF_LOG_HEXDUMP_ERROR(p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_ERROR(p_data, len)
#define NRF_LOG_HEXDUMP_WARNING(p_data, len) NRF_LOG_INTERNAL_HEXDUMP_WARNING(p_data, len)
#define NRF_LOG_HEXDUMP_INFO(p_data, len)    NRF_LOG_INTERNAL_HEXDUMP_INFO(p_data, len)
#define NRF_LOG_HEXDUMP_DEBUG(p_data, len)   NRF_LOG_INTERNAL_HEXDUMP_DEBUG(p_data, len)

/**
 * @brief Macro for logging hexdump without any prefix or timestamp.
 */
#define NRF_LOG_RAW_HEXDUMP_INFO(p_data, len) NRF_LOG_INTERNAL_RAW_HEXDUMP_INFO(p_data, len)

/**
 * @brief A macro for blocking reading from bidirectional backend used for logging.
 *
 * Macro call is blocking and returns when single byte is received.
 */
#define NRF_LOG_GETCHAR()                    NRF_LOG_INTERNAL_GETCHAR()

/**
 * @brief Function for copying a string to the internal logger buffer if logs are deferred.
 *
 * Use this function to store a string that is volatile (for example allocated
 * on stack) or that may change before the deferred logs are processed. Such string is copied
 * into the internal logger buffer and is persistent until the log is processed.
 *
 * @note If the logs are not deferred, then this function returns the input parameter.
 *
 * @param p_str Pointer to the user string.
 *
 * @return Address to the location where the string is stored in the internal logger buffer.
 */
uint32_t nrf_log_push(char * const p_str);

/**
 * @brief Macro to be used in a formatted string to a pass float number to the log.
 *
 * Macro should be used in formatted string instead of the %f specifier together with
 * @ref NRF_LOG_FLOAT macro.
 * Example: NRF_LOG_INFO("My float number" NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(f)))
 */
#define NRF_LOG_FLOAT_MARKER "%d.%02d"

/**
 * @brief Macro for dissecting a float number into two numbers (integer and residuum).
 */
#define NRF_LOG_FLOAT(val) (int32_t)(val), (uint32_t)(((val)-(int32_t)val)*100)

#endif // NRF_LOG_H_

/** @} */
