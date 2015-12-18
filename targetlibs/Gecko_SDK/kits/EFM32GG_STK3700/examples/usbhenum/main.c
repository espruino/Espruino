/**************************************************************************//**
 * @file main.c
 * @brief USB host stack device enumeration example project.
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

#include <stdio.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_usb.h"
#include "segmentlcd.h"

/**************************************************************************//**
 *
 * This example shows how the USB host stack can be used to "probe" the device
 * properties of any device which is attached to the host port.
 *
 * The device attached will not be configured.
 *
 *****************************************************************************/

/*** Variables ***/

STATIC_UBUF( tmpBuf, 1024 );
static USBH_Device_TypeDef device;

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main(void)
{
  char lcdbuffer[8];
  int connectionResult;
  USBH_Init_TypeDef is = USBH_INIT_DEFAULT;

  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  CMU_ClockEnable(cmuClock_GPIO, true);

  SegmentLCD_Init(false);
  SegmentLCD_Write("USBHOST");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  USBH_Init(&is);               /* Initialize USB HOST stack */

  for (;;)
  {
    /* Wait for device connection */

    /* Wait for maximum 10 seconds. */
    connectionResult = USBH_WaitForDeviceConnectionB(tmpBuf, 10);

    if ( connectionResult == USB_STATUS_OK )
    {
      SegmentLCD_Write("Device");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("Added");
      USBTIMER_DelayMs(500);

      if (USBH_QueryDeviceB(tmpBuf, sizeof(tmpBuf), USBH_GetPortSpeed())
          == USB_STATUS_OK)
      {
        USBH_InitDeviceData(&device, tmpBuf, NULL, 0, USBH_GetPortSpeed());

        SegmentLCD_UnsignedHex(device.devDesc.idVendor);
        sprintf(lcdbuffer, "%.4xh", device.devDesc.idProduct);
        SegmentLCD_Write(lcdbuffer);

      }
      else
      {
      }

      while ( USBH_DeviceConnected() ){}
      SegmentLCD_NumberOff();
      SegmentLCD_Write("Device");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("Removed");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("USBHOST");
    }

    else if ( connectionResult == USB_STATUS_DEVICE_MALFUNCTION )
    {
    }

    else if ( connectionResult == USB_STATUS_TIMEOUT )
    {
      SegmentLCD_Write("TIMEOUT");
    }
  }
}
