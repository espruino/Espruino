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

#ifndef ANT_BPWR_PAGE_LOGGER_H__
#define ANT_BPWR_PAGE_LOGGER_H__

#ifdef TRACE_BPWR_GENERAL_ENABLE
#include "app_trace.h"
#define LOG_BPWR app_trace_log
#else
#define LOG_BPWR(...)
#endif // TRACE_BPWR_GENERAL_ENABLE 

#ifdef TRACE_BPWR_PAGE_1_ENABLE
#include "app_trace.h"
#define LOG_PAGE1 app_trace_log
#else
#define LOG_PAGE1(...)
#endif // TRACE_BPWR_PAGE_1_ENABLE

#ifdef TRACE_BPWR_PAGE_16_ENABLE
#include "app_trace.h"
#define LOG_PAGE16 app_trace_log
#else
#define LOG_PAGE16(...)
#endif // TRACE_BPWR_PAGE_16_ENABLE 

#ifdef TRACE_BPWR_PAGE_17_ENABLE
#include "app_trace.h"
#define LOG_PAGE17 app_trace_log
#else
#define LOG_PAGE17(...)
#endif // TRACE_BPWR_PAGE_17_ENABLE 

#ifdef TRACE_BPWR_PAGE_18_ENABLE
#include "app_trace.h"
#define LOG_PAGE18 app_trace_log
#else
#define LOG_PAGE18(...)
#endif // TRACE_BPWR_PAGE_18_ENABLE 

#if ((defined TRACE_BPWR_PAGE_18_ENABLE) || (defined TRACE_BPWR_PAGE_17_ENABLE) || (defined TRACE_BPWR_PAGE_16_ENABLE))
#include "app_trace.h"
#define LOG_CADENCE app_trace_log
#else
#define LOG_CADENCE(...)
#endif // ((defined TRACE_BPWR_PAGE_18_ENABLE) || (defined TRACE_BPWR_PAGE_17_ENABLE) || (defined TRACE_BPWR_PAGE_16_ENABLE))

#endif // ANT_BPWR_PAGE_LOGGER_H__
