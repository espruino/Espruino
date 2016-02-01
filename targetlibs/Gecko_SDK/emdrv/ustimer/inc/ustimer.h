/**************************************************************************//**
 * @file  ustimer.h
 * @brief Microsecond delay function API definition.
 * @version 4.1.0
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
#ifndef __SILICON_LABS_USTIMER_H
#define __SILICON_LABS_USTIMER_H

#include <stdint.h>
#include "ecode.h"
#include "ustimer_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup USTIMER
 * @brief USTIMER Microsecond delay timer module, see @ref ustimer_doc page
 *        for detailed documentation.
 * @{
 ******************************************************************************/

#define ECODE_EMDRV_USTIMER_OK ( ECODE_OK ) ///< Success return value.

Ecode_t USTIMER_Init( void );
Ecode_t USTIMER_DeInit( void );
Ecode_t USTIMER_Delay( uint32_t usec );
Ecode_t USTIMER_DelayIntSafe( uint32_t usec );

#ifdef __cplusplus
}
#endif

/** @} (end group Ustimer) */
/** @} (end group Drivers) */

#endif
