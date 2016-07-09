#ifndef ANT_COMMON_PAGE_LOGGER_H__
#define ANT_COMMON_PAGE_LOGGER_H__

#ifdef TRACE_COMMON_PAGE_70_ENABLE
#include "app_trace.h"
#define LOG_PAGE70 app_trace_log
#else
#define LOG_PAGE70(...)
#endif // TRACE_COMMON_PAGE_70_ENABLE 

#ifdef TRACE_COMMON_PAGE_80_ENABLE
#include "app_trace.h"
#define LOG_PAGE80 app_trace_log
#else
#define LOG_PAGE80(...)
#endif // TRACE_COMMON_PAGE_80_ENABLE

#ifdef TRACE_COMMON_PAGE_81_ENABLE
#include "app_trace.h"
#define LOG_PAGE81 app_trace_log
#else
#define LOG_PAGE81(...)
#endif // TRACE_COMMON_PAGE_81_ENABLE 


#endif // ANT_COMMON_PAGE_LOGGER_H__
