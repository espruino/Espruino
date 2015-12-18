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

#define USB_HOST          /* Compile stack for host mode. */

/****************************************************************************
**                                                                         **
** Specify number of host channels used (in addition to EP0).              **
**                                                                         **
*****************************************************************************/
#define NUM_HC_USED 0       /* Not counting default control ep which  */
                            /* is assigned to host channels 0 and 1   */

/****************************************************************************
**                                                                         **
** Configure USB overcurrent flag                                          **
**                                                                         **
*****************************************************************************/
#define USB_VBUSOVRCUR_PORT       gpioPortF
#define USB_VBUSOVRCUR_PIN        6
#define USB_VBUSOVRCUR_POLARITY   USB_VBUSOVRCUR_POLARITY_LOW

#ifdef __cplusplus
}
#endif

#endif /* __USBCONFIG_H */
