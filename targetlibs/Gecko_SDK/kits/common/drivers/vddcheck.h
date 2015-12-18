/**************************************************************************//**
 * @file
 * @brief Vdd voltage check routines, using VCMP
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


#ifndef __VDDCHECK_H
#define __VDDCHECK_H

#include <stdint.h>
#include <stdbool.h>

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup VddCheck
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void VDDCHECK_Init(void);
void VDDCHECK_Disable(void);
bool VDDCHECK_LowVoltage(float vdd);

#ifdef __cplusplus
}
#endif

/** @} (end group VddCheck) */
/** @} (end group Drivers) */

#endif
