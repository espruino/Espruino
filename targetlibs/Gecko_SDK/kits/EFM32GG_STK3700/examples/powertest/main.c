/**************************************************************************//**
 * @file
 * @brief A very simple demonstration of different power modes.
 * @version 4.2.1
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

#include <stdlib.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_wdog.h"
#include "rtcdriver.h"
#include "bsp_trace.h"

/** Counts 1ms timeTicks */
volatile uint32_t msTicks;

/** Timer used for bringing the system back to EM0. */
RTCDRV_TimerID_t xTimerForWakeUp;

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}


/**************************************************************************//**
 * @brief SysTick_Disable
 * Disable systick interrupts
 *****************************************************************************/
void SysTick_Disable(void)
{
  SysTick->CTRL = 0x0000000;
}


/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  WDOG_Init_TypeDef wInit = WDOG_INIT_DEFAULT;
  int i;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize RTC timer. */
  RTCDRV_Init();
  RTCDRV_AllocateTimer( &xTimerForWakeUp);

  /* Watchdog setup - Use defaults, excepts for these :*/
  wInit.em2Run = true;
  wInit.em3Run = true;
  wInit.perSel = wdogPeriod_4k; /* 4k 1kHz periods should give ~4 seconds in EM3 */

  /* Do the demo forever. */

  /* EM0 - 1 sec HFXO  */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }
  Delay(1000);

  /* EM0 - 1 sec HFRCO */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }
  Delay(1000);

  /* Turn off systick */
  SysTick_Disable();

  /* EM1 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM1();

  /* EM2 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM2(true);

  /* EM1 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM1();

  /* EM2 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each 10 msec */
  for (i=0; i < 50; i++)
  {
    RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 10, NULL, NULL);
    EMU_EnterEM2(true);
    RTCDRV_Delay( 10);
  }

  /* EM2 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each 2 msec */
  for (i=0; i < 500; i++)
  {
    RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 2, NULL, NULL);
    EMU_EnterEM2(true);
  }

  /* EM2 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each msec */
  for (i=0; i < 1000; i++)
  {
    RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1, NULL, NULL);
    EMU_EnterEM2(true);
  }

  /* EM2 - 1 sec */
  RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 1000, NULL, NULL);
  EMU_EnterEM2(true);

  /* Start watchdog */
  WDOG_Init(&wInit);

  /* Enter EM3 - Watchdog will reset chip (and confuse debuggers) */
  EMU_EnterEM3(true);

  /* We will never reach this point */
  return 0;
}
