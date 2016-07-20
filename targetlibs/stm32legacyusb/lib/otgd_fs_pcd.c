/**
  ******************************************************************************
  * @file    otgd_fs_pcd.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Peripheral Device Interface low layer.
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


#ifdef STM32F10X_CL

#include "usb_lib.h"
#include "otgd_fs_cal.h"
#include "otgd_fs_pcd.h"

USB_OTG_PCD_DEV USB_OTG_PCD_dev;

extern USB_OTG_CORE_REGS     USB_OTG_FS_regs;
/*******************************************************************************
* Function Name  : PCD_Init
* Description    : Initialize the USB Device portion of the driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PCD_Init(void)
{
  uint32_t i = 0;
  USB_OTG_EP *ep;

  /**** SOFTWARE INIT *****/ 
  
  ep = &USB_OTG_PCD_dev.ep0;

  /* Init ep structure */
  ep->num = 0;
  ep->tx_fifo_num = 0;

  /* Control until ep is activated */
  ep->type = EP_TYPE_CTRL;
  ep->maxpacket = MAX_PACKET_SIZE;

  ep->xfer_buff = 0;
  ep->xfer_len = 0;

  for (i = 1; i < NUM_TX_FIFOS ; i++)
  {
    ep = &USB_OTG_PCD_dev.in_ep[i-1];

    /* Init ep structure */
    ep->is_in = 1;
    ep->num = i;
    ep->tx_fifo_num = i;

    /* Control until ep is activated */
    ep->type = EP_TYPE_CTRL;
    ep->maxpacket = MAX_PACKET_SIZE;
    ep->xfer_buff = 0;
    ep->xfer_len = 0;
  }

  for (i = 1; i < NUM_TX_FIFOS; i++)
  {
    ep = &USB_OTG_PCD_dev.out_ep[i-1];

    /* Init ep structure */
    ep->is_in = 0;
    ep->num = i;
    ep->tx_fifo_num = i;

    /* Control until ep is activated */
    ep->type = EP_TYPE_CTRL;
    ep->maxpacket = MAX_PACKET_SIZE;
    ep->xfer_buff = 0;
    ep->xfer_len = 0;
  }

  USB_OTG_PCD_dev.ep0.maxpacket = MAX_EP0_SIZE;
  USB_OTG_PCD_dev.ep0.type = EP_TYPE_CTRL;

  /**** HARDWARE INIT *****/
  
  /* Set the OTG_USB base registers address */
  OTGD_FS_SetAddress(USB_OTG_FS_BASE_ADDR);
  
  /* Disable all global interrupts */
  OTGD_FS_DisableGlobalInt();

  /*Init the Core */
  OTGD_FS_CoreInit();

  /* Init Device mode*/
  OTGD_FS_CoreInitDev();  
}

/*******************************************************************************
* Function Name  : PCD_EP_Open
* Description    : Configure an Endpoint
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t PCD_EP_Open(EP_DESCRIPTOR *epdesc)
{
  USB_OTG_EP *ep;


  if ((0x80 & epdesc->bEndpointAddress) != 0)
  {
    ep = PCD_GetInEP(epdesc->bEndpointAddress & 0x7F);
    ep->is_in = 1;
  }
  else
  {
    ep = PCD_GetOutEP(epdesc->bEndpointAddress & 0x7F);
    ep->is_in = 0;
  }

  ep->num   = epdesc->bEndpointAddress & 0x7F;
  ep->maxpacket = epdesc->wMaxPacketSize;
  ep->type = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;

  if (ep->is_in)
  {
    /* Assign a Tx FIFO */
    ep->tx_fifo_num = ep->num;
  }

  OTGD_FS_EPActivate(ep );

  return 0;
}

/*******************************************************************************
* Function Name  : PCD_EP_Close
* Description    : Called when an EP is disabled
* Input          : Endpoint address.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t PCD_EP_Close(uint8_t  ep_addr)
{

  USB_OTG_EP *ep;

  if ((0x80 & ep_addr) != 0)
  {
    ep = PCD_GetInEP(ep_addr & 0x7F);
  }
  else
  {
    ep = PCD_GetOutEP(ep_addr & 0x7F);
  }

  ep->num   = ep_addr & 0x7F;
  ep->is_in = (0x80 & ep_addr) != 0;

  OTGD_FS_EPDeactivate(ep );
  return 0;
}

/*******************************************************************************
* Function Name  : PCD_EP_Read
* Description    : Read data from Fifo
* Input          : Endpoint address.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t PCD_EP_Read (uint8_t ep_addr, uint8_t *pbuf, uint32_t buf_len)
{
  USB_OTG_EP *ep;
  uint32_t i = 0;

  ep = PCD_GetOutEP(ep_addr & 0x7F);

  /* copy received data into application buffer */
  for (i = 0 ; i < buf_len ; i++)
  {
    pbuf[i] = ep->xfer_buff[i];
  }

  /*setup and start the Xfer */
  ep->xfer_buff = pbuf;
  ep->xfer_len = buf_len;
  ep->xfer_count = 0;
  ep->is_in = 0;
  ep->num = ep_addr & 0x7F;

  if ( ep->num == 0 )
  {
    OTGD_FS_EP0StartXfer(ep);
  }
  else
  {
    OTGD_FS_EPStartXfer( ep );
  }

  return 0;
}

