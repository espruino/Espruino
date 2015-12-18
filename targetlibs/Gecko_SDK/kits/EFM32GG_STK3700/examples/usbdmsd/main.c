/**************************************************************************//**
 * @file main.c
 * @brief Mass Storage Device example.
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
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "retargetserial.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "msdd.h"
#include "msddmedia.h"
#include "descriptors.h"


/**************************************************************************//**
 *
 * This example shows how a Mass Storage Device (MSD) can be implemented.
 *
 * Different kinds of media can be used for data storage. Modify the
 * MSD_MEDIA #define macro in msdmedia.h to select between the different ones.
 *
 *****************************************************************************/

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = MSDD_StateChangeEvent,
  .setupCmd        = MSDD_SetupCmd,
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
int main( void )
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false);
  CMU_ClockEnable(cmuClock_GPIO, true);

  SegmentLCD_Init(false);

  /* Initialize the Mass Storage Media. */
  if ( !MSDDMEDIA_Init() )
  {
    EFM_ASSERT( false );
    for( ;; ){}
  }

  SegmentLCD_Write("usbdmsd");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Initialize the Mass Storage Device. */
  MSDD_Init(gpioPortE, 2);

  /* Initialize and start USB device stack. */
  USBD_Init(&usbInitStruct);

  for (;;)
  {
    if ( MSDD_Handler() )
    {
      /* There is no pending activity in the MSDD handler.  */
      /* Enter sleep mode to conserve energy.               */

      if ( USBD_SafeToEnterEM2() )
      {
        /* Disable IRQ's and perform test again to avoid race conditions ! */
        INT_Disable();
        if ( USBD_SafeToEnterEM2() )
        {
          EMU_EnterEM2(true);
        }
        INT_Enable();
      }
      else
      {
        EMU_EnterEM1();
      }
    }
  }
}
