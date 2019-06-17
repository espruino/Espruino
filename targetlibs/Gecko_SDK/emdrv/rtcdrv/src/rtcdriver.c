/***************************************************************************//**
 * @file rtcdriver.c
 * @brief RTCDRV timer API implementation.
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

#include <string.h>

#include "em_device.h"
#include "em_cmu.h"
#include "em_common.h"
#include "em_int.h"

#if defined( RTCC_PRESENT ) && ( RTCC_COUNT == 1 )
#define RTCDRV_USE_RTCC
#else
#define RTCDRV_USE_RTC
#endif

#if defined( RTCDRV_USE_RTCC )
#include "em_rtcc.h"
#else
#include "em_rtc.h"
#endif

#include "rtcdriver.h"
#if defined( EMDRV_RTCDRV_SLEEPDRV_INTEGRATION )
#include "sleep.h"
#endif

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

#if     defined( EMDRV_RTCDRV_SLEEPDRV_INTEGRATION ) \
    && !defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG     ) \
    &&  defined( RTCDRV_USE_RTC )
// Do not allow EM3/EM4 energy modes when the RTC is running.
#define EMODE_DYNAMIC
#endif

#if    defined( EMDRV_RTCDRV_SLEEPDRV_INTEGRATION ) \
    && defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG     ) \
    &&  defined( RTCDRV_USE_RTC )
// Always deny EM3/EM4 energy modes when wallclock is enabled.
#define EMODE_NEVER_ALLOW_EM3EM4
#endif

//
// Various #define's to enable use of both RTC and RTCC.
//
#if defined( RTCDRV_USE_RTCC )
#define TIMEDIFF( a, b )              ((a) - (b))
#define RTC_COUNTERGET()              RTCC_CounterGet()
#define RTC_COUNTER_BITS              32
#define RTC_ALL_INTS                  _RTCC_IF_MASK
#define RTC_OF_INT                    RTCC_IF_OF
#define RTC_COMP_INT                  RTCC_IF_CC1
#define RTC_COUNTER_MASK              (_RTCC_CNT_MASK)
#define RTC_MAX_VALUE                 (_RTCC_CNT_MASK)
#define RTC_INTDISABLE( x )           RTCC_IntDisable( x )
#define RTC_INTENABLE( x )            RTCC_IntEnable(  x )
#define RTC_INTCLEAR( x )             RTCC_IntClear(   x )
#define RTC_INTGET()                  RTCC_IntGet()
#define RTC_COUNTERRESET()            RTCC->CNT = _RTCC_CNT_RESETVALUE
#define RTC_COMPARESET( x )           RTCC_ChannelCCVSet( 1, x )
#define RTC_COMPAREGET()              RTCC_ChannelCCVGet( 1 )
#define NVIC_CLEARPENDINGIRQ()        NVIC_ClearPendingIRQ( RTCC_IRQn )
#define NVIC_DISABLEIRQ()             NVIC_DisableIRQ( RTCC_IRQn )
#define NVIC_ENABLEIRQ()              NVIC_EnableIRQ( RTCC_IRQn )

#else
// To get the math correct we must have the MSB of the underlying 24bit
// counter in the MSB position of a uint32_t datatype.
#define TIMEDIFF( a, b )              ((( (a)<<8) - ((b)<<8) ) >> 8 )
#define RTC_COUNTERGET()              RTC_CounterGet()
#define RTC_COUNTER_BITS              24
#define RTC_ALL_INTS                  _RTC_IF_MASK
#define RTC_OF_INT                    RTC_IF_OF
#define RTC_COMP_INT                  RTC_IF_COMP0
#define RTC_COUNTER_MASK              (_RTC_CNT_MASK)
#define RTC_MAX_VALUE                 (_RTC_CNT_MASK)
#define RTC_INTDISABLE( x )           RTC_IntDisable( x )
#define RTC_INTENABLE( x )            RTC_IntEnable(  x )
#define RTC_INTCLEAR( x )             RTC_IntClear(   x )
#define RTC_INTGET()                  RTC_IntGet()
#define RTC_COUNTERRESET()            RTC_CounterReset()
#define RTC_COMPARESET( x )           RTC_CompareSet( 0, (x) & _RTC_COMP0_MASK )
#define RTC_COMPAREGET()              RTC_CompareGet( 0 )
#define NVIC_CLEARPENDINGIRQ()        NVIC_ClearPendingIRQ( RTC_IRQn )
#define NVIC_DISABLEIRQ()             NVIC_DisableIRQ( RTC_IRQn )
#define NVIC_ENABLEIRQ()              NVIC_EnableIRQ( RTC_IRQn )
#endif

// Maximum number of ticks per overflow period (not the maximum tick value)
#define MAX_RTC_TICK_CNT              (RTC_MAX_VALUE+1UL)
#define RTC_CLOSE_TO_MAX_VALUE        (RTC_MAX_VALUE-100UL)

#if defined(_EFM32_GECKO_FAMILY)
// Assume 32kHz RTC/RTCC clock, cmuClkDiv_2 prescaler, 16 ticks per millisecond
#define RTC_DIVIDER                     ( cmuClkDiv_2 )
#else
// Assume 32kHz RTC/RTCC clock, cmuClkDiv_8 prescaler, 4 ticks per millisecond
#define RTC_DIVIDER                     ( cmuClkDiv_8 )
#endif

#define RTC_CLOCK                       ( 32768U )
#define MSEC_TO_TICKS_DIVIDER           ( 1000U * RTC_DIVIDER )
#define MSEC_TO_TICKS_ROUNDING_FACTOR   ( MSEC_TO_TICKS_DIVIDER / 2 )
#define MSEC_TO_TICKS( ms )             ( ( ( (uint64_t)(ms) * RTC_CLOCK )    \
                                            + MSEC_TO_TICKS_ROUNDING_FACTOR ) \
                                          / MSEC_TO_TICKS_DIVIDER )

#define TICKS_TO_MSEC_ROUNDING_FACTOR   ( RTC_CLOCK / 2 )
#define TICKS_TO_MSEC( ticks )          ( ( ( (uint64_t)(ticks)               \
                                              * RTC_DIVIDER * 1000U )         \
                                            + TICKS_TO_MSEC_ROUNDING_FACTOR ) \
                                          / RTC_CLOCK )

#define TICKS_TO_SEC_ROUNDING_FACTOR    ( RTC_CLOCK / 2 )
#define TICKS_TO_SEC( ticks )           ( ( ( (uint64_t)(ticks)               \
                                              * RTC_DIVIDER )                 \
                                            + TICKS_TO_SEC_ROUNDING_FACTOR )  \
                                          / RTC_CLOCK )
#define TICK_TIME_USEC                  ( 1000000 * RTC_DIVIDER / RTC_CLOCK )

typedef struct Timer
{
  uint64_t            remaining;
  uint64_t            ticks;
  int                 periodicCompensationUsec;
  unsigned int        periodicDriftUsec;
  RTCDRV_Callback_t   callback;
  bool                running;
  bool                doCallback;
  bool                allocated;
  RTCDRV_TimerType_t  timerType;
  void                *user;
} Timer_t;

static Timer_t            timer[ EMDRV_RTCDRV_NUM_TIMERS ];
static uint32_t           lastStart;
static volatile uint32_t  startTimerNestingLevel;
static bool               inTimerIRQ;
static bool               rtcRunning;
static bool               rtcdrvIsInitialized = false;
#if defined( EMODE_DYNAMIC )
static bool               sleepBlocked;
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
static volatile uint32_t  wallClockOverflowCnt;
static uint32_t           wallClockTimeBase;
#endif

#if defined( RTCDRV_USE_RTC )
static const RTC_Init_TypeDef initRTC =
{
  true,  // Start counting when init completed.
  false, // Disable updating RTC during debug halt.
  false  // Count until max. to wrap around.
};

#elif defined( RTCDRV_USE_RTCC )
static RTCC_Init_TypeDef initRTCC =
{
  true,                 /* Start counting when init completed. */
  false,                /* Disable updating RTC during debug halt. */
  false,                /* Prescaler counts until max. before wrap around. */
  false,                /* Counter counts until max. before wrap around. */
  rtccCntPresc_8,       /* Set RTCC prescaler to 8 */
  rtccCntTickPresc,     /* Count according to prescaler configuration */
