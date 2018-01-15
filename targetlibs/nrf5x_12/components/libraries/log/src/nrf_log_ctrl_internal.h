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
#ifndef NRF_LOG_CTRL_INTERNAL_H
#define NRF_LOG_CTRL_INTERNAL_H
/**
 * @cond (NODOX)
 * @defgroup nrf_log_ctrl_internal Auxiliary internal types declarations
 * @{
 * @internal
 */


#if NRF_LOG_ENABLED
#include "nordic_common.h"
#include "app_util_platform.h"

#define NRF_LOG_INTERNAL_INIT(timestamp_func) \
    nrf_log_init(timestamp_func)

#if (NRF_LOG_DEFERRED == 0)
#define NRF_LOG_INTERNAL_PROCESS() false
#define NRF_LOG_INTERNAL_FLUSH()
#define NRF_LOG_INTERNAL_FINAL_FLUSH()
#else
#define NRF_LOG_INTERNAL_PROCESS() nrf_log_frontend_dequeue()
#define NRF_LOG_INTERNAL_FLUSH()            \
    do {                                    \
        while (NRF_LOG_INTERNAL_PROCESS()); \
    } while(0)

#if NRF_LOG_BACKEND_SERIAL_USES_RTT
#define NRF_LOG_INTERNAL_BACKEND_FINAL NRF_BREAKPOINT
#else
#define NRF_LOG_INTERNAL_BACKEND_FINAL
#endif

#define NRF_LOG_INTERNAL_FINAL_FLUSH()      \
    do {                                    \
    (void)nrf_log_blocking_backend_set();   \
        NRF_LOG_INTERNAL_FLUSH();           \
        NRF_LOG_INTERNAL_BACKEND_FINAL;     \
    } while(0)

#endif

#define NRF_LOG_INTERNAL_HANDLERS_SET(default_handler, bytes_handler) \
    nrf_log_handlers_set(default_handler, bytes_handler)

#else // NRF_LOG_ENABLED
#define NRF_LOG_INTERNAL_PROCESS()            false
#define NRF_LOG_INTERNAL_FLUSH()
#define NRF_LOG_INTERNAL_INIT(timestamp_func) NRF_SUCCESS
#define NRF_LOG_INTERNAL_HANDLERS_SET(default_handler, bytes_handler) \
    UNUSED_PARAMETER(default_handler); UNUSED_PARAMETER(bytes_handler)
#define NRF_LOG_INTERNAL_FINAL_FLUSH()
#endif // NRF_LOG_ENABLED

/** @}
 * @endcond
 */
#endif // NRF_LOG_CTRL_INTERNAL_H
