/**************************************************************************//**
 * @file
 * @brief CALENDAR header file
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

#ifndef __CLOCKAPP_H
#define __CLOCKAPP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
void clockAppInit(void);
void clockAppDisplay(void);
void clockAppBackup(void);
void clockAppRestore(uint32_t);
void clockAppUpdate(void);
void clockAppPrintWakeupStatus(uint32_t);
void clockAppPrintRamWErr(void);
void clockAppPrintNoTimestamp(void);
void clockAppPrintResetCause(uint32_t);
void clockAppOverflow(void);

void gpioIrqInit(void);

#ifdef __cplusplus
}
#endif

#endif
