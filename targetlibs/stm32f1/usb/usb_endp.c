/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "usb_utils.h"
#include "usb_istr.h"
#include "usb_pwr.h"

#include "jshardware.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

uint8_t USB_Tx_State = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Handle_USBAsynchXfer.
* Description    : send data to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/
void Handle_USBAsynchXfer (void)
{  
  if(USB_Tx_State != 1)
  {
    unsigned char USB_TX_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
    int USB_Tx_length = 0;

    // try and fill the buffer
    int c;
    while (USB_Tx_length<VIRTUAL_COM_PORT_DATA_SIZE && 
           ((c = jshGetCharToTransmit(EV_USBSERIAL)) >=0) ) { // get byte to transmit
      USB_TX_Buffer[USB_Tx_length++] = c;
    }

    // if nothing, set state to 0
    if (USB_Tx_length==0) {
      USB_Tx_State = 0; 
      return;
    }

    USB_Tx_State = 1; 
    
#ifdef USE_STM3210C_EVAL
    USB_SIL_Write(EP1_IN, &USB_TX_Buffer[0], USB_Tx_length);  
#else
    UserToPMABufferCopy(&USB_TX_Buffer[0], ENDP1_TXADDR, USB_Tx_length);
    SetEPTxCount(ENDP1, USB_Tx_length);
    SetEPTxValid(ENDP1); 
#endif /* USE_STM3210C_EVAL */
  }  
  
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback (void)
{
  if (USB_Tx_State == 1)
  {
    unsigned char USB_TX_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
    int USB_Tx_length = 0;

    // try and fill the buffer
    int c;
    while (USB_Tx_length<VIRTUAL_COM_PORT_DATA_SIZE && 
           ((c = jshGetCharToTransmit(EV_USBSERIAL)) >= 0) ) { // get byte to transmit
      USB_TX_Buffer[USB_Tx_length++] = c;
    }
        
    // if nothing, set state to 0
    if (USB_Tx_length==0) {
      USB_Tx_State = 0; 
      return;
      }
        
    // else send data and keep going      
      UserToPMABufferCopy(&USB_TX_Buffer[0], ENDP1_TXADDR, USB_Tx_length);
      SetEPTxCount(ENDP1, USB_Tx_length);
      SetEPTxValid(ENDP1); 
    }
  }

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP3_OUT_Callback(void)
{
  uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
  int USB_Rx_Cnt;
  
  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = USB_SIL_Read(EP3_OUT, USB_Rx_Buffer);
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the USART Xfer */
  
  jshPushIOCharEvents(EV_USBSERIAL, USB_Rx_Buffer, USB_Rx_Cnt);
 
  /* Enable the receive of data on EP3 */
  SetEPRxStatus(ENDP3, jshHasEventSpaceForChars(VIRTUAL_COM_PORT_DATA_SIZE) ? EP_RX_VALID : EP_RX_NAK);
}


/*******************************************************************************
* Function Name  : SOF_Callback / INTR_SOFINTR_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void)
{
  static uint32_t FrameCount = 0;
  
  if(bDeviceState == CONFIGURED)
  {
    SetEPRxStatus(ENDP3, jshHasEventSpaceForChars(VIRTUAL_COM_PORT_DATA_SIZE) ? EP_RX_VALID : EP_RX_NAK);
   

    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;
      
      /* Check the data to be sent through IN pipe */
      Handle_USBAsynchXfer();
    }
  }  
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

