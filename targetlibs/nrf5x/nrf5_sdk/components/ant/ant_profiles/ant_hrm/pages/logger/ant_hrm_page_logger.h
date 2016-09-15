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
#ifndef ANT_HRM_PAGE_LOGGER_H__
#define ANT_HRM_PAGE_LOGGER_H__

#ifdef TRACE_HRM_GENERAL_ENABLE
#include "app_trace.h"
#define LOG_HRM app_trace_log
#else
#define LOG_HRM(...)
#endif // TRACE_HRM_GENERAL_ENABLE 

#ifdef TRACE_HRM_PAGE_0_ENABLE
#include "app_trace.h"
#define LOG_PAGE0 app_trace_log
#else
#define LOG_PAGE0(...)
#endif // TRACE_HRM_PAGE_0_ENABLE

#ifdef TRACE_HRM_PAGE_1_ENABLE
#include "app_trace.h"
#define LOG_PAGE1 app_trace_log
#else
#define LOG_PAGE1(...)
#endif // TRACE_HRM_PAGE_1_ENABLE 

#ifdef TRACE_HRM_PAGE_2_ENABLE
#include "app_trace.h"
#define LOG_PAGE2 app_trace_log
#else
#define LOG_PAGE2(...)
#endif // TRACE_HRM_PAGE_2_ENABLE 

#ifdef TRACE_HRM_PAGE_3_ENABLE
#include "app_trace.h"
#define LOG_PAGE3 app_trace_log
#else
#define LOG_PAGE3(...)
#endif // TRACE_HRM_PAGE_3_ENABLE 

#ifdef TRACE_HRM_PAGE_4_ENABLE
#include "app_trace.h"
#define LOG_PAGE4 app_trace_log
#else
#define LOG_PAGE4(...)
#endif // TRACE_HRM_PAGE_4_ENABLE 

#endif // ANT_HRM_UTILS_H__
