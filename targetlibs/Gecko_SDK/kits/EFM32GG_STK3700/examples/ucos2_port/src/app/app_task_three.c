/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                         uC/OS-II example code
*                                          Application task three
*
*                                   Silicon Labs EFM32 (EFM32GG990F1024)
*                                              with the
*                               Silicon Labs EFM32GG990F1024-STK Starter Kit
*
* @file   app_task_three.c
* @brief
* @version 4.2.1
******************************************************************************
* @section License
* <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
* 4. The source and compiled code may only be used on Energy Micro "EFM32"
*    microcontrollers and "EFR4" radios.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
* obligation to support this Software. Energy Micro AS is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Energy Micro AS will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
*********************************************************************************************************
*/
#include <includes.h>


/*
*********************************************************************************************************
*                                             DEFINITIONS
*********************************************************************************************************
*/
/* Width of LCD screen on STK board */
#define APPDEF_LCD_TXT_SIZE  7


/*
*********************************************************************************************************
*                                         APP_TaskThree()
* @brief      The Three task.
*
* @param[in]  p_arg       Argument passed to 'APP_TaskThree()' by 'OSTaskCreate()'.
* @exception  none
* @return     none.
*/
/* Notes      :(1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*
*
*********************************************************************************************************
*/
void APP_TaskThree(void *p_arg)
{
/* As USART connectors are not available on the STK by default,
 * therefore printf() functions are turned off.
 * Uncomment the macro definition in includes.h if serial
 * is connected to your STK board (USART1 or LEUART0)!    */
#ifdef USART_CONNECTED
  static char taskStringBuffer[APPDEF_LCD_TXT_SIZE+1U] = {'u','C','/','O','S','-','2','\0'};
  char indxChar;
  char *pMsgContent;
#endif /* end of #ifndef USART_CONNECTED */
  static int  ringPos = 0;


  (void)p_arg; /* Note(1) */

  while (1)
  { /* Task body, always written as an infinite loop  */

    /* Turn previous ring segment off                 */
    SegmentLCD_ARing(ringPos, 0);

    /* Increase ring position variable                */
    if (8u == ++ringPos)
    {
      ringPos = 0; /* 3bit overflow */
    }

    /* Turn updated ring segment on                   */
    SegmentLCD_ARing(ringPos, 1);

/* As USART connectors are not available on the STK by default,
 * therefore printf() functions are turned off.
 * Uncomment the macro definition in includes.h if serial
 * is connected to your STK board (USART1 or LEUART0)!    */
#ifdef USART_CONNECTED
    pMsgContent = OSMboxAccept(pSerialMsgObj);
    if ((void *)0 != pMsgContent)
    {
      /* Shift left the whole string by one... */
      for (indxChar = 0; indxChar < APPDEF_LCD_TXT_SIZE; indxChar++)
      {
        taskStringBuffer[indxChar] = taskStringBuffer[indxChar+1];
      }

      /* ...and concatenate the new character to the end */
      taskStringBuffer[APPDEF_LCD_TXT_SIZE-1] = *pMsgContent;

      /* Write the string on serial port     */
      printf("\nBuffer: %s", taskStringBuffer);

      /* Write the string on LCD */
      SegmentLCD_Write(taskStringBuffer);
    }
#endif /* end of #ifndef USART0_CONNECTED */
    OSTimeDlyHMSM(0, 0, 0, 100);

    /* Delay task for 1 system tick (uC/OS-II suspends this task and executes
     * the next most important task) */
    OSTimeDly(1);
  }
}
