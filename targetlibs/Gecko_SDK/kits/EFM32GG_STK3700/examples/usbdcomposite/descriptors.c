/***************************************************************************//**
 * @file descriptors.c
 * @brief USB descriptors for composite device example project.
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
#include "descriptors.h"

EFM32_ALIGN(4)
const USB_DeviceDescriptor_TypeDef USBDESC_deviceDesc __attribute__ ((aligned(4)))=
{
  .bLength            = USB_DEVICE_DESCSIZE,
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = USB_CLASS_MISCELLANEOUS,
  .bDeviceSubClass    = USB_CLASS_MISC_COMMON_SUBCLASS,
  .bDeviceProtocol    = USB_CLASS_MISC_IAD_PROTOCOL,
  .bMaxPacketSize0    = USB_FS_CTRL_EP_MAXSIZE,
  .idVendor           = 0x10C4,
  .idProduct          = 0x0008,
  .bcdDevice          = 0x0000,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1
};

#if ( NUM_EP_USED != (MSD_NUM_EP_USED + VUD_NUM_EP_USED + CDC_NUM_EP_USED) )
#error "Incorrect endpoint count."
#endif

#define CDC_MISC_DESCRIPTOR_LEN ( USB_CDC_HEADER_FND_DESCSIZE  +        \
                                  USB_CDC_CALLMNG_FND_DESCSIZE +        \
                                  USB_CDC_ACM_FND_DESCSIZE     +        \
                                  USB_INTERFACE_ASSOCIATION_DESCSIZE +  \
                                  5 )/*CDC Union Functional descriptor length*/

#define CONFIG_DESC_TOTAL_LEN                             \
        ( USB_CONFIG_DESCSIZE                         +   \
          ( USB_INTERFACE_DESCSIZE * NUM_INTERFACES ) +   \
          ( USB_ENDPOINT_DESCSIZE  * NUM_EP_USED )    +   \
          CDC_MISC_DESCRIPTOR_LEN )

