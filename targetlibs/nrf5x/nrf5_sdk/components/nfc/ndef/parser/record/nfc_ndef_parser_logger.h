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

#ifndef NFC_NDEF_PARSER_LOGGER_H__
#define NFC_NDEF_PARSER_LOGGER_H__

#ifdef NDEF_PARSER_LOG_ENABLE
    #ifdef ENABLE_DEBUG_LOG_SUPPORT

        #include "app_trace.h"

        #define NDEF_PARSER_TRACE app_trace_log

    #elif defined(NRF_LOG_USES_RTT)

        #include "nrf_log.h"


        #define NDEF_PARSER_TRACE(...)  NRF_LOG_PRINTF(##__VA_ARGS__)
  #endif
#endif

#ifndef NDEF_PARSER_TRACE
    #define NDEF_PARSER_TRACE(...)
#endif

#endif // NFC_NDEF_PARSER_LOGGER_H__