#if defined(_RTCC_CTRL_BUMODETSEN_MASK)
  false,                /* Disable storing RTCC counter value in RTCC_CCV2 upon backup mode entry. */
#endif
  false,                /* LFXO fail detection disabled */
  rtccCntModeNormal,    /* Use RTCC in normal mode and not in calender mode */
  false                 /* No leap year correction. */
};

static RTCC_CCChConf_TypeDef initRTCCCompareChannel =
{
  rtccCapComChModeCompare,    /* Use Compare mode */
  rtccCompMatchOutActionPulse,/* Don't care */
  rtccPRSCh0,                 /* PRS not used */
  rtccInEdgeNone,             /* Capture Input not used */
  rtccCompBaseCnt,            /* Compare with Base CNT register */
  0,                          /* Compare mask */
  rtccDayCompareModeMonth     /* Don't care */
};
#endif

// default to LFXO unless specifically directed to use LFRCO
#if defined(RTCDRV_USE_LFRCO)
  #define RTCDRV_OSC cmuSelect_LFRCO
#else
  #define RTCDRV_OSC cmuSelect_LFXO
#endif

static void checkAllTimers( uint32_t timeElapsed );
static void delayTicks( uint32_t ticks );
static void executeTimerCallbacks( void );
static void rescheduleRtc( uint32_t rtcCnt );

/// @endcond

/***************************************************************************//**
 * @brief
 *    Allocate timer.
 *
 * @details
 *    Reserve a timer instance.
 *
 * @param[out] id The id of the reserved timer.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ALL_TIMERS_USED when no timers are available.@n
 *    @ref ECODE_EMDRV_RTCDRV_PARAM_ERROR if an invalid id pointer was supplied.
 ******************************************************************************/
