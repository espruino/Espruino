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
*                                         uC/OS-III example code
*                                          Application task one
*
*                                   Silicon Labs EFM32 (EFM32GG990F1024)
*                                              with the
*                            Silicon Labs EFM32GG990F1024-STK Starter Kit
*
* @file   app_task_one.c
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
*                                         APP_TaskOne()
* @brief      The One task.
*
* @param[in]  p_arg       Argument passed to 'APP_TaskOne()' by 'OSTaskCreate()'.
* @exception  none
* @return     none.
*/
/* Notes      :(1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*
*
*********************************************************************************************************
*/
void APP_TaskOne(void *p_arg)
{
  OS_ERR err = OS_ERR_NONE;
  static CPU_INT32U ledPos = 0U; /* LED position variable  */


  (void)p_arg; /* Note(1) */

  while (1U)
  { /* Task body, always written as an infinite loop  */

    /* Delay with 100msec                             */
    OSTimeDlyHMSM(0U, 0U, 0U, 100U, (OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_STRICT), &err);

    /* Animate LEDs */
    BSP_LedToggle(ledPos);

    /* Modify LED position variable                   */
    if (4u == ++ledPos)
    {
      ledPos = 0; /* 3bit overflow */
    }

    /* Delay task for 1 system tick (uC/OS-III suspends this task and executes
     * the next most important task) */
    OSTimeDly(1U, OS_OPT_TIME_DLY, &err);
  }
}
