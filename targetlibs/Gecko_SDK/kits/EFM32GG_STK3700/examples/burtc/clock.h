/***************************************************************************//**
 * @file
 * @brief CLOCK header file
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/


#ifndef __CLOCK_H
#define __CLOCK_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes*/
void clockInit(struct tm * timeptr);
void clockSetCal(struct tm * timeptr);
void clockSetStartTime(time_t offset);
time_t clockGetStartTime(void);
uint32_t clockOverflow(void);
void clockSetOverflowCounter(uint32_t of);
uint32_t clockGetOverflowCounter(void);

#ifdef __cplusplus
}
#endif

#endif
