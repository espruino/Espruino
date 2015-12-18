/**************************************************************************//**
 * @file main.c
 * @brief USB Composite Device example.
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
#include "em_assert.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "msdd.h"
#include "msddmedia.h"
#include "vud.h"
#include "cdc.h"
#include "descriptors.h"

/**************************************************************************//**
 *
 * This example shows how a Composite USB Device can be implemented.
 *
 *****************************************************************************/

int SetupCmd(const USB_Setup_TypeDef *setup);
void StateChangeEvent( USBD_State_TypeDef oldState,
                       USBD_State_TypeDef newState );

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = StateChangeEvent,
  .setupCmd        = SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

const USBD_Init_TypeDef usbInitStruct =
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
int main( void )
{
  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false);

  /* Initialize LCD driver */
  SegmentLCD_Init(false);
  SegmentLCD_Write("usbcomp");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Initialize LED driver */
  BSP_LedsInit();

  /* Initialize the Mass Storage Media. */
  if ( !MSDDMEDIA_Init() )
  {
    EFM_ASSERT( false );
    for( ;; ){}
  }

  VUD_Init();                   /* Initialize the vendor unique device. */
  CDC_Init();                   /* Initialize the communication class device. */
  MSDD_Init( -1, 0 );           /* Initialize the Mass Storage Device.  */

  /* Initialize and start USB device stack. */
  USBD_Init(&usbInitStruct);

  /*
   * When using a debugger it is practical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect();         */
  /* USBTIMER_DelayMs( 1000 );  */
  /* USBD_Connect();            */

  for (;;)
  {
    MSDD_Handler();             /* Serve the MSD device. */
  }
}

/**************************************************************************//**
 * @brief
 *   Called whenever a USB setup command is received.
 *
 * @param[in] setup
 *   Pointer to an USB setup packet.
 *
 * @return
 *   An appropriate status/error code. See USB_Status_TypeDef.
 *****************************************************************************/
int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int retVal;

  /* Call SetupCmd handlers for all functions within the composite device. */

  retVal = MSDD_SetupCmd( setup );

  if ( retVal == USB_STATUS_REQ_UNHANDLED )
  {
    retVal = VUD_SetupCmd( setup );
  }

  if ( retVal == USB_STATUS_REQ_UNHANDLED )
  {
    retVal = CDC_SetupCmd( setup );
  }

  return retVal;
}

/**************************************************************************//**
 * @brief
 *   Called whenever the USB device has changed its device state.
 *
 * @param[in] oldState
 *   The device USB state just leaved. See USBD_State_TypeDef.
 *
 * @param[in] newState
 *   New (the current) USB device state. See USBD_State_TypeDef.
 *****************************************************************************/
void StateChangeEvent( USBD_State_TypeDef oldState,
                       USBD_State_TypeDef newState )
{
  /* Call device StateChange event handlers for all relevant functions within
     the composite device. */

  MSDD_StateChangeEvent( oldState, newState );
  CDC_StateChangeEvent(  oldState, newState );
}
