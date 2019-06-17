/***************************************************************************//**
 * @file rtcdriver.h
 * @brief RTCDRV timer API definition.
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef __SILICON_LABS_RTCDRV_H__
#define __SILICON_LABS_RTCDRV_H__

#include <stdint.h>
#include <stdbool.h>

#include "ecode.h"
#include "rtcdrv_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup RTCDRV
 * @brief RTCDRV timer module, see @ref rtcdrv_doc page for detailed
 *        documentation.
 * @{
 ******************************************************************************/

#define ECODE_EMDRV_RTCDRV_OK                   ( ECODE_OK )                             ///< Success return value.
#define ECODE_EMDRV_RTCDRV_ALL_TIMERS_USED      ( ECODE_EMDRV_RTCDRV_BASE | 0x00000001 ) ///< No timers available.
#define ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID     ( ECODE_EMDRV_RTCDRV_BASE | 0x00000002 ) ///< Illegal timer id.
#define ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED  ( ECODE_EMDRV_RTCDRV_BASE | 0x00000003 ) ///< Timer is not allocated.
#define ECODE_EMDRV_RTCDRV_PARAM_ERROR          ( ECODE_EMDRV_RTCDRV_BASE | 0x00000004 ) ///< Illegal input parameter.
#define ECODE_EMDRV_RTCDRV_TIMER_NOT_RUNNING    ( ECODE_EMDRV_RTCDRV_BASE | 0x00000005 ) ///< Timer is not running.

/// @brief Timer ID.
typedef uint32_t RTCDRV_TimerID_t;

/***************************************************************************//**
 * @brief
 *  Typedef for the user supplied callback function which is called when
 *  a timer elapse.
 *
 * @note This callback is called from within an interrupt handler with
 *       interrupts disabled.
 *
 * @param[in] id
 *   The timer id.
 *
 * @param[in] user
 *   Extra parameter for user application.
 ******************************************************************************/
typedef void (*RTCDRV_Callback_t)( RTCDRV_TimerID_t id, void *user );

/// @brief Timer type enumerator.
typedef enum {
  rtcdrvTimerTypeOneshot=0,    ///< Oneshot timer.
  rtcdrvTimerTypePeriodic=1    ///< Periodic timer.
} RTCDRV_TimerType_t;

Ecode_t   RTCDRV_AllocateTimer( RTCDRV_TimerID_t *id );
Ecode_t   RTCDRV_DeInit( void );
Ecode_t   RTCDRV_Delay( uint32_t ms );
Ecode_t   RTCDRV_FreeTimer( RTCDRV_TimerID_t id );
Ecode_t   RTCDRV_Init( void );
Ecode_t   RTCDRV_IsRunning( RTCDRV_TimerID_t id, bool *isRunning );
Ecode_t   RTCDRV_StartTimer( RTCDRV_TimerID_t id,
                             RTCDRV_TimerType_t type,
                             uint32_t timeout,
                             RTCDRV_Callback_t callback,
                             void *user );
Ecode_t   RTCDRV_StopTimer( RTCDRV_TimerID_t id );
Ecode_t   RTCDRV_TimeRemaining( RTCDRV_TimerID_t id, uint32_t *timeRemaining );

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
uint32_t  RTCDRV_GetWallClock( void );
uint32_t  RTCDRV_GetWallClockTicks32( void );
uint64_t  RTCDRV_GetWallClockTicks64( void );
uint64_t  RTCDRV_MsecsToTicks( uint32_t ms );
uint64_t  RTCDRV_SecsToTicks( uint32_t secs );
Ecode_t   RTCDRV_SetWallClock( uint32_t secs );
uint32_t  RTCDRV_TicksToMsec( uint64_t ticks );
uint32_t  RTCDRV_TicksToSec( uint64_t ticks );
#endif

/** @} (end addtogroup RTCDRV) */
/** @} (end addtogroup EM_Drivers) */

#ifdef __cplusplus
}
#endif

#endif /* __SILICON_LABS_RTCDRV_H__ */
