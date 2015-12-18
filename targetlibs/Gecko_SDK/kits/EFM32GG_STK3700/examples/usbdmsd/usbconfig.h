/***************************************************************************//**
 * @file usbconfig.h
 * @brief USB protocol stack library, application supplied configuration options.
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

#ifndef __USBCONFIG_H
#define __USBCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//#define BUSPOWERED      /* Uncomment to build buspowered device */

#define USB_DEVICE        /* Compile stack for device mode. */

#if defined(BUSPOWERED)
  #define USB_PWRSAVE_MODE (USB_PWRSAVE_MODE_ONSUSPEND)
#else
  #define USB_PWRSAVE_MODE (USB_PWRSAVE_MODE_ONSUSPEND | USB_PWRSAVE_MODE_ONVBUSOFF)
#endif


/****************************************************************************
**                                                                         **
** Specify number of endpoints used (in addition to EP0).                  **
**                                                                         **
*****************************************************************************/
#define NUM_EP_USED 2

/****************************************************************************
**                                                                         **
** Specify number of application timers you need.                          **
**                                                                         **
*****************************************************************************/
#define NUM_APP_TIMERS 1

/****************************************************************************
**                                                                         **
** USB Mass storage class device driver definitions.                       **
**                                                                         **
*****************************************************************************/
#define MSD_INTERFACE_NO  ( 0 )
#define MSD_BULK_OUT      ( 0x01 ) /* Endpoint for MSD data reception.       */
#define MSD_BULK_IN       ( 0x81 ) /* Endpoint for MSD data transmission.    */

#ifdef __cplusplus
}
#endif

#endif /* __USBCONFIG_H */
