/***************************************************************************//**
 * @file
 * @brief Real Time Counter (RTC) driver, implements simple trigger functionality
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

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#include <stddef.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_rtc.h"
#include "rtcdrv.h"

static void          (*rtcCb)(void);        /**< RTC callback */
static uint8_t       rtcInitialized = 0;    /**< 1 if rtc is initialized */
static volatile bool rtcDelayComplete;      /**< True while CB delay and false while polled delay  */
static uint32_t      rtcFreq;               /**< RTC Frequence. 32.768 kHz */

/***************************************************************************//**
 * @brief Delay RTC callback
 ******************************************************************************/
static void DelayCB(void)
{
  rtcDelayComplete = true;
}

/***************************************************************************//**
 * @brief
 *  Setup RTC with selected clock source and prescaler.
 *
 * @param lfaClockSrc
 *  Clock source to use for LFA clock
 * @param rtcPrescale
 *  RTC prescaler
 ******************************************************************************/
void RTCDRV_Setup(CMU_Select_TypeDef lfaClockSrc, CMU_ClkDiv_TypeDef rtcPrescale)
{
  RTC_Init_TypeDef init;

  rtcInitialized = 1;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, lfaClockSrc);

  /* Use the prescaler to reduce power consumption. */
  CMU_ClockDivSet(cmuClock_RTC, rtcPrescale);

  rtcFreq = CMU_ClockFreqGet(cmuClock_RTC);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = false; /* Count to max before wrapping */
  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);
}

/***************************************************************************//**
 * @brief RTC delay function
 * @param msec Number of msec to delay
 * @param useEM2 Enter EM2 while waiting
 ******************************************************************************/
void RTCDRV_Delay(uint32_t msec, bool useEM2)
{
  rtcDelayComplete = false;
  RTCDRV_Trigger(msec, DelayCB);

  while (!rtcDelayComplete)
  {
    if (useEM2)
    {
      EMU_EnterEM2(true);
    }
  }
}

/***************************************************************************//**
 * @brief RTC Interrupt Handler, invoke callback function if defined.
 ******************************************************************************/
void RTC_IRQHandler(void)
{
  /* Disable RTC */
  RTC_Enable(false);

  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Disable interrupt */
  RTC_IntDisable(RTC_IF_COMP0);

  /* Trigger callback if defined */
  if (rtcCb)
  {
    rtcCb();
  }
}


/***************************************************************************//**
 * @brief RTC trigger enable
 * @param msec Enable trigger in msec
 * @param cb Callback invoked when @p msec elapsed
 ******************************************************************************/
void RTCDRV_Trigger(uint32_t msec, void (*cb)(void))
{
  /* Disable RTC - this will also reset the counter. */
  RTC_Enable(false);

  /* Auto init if not configured already */
  if (!rtcInitialized)
  {
    /* Default to LFRCO as clock source and prescale by 32. */
    RTCDRV_Setup(cmuSelect_LFRCO, cmuClkDiv_32);
  }

  /* Register callback */
  rtcCb = cb;

  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Calculate trigger value in ticks based on 32768Hz clock */
  RTC_CompareSet(0, (rtcFreq * msec) / 1000);

  /* Enable RTC */
  RTC_Enable(true);

  /* Enable interrupt on COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);
}

/** @endcond */