Ecode_t RTCDRV_AllocateTimer( RTCDRV_TimerID_t *id )
{
  int i      = 0;
  int retVal = 0;

  INT_Disable();
  // Iterate through the table of the timers until the first available.
  while ( ( i < EMDRV_RTCDRV_NUM_TIMERS ) && ( timer[ i ].allocated ) ) {
    i++;
  }

  // Check if we reached the end of the table.
  if ( i == EMDRV_RTCDRV_NUM_TIMERS ) {
    retVal = ECODE_EMDRV_RTCDRV_ALL_TIMERS_USED;
  } else {
    // Check if a NULL pointer was passed.
    if ( id != NULL ) {
      timer[ i ].allocated = true;
      *id = i;
      retVal = ECODE_EMDRV_RTCDRV_OK;
    } else {
      retVal = ECODE_EMDRV_RTCDRV_PARAM_ERROR;
    }
  }
  INT_Enable();

  return retVal;
}

/***************************************************************************//**
 * @brief
 *    Millisecond delay function.
 *
 * @param[in] ms Milliseconds to stay in the delay function.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK.
 ******************************************************************************/
Ecode_t RTCDRV_Delay( uint32_t ms )
{
  uint64_t totalTicks;

  totalTicks = MSEC_TO_TICKS( ms );

  while ( totalTicks > RTC_CLOSE_TO_MAX_VALUE ) {
    delayTicks( RTC_CLOSE_TO_MAX_VALUE );
    totalTicks -= RTC_CLOSE_TO_MAX_VALUE;
  }
  delayTicks( totalTicks );

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Free timer.
 *
 * @details
 *    Release a reserved timer.
 *
 * @param[out] id The id of the timer to release.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID if id has an illegal value.
 ******************************************************************************/
Ecode_t RTCDRV_FreeTimer( RTCDRV_TimerID_t id )
{
  // Check if valid timer ID.
  if ( id >= EMDRV_RTCDRV_NUM_TIMERS ) {
    return ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID;
  }

  INT_Disable();

  timer[ id ].running   = false;
  timer[ id ].allocated = false;

  INT_Enable();

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Initialize RTCDRV driver.
 *
 * @details
 *    Will enable all necessary clocks. Initializes internal datastructures
 *    and configures the underlying hardware timer.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK.
 ******************************************************************************/
Ecode_t RTCDRV_Init( void )
{
  if ( rtcdrvIsInitialized == true ) {
    return ECODE_EMDRV_RTCDRV_OK;
  }
  rtcdrvIsInitialized = true;

  // Ensure LE modules are clocked.
  CMU_ClockEnable( cmuClock_CORELE, true );

#if defined( CMU_LFECLKEN0_RTCC )
  // Enable LFECLK in CMU (will also enable oscillator if not enabled).
  CMU_ClockSelectSet( cmuClock_LFE, RTCDRV_OSC );
#else
  // Enable LFACLK in CMU (will also enable oscillator if not enabled).
  CMU_ClockSelectSet( cmuClock_LFA, RTCDRV_OSC );
#endif

#if defined( RTCDRV_USE_RTC )
  // Set clock divider.
  CMU_ClockDivSet( cmuClock_RTC, RTC_DIVIDER );

  // Enable RTC module clock.
  CMU_ClockEnable( cmuClock_RTC, true );

  // Initialize RTC.
  RTC_Init( &initRTC );

#elif defined( RTCDRV_USE_RTCC )
  // Set clock divider.
  initRTCC.presc = (RTCC_CntPresc_TypeDef)CMU_DivToLog2( RTC_DIVIDER );

  // Enable RTCC module clock.
  CMU_ClockEnable( cmuClock_RTCC, true );

  // Initialize RTCC.
  RTCC_Init( &initRTCC );

  // Set up Compare channel.
  RTCC_ChannelInit( 1, &initRTCCCompareChannel );
#endif

  // Disable RTC/RTCC interrupt generation.
  RTC_INTDISABLE( RTC_ALL_INTS );
  RTC_INTCLEAR( RTC_ALL_INTS );

  RTC_COUNTERRESET();

  // Clear and then enable RTC interrupts in NVIC.
  NVIC_CLEARPENDINGIRQ();
  NVIC_ENABLEIRQ();

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
  // Enable overflow interrupt for wallclock.
  RTC_INTENABLE( RTC_OF_INT );
#endif

  // Reset RTCDRV internal data structures/variables.
  memset( timer, 0, sizeof( timer ) );
  inTimerIRQ             = false;
  rtcRunning             = false;
  startTimerNestingLevel = 0;
#if defined( EMODE_DYNAMIC )
  sleepBlocked           = false;
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
  wallClockOverflowCnt = 0;
  wallClockTimeBase    = 0;

#if defined( EMODE_NEVER_ALLOW_EM3EM4 )
  // Always block EM3 and EM4 if wallclock is running.
  SLEEP_SleepBlockBegin( sleepEM3 );
#endif

#endif

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Deinitialize RTCDRV driver.
 *
 * @details
 *    Will disable interrupts and turn off the clock to the underlying hardware
 *    timer.
 *    If integration with SLEEP module is enabled it will remove any
 *    restriction that are set on energy mode usage.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK.
 ******************************************************************************/
Ecode_t RTCDRV_DeInit( void )
{
  // Disable and clear all interrupt sources.
  NVIC_DISABLEIRQ();
  RTC_INTDISABLE( RTC_ALL_INTS );
  RTC_INTCLEAR( RTC_ALL_INTS );
  NVIC_CLEARPENDINGIRQ();

  // Disable RTC module and its clock.
#if defined( RTCDRV_USE_RTC )
  RTC_Enable( false );
  CMU_ClockEnable( cmuClock_RTC, false );
#elif defined( RTCDRV_USE_RTCC )
  RTCC_Enable( false );
  CMU_ClockEnable( cmuClock_RTCC, false );
#endif

#if defined( EMODE_NEVER_ALLOW_EM3EM4 )
  // End EM3 and EM4 blocking.
  SLEEP_SleepBlockEnd( sleepEM3 );
#endif

#if defined( EMODE_DYNAMIC )
  // End EM3 and EM4 blocking if a block start has been set.
  if ( sleepBlocked ) {
    SLEEP_SleepBlockEnd( sleepEM3 );
  }
#endif

  // Mark the driver as uninitialized.
  rtcdrvIsInitialized = false;

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Check if a given timer is running.
 *
 * @param[in] id The id of the timer to query.
 *
 * @param[out] isRunning True if timer is running. False if not running.
 *                       Only valid if return status is ECODE_EMDRV_RTCDRV_OK.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID if id has an illegal value. @n
 *    @ref ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED if the timer is not reserved.@n
 *    @ref ECODE_EMDRV_RTCDRV_PARAM_ERROR if an invalid isRunning pointer was
 *         supplied.
 ******************************************************************************/
Ecode_t RTCDRV_IsRunning( RTCDRV_TimerID_t id, bool *isRunning )
{
  // Check if valid timer ID.
  if ( id >= EMDRV_RTCDRV_NUM_TIMERS ) {
    return ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID;
  }

  // Check pointer validity.
  if ( isRunning == NULL ) {
    return ECODE_EMDRV_RTCDRV_PARAM_ERROR;
  }

  INT_Disable();
  // Check if timer is reserved.
  if ( ! timer[ id ].allocated ) {
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED;
  }
  *isRunning = timer[ id ].running;
  INT_Enable();

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Start a timer.
 *
 * @note
 *    It is legal to start an already running timer.
 *
 * @param[in] id The id of the timer to start.
 * @param[in] type Timer type, oneshot or periodic. See @ref RTCDRV_TimerType_t.
 * @param[in] timeout Timeout expressed in milliseconds. If the timeout value
 *            is 0, the callback function will be called immediately and
 *            the timer will not be started.
 * @param[in] callback Function to call on timer expiry. See @ref
 *            RTCDRV_Callback_t. NULL is a legal value.
 * @param[in] user Extra callback function parameter for user application.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID if id has an illegal value.@n
 *    @ref ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED if the timer is not reserved.
 ******************************************************************************/
Ecode_t RTCDRV_StartTimer(  RTCDRV_TimerID_t id,
                            RTCDRV_TimerType_t type,
                            uint32_t timeout,
                            RTCDRV_Callback_t callback,
                            void *user )
{
  uint32_t timeElapsed, cnt, compVal, loopCnt = 0;
  uint32_t timeToNextTimerCompletion;

  // Check if valid timer ID.
  if ( id >= EMDRV_RTCDRV_NUM_TIMERS ) {
    return ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID;
  }

  INT_Disable();
  if ( ! timer[ id ].allocated ) {
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED;
  }

  if ( timeout == 0 ) {
    if ( callback != NULL ) {
      callback( id, user );
    }
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_OK;
  }

  cnt = RTC_COUNTERGET();

  timer[ id ].callback  = callback;
  timer[ id ].ticks     = MSEC_TO_TICKS( timeout );
  if (rtcdrvTimerTypePeriodic == type) {
    // Calculate compensation value for periodic timers.
    timer[ id ].periodicCompensationUsec = 1000 * timeout -
      (timer[ id ].ticks * TICK_TIME_USEC);
    timer[ id ].periodicDriftUsec = TICK_TIME_USEC/2;
  }
  // Add one tick in order to compensate if RTC is close to an increment event.
  timer[ id ].remaining = timer[ id ].ticks + 1;
  timer[ id ].running   = true;
  timer[ id ].timerType = type;
  timer[ id ].user      = user;

  if ( inTimerIRQ == true ) {
    // Exit now, remaining processing will be done in IRQ handler.
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_OK;
  }

  // StartTimer() may recurse, keep track of recursion level.
  if ( startTimerNestingLevel < UINT32_MAX ) {
    startTimerNestingLevel++;
  }

  if ( rtcRunning == false ) {

#if defined( RTCDRV_USE_RTC )
    lastStart = ( cnt ) & RTC_COUNTER_MASK;
#elif defined( RTCDRV_USE_RTCC )
    lastStart = cnt;
#endif

    RTC_INTCLEAR( RTC_COMP_INT );

    compVal = EFM32_MIN( timer[ id ].remaining, RTC_CLOSE_TO_MAX_VALUE );
    RTC_COMPARESET( cnt + compVal );

    // Start the timer system by enabling the compare interrupt.
    RTC_INTENABLE( RTC_COMP_INT );

#if defined( EMODE_DYNAMIC )
    // When RTC is running, we can not allow EM3 or EM4.
    if ( sleepBlocked == false ) {
      sleepBlocked = true;
      SLEEP_SleepBlockBegin( sleepEM3 );
    }
#endif

    rtcRunning = true;

  } else {

    // The timer system is running. We must stop, update timers with the time
    // elapsed so far, find the timer with the shortest timeout and then restart.
    // As StartTimer() may be called from the callbacks we only do this
    // processing at the first nesting level.
    if ( startTimerNestingLevel == 1  ) {

      timer[ id ].running = false;
      // This loop is repeated if CNT is incremented while processing.
      do {

        RTC_INTDISABLE( RTC_COMP_INT );

        timeElapsed = TIMEDIFF( cnt, lastStart );
#if defined( RTCDRV_USE_RTC )
        // Compensate for the fact that CNT is normally COMP0+1 after a
        // compare match event.
        if ( timeElapsed == RTC_MAX_VALUE ) {
          timeElapsed = 0;
        }
#endif

        // Update all timers with elapsed time.
        checkAllTimers( timeElapsed );

        // Execute timer callbacks.
        executeTimerCallbacks();

        // Set timer to running only after checkAllTimers() is called once.
        if ( loopCnt == 0 ) {
          timer[ id ].running = true;
        }
        loopCnt++;

        // Restart RTC according to next timeout.
        rescheduleRtc( cnt );

        cnt = RTC_COUNTERGET();
        timeElapsed = TIMEDIFF( cnt, lastStart );
        timeToNextTimerCompletion = TIMEDIFF( RTC_COMPAREGET(), lastStart );

        /* If the counter has passed the COMP(ARE) register value since we
           checked the timers, then we should recheck the timers and reschedule
           again. */
      }
      while ( rtcRunning && (timeElapsed > timeToNextTimerCompletion));
    }
  }

  if ( startTimerNestingLevel > 0 ) {
    startTimerNestingLevel--;
  }

  INT_Enable();
  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Stop a given timer.
 *
 * @param[in] id The id of the timer to stop.
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID if id has an illegal value. @n
 *    @ref ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED if the timer is not reserved.
 ******************************************************************************/
Ecode_t RTCDRV_StopTimer( RTCDRV_TimerID_t id )
{
  // Check if valid timer ID.
  if ( id >= EMDRV_RTCDRV_NUM_TIMERS ) {
    return ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID;
  }

  INT_Disable();
  if ( ! timer[ id ].allocated ) {
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED;
  }

  timer[ id ].running = false;
  INT_Enable();

  return ECODE_EMDRV_RTCDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Get time left before a given timer expires.
 *
 * @param[in] id The id of the timer to query.
 *
 * @param[out] timeRemaining Time left expressed in milliseconds.
 *                        Only valid if return status is ECODE_EMDRV_RTCDRV_OK.
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK on success.@n
 *    @ref ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID if id has an illegal value. @n
 *    @ref ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED if the timer is not reserved.@n
 *    @ref ECODE_EMDRV_RTCDRV_TIMER_NOT_RUNNING if the timer is not running.@n
 *    @ref ECODE_EMDRV_RTCDRV_PARAM_ERROR if an invalid timeRemaining pointer
 *         was supplied.
 ******************************************************************************/
Ecode_t RTCDRV_TimeRemaining( RTCDRV_TimerID_t id, uint32_t *timeRemaining )
{
  uint64_t tmp;
  uint32_t timeLeft, currentCnt, lastRtcStart;

  // Check if valid timer ID.
  if ( id >= EMDRV_RTCDRV_NUM_TIMERS ) {
    return ECODE_EMDRV_RTCDRV_ILLEGAL_TIMER_ID;
  }

  // Check pointer validity.
  if ( timeRemaining == NULL ) {
    return ECODE_EMDRV_RTCDRV_PARAM_ERROR;
  }

  INT_Disable();
  // Check if timer is reserved.
  if ( ! timer[ id ].allocated ) {
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_TIMER_NOT_ALLOCATED;
  }

  // Check if timer is running.
  if ( ! timer[ id ].running ) {
    INT_Enable();
    return ECODE_EMDRV_RTCDRV_TIMER_NOT_RUNNING;
  }

  timeLeft     = timer[ id ].remaining;
  currentCnt   = RTC_COUNTERGET();
  lastRtcStart = lastStart;
  INT_Enable();

  // Get number of RTC clock ticks elapsed since last RTC reschedule.
  currentCnt = TIMEDIFF( currentCnt, lastRtcStart );

  if ( currentCnt > timeLeft ) {
    timeLeft = 0;
  } else {
    timeLeft -= currentCnt;
  }

  tmp = TICKS_TO_MSEC( timeLeft );
  *timeRemaining = tmp;

  return ECODE_EMDRV_RTCDRV_OK;
}

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Get wallclock time.
 *
 * @return
 *    Seconds elapsed since RTCDRV was initialized.
 ******************************************************************************/
uint32_t RTCDRV_GetWallClock( void )
{
  return wallClockTimeBase
         + (uint32_t)TICKS_TO_SEC( RTCDRV_GetWallClockTicks32() );
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Get wallclock tick count as a 32bit value. At 4 ticks per millisecond,
 *    overflow occurs after approximately 12.5 days
 *
 * @return
 *    Wallclock tick counter.
 ******************************************************************************/
uint32_t RTCDRV_GetWallClockTicks32( void )
{
  uint32_t overflows, ticks;

  /* Need to re-read data in case overflow cnt is incremented while we read. */
  do
  {
    overflows = wallClockOverflowCnt;
    ticks     = RTC_COUNTERGET();
  } while ( overflows != wallClockOverflowCnt );

#if ( RTC_COUNTER_BITS < 32 )
  return ( overflows << RTC_COUNTER_BITS ) + ticks;
#else
  return ticks;
#endif
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Get wallclock tick count as a 64 bit value. This will never overflow.
 *
 * @return
 *    Wallclock tick counter.
 ******************************************************************************/
uint64_t RTCDRV_GetWallClockTicks64( void )
{
  uint64_t overflows, ticks;

  /* Need to re-read data in case overflow cnt is incremented while we read. */
  do
  {
    overflows = wallClockOverflowCnt;
    ticks     = RTC_COUNTERGET();
  } while ( overflows != wallClockOverflowCnt );

  return ( overflows << RTC_COUNTER_BITS ) + ticks;
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Set wallclock time.
 *
 * @param[in] secs Value to set (seconds).
 *
 * @return
 *    @ref ECODE_EMDRV_RTCDRV_OK
 ******************************************************************************/
Ecode_t RTCDRV_SetWallClock( uint32_t secs )
{
  wallClockTimeBase = secs - TICKS_TO_SEC( RTCDRV_GetWallClockTicks32() );
  return ECODE_EMDRV_RTCDRV_OK;
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Convert from milliseconds to RTC/RTCC ticks.
 *
 * @param[in] ms Millisecond value to convert.
 *
 * @return
 *    Number of ticks.
 ******************************************************************************/
uint64_t  RTCDRV_MsecsToTicks( uint32_t ms )
{
  return MSEC_TO_TICKS( ms );
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Convert from seconds to RTC/RTCC ticks.
 *
 * @param[in] secs Second value to convert.
 *
 * @return
 *    Number of ticks.
 ******************************************************************************/
uint64_t  RTCDRV_SecsToTicks( uint32_t secs )
{
  return MSEC_TO_TICKS( 1000 * secs );
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Convert from RTC/RTCC ticks to milliseconds.
 *
 * @param[in] ticks Number of ticks to convert.
 *
 * @return
 *    Number of milliseconds.
 ******************************************************************************/
uint32_t  RTCDRV_TicksToMsec( uint64_t ticks )
{
  return TICKS_TO_MSEC( ticks );
}
#endif

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
/***************************************************************************//**
 * @brief
 *    Convert from RTC/RTCC ticks to seconds.
 *
 * @param[in] ticks Number of ticks to convert.
 *
 * @return
 *    Number of seconds.
 ******************************************************************************/
uint32_t  RTCDRV_TicksToSec( uint64_t ticks )
{
  return TICKS_TO_MSEC( ticks ) / 1000;
}
#endif

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

#if defined( RTCDRV_USE_RTC )
void RTC_IRQHandler(void)
#elif defined( RTCDRV_USE_RTCC )
void RTCC_IRQHandler(void)
#endif
{
  uint32_t flags, timeElapsed, cnt, timeToNextTimerCompletion;

  INT_Disable();

  // CNT will normally be COMP0+1 at this point,
  // unless IRQ latency exceeded one tick period.

  flags = RTC_INTGET();

  if ( flags & RTC_COMP_INT ) {

    // Stop timer system by disabling the compare IRQ.
    // Update all timers with the time elapsed, call callbacks if needed,
    // then find the timer with the shortest timeout (if any at all) and
    // reenable the compare IRQ if needed.

    inTimerIRQ = true;

    cnt = RTC_COUNTERGET();

    // This loop is repeated if CNT is incremented while processing.
    do {

      RTC_INTDISABLE( RTC_COMP_INT );

      timeElapsed = TIMEDIFF( cnt, lastStart );

      // Update all timers with elapsed time.
      checkAllTimers( timeElapsed );

      // Execute timer callbacks.
      executeTimerCallbacks();

      // Restart RTC according to next timeout.
      rescheduleRtc( cnt );

      cnt = RTC_COUNTERGET();
      timeElapsed = TIMEDIFF( cnt, lastStart );
      timeToNextTimerCompletion = TIMEDIFF( RTC_COMPAREGET(), lastStart );
      /* If the counter has passed the COMP(ARE) register value since we
         checked the timers, then we should recheck the timers and reschedule
         again. */
    }
    while ( rtcRunning && (timeElapsed > timeToNextTimerCompletion));
    inTimerIRQ = false;
  }

#if defined( EMDRV_RTCDRV_WALLCLOCK_CONFIG )
  if ( flags & RTC_OF_INT )
  {
    RTC_INTCLEAR( RTC_OF_INT );
    wallClockOverflowCnt++;
  }
#endif

  INT_Enable();
}

static void checkAllTimers( uint32_t timeElapsed )
{
  int i;
#if defined( EMODE_DYNAMIC )
  int numOfTimersRunning = 0;
#endif

  // Iterate through the timer table.
  // Update time remaining, check for timeout and rescheduling of periodic
  // timers, check for callbacks.

  for ( i = 0; i < EMDRV_RTCDRV_NUM_TIMERS; i++ ) {
    timer[ i ].doCallback = false;
    if ( timer[ i ].running == true ) {
#if defined( EMODE_DYNAMIC )
      numOfTimersRunning++;
#endif
      if ( timer[ i ].remaining > timeElapsed ) {
        timer[ i ].remaining -= timeElapsed;
      } else {
        if ( timer[ i ].timerType == rtcdrvTimerTypeOneshot ) {
          timer[ i ].running = false;
#if defined( EMODE_DYNAMIC )
          numOfTimersRunning--;
#endif
        } else {
          // Compensate overdue periodic timers to avoid accumlating errors.
          timer[ i ].remaining = timer[ i ].ticks - timeElapsed +
                                 timer[ i ].remaining;
          if ( timer[ i ].periodicCompensationUsec > 0 ) {
            timer[ i ].periodicDriftUsec += timer[i].periodicCompensationUsec;
            if (timer[ i ].periodicDriftUsec >= TICK_TIME_USEC) {
              // Add a tick if the timer drift is longer than the time of
              // one tick.
              timer[ i ].remaining += 1;
              timer[ i ].periodicDriftUsec -= TICK_TIME_USEC;
            }
          }
          else {
            timer[ i ].periodicDriftUsec -= timer[i].periodicCompensationUsec;
            if (timer[ i ].periodicDriftUsec >= TICK_TIME_USEC) {
              // Subtract one tick if the timer drift is longer than the time
              // of one tick.
              timer[ i ].remaining -= 1;
              timer[ i ].periodicDriftUsec -= TICK_TIME_USEC;
            }
          }
        }
        if ( timer[ i ].callback != NULL ) {
          timer[ i ].doCallback = true;
        }
      }
    }
  }

#if defined( EMODE_DYNAMIC )
  // If no timers are running, we can remove block on EM3 and EM4 sleep modes.
  if ( ( numOfTimersRunning == 0 ) && ( sleepBlocked == true ) ) {
    sleepBlocked = false;
    SLEEP_SleepBlockEnd( sleepEM3 );
  }
#endif
}

static void delayTicks( uint32_t ticks )
{
  uint32_t startTime;
  volatile uint32_t now;

  if ( ticks ) {
    startTime = RTC_COUNTERGET();
    do {
      now = RTC_COUNTERGET();
    } while ( TIMEDIFF( now, startTime ) < ticks );
  }
}

static void executeTimerCallbacks( void )
{
  int i;

  for ( i = 0; i < EMDRV_RTCDRV_NUM_TIMERS; i++ ) {
    if ( timer[ i ].doCallback ) {
      timer[ i ].callback( i, timer[ i ].user );
    }
  }
}

static void rescheduleRtc( uint32_t rtcCnt )
{
  int i;
  uint64_t min = UINT64_MAX;

  // Find the timer with shortest timeout.
  for ( i = 0; i < EMDRV_RTCDRV_NUM_TIMERS; i++ ) {
    if (    ( timer[ i ].running   == true )
         && ( timer[ i ].remaining <  min  ) ) {
      min = timer[ i ].remaining;
    }
  }

  rtcRunning = false;
  if ( min != UINT64_MAX ) {
    min = EFM32_MIN( min, RTC_CLOSE_TO_MAX_VALUE );
#if defined( RTCDRV_USE_RTC )
    if ( inTimerIRQ == false ) {
      lastStart = ( rtcCnt ) & RTC_COUNTER_MASK;
    } else
#endif
    {
      lastStart = rtcCnt;
    }
    RTC_INTCLEAR( RTC_COMP_INT );

    RTC_COMPARESET( rtcCnt + min );

#if defined( EMODE_DYNAMIC )
    // When RTC is running, we can not allow EM3 or EM4.
    if ( sleepBlocked == false ) {
      sleepBlocked = true;
      SLEEP_SleepBlockBegin( sleepEM3 );
    }
#endif

    rtcRunning = true;

    // Reenable compare IRQ.
    RTC_INTENABLE( RTC_COMP_INT );
  }
}
/// @endcond

/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************//**
 * @addtogroup RTCDRV
 * @{

@page rtcdrv_doc RTCDRV Real Time Clock Timer driver

  The source files for the RTCDRV device driver library resides in the
  emdrv/rtcdrv folder, and are named rtcdriver.c and rtcdriver.h.

  @li @ref rtcdrv_intro
  @li @ref rtcdrv_conf
  @li @ref rtcdrv_api
  @li @ref rtcdrv_example

@n @section rtcdrv_intro Introduction

  The RTCDRV driver use the RTC peripheral of a device in Silicon Laboratories
  Gecko microcontroller family to provide a user configurable number of software
  millisecond timers.
  Two kinds of timers are supported; oneshot timers and periodic timers.
  Timers will, when their timeout period has expired, call a user supplied
  callback function.
  @note The callback function is called from within an interrupt handler with
  interrupts disabled.

  In addition to the timers, RTCDRV also offers an optional wallclock
  functionality. The wallclock keeps track of the number of seconds elapsed
  since RTCDRV was initialized.

@n @section rtcdrv_conf Configuration Options

  Some properties of the RTCDRV driver are compile-time configurable. These
  properties are stored in a file named @ref rtcdrv_config.h. A template for this
  file, containing default values, resides in the emdrv/config folder.
  Currently the configuration options are:
  @li The number of timers that RTCDRV provides.
  @li Inclusion of the wallclock functionality.
  @li Integration with SLEEP driver.

  When integration with the SLEEP driver is enabled, RTCDRV will keep the
  SLEEP driver updated with regards to which energy mode that can be
  safely used.

  To configure RTCDRV, provide your own configuration file. Here is a
  sample @ref rtcdrv_config.h file:
  @verbatim
#ifndef __SILICON_LABS_RTCDRV_CONFIG_H__
#define __SILICON_LABS_RTCDRV_CONFIG_H__

// Define how many timers RTCDRV provide.
#define EMDRV_RTCDRV_NUM_TIMERS     (4)

// Uncomment the following line to include wallclock functionality.
//#define EMDRV_RTCDRV_WALLCLOCK_CONFIG

// Uncomment the following line to enable integration with SLEEP driver.
//#define EMDRV_RTCDRV_SLEEPDRV_INTEGRATION

#endif
  @endverbatim

@n @section rtcdrv_api The API

  This section contain brief descriptions of the functions in the API. You will
  find detailed information on input and output parameters and return values by
  clicking on the hyperlinked function names. Most functions return an error
  code, @ref ECODE_EMDRV_RTCDRV_OK is returned on success,
  see @ref ecode.h and @ref rtcdriver.h for other error codes.

  Your application code must include one header file: @em rtcdriver.h.

  All functions defined in the API can be called from within interrupt handlers.

  @ref RTCDRV_Init(), @ref RTCDRV_DeInit() @n
    These functions initializes or deinitializes the RTCDRV driver. Typically
    @htmlonly RTCDRV_Init() @endhtmlonly is called once in your startup code.

  @ref RTCDRV_StartTimer(), @ref RTCDRV_StopTimer() @n
    Start/Stop a timer. When a timer expire, a user supplied callback function
    is called. A pointer to this function is passed to @htmlonly
    RTCDRV_StartTimer()@endhtmlonly. Refer to @ref TimerCallback for details of
    the callback prototype.
    Note that it is legal to start an already started timer, it will then just
    be restarted with the new timeout value.

  @ref RTCDRV_AllocateTimer(), @ref RTCDRV_FreeTimer() @n
    Reserve/release a timer. Many functions in the API require a timer ID as
    input parameter. Use @htmlonly RTCDRV_AllocateTimer() @endhtmlonly to
    aquire such a reference.

  @ref RTCDRV_TimeRemaining() @n
    Get time left to timer expiry.

  @ref RTCDRV_Delay() @n
    Millisecond delay function. This is an "active wait" delay function.

  @ref RTCDRV_IsRunning() @n
    Check if a timer is running.

  @ref RTCDRV_GetWallClock(), @ref RTCDRV_SetWallClock() @n
    Get or set wallclock time.

  @ref RTCDRV_GetWallClockTicks32(), @ref RTCDRV_GetWallClockTicks64() @n
    Get wallclock time expressed as number of RTC/RTCC counter ticks, available
    both as 32bit and 64 bit values.

  @ref RTCDRV_MsecsToTicks(), @ref RTCDRV_SecsToTicks(),
  @ref RTCDRV_TicksToMsec(), @ref RTCDRV_TicksToSec() @n
    Conversion functions between seconds, milliseconds and RTC/RTCC
    counter ticks.

  @n @anchor TimerCallback <b>The timer expiry callback function:</b> @n
  The callback function, prototyped as @ref RTCDRV_Callback_t(), is called from
  within the RTC peripheral interrupt handler on timer expiry.
  @htmlonly RTCDRV_Callback_t( RTCDRV_TimerID_t id )@endhtmlonly is called with
  the timer id as input parameter.

  @n <b>The timer type:</b> @n
  Timers are either of oneshot type or of periodic type.
  @li Oneshot timer run only once toward their expiry.
  @li Periodic timers will be automatically restartet when they expire.

  The timer type is an enumeration, see @ref RTCDRV_TimerType_t for details.

@n @section rtcdrv_example Example
  @verbatim
#include "rtcdriver.h"

int i = 0;
RTCDRV_TimerID_t id;

void myCallback( RTCDRV_TimerID_t id )
{
  // Timer has elapsed !

  i++;

  if ( i < 10 ) {
    // Restart timer
    RTCDRV_StartTimer( id, rtcdrvTimerTypeOneshot, 100, myCallback );
  }
}

int main( void )
{
  // Initialization of RTCDRV driver
  RTCDRV_Init();

  // Reserve a timer
  RTCDRV_AllocateTimer( &id );

  // Start a oneshot timer with 100 millisecond timeout
  RTCDRV_StartTimer( id, rtcdrvTimerTypeOneshot, 100, myCallback );
}
  @endverbatim

 * @}**************************************************************************/
