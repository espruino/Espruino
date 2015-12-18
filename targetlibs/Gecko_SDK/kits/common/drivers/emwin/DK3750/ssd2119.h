/***************************************************************************//**
 * @file
 * @brief Real Time Counter (RTC) peripheral API
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
#ifndef __SSD2119_H
#define __SSD2119_H

#include <stdbool.h>
#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#ifdef SSD2119_REGISTER_ACCESS_HOOKS
  #define LCD_DEVFUNC_WRITEREGISTER 0x30
  #define LCD_DEVFUNC_READREGISTER  0x31
  #define LCD_DEVFUNC_WRITEDATA     0x32
  #define LCD_DEVFUNC_READDATA      0x33
#endif

#define LCD_DEVFUNC_INITIALIZE    0x34
#define LCD_DEVFUNC_CONTRADDR     0x35

/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __SSD2119_H */
