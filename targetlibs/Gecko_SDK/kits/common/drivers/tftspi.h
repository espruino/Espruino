/**************************************************************************//**
 * @file
 * @brief EFM32GG_DK3750, SPI controller API for SSD2119 display interface
 *        when using Generic/Direct Drive mode
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


#ifndef __SPI_TFT_H
#define __SPI_TFT_H

#include <stdint.h>

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

void SPI_TFT_Init(void);
void SPI_TFT_WriteRegister(uint8_t reg, uint16_t data);

#ifdef __cplusplus
}
#endif

/** @} (end group Tft) */
/** @} (end group Drivers) */

#endif
