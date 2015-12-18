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

#define USB_DEVICE        /* Compile stack for device mode. */
#define USB_PWRSAVE_MODE (USB_PWRSAVE_MODE_ONSUSPEND | USB_PWRSAVE_MODE_ONVBUSOFF)

/****************************************************************************
**                                                                         **
** Specify total number of endpoints used (in addition to EP0).            **
**                                                                         **
*****************************************************************************/
#define NUM_EP_USED 5

/****************************************************************************
**                                                                         **
** Specify number of application timers you need.                          **
**                                                                         **
*****************************************************************************/
#define NUM_APP_TIMERS 2

/****************************************************************************
**                                                                         **
** Misc. definitions for the interfaces of the composite device.           **
**                                                                         **
*****************************************************************************/

/* Define interface numbers */
#define VUD_INTERFACE_NO        ( 0 )
#define MSD_INTERFACE_NO        ( 1 )
#define CDC_CTRL_INTERFACE_NO   ( 2 )
#define CDC_DATA_INTERFACE_NO   ( 3 )
#define NUM_INTERFACES          ( 4 )

#define MSD_NUM_EP_USED         ( 2 ) /* Number of EP's used by MSD function */
#define VUD_NUM_EP_USED         ( 0 ) /* Number of EP's used by VUD function */
#define CDC_NUM_EP_USED         ( 3 ) /* Number of EP's used by CDC function */

/* Define USB endpoint addresses for the interfaces */
#define CDC_EP_DATA_OUT   ( 0x01 ) /* Endpoint for CDC data reception.       */
#define CDC_EP_DATA_IN    ( 0x81 ) /* Endpoint for CDC data transmission.    */
#define CDC_EP_NOTIFY     ( 0x82 ) /* The notification endpoint (not used).  */

#define MSD_BULK_OUT      ( 0x02 ) /* Endpoint for MSD data reception.       */
#define MSD_BULK_IN       ( 0x83 ) /* Endpoint for MSD data transmission.    */

/* Define timer ID's */
#define CDC_TIMER_ID        ( 1 )
#define MSD_FLUSH_TIMER_ID  ( 0 )

/* Configuration options for the CDC driver. */
#define CDC_UART_TX_DMA_CHANNEL   ( 0 )
#define CDC_UART_RX_DMA_CHANNEL   ( 1 )
#define CDC_TX_DMA_SIGNAL         DMAREQ_USART1_TXBL
#define CDC_RX_DMA_SIGNAL         DMAREQ_USART1_RXDATAV
#define CDC_UART                  USART1
#define CDC_UART_CLOCK            cmuClock_USART1
#define CDC_UART_ROUTE            ( USART_ROUTE_RXPEN | \
                                    USART_ROUTE_TXPEN | \
                                    USART_ROUTE_LOCATION_LOC1 )
#define CDC_UART_TX_PORT          gpioPortD
#define CDC_UART_TX_PIN           0
#define CDC_UART_RX_PORT          gpioPortD
#define CDC_UART_RX_PIN           1
#define CDC_ENABLE_DK_UART_SWITCH()     /* Empty define, this is not a DK. */

#ifdef __cplusplus
}
#endif

#endif /* __USBCONFIG_H */
