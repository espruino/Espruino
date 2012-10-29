/**
  ******************************************************************************
  * @file    otgd_fs_int.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Endpoint interrupt's service routines.
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
#include "usb_utils.h"
#include "usb_type.h"
#include "otgd_fs_int.h"
#include "usb_lib.h"
#include "usb_istr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint8_t USBD_Data_Buffer  [RX_FIFO_SIZE];
__IO uint8_t IsocBuff [(ISOC_BUFFER_SZE * NUM_SUB_BUFFERS)];
__IO uint32_t IsocBufferIdx = 0;

extern USB_OTG_CORE_REGS  USB_OTG_FS_regs;

__IO uint16_t SaveRState;
__IO uint16_t SaveTState;

/* Extern variables ----------------------------------------------------------*/
extern void (*pEpInt_IN[7])(void);    /*  Handles IN  interrupts   */
extern void (*pEpInt_OUT[7])(void);   /*  Handles OUT interrupts   */

/* Private function prototypes -----------------------------------------------*/
static uint32_t PCD_ReadDevInEP( USB_OTG_EP *ep);
static uint32_t PCD_WriteEmptyTxFifo(uint32_t epnum);

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_Sof_ISR
* Description    : Handles the Start Of Frame detected interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_Sof_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef GINTSTS ;
  GINTSTS.d32 = 0;
  
  /* Call user function */
  INTR_SOFINTR_Callback();
    
  /* Clear interrupt */
  GINTSTS.b.sofintr = 1;
  USB_OTG_WRITE_REG32 (&USB_OTG_FS_regs.GREGS->GINTSTS, GINTSTS.d32);

  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_RxStatusQueueLevel_ISR
