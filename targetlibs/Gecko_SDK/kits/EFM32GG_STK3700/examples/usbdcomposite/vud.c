/**************************************************************************//**
 * @file vud.c
 * @brief Vendor unique USB device example.
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

#include "em_device.h"
#include "em_usb.h"
#include "bsp.h"

/**************************************************************************//**
 *
 * This example shows how a vendor unique device can be implemented.
 * A vendor unique device is a device which does not belong to any
 * USB class.
 *
 * Use the file EFM32_Vendor_Unique_Device.inf to install libusb device driver
 * on the host PC. This file reside in the usbdvud example subdirectory:
 * ../usbdvud/host/libusb/efm32-vendor-unique-device-1.2.5.0
 *
 *****************************************************************************/

/*** Typedef's and defines. ***/

#define LED0            1
#define LED1            2

#define VND_GET_LEDS    0x10
#define VND_SET_LED     0x11

/**************************************************************************//**
 * @brief Vendor Unique Device initialization.
 *****************************************************************************/
void VUD_Init( void )
{
  BSP_LedsSet(0);
}

/**************************************************************************//**
 * @brief Handle USB setup commands for Vendor Unique Device.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
int VUD_SetupCmd(const USB_Setup_TypeDef *setup)
{
  int             retVal;
  uint16_t        leds;
  static uint32_t buffer;
  uint8_t         *pBuffer = (uint8_t*) &buffer;

  retVal = USB_STATUS_REQ_UNHANDLED;

  if (setup->Type == USB_SETUP_TYPE_VENDOR)
  {
    switch (setup->bRequest)
    {
    case VND_GET_LEDS:
      /********************/
      *pBuffer = BSP_LedsGet() & 0x3;
      retVal   = USBD_Write(0, pBuffer, setup->wLength, NULL);
      break;

    case VND_SET_LED:
      /********************/
      leds = BSP_LedsGet() & 0x3;
      if (setup->wValue)
      {
        leds |= LED0 << setup->wIndex;
      }
      else
      {
        leds &= ~(LED0 << setup->wIndex);
      }
      BSP_LedsSet(leds);
      retVal = USB_STATUS_OK;
      break;
    }
  }

  return retVal;
}