EFM32_ALIGN(4)
const uint8_t USBDESC_configDesc[] __attribute__ ((aligned(4)))=
{
  /*** Configuration descriptor ***/
  USB_CONFIG_DESCSIZE,    /* bLength                                   */
  USB_CONFIG_DESCRIPTOR,  /* bDescriptorType                           */
  CONFIG_DESC_TOTAL_LEN,  /* wTotalLength (LSB)                        */
  CONFIG_DESC_TOTAL_LEN>>8, /* wTotalLength (MSB)                      */
  NUM_INTERFACES,         /* bNumInterfaces                            */
  1,                      /* bConfigurationValue                       */
  0,                      /* iConfiguration                            */
  CONFIG_DESC_BM_RESERVED_D7 |    /* bmAttrib: Self powered            */
  CONFIG_DESC_BM_SELFPOWERED,
  CONFIG_DESC_MAXPOWER_mA( 100 ), /* bMaxPower: 100 mA                 */

  /*** VUD (Vendor Unique Device) Function ***/
  /*** Interface descriptor ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  VUD_INTERFACE_NO,       /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  VUD_NUM_EP_USED,        /* bNumEndpoints         */
  0xFF,                   /* bInterfaceClass       */
  0,                      /* bInterfaceSubClass    */
  0,                      /* bInterfaceProtocol    */
  0,                      /* iInterface            */

  /*** MSD Function         ***/
  /*** Interface descriptor ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  MSD_INTERFACE_NO,       /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  MSD_NUM_EP_USED,        /* bNumEndpoints         */
  USB_CLASS_MSD,          /* bInterfaceClass       */
  USB_CLASS_MSD_SCSI_CMDSET, /* bInterfaceSubClass */
  USB_CLASS_MSD_BOT_TRANSPORT,/* bInterfaceProtocol*/
  0,                      /* iInterface            */

  /*** Endpoint descriptors ***/
  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  MSD_BULK_OUT,           /* bEndpointAddress (OUT)*/
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0,                      /* bInterval             */

  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  MSD_BULK_IN,            /* bEndpointAddress (IN) */
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0,                      /* bInterval             */

  /*** CDC Function                                                 ***/
  /*** IAD (Interface Association Descriptor) for the CDC function  ***/
  USB_INTERFACE_ASSOCIATION_DESCSIZE, /* bLength: Interface Descriptor size */
  USB_INTERFACE_ASSOCIATION_DESCRIPTOR, /* bDescriptorType      */
  CDC_CTRL_INTERFACE_NO,  /* bFirstInterface                    */
  2,                      /* bInterfaceCount                    */
  USB_CLASS_CDC,          /* bFunctionClass                     */
  USB_CLASS_CDC_ACM,      /* bFunctionSubClass                  */
  0,                      /* bFunctionProtocol                  */
  0,                      /* iFunction                          */

  /*** Communication Class Interface descriptor ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  CDC_CTRL_INTERFACE_NO,  /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  1,                      /* bNumEndpoints         */
  USB_CLASS_CDC,          /* bInterfaceClass       */
  USB_CLASS_CDC_ACM,      /* bInterfaceSubClass    */
  0,                      /* bInterfaceProtocol    */
  0,                      /* iInterface            */

  /*** CDC Header Functional descriptor ***/
  USB_CDC_HEADER_FND_DESCSIZE, /* bFunctionLength  */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_HFN,      /* bDescriptorSubtype    */
  0x20,                   /* bcdCDC spec.no LSB    */
  0x01,                   /* bcdCDC spec.no MSB    */

  /*** CDC Call Management Functional descriptor ***/
  USB_CDC_CALLMNG_FND_DESCSIZE, /* bFunctionLength */
  USB_CS_INTERFACE_DESCRIPTOR,  /* bDescriptorType */
  USB_CLASS_CDC_CMNGFN,   /* bDescriptorSubtype    */
  0,                      /* bmCapabilities        */
  CDC_DATA_INTERFACE_NO,  /* bDataInterface        */

  /*** CDC Abstract Control Management Functional descriptor ***/
  USB_CDC_ACM_FND_DESCSIZE, /* bFunctionLength     */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_ACMFN,    /* bDescriptorSubtype    */
  0x02,                   /* bmCapabilities        */
  /* The capabilities that this configuration supports:                   */
  /* D7..D4: RESERVED (Reset to zero)                                     */
  /* D3: 1 - Device supports the notification Network_Connection.         */
  /* D2: 1 - Device supports the request Send_Break                       */
  /* D1: 1 - Device supports the request combination of Set_Line_Coding,  */
  /*         Set_Control_Line_State, Get_Line_Coding, and the             */
  /*         notification Serial_State.                                   */
  /* D0: 1 - Device supports the request combination of Set_Comm_Feature, */
  /*         Clear_Comm_Feature, and Get_Comm_Feature.                    */

  /*** CDC Union Functional descriptor ***/
  5,                      /* bFunctionLength       */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_UNIONFN,  /* bDescriptorSubtype    */
  CDC_CTRL_INTERFACE_NO,  /* bControlInterface     */
  CDC_DATA_INTERFACE_NO,  /* bSubordinateInterface0*/

  /*** CDC Notification endpoint descriptor ***/
  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  CDC_EP_NOTIFY,          /* bEndpointAddress (IN) */
  USB_EPTYPE_INTR,        /* bmAttributes          */
  USB_FS_INTR_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0xFF,                   /* bInterval             */

  /*** Data Class Interface descriptor ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  CDC_DATA_INTERFACE_NO,  /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  2,                      /* bNumEndpoints         */
  USB_CLASS_CDC_DATA,     /* bInterfaceClass       */
  0,                      /* bInterfaceSubClass    */
  0,                      /* bInterfaceProtocol    */
  0,                      /* iInterface            */

  /*** CDC Data interface endpoint descriptors ***/
  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  CDC_EP_DATA_IN,         /* bEndpointAddress (IN) */
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0,                      /* bInterval             */

  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  CDC_EP_DATA_OUT,        /* bEndpointAddress (OUT)*/
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0,                      /* bInterval             */
};

STATIC_CONST_STRING_DESC_LANGID( langID, 0x04, 0x09 );
STATIC_CONST_STRING_DESC( iManufacturer, 'S','i','l','i','c','o','n',' ','L', \
                                         'a','b','o','r','a','t','o','r','i', \
                                         'e','s',' ','I','n','c','.' );
STATIC_CONST_STRING_DESC( iProduct     , 'E','F','M','3','2',' ','U','S','B', \
                                         ' ','C','o','m','p','o','s','i','t', \
                                         'e',' ','D','e','v','i', 'c','e' );
STATIC_CONST_STRING_DESC( iSerialNumber, '0','0','0','0','1','2',             \
                                         '3','4','5','6','7','8' );

const void * const USBDESC_strings[] =
{
  &langID,
  &iManufacturer,
  &iProduct,
  &iSerialNumber
};

/* Endpoint buffer sizes */
/* 1 = single buffer, 2 = double buffering, 3 = tripple buffering ... */
const uint8_t USBDESC_bufferingMultiplier[ NUM_EP_USED + 1 ] =
{
  1,        /* Common Control endpoint.           */
  2, 2,     /* MSD bulk endpoints.                */
  1, 2, 2   /* CDC interrupt and bulk endpoints.  */
};
