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
*                                      Master include header file
*
*                                   Silicon Labs EFM32 (EFM32GG990F1024)
*                                              with the
*                               Silicon Labs EFM32GG990F1024-STK Starter Kit
*
* @file   includes.h
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
#ifndef  __INCLUDES_H
#define  __INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef  OS_MASTER_FILE

/*
*********************************************************************************************************
*                                          STANDARD LIBRARIES
*********************************************************************************************************
*/
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/
#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_mem.h>


/*
*********************************************************************************************************
*                                                  OS
*********************************************************************************************************
*/
#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                               APP/BSP
*********************************************************************************************************
*/
#include  "app_task_one.h"
#include  "app_task_two.h"
#include  "app_task_three.h"
#include  "app_cfg.h"
#include  "bspos.h"
#include  "retargetserial.h"


/*
*********************************************************************************************************
*                                      ENERGYMICRO HEADER FILES
*********************************************************************************************************
*/
#include <bsp.h>
#include <segmentlcd.h>


/*
*********************************************************************************************************
*                                      ENERGYMICRO LIBRARY FILES
*********************************************************************************************************
*/
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_lcd.h>
#include <em_system.h>
#include <em_usart.h>
#include <em_chip.h>


/*
*********************************************************************************************************
*                                       ENERGYMICRO DRIVER FILES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          MACRO DEFINITIONS
*********************************************************************************************************
*/
/* Uncomment this macro definition if USART1 or LEUART0 is connected to your STK board! */
 #define USART_CONNECTED


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

/* declaration of global mailbox object for inter-task communication */
extern OS_EVENT *pSerialMsgObj;


#endif /* end of OS_MASTER_FILE */

#ifdef __cplusplus
}
#endif

#endif /* end of __INCLUDES_H (do not include header files after this!) */