/*******************************************************************************
* Function Name  : USBF_EP_Write
* Description    : Read data from Fifo
* Input          : ep
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t  PCD_EP_Write (uint8_t ep_addr, uint8_t *pbuf, uint32_t buf_len)
{
  USB_OTG_EP *ep;

  ep = PCD_GetInEP(ep_addr & 0x7f);

  /* assign data to EP structure buffer */
  ep->xfer_buff = pbuf;

  /* Setup and start the Transfer */
  ep->xfer_count = 0;
  ep->xfer_len = buf_len;
  ep->is_in = 1;
  ep->num = ep_addr & 0x7F;
  
  if ( ep->num == 0 )
  {
    OTGD_FS_EP0StartXfer(ep);
  }
  else
  {
    OTGD_FS_EPStartXfer( ep );
  }

  return 0;
}

/*******************************************************************************
* Function Name  : PCD_EP_Stall
* Description    : Stall an endpoint.
* Input          : Endpoint Address.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t  PCD_EP_Stall (uint8_t ep_addr)
{
  USB_OTG_EP *ep;

  if ((0x80 & ep_addr) != 0)
  {
    ep = PCD_GetInEP(ep_addr & 0x7F);
  }
  else
  {
    ep = PCD_GetOutEP(ep_addr & 0x7F);
  }

  ep->num   = ep_addr & 0x7F;
  ep->is_in = ((ep_addr & 0x80) == 0x80) ? 1 : 0;

  OTGD_FS_EPSetStall(ep);
  return (0);
}
/*******************************************************************************
* Function Name  : PCD_EP_ClrStall
* Description    : Clear stall condition on endpoints.
* Input          : Endpoint Address.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t  PCD_EP_ClrStall (uint8_t ep_addr)
{

  USB_OTG_EP *ep;

  if ((0x80 & ep_addr) != 0)
  {
    ep = PCD_GetInEP(ep_addr & 0x7F);
  }
  else
  {
    ep = PCD_GetOutEP(ep_addr & 0x7F);
  }

  ep->num   = ep_addr & 0x7F;
  ep->is_in = ((ep_addr & 0x80) == 0x80) ? 1 : 0;

  OTGD_FS_EPClearStall(ep);

  return (0);
}

/*******************************************************************************
* Function Name  : USBF_FCD_EP_Flush()
* Description    : This Function flushes the buffer.
* Input          : Endpoint Address.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t  PCD_EP_Flush (uint8_t ep_addr)
{

  uint8_t  is_out = 0;
  uint8_t  ep_nbr = 0;

  ep_nbr   = ep_addr & 0x7F;
  is_out = ((ep_addr & 0x80) == 0x80) ? 0 : 1;

  if (is_out == 0)
  {
    OTGD_FS_FlushTxFifo(ep_nbr);
  }
  else
  {
    OTGD_FS_FlushRxFifo();
  }
  PCD_EP_ClrStall(ep_addr);
  return (0);
}

/*******************************************************************************
* Function Name  : PCD_EP_SetAddress
* Description    : This Function set USB device address
* Input          : The new device Address to be set.
* Output         : None
* Return         : status
*******************************************************************************/
void  PCD_EP_SetAddress (uint8_t address)
{

  USB_OTG_DCFG_TypeDef dcfg;

  dcfg.d32 = 0;
  
  dcfg.b.devaddr = address;
  USB_OTG_MODIFY_REG32( &USB_OTG_FS_regs.DEV->DCFG, 0, dcfg.d32);
}


/*******************************************************************************
* Function Name  : PCD_GetInEP
* Description    : This function returns pointer to IN EP struct with number ep_num
* Input          : Endpoint Number.
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_EP* PCD_GetInEP(uint32_t ep_num)
{
  if (ep_num == 0)
  {
    return &USB_OTG_PCD_dev.ep0;
  }
  else
  {
    return &USB_OTG_PCD_dev.in_ep[ep_num - 1];
  }
}
/*******************************************************************************
* Function Name  : PCD_GetOutEP
* Description    : returns pointer to OUT EP struct with number ep_num
* Input          : Endpoint Number.
* Output         : None
* Return         : USBF_EP
*******************************************************************************/
USB_OTG_EP* PCD_GetOutEP(uint32_t ep_num)
{
  if (ep_num == 0)
  {
    return &USB_OTG_PCD_dev.ep0;
  }
  else
  {
    return &USB_OTG_PCD_dev.out_ep[ep_num - 1];
  }
}

/*******************************************************************************
* Function Name  : PCD_DevConnect
* Description    : Connect device
* Input         : None
* Output         : None
* Return         : status
*******************************************************************************/
void  PCD_DevConnect(void)
{

  USB_OTG_DCTL_TypeDef dctl;
  
  dctl.d32 = 0;

  dctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DCTL);

  /* Connect device */
  dctl.b.sftdiscon  = 0;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DCTL, dctl.d32);
  mDELAY(25);
}

/*******************************************************************************
* Function Name  : PCD_DevDisconnect
* Description    : Disconnect device
* Input         : None
* Output         : None
* Return         : status
*******************************************************************************/
void  PCD_DevDisconnect (void)
{

  USB_OTG_DCTL_TypeDef dctl;

  dctl.d32 = 0;
  
  dctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DCTL);

  /* Disconnect device for 20ms */
  dctl.b.sftdiscon  = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DCTL, dctl.d32);
  mDELAY(25);
}

/*******************************************************************************
* Function Name  : PCD_EP0_OutStart
* Description    : Configures EPO to receive SETUP packets.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PCD_EP0_OutStart(void)
{

  USB_OTG_DOEPTSIZ0_TypeDef doeptsize0;
  doeptsize0.d32 = 0;
  
  
  doeptsize0.b.supcnt = 3;
  doeptsize0.b.pktcnt = 1;
  doeptsize0.b.xfersize = 8 * 3;

  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DOUTEPS[0]->DOEPTSIZx, doeptsize0.d32 );

}

#endif /* STM32F10X_CL */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