* Description    : Handles the Rx Status Queue Level Interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_RxStatusQueueLevel_ISR(void)
{
  USB_OTG_GINTMSK_TypeDef int_mask;
  USB_OTG_GRXSTSP_TypeDef status;
  USB_OTG_EP *ep;

  int_mask.d32 = 0;
  status.d32 = 0;
  
  /* Disable the Rx Status Queue Level interrupt */
  int_mask.b.rxstsqlvl = 1;
  USB_OTG_MODIFY_REG32( &USB_OTG_FS_regs.GREGS->GINTMSK, int_mask.d32, 0);

  /* Get the Status from the top of the FIFO */
  status.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.GREGS->GRXSTSP );

  /* Get the related endpoint structure */
  ep = PCD_GetOutEP(status.b.epnum);

  switch (status.b.pktsts)
  {
    case STS_GOUT_NAK:
      break;
    case STS_DATA_UPDT:
      if (status.b.bcnt)
      {
        if (ep->type == EP_TYPE_ISOC)
        {
          /* Call user function */
          INTR_RXSTSQLVL_ISODU_Callback();         
          
          /* Copy the received buffer to the RAM */
          OTGD_FS_ReadPacket((uint8_t*)(IsocBuff + (ISOC_BUFFER_SZE * IsocBufferIdx)), status.b.bcnt);
          ep->xfer_buff = (uint8_t*)(IsocBuff + (ISOC_BUFFER_SZE * IsocBufferIdx));  
          
          /* Check if the end of the global buffer has been reached */
          if (IsocBufferIdx == (NUM_SUB_BUFFERS - 1))
          {
            /* Reset the buffer index */
            IsocBufferIdx = 0;                         
          }
          else
          {
            /* Increment the buffer index */
            IsocBufferIdx ++;
          }          
        }
        else
        {
          /* Copy the received buffer to the RAM */
          OTGD_FS_ReadPacket(USBD_Data_Buffer, status.b.bcnt);
          ep->xfer_buff = USBD_Data_Buffer;
        }
        
        /* Update the endpoint structure */
        ep->xfer_len  = status.b.bcnt;
        ep->xfer_count += status.b.bcnt;        
      }
      else
      {
        ep->xfer_len  = status.b.bcnt;
      }
      break;
    case STS_XFER_COMP:
      break;
    case STS_SETUP_COMP:
      break;
    case STS_SETUP_UPDT:
      /* Copy the setup packet received in Fifo into the setup buffer in RAM */
      OTGD_FS_ReadPacket(USBD_Data_Buffer, 8); 
      ep->xfer_buff = USBD_Data_Buffer;
      ep->xfer_count += status.b.bcnt;
      ep->xfer_len  = status.b.bcnt;
      break;
    default:
      break;
  }

  /* Call the user function */
  INTR_RXSTSQLVL_Callback();
  
  /* Enable the Rx Status Queue Level interrupt */
  USB_OTG_MODIFY_REG32( &USB_OTG_FS_regs.GREGS->GINTMSK, 0, int_mask.d32);
  
  /* Clear interrupt: this is a read only bit, it cannot be cleared by register 
     access */

  return 1;
}
/*******************************************************************************
* Function Name  : OTGD_FS_Handle_GInNakEff_ISR
* Description    : Handles the Global IN Endpoints NAK Effective interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_GInNakEff_ISR(void)
{
 
  /* Call user function */
  INTR_GINNAKEFF_Callback();
  
  /* Clear interrupt: This is a read only bit, it cannot be cleared by register 
     access */
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_GOutNakEff_ISR
* Description    : Handles the Global OUT Endpoints NAK Effective interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_GOutNakEff_ISR(void)
{
  /* Call user function */
  INTR_GOUTNAKEFF_Callback();  
  
  /* Clear interrupt: This is a read only bit, it cannot be cleared by register 
     access */

  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_EarlySuspend_ISR
* Description    : Handles the Early Suspend detected interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_EarlySuspend_ISR(void )
{
  USB_OTG_GINTSTS_TypeDef gintsts;
  USB_OTG_GINTMSK_TypeDef gintmsk;

  gintsts.d32 = 0;
  gintmsk.d32 = 0;
  
  
  /* Call user function */
  INTR_ERLYSUSPEND_Callback();  
  
  gintmsk.b.erlysuspend = 1;
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.GREGS->GINTMSK, gintmsk.d32, 0 );

  /* Clear interrupt */
  gintsts.b.erlysuspend = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_USBSuspend_ISR
* Description    : Handles the Suspend condition detected interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_USBSuspend_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  
  
  gintsts.d32 = 0;
  /* Call user function */
  INTR_USBSUSPEND_Callback();
  
  /* Clear interrupt */
  gintsts.b.usbsuspend = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_UsbReset_ISR
* Description    : This interrupt occurs when a USB Reset is detected.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_UsbReset_ISR(void)
{
  USB_OTG_DAINT_TypeDef daintmsk;
  USB_OTG_DOEPMSKx_TypeDef doepmsk;
  USB_OTG_DIEPMSKx_TypeDef diepmsk;
  USB_OTG_DCFG_TypeDef dcfg;
  USB_OTG_DCTL_TypeDef dctl;
  USB_OTG_GINTSTS_TypeDef gintsts;
  uint32_t i = 0;
  
  daintmsk.d32 = 0;
  doepmsk.d32 = 0;
  diepmsk.d32 = 0;
  dcfg.d32 =0;
  dctl.d32 = 0;
  gintsts.d32 = 0;

  /* Clear the Remote Wakeup Signalling */
  dctl.b.rmtwkupsig = 1;
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DCTL, dctl.d32, 0 );

  /* Flush the NP Tx FIFO */
  OTGD_FS_FlushTxFifo( 0 );
  
  /* clear pending interrupts */
  for (i = 0; i < NUM_TX_FIFOS ; i++)
  {
    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DINEPS[i]->DIEPINTx, 0xFF);
    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DOUTEPS[i]->DOEPINTx, 0xFF);
  }
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DAINT, 0xFFFFFFFF );  

  daintmsk.ep.in = 1;
  daintmsk.ep.out = 1;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DAINTMSK, daintmsk.d32 );

  doepmsk.b.setup = 1;
  doepmsk.b.b2bsetup = 1;
  doepmsk.b.xfercompl = 1;
  doepmsk.b.epdis = 1;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DOEPMSK, doepmsk.d32 );

  diepmsk.b.xfercompl = 1;
  diepmsk.b.timeout = 1;
  diepmsk.b.epdis = 1;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DIEPMSK, diepmsk.d32 );

  /* Reset Device Address */
  dcfg.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.DEV->DCFG);
  dcfg.b.devaddr = 0;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DCFG, dcfg.d32);

  /* setup EP0 to receive SETUP packets */
  PCD_EP0_OutStart();

  /* Clear interrupt */
  gintsts.d32 = 0;
  gintsts.b.usbreset = 1;
  USB_OTG_WRITE_REG32 (&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);

  /* Call the user reset function */
  OTGD_FS_DEVICE_RESET; 
  
  /* Call user function */
  INTR_USBRESET_Callback();  
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_EnumDone_ISR
* Description    : Reads the device status register and set the device speed
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_EnumDone_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;
  USB_OTG_GUSBCFG_TypeDef gusbcfg;

  gintsts.d32 = 0;
  gusbcfg.d32 = 0;
  
  OTGD_FS_EP0Activate();

  /* Set USB turnaround time */
  gusbcfg.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GUSBCFG);
  gusbcfg.b.usbtrdtim = 9;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GUSBCFG, gusbcfg.d32);

  /* Call user function */
  INTR_ENUMDONE_Callback();
  
  /* Clear interrupt */
  gintsts.b.enumdone = 1;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32 );
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_IsoOutDrop_ISR
* Description    : Handles the Isochronous Out packet Dropped interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_IsoOutDrop_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  

  gintsts.d32 = 0;
  /* Call user function */
  INTR_ISOOUTDROP_Callback();
  
  /* Clear interrupt */
  gintsts.b.isooutdrop = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_EOPF_ISR
