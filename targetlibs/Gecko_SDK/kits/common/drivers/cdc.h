/***************************************************************************//**
 * @file  cdc.h
 * @brief USB Communication Device Class (CDC) driver.
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
#ifndef __CDC_H
#define __CDC_H

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Cdc
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void CDC_Init( void );
int  CDC_SetupCmd(const USB_Setup_TypeDef *setup);
void CDC_StateChangeEvent( USBD_State_TypeDef oldState,
                           USBD_State_TypeDef newState);

#ifdef __cplusplus
}
#endif

/** @} (end group Cdc) */
/** @} (end group Drivers) */

#endif /* __CDC_H */
