/**************************************************************************//**
 * @file main.c
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
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "descriptors.h"

/**************************************************************************//**
 *
 * This example shows how a vendor unique device can be implemented.
 * A vendor unique device is a device which does not belong to any
 * USB class.
 *
 * Use the file EFM32_Vendor_Unique_Device.inf to install libusb device driver
 * on the host PC. This file reside in example subdirectory:
 * ./host/libusb/efm32-vendor-unique-device-1.2.5.0
 *
 *****************************************************************************/

#define LED0            0
#define LED1            1

#define VND_GET_LEDS    0x10
#define VND_SET_LED     0x11

static int SetupCmd(const USB_Setup_TypeDef *setup);

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = NULL,
  .setupCmd        = SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

static const USBD_Init_TypeDef usbInitStruct =
{
  .deviceDescriptor    = &USBDESC_deviceDesc,
  .configDescriptor    = USBDESC_configDesc,
  .stringDescriptors   = USBDESC_strings,
  .numberOfStrings     = sizeof(USBDESC_strings)/sizeof(void*),
  .callbacks           = &callbacks,
  .bufferingMultiplier = USBDESC_bufferingMultiplier,
  .reserved            = 0
};

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  SegmentLCD_Init(false);
  SegmentLCD_Write("usb vud");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Initialize LED driver */
  BSP_LedsInit();
  BSP_LedClear(LED0);
  BSP_LedClear(LED1);

  /* Initialize and start USB device stack. */
  USBD_Init( &usbInitStruct );

  /*
   * When using a debugger it is pratical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect(); */
  /* USBTIMER_DelayMs( 1000 ); */
  /* USBD_Connect(); */

  for (;;)
  {
  }
}

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
static int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int             retVal;
  static uint32_t buffer;
  uint8_t         *pBuffer = (uint8_t*) &buffer;

  retVal = USB_STATUS_REQ_UNHANDLED;

  if (setup->Type == USB_SETUP_TYPE_VENDOR)
  {
    switch (setup->bRequest)
    {
    case VND_GET_LEDS:
      /********************/
      *pBuffer = (uint8_t)BSP_LedsGet();
      retVal   = USBD_Write(0, pBuffer, setup->wLength, NULL);
      break;

    case VND_SET_LED:
      /********************/
      if (setup->wValue)
      {
        if ( setup->wIndex == LED0 )
          BSP_LedSet(LED0);
        else if ( setup->wIndex == LED1 )
          BSP_LedSet(LED1);
      }
      else
      {
        if ( setup->wIndex == LED0 )
          BSP_LedClear(LED0);
        else if ( setup->wIndex == LED1 )
          BSP_LedClear(LED1);
      }
      retVal = USB_STATUS_OK;
      break;
    }
  }

  return retVal;
}
