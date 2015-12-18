/**************************************************************************//**
 * @file
 * @brief FreeRTOS Tickless Demo for Energy Micro EFM32GG_STK3700 Starter Kit
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


#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"


#include "em_chip.h"
#include "bsp_trace.h"
#include "segmentlcd.h"

#include "sleep.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)
#define DELAY ( 700 / portTICK_RATE_MS )

/* Semaphores used by application to synchronize two tasks */
xSemaphoreHandle sem;
/* Text to display */
char text[8];
/* Counter start value*/
int count = 0;
/**************************************************************************//**
 * @brief LcdPrint task which is showing numbers on the display
 * @param *pParameters pointer to parameters passed to the function
 *****************************************************************************/
static void LcdPrint(void *pParameters)
{
  pParameters = pParameters;   /* to quiet warnings */
  for (;;)
  {
    /* Wait for semaphore, then display next number */
   if (pdTRUE == xSemaphoreTake(sem, portMAX_DELAY)) {	
    SegmentLCD_Write(text);
    }
  }
}
/**************************************************************************//**
 * @brief Count task which is preparing next number to display
 * @param *pParameters pointer to parameters passed to the function
 *****************************************************************************/
static void Count(void *pParameters)
{
  pParameters = pParameters;   /* to quiet warnings */

  for (;;)
  {
    count = (count + 1) & 0xF;
    text[0] = count >= 10 ? '1' : '0';
    text[1] = count % 10 + '0';
    text[2] = '\0';
    xSemaphoreGive(sem);
    vTaskDelay(DELAY);
  }
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize SLEEP driver, no calbacks are used */
  SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
  /* do not let to sleep deeper than define */
  SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE+1));
#endif

  /* Initialize the LCD driver */
  SegmentLCD_Init(false);

  /* Create standard binary semaphore */
  vSemaphoreCreateBinary(sem);

  /* Create two task to show numbers from 0 to 15 */
  xTaskCreate(Count, (const char *) "Count", STACK_SIZE_FOR_TASK, NULL, TASK_PRIORITY, NULL);
  xTaskCreate(LcdPrint, (const char *) "LcdPrint", STACK_SIZE_FOR_TASK, NULL, TASK_PRIORITY, NULL);

  /* Start FreeRTOS Scheduler */
  vTaskStartScheduler();

  return 0;
}
