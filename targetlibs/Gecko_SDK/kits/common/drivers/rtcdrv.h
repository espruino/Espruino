/***************************************************************************//**
 * @file
 * @brief Real Time Counter (RTC) driver prototypes and definitions
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

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#ifndef __RTCDRV_H
#define __RTCDRV_H

#include "em_device.h"
#include "em_cmu.h"

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup RtcDrv
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void RTCDRV_Delay(uint32_t msec, bool useEM2);
void RTCDRV_Trigger(uint32_t msec, void (*cb)(void));
void RTCDRV_Setup(CMU_Select_TypeDef lfaClockSrc, CMU_ClkDiv_TypeDef rtcPrescale);

#ifdef __cplusplus
}
#endif

/** @} (end group RtcDrv) */
/** @} (end group Drivers) */

#endif

/** @endcond */
