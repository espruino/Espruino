/**************************************************************************//**
 * @file  ustimer.c
 * @brief Microsecond delay functions.
 * @version 4.1.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdbool.h>
#include "em_device.h"
#include "em_common.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_int.h"
#include "em_timer.h"

#include "ustimer.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define USTIMER_TIMER0 0
#define USTIMER_TIMER1 1
#define USTIMER_TIMER2 2
#define USTIMER_TIMER3 3

#ifndef USTIMER_TIMER
#define USTIMER_TIMER USTIMER_TIMER0
#endif

#if ( USTIMER_TIMER == USTIMER_TIMER0 ) && ( TIMER_COUNT >= 1 )
  #define TIMER             TIMER0
  #define TIMER_CLK         cmuClock_TIMER0
  #define TIMER_IRQ         TIMER0_IRQn
  #define TIMER_IRQHandler  TIMER0_IRQHandler

#elif ( USTIMER_TIMER == USTIMER_TIMER1 ) && ( TIMER_COUNT >= 2 )
  #define TIMER             TIMER1
  #define TIMER_CLK         cmuClock_TIMER1
  #define TIMER_IRQ         TIMER1_IRQn
  #define TIMER_IRQHandler  TIMER1_IRQHandler

#elif ( USTIMER_TIMER == USTIMER_TIMER2 ) && ( TIMER_COUNT >= 3 )
  #define TIMER             TIMER2
  #define TIMER_CLK         cmuClock_TIMER2
  #define TIMER_IRQ         TIMER2_IRQn
  #define TIMER_IRQHandler  TIMER2_IRQHandler

#elif ( USTIMER_TIMER == USTIMER_TIMER3 ) && ( TIMER_COUNT == 4 )
  #define TIMER             TIMER3
  #define TIMER_CLK         cmuClock_TIMER3
  #define TIMER_IRQ         TIMER3_IRQn
  #define TIMER_IRQHandler  TIMER3_IRQHandler

#else
#error "Illegal USTIMER TIMER selection"
#endif

static uint32_t freq;
static uint32_t minTicks;
static volatile bool timeElapsed = false;

static void DelayTicksEM1( uint16_t ticks );
static void DelayTicksPolled( uint16_t ticks );

/** @endcond */

/***************************************************************************//**
 * @brief
 *   Activate and initialize the hardware timer used to pace the 1 microsecond
 *   delay functions.
 *
 * @note
 *   Call this function whenever the HFPERCLK and/or HFCORECLK frequency is
 *   changed.
 *
 * @return
 *    @ref ECODE_EMDRV_USTIMER_OK.
 ******************************************************************************/
Ecode_t USTIMER_Init( void )
{
  TIMER_Init_TypeDef timerInit     = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  uint32_t coreClockScale;

  timerCCInit.mode = timerCCModeCompare;
  CMU_ClockEnable( TIMER_CLK, true );
  TIMER_TopSet( TIMER, 0xFFFF );
  TIMER_InitCC( TIMER, 0, &timerCCInit );

  /* Run timer at slowest frequency that still gives less than 1 us per tick */
  timerInit.prescale = (TIMER_Prescale_TypeDef)_TIMER_CTRL_PRESC_DIV1;
  do
  {
    TIMER_Init( TIMER, &timerInit );

    freq = CMU_ClockFreqGet( cmuClock_HFPER );
    freq /= 1 << timerInit.prescale;
    timerInit.prescale++;
  }
  while ( ( timerInit.prescale <= _TIMER_CTRL_PRESC_DIV1024 )
          && ( freq > 2000000 ) );

  /* Figure out the minimum delay we can have when using timer interrupt
   * to avoid that actual delay become a full timer counter lap.
   * We are assuming that this number scales with mcu core clock.
   * The number is found by trial and err on a GG running at 14MHz.
   */
  coreClockScale = ( 4 * 48000000 ) / CMU_ClockFreqGet( cmuClock_CORE );
  minTicks = ( ( (uint64_t)freq * coreClockScale ) + 500000 ) / 1000000;
  timeElapsed = false;

  TIMER_IntDisable( TIMER, TIMER_IEN_CC0 );
  NVIC_ClearPendingIRQ( TIMER_IRQ );
  NVIC_EnableIRQ( TIMER_IRQ );

  return ECODE_EMDRV_USTIMER_OK;
}

/***************************************************************************//**
 * @brief
 *    Deinitialize USTIMER driver.
 *
 * @details
 *    Will disable interrupts and turn off the clock to the underlying hardware
 *    timer.
 *
 * @return
 *    @ref ECODE_EMDRV_USTIMER_OK.
 ******************************************************************************/
Ecode_t USTIMER_DeInit( void )
{
  NVIC_DisableIRQ( TIMER_IRQ );
  TIMER_IntDisable( TIMER, TIMER_IEN_CC0 );

  TIMER_IntClear( TIMER, TIMER_IFC_CC0 );
  NVIC_ClearPendingIRQ( TIMER_IRQ );

  TIMER_Enable( TIMER, false );
  CMU_ClockEnable( TIMER_CLK, false );

  return ECODE_EMDRV_USTIMER_OK;
}

/***************************************************************************//**
 * @brief
 *   Delay a given number of microseconds.
 *
 * @details
 *   The mcu is put in EM1 during the delay.
 *
 * @note
 *   This function assumes that the timer interrupt needed to wake the mcu
 *   up from EM1 is not blocked. This function is not thread safe.
 *
 * @param[in] usec
 *   Number of microseconds to delay.
 *
 * @return
 *    @ref ECODE_EMDRV_USTIMER_OK.
 ******************************************************************************/
