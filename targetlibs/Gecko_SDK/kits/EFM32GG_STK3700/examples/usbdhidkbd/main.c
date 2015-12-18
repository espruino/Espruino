/**************************************************************************//**
 * @file main.c
 * @brief USB HID keyboard device example.
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
#include "em_gpio.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "descriptors.h"

/**************************************************************************//**
 *
 * This example shows how a HID keyboard can be implemented.
 *
 *****************************************************************************/

/*** Typedef's and defines. ***/

#define SCAN_TIMER              1       /* Timer used to scan keyboard. */
#define SCAN_RATE               50

#define ACTIVITY_LED_PORT       gpioPortE   /* LED0. */
#define ACTIVITY_LED_PIN        2
#define BUTTON_PORT             gpioPortB
#define BUTTON_PIN              9

/*** Function prototypes. ***/

static void StateChange( USBD_State_TypeDef oldState,
                         USBD_State_TypeDef newState );

/*** Variables ***/

static int      keySeqNo;           /* Current position in report table. */
static bool     keyPushed;          /* Current pushbutton status. */

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = StateChange,
  .setupCmd        = HIDKBD_SetupCmd,
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
  HIDKBD_Init_t hidInitStruct;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockEnable( cmuClock_GPIO, true );
  GPIO_PinModeSet( BUTTON_PORT, BUTTON_PIN, gpioModeInputPull, 1 );
  GPIO_PinModeSet( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN, gpioModePushPull, 0 );
  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
  SegmentLCD_Init( false );
  SegmentLCD_Write( "usb kbd" );
  SegmentLCD_Symbol( LCD_SYMBOL_GECKO, true );

  /* Initialize HID keyboard driver. */
  hidInitStruct.hidDescriptor = (void*)USBDESC_HidDescriptor;
  hidInitStruct.setReportFunc = NULL;
  HIDKBD_Init( &hidInitStruct );

  /* Initialize and start USB device stack. */
  USBD_Init( &usbInitStruct );

  /*
   * When using a debugger it is practical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect();      */
  /* USBTIMER_DelayMs(1000); */
  /* USBD_Connect();         */

  for (;;)
  {
  }
}

/**************************************************************************//**
 * @brief
 *   Timeout function for keyboard scan timer.
 *   Scan keyboard to check for key press/release events.
 *   This function is called at a fixed rate.
 *****************************************************************************/
static void ScanTimeout( void )
{
  bool pushed;
  HIDKBD_KeyReport_t *report;

  /* Check pushbutton */
  pushed = GPIO_PinInGet( BUTTON_PORT, BUTTON_PIN ) == 0;

  if (!keyPushed)
    GPIO_PinOutToggle( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN );

  if ( pushed != keyPushed )  /* Any change in keyboard status ? */
  {
    if ( pushed )
    {
      report = (void*)&USBDESC_reportTable[ keySeqNo ];
    }
    else
    {
      report = (void*)&USBDESC_noKeyReport;
    }

    /* Pass keyboard report on to the HID keyboard driver. */
    HIDKBD_KeyboardEvent( report );
  }

  /* Keep track of the new keypush event (if any) */
  if ( pushed && !keyPushed )
  {
    /* Advance to next position in report table */
    keySeqNo++;
    if ( keySeqNo == (sizeof(USBDESC_reportTable) / sizeof(HIDKBD_KeyReport_t)))
    {
      keySeqNo = 0;
    }
    GPIO_PinOutSet( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN );
  }
  keyPushed = pushed;

  /* Restart keyboard scan timer */
  USBTIMER_Start( SCAN_TIMER, SCAN_RATE, ScanTimeout );
}

/**************************************************************************//**
 * @brief
 *   Callback function called each time the USB device state is changed.
 *   Starts keyboard scanning when device has been configured by USB host.
 *
 * @param[in] oldState The device state the device has just left.
 * @param[in] newState The new device state.
 *****************************************************************************/
static void StateChange(USBD_State_TypeDef oldState,
                        USBD_State_TypeDef newState)
{
  /* Call HIDKBD drivers state change event handler. */
  HIDKBD_StateChangeEvent( oldState, newState );

  if ( newState == USBD_STATE_CONFIGURED )
  {
    /* We have been configured, start scanning the keyboard ! */
    if ( oldState != USBD_STATE_SUSPENDED ) /* Resume ?   */
    {
      keySeqNo        = 0;
      keyPushed       = false;
      GPIO_PinOutSet( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN );
    }
    USBTIMER_Start( SCAN_TIMER, SCAN_RATE, ScanTimeout );
  }

  else if ( ( oldState == USBD_STATE_CONFIGURED ) &&
            ( newState != USBD_STATE_SUSPENDED  )    )
  {
    /* We have been de-configured, stop keyboard scanning. */
    USBTIMER_Stop( SCAN_TIMER );
    GPIO_PinOutClear( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN );
  }

  else if ( newState == USBD_STATE_SUSPENDED )
  {
    /* We have been suspended, stop keyboard scanning. */
    /* Reduce current consumption to below 2.5 mA.     */
    GPIO_PinOutClear( ACTIVITY_LED_PORT, ACTIVITY_LED_PIN );
    USBTIMER_Stop( SCAN_TIMER );
  }
}
