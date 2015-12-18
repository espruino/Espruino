/******************************************************************************
 * @file
 * @brief Backup power domain and backup real time counter application note
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

#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_rmu.h"
#include "em_rtc.h"
#include "em_burtc.h"
#include "segmentlcd.h"
#include "clock.h"
#include "clockApp.h"
#include "bsp_trace.h"

/* Declare variables */
static uint32_t resetcause = 0;

/* Declare BURTC variables */
static uint32_t burtcCountAtWakeup = 0;

/* Calendar struct for initial date setting */
struct tm initialCalendar;

void budSetup(void);
void burtcSetup(void);

/******************************************************************************
 * @brief  Main function
 *
 *****************************************************************************/
int main( void )
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Read and clear RMU->RSTCAUSE as early as possible */
  resetcause = RMU->RSTCAUSE;
  RMU_ResetCauseClear();

  /* Enable clock to low energy modules */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Read Backup Real Time Counter value */
  burtcCountAtWakeup = BURTC_CounterGet();

  /* Configure Backup Domain */
  budSetup();

  /* Start LFXO and wait until it is stable */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

  /* Setting up a structure to initialize the calendar
     for January 1 2012 12:00:00
     The struct tm is declared in time.h
     More information for time.h library in http://en.wikipedia.org/wiki/Time.h */
  initialCalendar.tm_sec    = 0;   /* 0 seconds (0-60, 60 = leap second)*/
  initialCalendar.tm_min    = 0;   /* 0 minutes (0-59) */
  initialCalendar.tm_hour   = 12;  /* 6 hours (0-23) */
  initialCalendar.tm_mday   = 1;   /* 1st day of the month (1 - 31) */
  initialCalendar.tm_mon    = 0;   /* January (0 - 11, 0 = January) */
  initialCalendar.tm_year   = 112; /* Year 2012 (year since 1900) */
  initialCalendar.tm_wday   = 0;   /* Sunday (0 - 6, 0 = Sunday) */
  initialCalendar.tm_yday   = 0;   /* 1st day of the year (0-365) */
  initialCalendar.tm_isdst  = -1;  /* Daylight saving time; enabled (>0), disabled (=0) or unknown (<0) */

  /* Set the calendar */
  clockInit(&initialCalendar);

  /* Initialize display ++ */
  clockAppInit();

  /* If waking from backup mode, restore time from retention registers */
  if ( !(resetcause & RMU_RSTCAUSE_BUBODBUVIN) && (resetcause & RMU_RSTCAUSE_BUMODERST) )
  {
    /* Check if retention registers were being written to when backup mode was entered */
    if ( (BURTC->STATUS & BURTC_STATUS_RAMWERR) >> _BURTC_STATUS_RAMWERR_SHIFT )
    {
    }

    /* Check if timestamp is written */
    if (! ((BURTC->STATUS & BURTC_STATUS_BUMODETS) >> _BURTC_STATUS_BUMODETS_SHIFT) )
    {
    }

    /* Restore time from backup RTC + retention memory and print backup info*/
    clockAppRestore( burtcCountAtWakeup );

    /* Reset timestamp */
    BURTC_StatusClear();
  }

  /* If normal startup, initialize BURTC */
  else
  {
    /* Setup BURTC */
    burtcSetup();

    /* Backup initial calendar (also to initialize retention registers) */
    clockAppBackup();

    /* Update display if necessary */
    clockAppDisplay();
  }


  /* Enable BURTC interrupts */
  NVIC_ClearPendingIRQ(BURTC_IRQn);
  NVIC_EnableIRQ(BURTC_IRQn);

  /* ---------- Eternal while loop ---------- */
  while (1)
  {
    /* Sleep while waiting for interrupt */
    EMU_EnterEM2(true);

    /* Update display if necessary */
    clockAppDisplay();
  }
}


/***************************************************************************//**
 * @brief Set up backup domain.
 ******************************************************************************/
void budSetup(void)
{
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  EMU_BUPDInit_TypeDef bupdInit = EMU_BUPDINIT_DEFAULT;

  /* Unlock configuration */
  EMU_EM4Lock(false);

  /* Enable backup status pin */
  bupdInit.statusPinEnable = false;

  /* Enable backup power domain */
  bupdInit.enable = true;

  /* Normal operation: Connect main power to backup power through diode */
  bupdInit.inactivePower = emuPower_MainBU;

  /* Backup mode: No connection between backup power and main power */
  bupdInit.activePower = emuPower_None;

  /* Set backup "charging" resistor */
  bupdInit.resistor = emuRes_Res3;

  EMU_BUPDInit(&bupdInit);

  /* Wait until backup power functionality is ready */
  EMU_BUReady();

  /* Release reset for backup domain */
  RMU_ResetControl(rmuResetBU, rmuResetModeClear);

  /* Enable BU_VIN pin */
  bupdInit.enable = true;

  /* Enable voltage regulator in backup mode */
  em4Init.vreg = true;

  /* Configure oscillators in EM4 */
  em4Init.osc = emuEM4Osc_LFXO;

  /* Lock configuration in case of brown out */
  em4Init.lockConfig = true;

  EMU_EM4Init(&em4Init);
}


/**************************************************************************//**
 * @brief   Configure backup RTC
 *****************************************************************************/
void burtcSetup(void)
{
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

  /* Select LFXO as clock source for BURTC */
  burtcInit.clkSel = burtcClkSelLFXO;
  /* Enable BURTC operation in EM0-EM4 */
  burtcInit.mode = burtcModeEM4;
  /* Set prescaler to max. Resolution is not that important here */
  burtcInit.clkDiv = 128;
  /* Enable BURTC timestamp upon backup mode entry*/
  burtcInit.timeStamp = true;
  /* Counter doesn't wrap around when CNT == COMP0 */
  burtcInit.compare0Top = false;
  burtcInit.enable = true;

  /* Enable interrupt on compare match */
  BURTC_IntClear(BURTC_IEN_COMP0);
  BURTC_IntEnable(BURTC_IEN_COMP0);
  BURTC_Init(&burtcInit);
}
