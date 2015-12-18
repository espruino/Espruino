/**************************************************************************//**
 * @file
 * @brief Binary Support Package demo for EFM32TG_STK3300
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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
/* CMSIS package */
#include "em_device.h"
/* emlib */
#include "em_cmu.h"
#include "em_chip.h"
/* STK Board Support Package */
#include "bsp.h"
#include "bsp_trace.h"
/* STK Drivers */
#include "vddcheck.h"
#include "segmentlcd.h"

/* Prototypes for local functions */
void SysTick_Handler(void);
void Delay(uint32_t dlyTicks);

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
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
  int   value, delayCount = 0, hfrcoband = 0;
  float current, voltage;
  bool  vboost;
  char  buffer[8];

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize board support package */
  BSP_Init(BSP_INIT_BCC);

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClockGet() / 1000)) while (1) ;

  /* Initialize voltage comparator, to check supply voltage */
  VDDCHECK_Init();

  /* Check if voltage is below 3V, if so use voltage boost */
  if (VDDCHECK_LowVoltage(2.9))
  {
    vboost = true;
  }
  else
  {
    vboost = false;
  }

  /* Disable Voltage Comparator */
  VDDCHECK_Disable();

  /* Initialize segment LCD */
  SegmentLCD_Init(vboost);

  /* Infinite loop */
  while (1)
  {
    /* Read and display current */
    current = BSP_CurrentGet();
    value   = (int)(1000 * current);

    /* Check that we fall within displayable value */
    if ((value > 0) && (value < 10000))
    {
      SegmentLCD_Number(value);
    }
    else
    {
      SegmentLCD_Number(-1);
    }

    /* Alternate between voltage and clock frequency */
    if (((delayCount / 10) & 1) == 0)
    {
      voltage = BSP_VoltageGet();
      value   = (int)(voltage * 100);
      SegmentLCD_Symbol(LCD_SYMBOL_DP6, 1);
      sprintf(buffer, "Volt%3d", value);
      SegmentLCD_Write(buffer);
    }
    else
    {
      SegmentLCD_Symbol(LCD_SYMBOL_DP6, 0);
      sprintf(buffer, "%3u MHz", (int)(SystemCoreClockGet() / 1000000));
      SegmentLCD_Write(buffer);
    }
    /* After 5 seconds, use another HFRCO band */
    if (delayCount % 50 == 0)
    {
      switch (hfrcoband)
      {
      case 0:
        CMU_HFRCOBandSet(cmuHFRCOBand_11MHz);
        break;
      case 1:
        CMU_HFRCOBandSet(cmuHFRCOBand_14MHz);
        break;
      case 2:
        CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);
        break;
      default:
        CMU_HFRCOBandSet(cmuHFRCOBand_28MHz);
        /* Restart iteartion */
        hfrcoband = -1;
        break;
      }
      hfrcoband++;
      /* Recalculate delay tick count and baudrate generation */
      if (SysTick_Config(SystemCoreClockGet() / 1000)) while (1) ;
      BSP_Init(BSP_INIT_BCC);
    }

    Delay(100);
    delayCount++;
  }
}