* Description    : Handles the Expected End Of Periodic Frame interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_EOPF_ISR(void )
{
  USB_OTG_GINTSTS_TypeDef gintsts;
  USB_OTG_GINTMSK_TypeDef gintmsk;
  
  gintsts.d32 = 0;
  gintmsk.d32 = 0;
  
  gintmsk.b.eopframe = 1;
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.GREGS->GINTMSK, gintmsk.d32, 0 );

  /* Call user function */
  INTR_EOPFRAME_Callback();
  
  /* Clear interrupt */
  gintsts.b.eopframe = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  return 1;
}
/*******************************************************************************
* Function Name  : OTGD_FS_Handle_InEP_ISR
* Description    : Handles all IN endpoints interrupts.
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_InEP_ISR(void)
{
  USB_OTG_DIEPINTx_TypeDef diepint;

  uint32_t ep_intr = 0;
  uint32_t epnum = 0;
  USB_OTG_EP *ep;
  uint32_t fifoemptymsk = 0;

  diepint.d32 = 0;  
  ep_intr = OTGD_FS_ReadDevAllInEPItr();
  while ( ep_intr )
  {
    if (ep_intr&0x1) /* In ITR */
    {
      ep = PCD_GetInEP(epnum);
      diepint.d32 = PCD_ReadDevInEP(ep); /* Get In ITR status */
      if ( diepint.b.xfercompl )
      {
        fifoemptymsk = 0x1 << ep->num;
        USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DIEPEMPMSK, fifoemptymsk, 0);

        /* Clear the Interrupt flag */ 
        CLEAR_IN_EP_INTR(epnum, xfercompl);
        
        if (epnum == 0)  
        {        
          /* Call the core IN process for EP0 */ 
          In0_Process();
          
          /* before terminate set Tx & Rx status */
          OTG_DEV_SetEPRxStatus(epnum, SaveRState);
          OTG_DEV_SetEPTxStatus(epnum, SaveTState);
        }
        else
        {
          /* Call the relative IN endpoint callback */
          (*pEpInt_IN[epnum -1])();
        } 
      }
      if ( diepint.b.timeout )
      {
        CLEAR_IN_EP_INTR(epnum, timeout);
      }
      if (diepint.b.intktxfemp)
      {
        CLEAR_IN_EP_INTR(epnum, intktxfemp);
      }
      if (diepint.b.inepnakeff)
      {
        CLEAR_IN_EP_INTR(epnum, inepnakeff);
      }
      if (diepint.b.txfempty)
      {      
         if ((epnum == 0) || (OTG_DEV_GetEPTxStatus(epnum) == DEV_EP_TX_VALID))
        {
          PCD_WriteEmptyTxFifo(epnum);          
        }

        CLEAR_IN_EP_INTR(epnum, txfempty);          
      }
      if ( diepint.b.epdis)
      { 
        /* Reset Endpoint Frame ID to 0 */
        ep->even_odd_frame = 0;

        CLEAR_IN_EP_INTR(epnum, epdis);
      }      
    }
    epnum++;
    ep_intr >>= 1;
  }

  /* Call user function */
  INTR_INEPINTR_Callback();
  
  return 1;
}


