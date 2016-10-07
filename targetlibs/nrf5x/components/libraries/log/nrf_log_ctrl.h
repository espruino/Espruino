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
#ifndef NRF_LOG_CTRL_H
#define NRF_LOG_CTRL_H

/**@file
 * @addtogroup nrf_log Logger module
 * @ingroup    app_common
 *
 * @defgroup nrf_log_ctrl Functions for controlling nrf_log
 * @{
 * @ingroup  nrf_log
 * @brief    The nrf_log control interface.
 */

#include "sdk_config.h"
#include "sdk_errors.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrf_log_ctrl_internal.h"
/**
 * @brief Timestamp function prototype.
 *
 * @return Timestamp value.
 */
typedef uint32_t (*nrf_log_timestamp_func_t)(void);

/**@brief Macro for initializing the logs.
 *
 * @note If timestamps are disabled in the configuration, then the provided pointer
 * can be NULL. Otherwise, it is expected that timestamp_getter is not NULL.
 *
 * @param timestamp_func Function that returns the timestamp.
 *
 * @return  NRF_SUCCESS after successful initialization, otherwise an error code.
 */
#define NRF_LOG_INIT(timestamp_func) NRF_LOG_INTERNAL_INIT(timestamp_func)


/**@brief Macro for processing a single log entry from a queue of deferred logs.
 *
 * You can call this macro from the main context or from the error handler to process
 * log entries one by one.
 *
 * @note If logs are not deferred, this call has no use and is defined as 'false'.
 *
 * @retval true    There are more logs to process in the buffer.
 * @retval false   No more logs in the buffer.
 */
#define NRF_LOG_PROCESS()    NRF_LOG_INTERNAL_PROCESS()

/** @brief Macro for processing all log entries from the buffer.
 * It blocks until all buffered entries are processed by the backend.
 *
 * @note If logs are not deferred, this call has no use and is empty.
 */
#define NRF_LOG_FLUSH()      NRF_LOG_INTERNAL_FLUSH()

/** @brief Macro for flushing log data before reset.
 *
 * @note If logs are not deferred, this call has no use and is empty.
 *
 * @note If RTT is used, then a breakpoint is hit once flushed.
 */
#define NRF_LOG_FINAL_FLUSH() NRF_LOG_INTERNAL_FINAL_FLUSH()

/** @brief Macro for changing functions that are used to handle log entries.
 *
 * @param default_handler Function for handling log entries.
 * @param bytes_handler   Function for handling hexdump entries.
 *
 */
#define NRF_LOG_HANDLERS_SET(default_handler, bytes_handler) \
    NRF_LOG_INTERNAL_HANDLERS_SET(default_handler, bytes_handler)

/**
 * @brief Function prototype for handling a log entry.
 *
 * The backend must implement such prototype.
 *
 * @param severity_level Severity level of the entry.
 * @param p_timestamp    Pointer to the timestamp value. No timestamp if NULL.
 * @param p_str          Pointer to a formatted string.
 * @param p_args         Pointer to an array of arguments for a formatted string.
 * @param nargs          Number of arguments in p_args.
 *
 * @retval true          If entry is successfully processed.
 * @retval false         If entry is not processed.
 */
typedef bool (*nrf_log_std_handler_t)(
    uint8_t                severity_level,
    const uint32_t * const p_timestamp,
    const char * const     p_str,
    uint32_t             * p_args,
    uint32_t               nargs);

/**
 * @brief Function prototype for handling a bytes-dumping log entry.
 *
 * The backend must implement such prototype. Two buffers are needed because data
 * is stored internally in a circular buffer so it can be fragmented into up to
 * two pieces.
 *
 * @param severity_level Severity level of the entry.
 * @param p_timestamp    Pointer to a timestamp value. No timestamp if NULL.
 * @param p_str          Prefix string for the bytes dump.
 * @param offset         Indication of how many bytes have already been processed.
 * @param p_buf0         Pointer to the first part of data.
 * @param buf0_length    Number of bytes in the first part.
 * @param p_buf1         Pointer to the second part of data. Optional.
 * @param buf1_length    Number of bytes in the second part.
 *
 * @return Number of bytes processed. If all bytes are processed, it should be a sum of
 *         buf0_length and buf1_length
 */
typedef uint32_t (*nrf_log_hexdump_handler_t)(
    uint8_t                severity_level,
    const uint32_t * const p_timestamp,
    const char * const     p_str,
    uint32_t               offset,
    const uint8_t * const  p_buf0,
    uint32_t               buf0_length,
    const uint8_t * const  p_buf1,
    uint32_t               buf1_length);


/**
 * @brief Function for initializing the frontend and the default backend.
 *
 * @ref NRF_LOG_INIT calls this function to initialize the frontend and the backend.
 * If custom backend is used, then @ref NRF_LOG_INIT should not be called.
 * Instead, frontend and user backend should be verbosely initialized.
 *
 * @param timestamp_func Function for getting a 32-bit timestamp.
 *
 * @return Error status.
 *
 */
ret_code_t nrf_log_init(nrf_log_timestamp_func_t timestamp_func);

/**
 * @brief Function for reinitializing the backend in blocking mode.
 */
ret_code_t nrf_log_blocking_backend_set(void);

/**
 * @brief Function for initializing the logger frontend.
 *
 * The frontend is initialized with functions for handling log entries. Those
 * functions are provided by the backend.
 *
 * @note This function needs to be called directly only if the @ref NRF_LOG_INIT macro
 * is not used to initialize the logger.
 *
 * @param std_handler      Function for handling standard log entries.
 * @param hexdump_handler  Function for handling hexdump log entries.
 * @param timestamp_func   Function for getting a timestamp. It cannot be NULL
 *                         unless timestamping is disabled.
 */
void nrf_log_frontend_init(nrf_log_std_handler_t     std_handler,
                           nrf_log_hexdump_handler_t hexdump_handler,
                           nrf_log_timestamp_func_t  timestamp_func);

/**
 * @brief Function for updating functions that handle log entries.
 *
 * @note Use this feature to change the log handling behavior in certain
 * situations, like in a fault handler.
 *
 * @param std_handler      Function for handling standard log entries.
 * @param hexdump_handler  Function for handling hexdump log entries.
 */
void nrf_log_handlers_set(nrf_log_std_handler_t     std_handler,
                          nrf_log_hexdump_handler_t hexdump_handler);

/**
 * @brief Function for handling a single log entry.
 *
 * Use this function only if the logs are buffered. It takes a single entry from the
 * buffer and attempts to process it.
 *
 * @retval true  If there are more entries to process.
 * @retval false If there are no more entries to process.
 */
bool nrf_log_frontend_dequeue(void);


#endif // NRF_LOG_CTRL_H

/**
 *@}
 **/
