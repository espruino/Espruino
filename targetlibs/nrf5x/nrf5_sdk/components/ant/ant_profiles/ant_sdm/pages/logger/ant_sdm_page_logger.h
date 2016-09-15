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

#ifndef ANT_SDM_PAGE_LOGGER_H__
#define ANT_SDM_PAGE_LOGGER_H__

#ifdef TRACE_SDM_GENERAL_ENABLE
#include "app_trace.h"
#define LOG_SDM app_trace_log
#else
#define LOG_SDM(...)
#endif // TRACE_SDM_GENERAL_ENABLE 

#ifdef TRACE_SDM_PAGE_1_ENABLE
#include "app_trace.h"
#define LOG_PAGE1 app_trace_log
#else
#define LOG_PAGE1(...)
#endif // TRACE_SDM_PAGE_1_ENABLE

#ifdef TRACE_SDM_PAGE_2_ENABLE
#include "app_trace.h"
#define LOG_PAGE2 app_trace_log
#else
#define LOG_PAGE2(...)
#endif // TRACE_SDM_PAGE_2_ENABLE

#ifdef TRACE_SDM_PAGE_3_ENABLE
#include "app_trace.h"
#define LOG_PAGE3 app_trace_log
#else
#define LOG_PAGE3(...)
#endif // TRACE_SDM_PAGE_3_ENABLE

#ifdef TRACE_SDM_PAGE_16_ENABLE
#include "app_trace.h"
#define LOG_PAGE16 app_trace_log
#else
#define LOG_PAGE16(...)
#endif // TRACE_SDM_PAGE_16_ENABLE

#ifdef TRACE_SDM_PAGE_22_ENABLE
#include "app_trace.h"
#define LOG_PAGE22 app_trace_log
#else
#define LOG_PAGE22(...)
#endif // TRACE_SDM_PAGE_22_ENABLE

#if (defined TRACE_SDM_PAGE_2_ENABLE) || (defined TRACE_SDM_PAGE_3_ENABLE)
#include "app_trace.h"
#define LOG_SPEED app_trace_log
#else
#define LOG_SPEED(...)
#endif // (defined TRACE_SDM_PAGE_2_ENABLE) || (defined TRACE_SDM_PAGE_3_ENABLE)

#endif // ANT_SDM_PAGE_LOGGER_H__
