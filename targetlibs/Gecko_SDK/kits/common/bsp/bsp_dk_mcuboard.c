/***************************************************************************//**
 * @file
 * @brief Board support package API for functions on MCU plugin boards.
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



#include <stdbool.h>
#include "bsp.h"
#include "em_gpio.h"
#include "em_cmu.h"

/***************************************************************************//**
 * @addtogroup BSP
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup BSP_DK API for DK's
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief Disable MCU plugin board peripherals.
 * @return @ref BSP_STATUS_OK.
 *****************************************************************************/
int BSP_McuBoard_DeInit( void )
{
#ifdef BSP_MCUBOARD_USB
  /* Disable GPIO port pin mode. */
  GPIO_PinModeSet( BSP_USB_STATUSLED_PORT, BSP_USB_STATUSLED_PIN, gpioModeDisabled, 0 );
  GPIO_PinModeSet( BSP_USB_OCFLAG_PORT, BSP_USB_OCFLAG_PIN, gpioModeDisabled, 0 );
  GPIO_PinModeSet( BSP_USB_VBUSEN_PORT, BSP_USB_VBUSEN_PIN, gpioModeDisabled, 0 );
#endif

  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Enable MCU plugin board peripherals.
 * @return @ref BSP_STATUS_OK.
 *****************************************************************************/
int BSP_McuBoard_Init( void )
{
#ifdef BSP_MCUBOARD_USB
  /* Make sure that the CMU clock to the GPIO peripheral is enabled  */
  CMU_ClockEnable( cmuClock_GPIO, true );

  /* USB status LED - configure PE1 as push pull */
  GPIO_PinModeSet( BSP_USB_STATUSLED_PORT, BSP_USB_STATUSLED_PIN, gpioModePushPull, 0 );

  /* USB PHY overcurrent status input */
  GPIO_PinModeSet( BSP_USB_OCFLAG_PORT, BSP_USB_OCFLAG_PIN, gpioModeInput, 0 );

  /* USB VBUS switch - configure PF5 as push pull - Default OFF */
  GPIO_PinModeSet( BSP_USB_VBUSEN_PORT, BSP_USB_VBUSEN_PIN, gpioModePushPull, 0 );
#endif

  return BSP_STATUS_OK;
}

/**************************************************************************//**
 * @brief Set state of MCU plugin board USB status LED.
 * @param[in] enable Set to true to turn on LED, false to turn off.
 * @return @ref BSP_STATUS_OK on plugin boards with USB capability,
 *         @ref BSP_STATUS_NOT_IMPLEMENTED otherwise.
 *****************************************************************************/
int BSP_McuBoard_UsbStatusLedEnable( bool enable )
{
#ifdef BSP_MCUBOARD_USB
  if ( enable )
  {
    GPIO_PinOutSet( BSP_USB_STATUSLED_PORT, BSP_USB_STATUSLED_PIN );
  }
  else
  {
    GPIO_PinOutClear( BSP_USB_STATUSLED_PORT, BSP_USB_STATUSLED_PIN );
  }

  return BSP_STATUS_OK;
#else

  (void)enable;
  return BSP_STATUS_NOT_IMPLEMENTED;
#endif
}

/**************************************************************************//**
 * @brief Get state MCU plugin board VBUS overcurrent flag.
 * @return True if overcurrent situation exist, false otherwise.
 *****************************************************************************/
bool BSP_McuBoard_UsbVbusOcFlagGet( void )
{
#ifdef BSP_MCUBOARD_USB
  bool flag;

  if ( !GPIO_PinInGet( BSP_USB_OCFLAG_PORT, BSP_USB_OCFLAG_PIN ) )
  {
    flag = true;
  }
  else
  {
    flag = false;
  }

  return flag;
#else

  return false;
#endif
}

/**************************************************************************//**
 * @brief Enable MCU plugin board VBUS power switch.
 * @param[in] enable Set to true to turn on VBUS power, false to turn off.
 * @return @ref BSP_STATUS_OK on plugin boards with USB capability,
 *         @ref BSP_STATUS_NOT_IMPLEMENTED otherwise.
 *****************************************************************************/
int BSP_McuBoard_UsbVbusPowerEnable( bool enable )
{
#ifdef BSP_MCUBOARD_USB
  GPIO_PinModeSet( BSP_USB_VBUSEN_PORT, BSP_USB_VBUSEN_PIN, gpioModePushPull, enable );

  return BSP_STATUS_OK;
#else

  (void)enable;
  return BSP_STATUS_NOT_IMPLEMENTED;
#endif
}

/** @} (end group BSP_DK) */
/** @} (end group BSP) */
