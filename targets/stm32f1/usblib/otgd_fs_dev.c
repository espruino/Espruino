/**
  ******************************************************************************
  * @file    otgd_fs_dev.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   High Layer device mode interface and wrapping layer.
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

/* Includes ------------------------------------------------------------------*/
#include "otgd_fs_dev.h"
#include "usb_regs.h"
#include "otgd_fs_cal.h"
#include "otgd_fs_pcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : OTG_DEV_Init
* Description    : Initialize the OTG Device IP and EP0.
* Input          : None.
* Output         : None.
* Return         : None. 
*******************************************************************************/
void OTG_DEV_Init(void)
{
  EP_DESCRIPTOR ep_descriptor;
  
  /* Init peripheral driver */
  PCD_Init();
  
  /* Configure and open the IN control EP0 */ 
  ep_descriptor.bEndpointAddress = 0x80;
  ep_descriptor.wMaxPacketSize = 64;  
  ep_descriptor.bmAttributes = USB_ENDPOINT_XFER_CONTROL; 
  PCD_EP_Open(&ep_descriptor);
  
  /* Configure and open the OUT control EP0 */ 
  ep_descriptor.bEndpointAddress = 0x00;
  PCD_EP_Open(&ep_descriptor);    

  OTGD_FS_EPStartXfer(PCD_GetOutEP(0));
  
  /* Enable EP0 to start receiving setup packets */  
  PCD_EP0_OutStart();  
  
  /* Enable USB Global interrupt */
  OTGD_FS_EnableGlobalInt();     
}


/*******************************************************************************
* Function Name  : OTG_DEV_EP_Init
* Description    : Initialize the selected endpoint parameters
* Input          : - bEpAdd: address of the endpoint (epnum|epdir) 
*                     example: EP1 OUT -> 0x01 and EP1 IN 0x81.
*                  - bEpType: OTG_DEV_EP_TYPE_CONTROL, OTG_DEV_EP_TYPE_ISOC, 
*                     OTG_DEV_EP_TYPE_BULK, OTG_DEV_EP_TYPE_INT
*                  - wEpMaxPackSize: The EP max packet size.
* Output         : None.
* Return         : Status: New status to be set for the endpoint: 
*******************************************************************************/
void OTG_DEV_EP_Init(uint8_t bEpAdd, uint8_t bEpType, uint16_t wEpMaxPackSize)
{
  EP_DESCRIPTOR ep_descriptor;
  USB_OTG_EP *ep;
  
  /* Set the EP parameters in a structure */
  ep_descriptor.bEndpointAddress = bEpAdd;
  ep_descriptor.bmAttributes = bEpType; 
  ep_descriptor.wMaxPacketSize = wEpMaxPackSize;

  PCD_EP_Flush(bEpAdd);
  
  /* Open the EP with entered parameters */   
  PCD_EP_Open(&ep_descriptor); 
  
  /* Activate the EP if it is an OUT EP */
  if ((bEpAdd & 0x80) == 0)
  {
    ep = PCD_GetOutEP(bEpAdd & 0x7F);
    OTGD_FS_EPStartXfer(ep);
  } 
  else
  {
    ep = PCD_GetInEP(bEpAdd & 0x7F);
    ep->even_odd_frame = 0;    
    OTG_DEV_SetEPTxStatus(bEpAdd, DEV_EP_TX_NAK);
  }
  
}

/*******************************************************************************
* Function Name  : OTG_DEV_GetEPTxStatus
* Description    : Set the related endpoint status.
* Input          : Number of the endpoint.
* Output         : None.
* Return         : Status: New status to be set for the endpoint: 
*******************************************************************************/
uint32_t OTG_DEV_GetEPTxStatus(uint8_t bEpnum) 
{
  USB_OTG_EP *ep;
  uint32_t status = 0;
  
  ep = PCD_GetInEP(bEpnum & 0x7F); 
  
  status = OTGD_FS_GetEPStatus(ep); 
  
  return status; 
}

/*******************************************************************************
* Function Name  : OTG_DEV_GetEPRxStatus
* Description    : returns the related endpoint status.
* Input          : Number of the endpoint.
* Output         : None.
* Return         : Status: New status to be set for the endpoint: 
*******************************************************************************/
uint32_t OTG_DEV_GetEPRxStatus(uint8_t bEpnum)
{
  USB_OTG_EP *ep;
  uint32_t status = 0;
  
  ep = PCD_GetOutEP(bEpnum & 0x7F); 
  
  status = OTGD_FS_GetEPStatus(ep); 
  
  return status;
}

/*******************************************************************************
* Function Name  : OTG_DEV_SetEPTxStatus
* Description    : Sets the related endpoint status.
* Input          : - bEpnum: Number of the endpoint.
*                  - Status: New status to be set for the endpoint. It can be
*                    DEV_EP_TX_VALID, DEV_EP_TX_STALL, DEV_EP_TX_NAK or 
*                    DEV_EP_TX_DISABLE.
* Output         : None.
* Return         : None.
*******************************************************************************/
void OTG_DEV_SetEPTxStatus(uint8_t bEpnum, uint32_t Status) 
{
  USB_OTG_EP *ep;
   
  ep = PCD_GetInEP(bEpnum & 0x7F); 
  
  if ((bEpnum == 0x80) && (Status == DEV_EP_TX_STALL))
  {
    ep->is_in = 1;
  }
  
  OTGD_FS_SetEPStatus(ep, Status); 
}

