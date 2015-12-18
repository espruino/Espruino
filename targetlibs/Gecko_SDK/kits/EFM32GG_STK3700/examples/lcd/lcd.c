/**************************************************************************//**
 * @file
 * @brief LCD controller demo for EFM32GG_STK3700 development kit
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
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/* Locatl prototypes */
void Delay(uint32_t dlyTicks);

/**************************************************************************//**
 * @brief SysTick_Handler
 *   Interrupt Service Routine for system tick counter
 * @note
 *   No wrap around protection
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
  int i;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Enable two leds to show we're alive */
  BSP_LedsInit();
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  /* Enable LCD without voltage boost */
  SegmentLCD_Init(false);

  /* Infinite loop with test pattern. */
  while (1)
  {
    /* Enable all segments */
    SegmentLCD_AllOn();
    Delay(500);

    /* Disable all segments */
    SegmentLCD_AllOff();

    /* Write a number */
    for (i = 0; i < 10; i++)
    {
      SegmentLCD_Number(i * 1111);
      Delay(200);
    }
    /* Write some text */
    SegmentLCD_Write("Silicon");
    Delay(500);
    SegmentLCD_Write("Labs");
    Delay(500);
    SegmentLCD_Write("Giant");
    Delay(500);
    SegmentLCD_Write("Gecko");
    Delay(1000);

    SegmentLCD_AllOff();

    /* Test segments */
    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_ANT, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_MINUS, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL10, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP2, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP4, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP6, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);

    SegmentLCD_Battery(0);
    SegmentLCD_Battery(1);
    SegmentLCD_Battery(2);
    SegmentLCD_Battery(3);
    SegmentLCD_Battery(4);

    SegmentLCD_ARing(0, 1);
    SegmentLCD_ARing(1, 1);
    SegmentLCD_ARing(2, 1);
    SegmentLCD_ARing(3, 1);
    SegmentLCD_ARing(4, 1);
    SegmentLCD_ARing(5, 1);
    SegmentLCD_ARing(6, 1);
    SegmentLCD_ARing(7, 1);

    Delay(1000);
  }
}