/*******************************************************************************
* Function Name  : OTGD_FS_Handle_OutEP_ISR
* Description    : Handles all OUT endpoints interrupts.
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
uint32_t OTGD_FS_Handle_OutEP_ISR(void)
{
  uint32_t ep_intr = 0;
  USB_OTG_DOEPINTx_TypeDef doepint;
  uint32_t epnum = 0;
  USB_OTG_EP *ep;
  
  doepint.d32 = 0;

  /* Read in the device interrupt bits */
  ep_intr = OTGD_FS_ReadDevAllOutEp_itr();
  
  while ( ep_intr )
  {
    if (ep_intr&0x1)
    {
      /* Get EP pointer */
      ep = PCD_GetOutEP(epnum);
      doepint.d32 = OTGD_FS_ReadDevOutEP_itr(ep);

      /* Transfer complete */
      if ( doepint.b.xfercompl )
      {
        /* Clear the bit in DOEPINTn for this interrupt */
        CLEAR_OUT_EP_INTR(epnum, xfercompl);
        
        if (epnum == 0)  
        { 
          /* Call the OUT process for the EP0 */
          Out0_Process();
        }
        else
        {
          (*pEpInt_OUT[epnum-1])();
        }
      }
      /* Endpoint disable  */
      if ( doepint.b.epdis)
      {
        /* Clear the bit in DOEPINTn for this interrupt */
        CLEAR_OUT_EP_INTR(epnum, epdis);
      }
      /* Setup Phase Done (control EPs) */
      if ( doepint.b.setup )
      {
        if (epnum == 0)  
        {        
          /* Call the SETUP process for the EP0 */
          Setup0_Process();  

          /* Before exit, update the Tx status */
          OTG_DEV_SetEPTxStatus(0x80, SaveTState); 
        }
        else
        {
          /* Other control endpoints */
        }  
        
        /* Clear the EP Interrupt */
        CLEAR_OUT_EP_INTR(epnum, setup);
      }
      /* Back to back setup received */
      if ( doepint.b.b2bsetup )
      {
        if (epnum == 0)  
        {        
          /* Call the SETUP process for the EP0 */
          Setup0_Process();  

          /* Before exit, update the Tx status */
          OTG_DEV_SetEPTxStatus(0x80, SaveTState);  
        }
      }
    }
    epnum++;
    ep_intr >>= 1;
  }

  /* Call user function */
  INTR_OUTEPINTR_Callback();  
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_IncomplIsoIn_ISR
* Description    : Handles the Incomplete Isochronous IN transfer error interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_IncomplIsoIn_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  
  
  gintsts.d32 = 0;

  /* Call user function */
  INTR_INCOMPLISOIN_Callback(); 
  
  /* Clear interrupt */
  gintsts.b.incomplisoin = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_IncomplIsoOut_ISR
* Description    : Handles the Incomplete Isochronous OUT transfer error interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_IncomplIsoOut_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  

  gintsts.d32 = 0;
  
  /* Call user function */
  INTR_INCOMPLISOOUT_Callback();
  
  /* Clear interrupt */
  gintsts.b.outepintr = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);
  
  return 1;
}

/*******************************************************************************
* Function Name  : OTGD_FS_Handle_Wakeup_ISR
* Description    : Handles the Wakeup or Remote Wakeup detected interrupt.
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
uint32_t OTGD_FS_Handle_Wakeup_ISR(void)
{
  USB_OTG_GINTSTS_TypeDef gintsts;

  gintsts.d32 = 0;
  /* Call user function */
  INTR_WKUPINTR_Callback();
  
  /* Clear interrupt */
  gintsts.b.wkupintr = 1;
  USB_OTG_WRITE_REG32 (&USB_OTG_FS_regs.GREGS->GINTSTS, gintsts.d32);

  return 1;
}
/*******************************************************************************
* Function Name  : PCD_ReadDevInEP
* Description    : Reads all the Endpoints flags.
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
static uint32_t PCD_ReadDevInEP( USB_OTG_EP *ep)
{
  uint32_t v = 0, msk = 0, emp=0;
  
  msk = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DIEPMSK);
  emp = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DIEPEMPMSK);
  msk |= ((emp >> ep->num) & 0x1) << 7;
  v = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DINEPS[ep->num]->DIEPINTx) & msk;
  
  return v;
}

/*******************************************************************************
* Function Name  : PCD_WriteEmptyTxFifo
* Description    : Checks Fifo for the next packet to be loaded.
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
static uint32_t PCD_WriteEmptyTxFifo(uint32_t epnum)
{
  USB_OTG_DTXFSTS_TypeDef txstatus;
  USB_OTG_EP *ep;
  uint32_t len = 0;
  uint32_t dwords = 0;
  uint32_t fifoemptymsk = 0;
  
  txstatus.d32 = 0;
  
  ep = PCD_GetInEP(epnum); 
  
  len = ep->xfer_len - ep->xfer_count;

  if (len > ep->maxpacket)
  {
    len = ep->maxpacket;
  }
  
  dwords = (len + 3) / 4;
  txstatus.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.DINEPS[epnum]->DTXFSTSx);

  
  while  ((txstatus.b.txfspcavail > dwords) &&
          (ep->xfer_count < ep->xfer_len) &&
          (ep->xfer_len) != 0)
  {
    len = ep->xfer_len - ep->xfer_count;

    if (len > ep->maxpacket)
    {
      len = ep->maxpacket;
    }
    dwords = (len + 3) / 4;

    OTGD_FS_WritePacket(ep->xfer_buff, epnum, len);    
    
    ep->xfer_count += len;
    ep->xfer_buff += len; 

    txstatus.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DINEPS[epnum]->DTXFSTSx); 
    
    /* Mask the TxFIFOEmpty interrupt to prevent re-entring this routine */
    if (ep->xfer_len == ep->xfer_count)
    {
      fifoemptymsk = 0x1 << ep->num;
      USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DIEPEMPMSK, fifoemptymsk, 0);    
    }
  }
  
  return 1;
}
#endif  /* STM32F10X_CL */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
