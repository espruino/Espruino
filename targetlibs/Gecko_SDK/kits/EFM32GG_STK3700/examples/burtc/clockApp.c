/**************************************************************************//**
 * @file
 * @brief Application handling calendar display and user input in
 *        EFM32 Backup Power Domain Application Note
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
#include <stdio.h>
#include "em_device.h"
#include "em_gpio.h"
#include "em_burtc.h"
#include "em_cmu.h"
#include "clock.h"
#include "clockApp.h"
#include "segmentlcd.h"

/* Calendar struct */
static struct tm calendar;
static time_t startTime;

/** Interrupt pin used to detect STK button/joystick activity */
#define UIF_PB0_PORT    gpioPortB
#define UIF_PB0_PIN     9
#define UIF_PB1_PORT    gpioPortB
#define UIF_PB1_PIN     10

/* Declare variables for LCD output*/
static char lcdStringBuf[8];
static char *lcdString = lcdStringBuf;
static bool lcdUpdate = true;


/* Declare variables for time keeping */
static uint32_t  burtcCount = 0;
static uint32_t  burtcOverflowCounter = 0;
static uint32_t  burtcOverflowIntervalRem;
// static uint32_t  burtcOverflowInterval;
static uint32_t  burtcTimestamp;
static time_t    currentTime;

/* Clock defines */
#define LFXO_FREQUENCY 32768
#define BURTC_PRESCALING 128
#define UPDATE_INTERVAL 1
#define COUNTS_PER_SEC (LFXO_FREQUENCY/BURTC_PRESCALING)
#define COUNTS_BETWEEN_UPDATE (UPDATE_INTERVAL*COUNTS_PER_SEC)

/**************************************************************************//**
 * @brief GPIO Odd Interrupt Handler
 *
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Clear GPIO interrupt */
  GPIO_IntClear(1 << UIF_PB0_PIN);

  startTime = clockGetStartTime( );
  calendar = *localtime( &startTime );

  calendar.tm_hour++;

  /* Set new epoch offset */
  startTime = mktime(&calendar);
  clockSetStartTime( startTime );
  clockAppBackup( );
  lcdUpdate = true;

}

/**************************************************************************//**
 * @brief GPIO Even Interrupt Handler
 *
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Clear GPIO interrupt */
  GPIO_IntClear(1 << UIF_PB1_PIN);

  startTime = clockGetStartTime( );
  calendar = *localtime( &startTime );

  calendar.tm_min++;

  /* Set new epoch offset */
  startTime = mktime(&calendar);
  clockSetStartTime( startTime );
  clockAppBackup( );
  lcdUpdate = true;

}


/***************************************************************************//**
 * @brief RTC Interrupt Handler, invoke callback if defined.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *        Do critical tasks in interrupt handler. Other tasks are handled in main
 *        while loop.
 ******************************************************************************/
void BURTC_IRQHandler(void)
{
  uint32_t irq;

  irq = BURTC_IntGet();
  BURTC_IntClear(irq);

  /* Interrupt source: compare match */
  /*   Increment compare value and
   *   update TFT display            */
  if ( irq & BURTC_IF_COMP0 )
  {
    BURTC_CompareSet(0, BURTC->COMP0 + COUNTS_BETWEEN_UPDATE );
  }

  /* Interrupt source: counter overflow */
  /*   Increase overflow counter
   *   and backup calendar              */
  if ( irq & BURTC_IF_OF )
  {
    clockOverflow( );
    clockAppBackup();
  }

  lcdUpdate = true;
}


/**************************************************************************//**
 * @brief Initialize clock application
 *****************************************************************************/
void clockAppInit(void)
{
  /* Compute overflow interval (integer) and remainder */
  // burtcOverflowInterval  =  ((uint64_t)UINT32_MAX+1)/COUNTS_BETWEEN_UPDATE; /* in seconds */
  burtcOverflowIntervalRem = ((uint64_t)UINT32_MAX+1)%COUNTS_BETWEEN_UPDATE;

  // burtcSetComp( COUNTS_BETWEEN_UPDATE );
  BURTC_CompareSet(0, COUNTS_BETWEEN_UPDATE );

  /* Initialize interrupts for STK */
  gpioIrqInit();

  /* Initialize LCD for output */
  SegmentLCD_Init(false);
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
}



