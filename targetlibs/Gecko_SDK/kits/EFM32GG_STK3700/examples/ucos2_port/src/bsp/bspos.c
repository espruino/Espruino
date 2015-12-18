/***************************************************************************//**
 * @file
 * @brief uC/OS-II example - Board Support Package (BSP) for
 * Energy Micro EFM32G890F128-STK Starter Kit
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

#include <includes.h>


/***************************************************************************//**
 *                                BSPOS_Init()
 * @brief      Board Support Package Initialization.
 *
 * @param[in]  none
 * @exception  none
 * @return     none
 *
 ******************************************************************************/
void  BSPOS_Init (void)
{
  /* Initialize LEDs */
  BSP_LedsInit();

  /* Set external crystal oscillator */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  /* Enable module clocks */
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_HFPER, true);
}