/*******************************************************************************
* Function Name  : OTG_DEV_SetEPRxStatus
* Description    : Sets the related endpoint status.
* Input          : - bEpnum: Number of the endpoint.
*                  - Status: New status to be set for the endpoint. It can be
*                    DEV_EP_RX_VALID, DEV_EP_RX_STALL, DEV_EP_RX_NAK or 
*                    DEV_EP_RX_DISABLE.
* Output         : None.
* Return         : None.
*******************************************************************************/
void OTG_DEV_SetEPRxStatus(uint8_t bEpnum, uint32_t Status)                           
{
  USB_OTG_EP *ep;
 
  ep = PCD_GetOutEP(bEpnum & 0x7F); 
  
  OTGD_FS_SetEPStatus(ep, Status); 
}

/*******************************************************************************
* Function Name  : USB_DevDisconnect
* Description    : Disconnect the Pull-up resist.
* Input          : bEpNum: Endpoint Number. 
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_DevDisconnect(void)
{
  PCD_DevDisconnect();
}

/*******************************************************************************
* Function Name  : USB_DevConnect
* Description    : Disconnect the .
* Input          : bEpNum: Endpoint Number. 
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_DevConnect(void)
{
  PCD_DevConnect();
}

/*-*-*-*-*-*-*-*-*-* Replace the usb_regs.h defines -*-*-*-*-*-*-*-*-*-*-*-*-*/

/*******************************************************************************
* Function Name  : SetEPTxStatus
* Description    : Set the status of Tx endpoint.
* Input          : bEpNum: Endpoint Number. 
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPTxStatus(uint8_t bEpNum, uint16_t wState)
{
  _SetEPTxStatus(bEpNum, wState);
}

/*******************************************************************************
* Function Name  : SetEPRxStatus
* Description    : Set the status of Rx endpoint.
* Input          : bEpNum: Endpoint Number. 
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPRxStatus(uint8_t bEpNum, uint16_t wState)
{
  _SetEPRxStatus(bEpNum, wState);
}

/*******************************************************************************
* Function Name  : GetEPTxStatus
* Description    : Returns the endpoint Tx status.
* Input          : bEpNum: Endpoint Number. 
* Output         : None.
* Return         : Endpoint TX Status
*******************************************************************************/
uint16_t GetEPTxStatus(uint8_t bEpNum) 
{
  return(_GetEPTxStatus(bEpNum));
}

/*******************************************************************************
* Function Name  : GetEPRxStatus
* Description    : Returns the endpoint Rx status.
* Input          : bEpNum: Endpoint Number. 
* Output         : None.
* Return         : Endpoint RX Status
*******************************************************************************/
uint16_t GetEPRxStatus(uint8_t bEpNum) 
{
  return(_GetEPRxStatus(bEpNum));
}

/*******************************************************************************
* Function Name  : SetEPTxValid
* Description    : Valid the endpoint Tx Status.
* Input          : bEpNum: Endpoint Number.  
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPTxValid(uint8_t bEpNum)
{
  _SetEPTxStatus(bEpNum, EP_TX_VALID);
}

/*******************************************************************************
* Function Name  : SetEPRxValid
* Description    : Valid the endpoint Rx Status.
* Input          : bEpNum: Endpoint Number. 
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPRxValid(uint8_t bEpNum)
{
  _SetEPRxStatus(bEpNum, EP_RX_VALID);
}

/*******************************************************************************
* Function Name  : GetTxStallStatus
* Description    : Returns the Stall status of the Tx endpoint.
* Input          : bEpNum: Endpoint Number. 
* Output         : None.
* Return         : Tx Stall status.
*******************************************************************************/
uint16_t GetTxStallStatus(uint8_t bEpNum)
{
  return(_GetTxStallStatus(bEpNum));
}

/*******************************************************************************
* Function Name  : GetRxStallStatus
* Description    : Returns the Stall status of the Rx endpoint. 
* Input          : bEpNum: Endpoint Number. 
* Output         : None.
* Return         : Rx Stall status.
*******************************************************************************/
uint16_t GetRxStallStatus(uint8_t bEpNum)
{
  return(_GetRxStallStatus(bEpNum));
}

/*******************************************************************************
* Function Name  : SetEPTxCount.
* Description    : Set the Tx count.
* Input          : bEpNum: Endpoint Number.
*                  wCount: new count value.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPTxCount(uint8_t bEpNum, uint16_t wCount)
{
}

/*******************************************************************************
* Function Name  : SetEPRxCount
* Description    : Set the Rx count.
* Input          : bEpNum: Endpoint Number. 
*                  wCount: the new count value.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPRxCount(uint8_t bEpNum, uint16_t wCount)
{ 
}

/*******************************************************************************
* Function Name  : ToWord
* Description    : merge two byte in a word.
* Input          : bh: byte high, bl: bytes low.
* Output         : None.
* Return         : resulted word.
*******************************************************************************/
uint16_t ToWord(uint8_t bh, uint8_t bl)
{
  uint16_t wRet = 0;
  wRet = (uint16_t)bl | ((uint16_t)bh << 8);
  return(wRet);
}

/*******************************************************************************
* Function Name  : ByteSwap
* Description    : Swap two byte in a word.
* Input          : wSwW: word to Swap.
* Output         : None.
* Return         : resulted word.
*******************************************************************************/
uint16_t ByteSwap(uint16_t wSwW)
{
  uint8_t bTemp = 0;
  uint16_t wRet = 0;
  
  bTemp = (uint8_t)(wSwW & 0xff);
  wRet =  (wSwW >> 8) | ((uint16_t)bTemp << 8);
  return(wRet);
}

#endif /* STM32F10X_CL */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