/**************************************************************************//**
 * @brief  Backup CALENDAR to retention registers
 *
 *   Retention register 0 : Number of BURTC overflows
 *   Retention register 1 : Epoch offset
 *****************************************************************************/
void clockAppBackup(void)
{
  /* Write overflow counter to retention memory */
  BURTC_RetRegSet(0, clockGetOverflowCounter() );

  /* Write local epoch offset to retention memory */
  BURTC_RetRegSet(1, clockGetStartTime());
}


/**************************************************************************//**
 * @brief  Restore CALENDAR from retention registers
 *
 * @param[in] burtcCountAtWakeup BURTC value at power up. Only used for printout
 *****************************************************************************/
void clockAppRestore(uint32_t burtcCountAtWakeup)
{
  uint32_t burtcStart;
  uint32_t nextUpdate;
  (void)burtcCountAtWakeup;                       /* Unused parameter */


  /* Store BURTC->CNT for consistency in display output within this function */
  burtcCount = BURTC_CounterGet();

  /* Timestamp is BURTC value at time of main power loss */
  burtcTimestamp = BURTC_TimestampGet();

  /* Read overflow counter from retention memory */
  burtcOverflowCounter = BURTC_RetRegGet(0);

  /* Check for overflow while in backup mode
     Assume that overflow interval >> backup source capacity
     i.e. that overflow has only occured once during main power loss */
  if ( burtcCount < burtcTimestamp )
  {
    burtcOverflowCounter++;
  }

  /* Restore epoch offset from retention memory */
  clockSetStartTime(BURTC_RetRegGet(1));

  /* Restore clock overflow counter */
  clockSetOverflowCounter(burtcOverflowCounter);

  /* Calculate start point for current BURTC count cycle
     If (COUNTS_BETWEEN_UPDATE/burtcOverflowInterval) is not an integer,
     BURTC value at first update is different between each count cycle */
  burtcStart = (burtcOverflowCounter * (COUNTS_BETWEEN_UPDATE - burtcOverflowIntervalRem)) % COUNTS_BETWEEN_UPDATE;

  /*  Calculate next update compare value
      Add 1 extra UPDATE_INTERVAL to be sure that counter doesn't
      pass COMP value before interrupts are enabled */
  nextUpdate = burtcStart + ((burtcCount / COUNTS_BETWEEN_UPDATE) +1 ) * COUNTS_BETWEEN_UPDATE ;

  BURTC_CompareSet(0, nextUpdate);
}


/**************************************************************************//**
 * @brief  Show current time on LCD display
 *
 *****************************************************************************/
void clockAppDisplay(void)
{
  static bool initialized = false;
  if (!initialized)
  {
    SegmentLCD_Symbol(LCD_SYMBOL_COL3, true);
    SegmentLCD_Symbol(LCD_SYMBOL_COL5, true);
    initialized = true;
  }
  if ( lcdUpdate )
  {
    currentTime = time( NULL );
    calendar = *localtime( &currentTime );
    sprintf( lcdString, "%02d%02d%02d", calendar.tm_hour,
                                        calendar.tm_min,
                                        calendar.tm_sec );

    /* Print string to TFT display using GLIB_drawString routine */
    SegmentLCD_Write(lcdString);

    /* Clear tftUpdate flag */
    lcdUpdate = false;
  }
}


/**************************************************************************//**
 * @brief Initialize GPIO interrupt for STK buttons
 *****************************************************************************/
void gpioIrqInit(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure interrupt pins as input */
  GPIO_PinModeSet(UIF_PB0_PORT, UIF_PB0_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(UIF_PB1_PORT, UIF_PB1_PIN, gpioModeInput, 0);

  /* Set falling edge interrupt and clear/enable it */
  GPIO_IntConfig(UIF_PB0_PORT, UIF_PB0_PIN, false, true, true);
  GPIO_IntConfig(UIF_PB1_PORT, UIF_PB1_PIN, false, true, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
