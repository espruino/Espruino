/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#ifndef HAL_NFC_LOGGER_H__
#define HAL_NFC_LOGGER_H__

#ifdef ENABLE_DEBUG_LOG_SUPPORT

#include "app_trace.h"

#define LOG_HAL_NFC app_trace_log

#elif defined(NRF_LOG_USES_RTT)

#include "nrf_log.h"
//#include "SEGGER_RTT.h"

#define LOG_HAL_NFC(...)  NRF_LOG_PRINTF(##__VA_ARGS__)

#else

#define LOG_HAL_NFC(...)

#endif // ENABLE_DEBUG_LOG_SUPPORT

#endif // HAL_NFC_LOGGER_H__
