/**
  ******************************************************************************
  * @file    otgd_fs_pcd.h
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Header file of the High Layer device mode interface and wrapping layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


#ifndef __USB_OTG_PCD_H__
#define __USB_OTG_PCD_H__

#include "otgd_fs_regs.h"

#define MAX_EP0_SIZE                    0x40
#define MAX_PACKET_SIZE                 0x400


#define USB_ENDPOINT_XFER_CONTROL       0
#define USB_ENDPOINT_XFER_ISOC          1
#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_XFER_INT           3
#define USB_ENDPOINT_XFERTYPE_MASK      3


/********************************************************************************
                              ENUMERATION TYPE
********************************************************************************/
enum usb_device_speed {
  USB_SPEED_UNKNOWN = 0,
  USB_SPEED_LOW, USB_SPEED_FULL,
  USB_SPEED_HIGH
};
/********************************************************************************
                              Data structure type
********************************************************************************/
typedef struct usb_ep_descriptor
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
}
EP_DESCRIPTOR , *PEP_DESCRIPTOR;
/********************************************************************************
                     USBF LAYER UNION AND STRUCTURES
********************************************************************************/
typedef struct USB_OTG_USBF
{

  USB_OTG_EP ep0;
  USB_OTG_EP in_ep[ NUM_TX_FIFOS - 1];
  USB_OTG_EP out_ep[ NUM_TX_FIFOS - 1];
}
USB_OTG_PCD_DEV , *USB_OTG_PCD_PDEV;
/********************************************************************************
                     EXPORTED FUNCTION FROM THE USB_OTG LAYER
********************************************************************************/
void        PCD_Init(void);
void        PCD_DevConnect (void);
void        PCD_DevDisconnect (void);
void        PCD_EP_SetAddress (uint8_t address);
uint32_t    PCD_EP_Open(EP_DESCRIPTOR *epdesc);
uint32_t    PCD_EP_Close  ( uint8_t  ep_addr);
uint32_t    PCD_EP_Read  ( uint8_t  ep_addr, 
                           uint8_t  *pbuf, 
                           uint32_t   buf_len);
uint32_t    PCD_EP_Write ( uint8_t  ep_addr, 
                           uint8_t  *pbuf, 
                           uint32_t   buf_len);
uint32_t    PCD_EP_Stall (uint8_t   epnum);
uint32_t    PCD_EP_ClrStall (uint8_t epnum);
uint32_t    PCD_EP_Flush (uint8_t epnum);
uint32_t    PCD_Handle_ISR(void);

USB_OTG_EP* PCD_GetOutEP(uint32_t ep_num) ;
USB_OTG_EP* PCD_GetInEP(uint32_t ep_num);
void        PCD_EP0_OutStart(void);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
