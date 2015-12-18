/**************************************************************************//**
 * @file
 * @brief Energy Mode LCD demo and test, prototypes and definitions
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

#ifndef __LCDTEST_H
#define __LCDTEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Test patterns */
//void ScrollText(char *scrolltext);
//void BlinkTest(void);
//void AnimateTest(void);

/* Utility functions */
//void RtcTrigger( RTCDRV_TimerID_t id, void *user);
//void EM1Sleep(uint32_t msec);
//void EM2Sleep(uint32_t msec);
//void EM3Sleep(void);
//void EM4Sleep(void);

/* Main test iteration routine */
void Test(void);

#ifdef __cplusplus
}
#endif

#endif
