/***************************************************************************//**
 * @file
 * @brief Board support package API for GPIO leds on STK's.
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



#include "bsp.h"

#if defined( BSP_DK_LEDS )
/***************************************************************************//**
 * @addtogroup BSP
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup BSPCOMMON API common for all kits
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief Initialize LED drivers.
 * @note  LED's are initially turned off.
 * @return
 *   @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_LedsInit(void)
{
  BSP_RegisterWrite(BSP_LED_PORT, 0);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Get status of all LED's.
 * @return
 *   Bitmask with current status for all LED's.
 *****************************************************************************/
uint32_t BSP_LedsGet(void)
{
  return BSP_RegisterRead(BSP_LED_PORT) & BSP_LED_MASK;
}

/**************************************************************************//**
 * @brief Update all LED's.
 * @param[in] leds Bitmask representing new status for all LED's. A 1 turns
 *                 a LED on, a 0 turns a LED off.
 * @return
 *   @ref BSP_STATUS_OK
 *****************************************************************************/
int BSP_LedsSet(uint32_t leds)
{
  BSP_RegisterWrite(BSP_LED_PORT, leds & BSP_LED_MASK);
  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Turn off a single LED.
 * @param[in] ledNo The number of the LED (counting from zero) to turn off.
 * @return
 *   @ref BSP_STATUS_OK or @ref BSP_STATUS_ILLEGAL_PARAM if illegal LED number.
 *****************************************************************************/
int BSP_LedClear(int ledNo)
{
  uint32_t tmp;

  if ((ledNo >= 0) && (ledNo < BSP_NO_OF_LEDS))
  {
    tmp = BSP_RegisterRead(BSP_LED_PORT) & BSP_LED_MASK;
    tmp &= ~( 1 << ledNo );
    BSP_RegisterWrite(BSP_LED_PORT, tmp);
    return BSP_STATUS_OK;
  }
  return BSP_STATUS_ILLEGAL_PARAM;
}

/**************************************************************************//**
 * @brief Get current status of a single LED.
 * @param[in] ledNo The number of the LED (counting from zero) to check.
 * @return
 *   1 if LED is on, 0 if LED is off, @ref BSP_STATUS_ILLEGAL_PARAM if illegal
 *   LED number.
 *****************************************************************************/
int BSP_LedGet(int ledNo)
{
  if ((ledNo >= 0) && (ledNo < BSP_NO_OF_LEDS))
  {
    if ( BSP_RegisterRead(BSP_LED_PORT) & BSP_LED_MASK & (1 << ledNo) )
      return 1;

    return 0;
  }
  return BSP_STATUS_ILLEGAL_PARAM;
}

/**************************************************************************//**
 * @brief Turn on a single LED.
 * @param[in] ledNo The number of the LED (counting from zero) to turn on.
 * @return
 *   @ref BSP_STATUS_OK or @ref BSP_STATUS_ILLEGAL_PARAM if illegal LED number.
 *****************************************************************************/
int BSP_LedSet(int ledNo)
{
  uint32_t tmp;

  if ((ledNo >= 0) && (ledNo < BSP_NO_OF_LEDS))
  {
    tmp = BSP_RegisterRead(BSP_LED_PORT) & BSP_LED_MASK;
    tmp |= 1 << ledNo;
    BSP_RegisterWrite(BSP_LED_PORT, tmp);
    return BSP_STATUS_OK;
  }
  return BSP_STATUS_ILLEGAL_PARAM;
}

/**************************************************************************//**
 * @brief Toggle a single LED.
 * @param[in] ledNo The number of the LED (counting from zero) to toggle.
 * @return
 *   @ref BSP_STATUS_OK or @ref BSP_STATUS_ILLEGAL_PARAM if illegal LED number.
 *****************************************************************************/
int BSP_LedToggle(int ledNo)
{
  uint32_t tmp;

  if ((ledNo >= 0) && (ledNo < BSP_NO_OF_LEDS))
  {
    tmp = BSP_RegisterRead(BSP_LED_PORT) & BSP_LED_MASK;
    tmp ^= 1 << ledNo;
    BSP_RegisterWrite(BSP_LED_PORT, tmp);
    return BSP_STATUS_OK;
  }
  return BSP_STATUS_ILLEGAL_PARAM;
}

/** @} */
/** @} */
#endif  /* BSP_DK_LEDS */
