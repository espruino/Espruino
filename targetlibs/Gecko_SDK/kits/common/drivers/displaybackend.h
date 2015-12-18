/**************************************************************************//**
 * @file displaybackend.h
 * @brief Display device backend interface
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




#ifndef _DISPLAY_BACKEND_H_
#define _DISPLAY_BACKEND_H_

#include "display.h"

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Display
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 **************************    FUNCTION PROTOTYPES    **************************
 ******************************************************************************/

EMSTATUS DISPLAY_DeviceRegister(DISPLAY_Device_t *device);


#ifdef __cplusplus
}
#endif

/** @} (end group Display) */
/** @} (end group Drivers) */

#endif /* _DISPLAY_BACKEND_H_ */
