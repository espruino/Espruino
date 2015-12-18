/**************************************************************************//**
 * @file
 * @brief EFM32GG_DK3750, TFT Initialization and setup for Direct Drive mode
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


#ifndef __TFTDIRECT_H
#define __TFTDIRECT_H

#include <stdbool.h>
#include "em_device.h"
#include "em_ebi.h"

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Tft
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void TFT_DirectGPIOConfig(void);
bool TFT_DirectInit(const EBI_TFTInit_TypeDef *tftInit);

#ifdef __cplusplus
}
#endif

/** @} (end group Tft) */
/** @} (end group Drivers) */

#endif