Ecode_t USTIMER_Delay( uint32_t usec )
{
  uint64_t totalTicks;

  totalTicks = ( ( (uint64_t)freq * usec ) + 500000 ) / 1000000;

  /* The timer counter is 16 bits wide, split the total wait up in chunks */
  /* of a little less than 2^16.                                          */
  while ( totalTicks > 65000 )
  {
    DelayTicksEM1( 65000 );
    totalTicks -= 65000;
  }
  DelayTicksEM1( (uint16_t)totalTicks );

  return ECODE_EMDRV_USTIMER_OK;
}

/***************************************************************************//**
 * @brief
 *   Delay a given number of microseconds.
 *
 * @details
 *   This is a busy wait delay not using energy modes to conserve power.
 *
 * @note
 *   This function can be used in any context (interrupts can be disabled).
 *   This function is thread safe.
 *
 * @param[in] usec
 *   Number of microseconds to delay.
 *
 * @return
 *    @ref ECODE_EMDRV_USTIMER_OK.
 ******************************************************************************/
Ecode_t USTIMER_DelayIntSafe( uint32_t usec )
{
  uint64_t totalTicks;

  totalTicks = ( ( (uint64_t)freq * usec ) + 500000 ) / 1000000;

  /* The timer counter is 16 bits wide, split the total wait up in chunks */
  /* of a little less than 2^16.                                          */
  while ( totalTicks > 65000 )
  {
    DelayTicksPolled( 65000 );
    totalTicks -= 65000;
  }
  DelayTicksPolled( (uint16_t)totalTicks );

  return ECODE_EMDRV_USTIMER_OK;
}


/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

void TIMER_IRQHandler( void )
{
  uint32_t flags;

  flags = TIMER_IntGet( TIMER );

  if ( flags & TIMER_IF_CC0 )
  {
    TIMER_IntClear( TIMER, TIMER_IFC_CC0 );
    timeElapsed = true;
  }
}

static void DelayTicksPolled( uint16_t ticks )
{
  uint16_t startTime;
  volatile uint16_t now;

  if ( ticks )
  {
    startTime = TIMER_CounterGet( TIMER );
    do
    {
      now = TIMER_CounterGet( TIMER );
    } while ( (uint16_t)( now - startTime ) < ticks );
  }
}

static void DelayTicksEM1( uint16_t ticks )
{
  if ( ticks )
  {
    /* Arm TIMER compare interrupt */

    INT_Disable();

    /* The following lines costs 2.7us@48MHz and 7.5us@14MHz (measured with GG)*/
    TIMER_CompareSet( TIMER, 0,
                      TIMER_CounterGet( TIMER ) + EFM32_MAX( minTicks, ticks ) );
    TIMER_IntClear( TIMER, TIMER_IFC_CC0 );
    TIMER_IntEnable( TIMER, TIMER_IEN_CC0 );

    INT_Enable();

    while ( ! timeElapsed )
    {
      EMU_EnterEM1();
    }
    timeElapsed = false;

    TIMER_IntDisable( TIMER, TIMER_IEN_CC0 );
  }
}

/** @endcond */

/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************//**
 * @{

@page ustimer_doc USTIMER Microsecond delay timer module

   Implements microsecond delays.

   The delay is implemented using a hardware timer. @ref USTIMER_Init() must
   be called prior to using the delay functions. @ref USTIMER_Init() must
   also be called if HFCORECLK and/or HFPERCLK is changed.


  The source files for the USTIMER driver library resides in the
  emdrv/ustimer folder, and are named ustimer.c and ustimer.h.

  @li @ref ustimer_intro
  @li @ref ustimer_conf
  @li @ref ustimer_api
  @li @ref ustimer_example

@n @section ustimer_intro Introduction

  The USTIMER driver implements microsecond delay functions. The delay is timed
  using a hardware TIMER resource. Two delay functions are available, one which
  use energy mode EM1 to preserve energy while waiting, and one which performs
  busy wait.

@n @section ustimer_conf Configuration Options

  By default the module use TIMER0. Timer resource selection is stored in a
  file named @ref ustimer_config.h. A template for this file, containing default
  value, resides in the emdrv/config folder.

  To configure USTIMER, provide your own configuration file. Here is a
  sample @ref ustimer_config.h file:
  @verbatim
#ifndef __SILICON_LABS_USTIMER_CONFIG_H__
#define __SILICON_LABS_USTIMER_CONFIG_H__

/// USTIMER configuration option. Use this define to select a TIMER resource.
#define USTIMER_TIMER USTIMER_TIMER3

#endif
  @endverbatim

@n @section ustimer_api The API

  This section contain brief descriptions of the functions in the API. You will
  find detailed information on input and output parameters and return values by
  clicking on the hyperlinked function names. Most functions return an error
  code, @ref ECODE_EMDRV_USTIMER_OK is returned on success,
  see @ref ecode.h and @ref ustimer.h for other error codes.

  Your application code must include one header file: @em ustimer.h.

  @ref USTIMER_Init(), @ref USTIMER_DeInit() @n
    These functions initializes or deinitializes the USTIMER driver. Typically
    @htmlonly USTIMER_Init() @endhtmlonly is called once in your startup code.

  @ref USTIMER_Delay()
    Delay a given number of microseconds. The mcu is put in EM1 during the delay.

  @ref USTIMER_DelayIntSafe()
    Delay a given number of microseconds. The mcu is not put in EM1 during the
    delay.
    This function can be called in any context and is also thread safe.


@n @section ustimer_example Example
  @verbatim
#include "ustimer.h"

int main( void )
{
  // Initialization of USTIMER driver
  USTIMER_Init();

  // Wait for 250 microseconds
  USTIMER_Delay( 250 );
}
  @endverbatim

 * @}**************************************************************************/
