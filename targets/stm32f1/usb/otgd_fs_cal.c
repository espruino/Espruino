/**
  ******************************************************************************
  * @file    otgd_fs_cal.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   OTG FS Device Core Access Layer interface.
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
#include "otgd_fs_cal.h"
#include "usb_conf.h"
#include "otgd_fs_regs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USB_OTG_CORE_REGS     USB_OTG_FS_regs;

/* Private function prototypes -----------------------------------------------*/
static USB_OTG_Status OTGD_FS_SetDeviceMode(void);
static USB_OTG_Status OTGD_FS_CoreReset(void);

extern uint32_t STM32_PCD_OTG_ISR_Handler (void);

/******************************************************************************/
/*                           Common Core Layer                                */
/******************************************************************************/

/*******************************************************************************
* Function Name  : OTGD_FS_WritePacket
* Description    : Writes a packet into the Tx FIFO associated with the EP
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_WritePacket(uint8_t *src, uint8_t ep_num, uint16_t bytes)
{
  USB_OTG_Status status = USB_OTG_OK;
  uint32_t dword_count = 0 , i = 0;
  __IO uint32_t *fifo;

  /* Find the DWORD length, padded by extra bytes as necessary if MPS
   * is not a multiple of DWORD */
  dword_count =  (bytes + 3) / 4;

  fifo = USB_OTG_FS_regs.FIFO[ep_num];

  for (i = 0; i < dword_count; i++, src += 4)
  {
    USB_OTG_WRITE_REG32( fifo, *((__packed uint32_t *)src) );
  }

  return status;
}
/*******************************************************************************
* Function Name  : OTGD_FS_ReadPacket
* Description    : Reads a packet from the Rx FIFO
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
void* OTGD_FS_ReadPacket(uint8_t *dest, uint16_t bytes)
{
  uint32_t i = 0;
  uint32_t word_count = (bytes + 3) / 4;

  __IO uint32_t *fifo = USB_OTG_FS_regs.FIFO[0];
  uint32_t *data_buff = (uint32_t *)dest;

  for (i = 0; i < word_count; i++, data_buff++)
  {
    *data_buff = USB_OTG_READ_REG32(fifo);
  }

  /* Return the buffer pointer because if the transfer is composed of several packets,
     the data of the next packet must be stored following the previous packet's data         */
  return ((void *)data_buff);
}

/*******************************************************************************
* Function Name  : OTGD_FS_SetAddress
* Description    : Initialize core registers addresses.
* Input          : BaseAddress
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_SetAddress(uint32_t BaseAddress)
{
  uint32_t i = 0;
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_FS_regs.GREGS = (USB_OTG_GREGS *)(BaseAddress +\
                           USB_OTG_CORE_GLOBAL_REGS_OFFSET);

  USB_OTG_FS_regs.DEV    =  (USB_OTG_DEV  *)  (BaseAddress +\
                           USB_OTG_DEV_GLOBAL_REG_OFFSET);

  for (i = 0; i < NUM_TX_FIFOS; i++)
  {
    USB_OTG_FS_regs.DINEPS[i]  = (USB_OTG_DINEPS *)  (BaseAddress + \
                  USB_OTG_DEV_IN_EP_REG_OFFSET + (i * USB_OTG_EP_REG_OFFSET));
    
    USB_OTG_FS_regs.DOUTEPS[i] = (USB_OTG_DOUTEPS *) (BaseAddress + \
                 USB_OTG_DEV_OUT_EP_REG_OFFSET + (i * USB_OTG_EP_REG_OFFSET));
  }

  for (i = 0; i < NUM_TX_FIFOS; i++)
  {
    USB_OTG_FS_regs.FIFO[i] = (uint32_t *)(BaseAddress + \
                    USB_OTG_DATA_FIFO_OFFSET + (i * USB_OTG_DATA_FIFO_SIZE));
  }

  USB_OTG_FS_regs.PCGCCTL = (uint32_t *)(BaseAddress + USB_OTG_PCGCCTL_OFFSET);

  return status;
}
/*******************************************************************************
* Function Name  : OTGD_FS_CoreInit
* Description    : Initialize the USB_OTG controller registers and prepares the core
                   for device mode or host mode operation.
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_CoreInit(void)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_GUSBCFG_TypeDef usbcfg;
  USB_OTG_GCCFG_TypeDef    gccfg;
 
  usbcfg.d32 = 0;
  gccfg.d32  = 0;
  
  usbcfg.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GUSBCFG);
  usbcfg.b.physel = 1;
  USB_OTG_WRITE_REG32 (&USB_OTG_FS_regs.GREGS->GUSBCFG, usbcfg.d32);

  /* init and configure the phy */
  gccfg.d32 = 0;
  gccfg.b.vbussensingB = 1;
  gccfg.b.pwdn = 1;
  USB_OTG_WRITE_REG32 (&USB_OTG_FS_regs.GREGS->GCCFG, gccfg.d32);
  mDELAY(50);

  /* Reset after a PHY select and set Host mode */
  OTGD_FS_CoreReset();

  /* Set Device Mode */
  OTGD_FS_SetDeviceMode();

  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_CoreReset
* Description    : Soft reset of the core
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
static USB_OTG_Status OTGD_FS_CoreReset(void)
{
  USB_OTG_Status status = USB_OTG_OK;
  __IO USB_OTG_GRSTCTL_TypeDef greset;
  uint32_t timeout = 0;

  greset.d32 = 0;
  
  /* Wait for AHB master IDLE state. */
  do
  {
    uDELAY(5);
    greset.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GRSTCTL);
    if (++timeout > USB_OTG_TIMEOUT)
    {
      return USB_OTG_OK;
    }
  }
  while (greset.b.ahbidle == 0);

  /* Core Soft Reset */
  timeout = 0;
  greset.b.csftrst = 1;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GRSTCTL, greset.d32 );
  
  do
  {
    greset.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GRSTCTL);
    if (++timeout > USB_OTG_TIMEOUT)
    {
      break;
    }
  }
  while (greset.b.csftrst == 1);

  /* Wait for 3 PHY Clocks*/
  uDELAY(5);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_EnableGlobalInt
* Description    : Enables the controller's Global Int in the AHB Config reg
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EnableGlobalInt(void)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_GAHBCFG_TypeDef  ahbcfg;

  ahbcfg.d32 = 0;
  
  ahbcfg.b.gintmsk = 1; /* Enable interrupts */
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.GREGS->GAHBCFG, 0, ahbcfg.d32);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_DisableGlobalInt
* Description    : Disables the controller's Global Int in the AHB Config reg
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_DisableGlobalInt(void)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_GAHBCFG_TypeDef ahbcfg;

  ahbcfg.d32 = 0;
  ahbcfg.b.gintmsk = 1; /* Enable interrupts */
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.GREGS->GAHBCFG, ahbcfg.d32, 0);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_FlushTxFifo
* Description    : Flush a Tx FIFO
* Input          : FIFO num
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_FlushTxFifo (uint32_t num )
{

  USB_OTG_Status status = USB_OTG_OK;
  __IO USB_OTG_GRSTCTL_TypeDef greset;
  uint32_t timeout = 0;

  greset.d32 = 0;
    
  greset.b.txfflsh = 1;
  greset.b.txfnum  = num;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GRSTCTL, greset.d32 );

  do
  {
    greset.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.GREGS->GRSTCTL);
    if (++timeout > USB_OTG_TIMEOUT)
    {
      break;
    }
  }
  while (greset.b.txfflsh == 1);

  /* Wait for 3 PHY Clocks*/
  uDELAY(5);

  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_FlushRxFifo
* Description    : Flush a Rx FIFO
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_FlushRxFifo( void )
{
  USB_OTG_Status status = USB_OTG_OK;
  __IO USB_OTG_GRSTCTL_TypeDef greset;
  uint32_t timeout = 0;

  greset.d32 = 0;
  
  greset.b.rxfflsh = 1;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GRSTCTL, greset.d32 );

  do
  {
    greset.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.GREGS->GRSTCTL);
    if (++timeout > USB_OTG_TIMEOUT)
    {
      break;
    }
  }
  while (greset.b.rxfflsh == 1);

  /* Wait for 3 PHY Clocks*/
  uDELAY(5);

  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_SetDeviceMode
* Description    : Set device mode
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_SetDeviceMode(void)
{

  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_GUSBCFG_TypeDef usbcfg ;
  
  usbcfg.d32 = 0;

  usbcfg.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GUSBCFG);

  usbcfg.b.force_dev = 1;

  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.GREGS->GUSBCFG, usbcfg.d32);

  mDELAY(50);

  return status;
}
/*******************************************************************************
* Function Name  : IsDeviceMode
* Description    : check device mode
* Input          : None
* Output         : None
* Return         : current mode
*******************************************************************************/
uint32_t USBD_FS_IsDeviceMode(void)
{
  return ((USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS ) & 0x1) == 0 );
}

/*******************************************************************************
* Function Name  : OTGD_FS_ReadCoreItr
* Description    : returns the Core Interrupt register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t OTGD_FS_ReadCoreItr(void)
{
  uint32_t v = 0;

  v = USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GINTSTS);
  v &= USB_OTG_READ_REG32(&USB_OTG_FS_regs.GREGS->GINTMSK);

  return v;
}

/*******************************************************************************
* Function Name  : OTGD_FS_ReadOtgItr
* Description    : returns the USB_OTG Interrupt register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t OTGD_FS_ReadOtgItr (void)
{
  return (USB_OTG_READ_REG32 (&USB_OTG_FS_regs.GREGS->GOTGINT));
}

/******************************************************************************/
/*                           PCD Core Layer                                   */
/******************************************************************************/

/*******************************************************************************
* Function Name  : InitDevSpeed
* Description    : Initializes the DevSpd field of the DCFG register depending
                   on the PHY type and the enumeration speed of the device.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void InitDevSpeed(void)
{
  USB_OTG_DCFG_TypeDef  dcfg;

  dcfg.d32 = 0;
  
  dcfg.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DCFG);
  dcfg.b.devspd = 0x3;  /* Full speed PHY */
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DCFG, dcfg.d32);
}
/*******************************************************************************
* Function Name  : OTGD_FS_CoreInitDev
* Description    : Initialize the USB_OTG controller registers for device mode
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_CoreInitDev (void)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef  depctl;
  USB_OTG_DCFG_TypeDef   dcfg;
  USB_OTG_FIFOSIZ_TypeDef txfifosize0;
  USB_OTG_FIFOSIZ_TypeDef txfifosize;
  uint32_t i = 0;
  
  depctl.d32 = 0;
  dcfg.d32 = 0;
  txfifosize0.d32 = 0;
  txfifosize.d32 = 0;
  
  /* Set device speed */
  InitDevSpeed ();

  /* Restart the Phy Clock */
  USB_OTG_WRITE_REG32(USB_OTG_FS_regs.PCGCCTL, 0);

  /* Device configuration register */
  dcfg.d32 = USB_OTG_READ_REG32( &USB_OTG_FS_regs.DEV->DCFG);
  dcfg.b.perfrint = DCFG_FRAME_INTERVAL_80;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DCFG, dcfg.d32 );
  
  /* set Rx FIFO size */
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GRXFSIZ, RX_FIFO_SIZE);

  /* EP0 TX*/
  txfifosize0.b.depth     = TX0_FIFO_SIZE;
  txfifosize0.b.startaddr = RX_FIFO_SIZE;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->DIEPTXF0, txfifosize0.d32 );

  
  /* EP1 TX*/
  txfifosize.b.startaddr = txfifosize0.b.startaddr + txfifosize0.b.depth;
  txfifosize.b.depth = TX1_FIFO_SIZE;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->DIEPTXFx[0], txfifosize.d32 );

    
  /* EP2 TX*/
  txfifosize.b.startaddr += txfifosize.b.depth;
  txfifosize.b.depth = TX2_FIFO_SIZE;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->DIEPTXFx[1], txfifosize.d32 );

  
  /* EP3 TX*/  
  txfifosize.b.startaddr += txfifosize.b.depth;
  txfifosize.b.depth = TX3_FIFO_SIZE;
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->DIEPTXFx[2], txfifosize.d32 );

  
  /* Flush the FIFOs */
  OTGD_FS_FlushTxFifo(0x10); /* all Tx FIFOs */
  OTGD_FS_FlushRxFifo();

  /* Clear all pending Device Interrupts */
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DIEPMSK, 0 );
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DOEPMSK, 0 );
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DAINT, 0xFFFFFFFF );
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DEV->DAINTMSK, 0 );

  for (i = 0; i < NUM_TX_FIFOS; i++)
  {
    depctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DINEPS[i]->DIEPCTLx);
    if (depctl.b.epena)
    {
      depctl.d32 = 0;
      depctl.b.epdis = 1;
      depctl.b.snak = 1;
    }
    else
    {
      depctl.d32 = 0;
    }

    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DINEPS[i]->DIEPCTLx, depctl.d32);


    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DINEPS[i]->DIEPTSIZx, 0);
    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DINEPS[i]->DIEPINTx, 0xFF);
  }

  for (i = 0; i < 1/* NUM_OUT_EPS*/; i++)
  {
    depctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DOUTEPS[i]->DOEPCTLx);
    if (depctl.b.epena)
    {
      depctl.d32 = 0;
      depctl.b.epdis = 1;
      depctl.b.snak = 1;
    }
    else
    {
      depctl.d32 = 0;
    }

    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DOUTEPS[i]->DOEPCTLx, depctl.d32);

    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DOUTEPS[i]->DOEPTSIZx, 0);
    USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.DOUTEPS[i]->DOEPINTx, 0xFF);
  }
  
  OTGD_FS_EnableDevInt();

  return status;
}
/*******************************************************************************
* Function Name  : OTGD_FS_EnableDevInt
* Description    : Enables the Device mode interrupts
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EnableDevInt(void)
{

  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_GINTMSK_TypeDef intr_mask;
  
  intr_mask.d32 = 0;

  /* Disable all interrupts. */
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GINTMSK, 0);

  /* Clear any pending interrupts */
  USB_OTG_WRITE_REG32( &USB_OTG_FS_regs.GREGS->GINTSTS, 0xFFFFFFFF);

  /* Enable the defined interrupts in  Device mode */
#ifdef INTR_SOFINTR
  intr_mask.b.sofintr = 1;
#endif /* INTR_SOFINTR */
#ifdef INTR_RXSTSQLVL
  intr_mask.b.rxstsqlvl = 1;
#endif /* INTR_RXSTSQLVL */
#ifdef INTR_GINNAKEFF
  intr_mask.b.ginnakeff = 1;
#endif /* INTR_GINNAKEFF */
#ifdef INTR_GOUTNAKEFF
  intr_mask.b.goutnakeff = 1;
#endif /* INTR_GOUTNAKEFF */
#ifdef INTR_ERLYSUSPEND
  intr_mask.b.erlysuspend = 1;
#endif /* INTR_ERLYSUSPEND */
#ifdef INTR_USBSUSPEND
  intr_mask.b.usbsuspend = 1;
#endif /* INTR_USBSUSPEND */
#ifdef INTR_USBRESET
  intr_mask.b.usbreset = 1;
#endif /* INTR_USBRESET */
#ifdef INTR_ENUMDONE
  intr_mask.b.enumdone = 1;
#endif /* INTR_ENUMDONE */
#ifdef INTR_ISOOUTDROP
  intr_mask.b.isooutdrop = 1;
#endif /* INTR_ISOOUTDROP */
#ifdef INTR_EOPFRAME
  intr_mask.b.eopframe = 1;
#endif /* INTR_EOPFRAME */
#ifdef INTR_INEPINTR
  intr_mask.b.inepintr = 1;
#endif /* INTR_INEPINTR */
#ifdef INTR_OUTEPINTR
  intr_mask.b.outepintr = 1;
#endif /* INTR_OUTEPINTR */
#ifdef INTR_INCOMPLISOIN
  intr_mask.b.incomplisoin = 1;
#endif /* INTR_INCOMPLISOIN */
#ifdef INTR_INCOMPLISOOUT
  intr_mask.b.incomplisoout = 1;
#endif /* INTR_INCOMPLISOOUT */
#ifdef INTR_DISCONNECT
  intr_mask.b.disconnect = 1;
#endif /* INTR_DISCONNECT */
#ifdef INTR_WKUPINTR
  intr_mask.b.wkupintr = 1;
#endif /* INTR_WKUPINTR */

  USB_OTG_MODIFY_REG32( &USB_OTG_FS_regs.GREGS->GINTMSK, intr_mask.d32, intr_mask.d32);
  return status;
  
}
/*******************************************************************************
* Function Name  : OTGD_FS_EP0Activate
* Description    : enables EP0 OUT to receive SETUP packets and configures EP0
                   IN for transmitting packets
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status  OTGD_FS_EP0Activate(void)
{
  USB_OTG_Status          status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef diepctl;
  USB_OTG_DCTL_TypeDef    dctl;

  diepctl.d32 = 0;
  dctl.d32 = 0;
  
  diepctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DINEPS[0]->DIEPCTLx);
  diepctl.b.mps = DEP0CTL_MPS_64;
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DINEPS[0]->DIEPCTLx, diepctl.d32);

  dctl.b.cgnpinnak = 1;
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DCTL, dctl.d32, dctl.d32);
  
  return status;
}
/*******************************************************************************
* Function Name  : OTGD_FS_EPActivate
* Description    : Activates an EP
* Input          : ep
* Output         : None
* Return         : num_in_ep
*******************************************************************************/
USB_OTG_Status OTGD_FS_EPActivate(USB_OTG_EP *ep)
{

  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef depctl;
  USB_OTG_DAINT_TypeDef   daintmsk;
  __IO uint32_t *addr;


  depctl.d32 = 0;
  daintmsk.d32 = 0;
  
  /* Read DEPCTLn register */
  if (ep->is_in == 1)
  {
    addr = &USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx;
    daintmsk.ep.in = 1 << ep->num;
  }
  else
  {
    addr = &USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx;
    daintmsk.ep.out = 1 << ep->num;
  }

  /* If the EP is already active don't change the EP Control
   * register. */
  depctl.d32 = USB_OTG_READ_REG32(addr);
  if (!depctl.b.usbactep)
  {
    depctl.b.mps    = ep->maxpacket;
    depctl.b.eptype = ep->type;
    depctl.b.txfnum = ep->tx_fifo_num;
    depctl.b.setd0pid = 1;
    depctl.b.usbactep = 1;
    USB_OTG_WRITE_REG32(addr, depctl.d32);
  }

  /* Enable the Interrupt for this EP */
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DAINTMSK, 0, daintmsk.d32);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_EPDeactivate
* Description    : Deactivates an EP
* Input          : ep
* Output         : None
* Return         : num_in_ep
*******************************************************************************/
USB_OTG_Status OTGD_FS_EPDeactivate(USB_OTG_EP *ep)
{

  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef depctl;
  __IO uint32_t *addr;
  USB_OTG_DAINT_TypeDef daintmsk;

  depctl.d32 = 0;
  daintmsk.d32 = 0;
  
  /* Read DEPCTLn register */
  if (ep->is_in == 1)
  {
    addr = &USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx;
    daintmsk.ep.in = 1 << ep->num;
  }
  else
  {
    addr = &USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx;
    daintmsk.ep.out = 1 << ep->num;
  }

  depctl.b.usbactep = 0;
  USB_OTG_WRITE_REG32(addr, depctl.d32);

  /* Disable the Interrupt for this EP */
  USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DAINTMSK, daintmsk.d32, 0);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_EPStartXfer
* Description    : Handle the setup for data xfer for an EP and starts the xfer
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EPStartXfer(USB_OTG_EP *ep)
{
  USB_OTG_DSTS_TypeDef dsts;  
  USB_OTG_Status status = USB_OTG_OK;
  __IO USB_OTG_DEPCTLx_TypeDef depctl;
  OTG_FS_DEPTSIZx_TypeDef deptsiz;

  depctl.d32 = 0;
  deptsiz.d32 = 0;
  
  /* IN endpoint */
  if (ep->is_in == 1)
  {

    depctl.d32  = USB_OTG_READ_REG32(&(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx));
    deptsiz.d32 = USB_OTG_READ_REG32(&(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPTSIZx));

    /* Zero Length Packet? */
    if (ep->xfer_len == 0)
    {
      deptsiz.b.xfersize = 0;
      deptsiz.b.pktcnt = 1;

    }
    else
    {
      /* Program the transfer size and packet count
       * as follows: xfersize = N * maxpacket +
       * short_packet pktcnt = N + (short_packet
       * exist ? 1 : 0)
       */
      deptsiz.b.xfersize = ep->xfer_len;
      deptsiz.b.pktcnt = (ep->xfer_len - 1 + ep->maxpacket) / ep->maxpacket;
      
      if (ep->type == EP_TYPE_ISOC)
      {
        deptsiz.b.mcount = 1;
      }      
    }
    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DINEPS[ep->num]->DIEPTSIZx, deptsiz.d32);

    if (ep->type != EP_TYPE_ISOC)
    {
      /* Enable the Tx FIFO Empty Interrupt for this EP */
      uint32_t fifoemptymsk = 0;
      fifoemptymsk = 1 << ep->num;
      USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DIEPEMPMSK, 0, fifoemptymsk);
    }
   
    /* EP enable, IN data in FIFO */
    depctl.b.cnak = 1;
    depctl.b.epena = 1;
    
    if (ep->type == EP_TYPE_ISOC)
    {
      dsts.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DSTS);
      
      if (((dsts.b.soffn)&0x1)==0)
      {
        depctl.b.setoddfrm=1;
      }
      else
      {
        depctl.b.setd0pid=1;
      }
    }  
    
    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx, depctl.d32); 
    
    if (ep->type == EP_TYPE_ISOC)
    {
      /*write buffer in TXFIFO*/
      /* user should ensure that ep->xfer_len <= ep->maxpacket */
      OTGD_FS_WritePacket(ep->xfer_buff, ep->num, ep->xfer_len);    
    }
  }
  else
  {
    /* OUT endpoint */
    depctl.d32  = USB_OTG_READ_REG32(&(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx));
    deptsiz.d32 = USB_OTG_READ_REG32(&(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPTSIZx));

    /* Program the transfer size and packet count as follows:
     * pktcnt = N
     * xfersize = N * maxpacket
     */
    if (ep->xfer_len == 0)
    {
      deptsiz.b.xfersize = ep->maxpacket;
      deptsiz.b.pktcnt = 1;
    }
    else
    {
      deptsiz.b.pktcnt = (ep->xfer_len + (ep->maxpacket - 1)) / ep->maxpacket;
      deptsiz.b.xfersize = deptsiz.b.pktcnt * ep->maxpacket;
    }
    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPTSIZx, deptsiz.d32);

    if (ep->type == EP_TYPE_ISOC)
    {

      if (ep->even_odd_frame)
      {
        depctl.b.setoddfrm = 1;
      }
      else
      {
        depctl.b.setd0pid = 1;
      }
    }

    /* EP enable */
    depctl.b.cnak = 1;
    depctl.b.epena = 1;

    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx, depctl.d32);

  }
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_EP0StartXfer
* Description    : Handle the setup for a data xfer for EP0 and starts the xfer
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EP0StartXfer(USB_OTG_EP *ep)
{

  USB_OTG_Status                    status = USB_OTG_OK;
  uint32_t                          fifoemptymsk = 0;
  USB_OTG_DEPCTLx_TypeDef           depctl;
  OTG_FS_DEPTSIZx_TypeDef           deptsiz;
  USB_OTG_DINEPS                    *in_regs ;

  depctl.d32 = 0;
  deptsiz.d32 = 0;
  
  /* IN endpoint */
  if (ep->is_in == 1)
  {
    in_regs = USB_OTG_FS_regs.DINEPS[0];
    depctl.d32  = USB_OTG_READ_REG32(&in_regs->DIEPCTLx);
    deptsiz.d32 = USB_OTG_READ_REG32(&in_regs->DIEPTSIZx);

    /* Zero Length Packet? */
    if (ep->xfer_len == 0)
    {
      deptsiz.b.xfersize = 0;
      deptsiz.b.pktcnt = 1;
    }
    else
    {
      if (ep->xfer_len > ep->maxpacket)
      {
        ep->xfer_len = ep->maxpacket;
        deptsiz.b.xfersize = ep->maxpacket;
      }
      else
      {
        deptsiz.b.xfersize = ep->xfer_len;
      }
      deptsiz.b.pktcnt = 1;

    }
    USB_OTG_WRITE_REG32(&in_regs->DIEPTSIZx, deptsiz.d32);

    /* EP enable, IN data in FIFO */
    depctl.b.cnak = 1;
    depctl.b.epena = 1;
    USB_OTG_WRITE_REG32(&in_regs->DIEPCTLx, depctl.d32);

    /* Enable the Tx FIFO Empty Interrupt for this EP */
    if (ep->xfer_len > 0)
    {
      fifoemptymsk |= 1 << ep->num;
      USB_OTG_MODIFY_REG32(&USB_OTG_FS_regs.DEV->DIEPEMPMSK, 0, fifoemptymsk);
    }
  }
  else
  {
    /* OUT endpoint */
    depctl.d32  = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DOUTEPS[0]->DOEPCTLx);
    deptsiz.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DOUTEPS[0]->DOEPTSIZx);

    /* Program the transfer size and packet count as follows:
     * xfersize = N * (maxpacket + 4 - (maxpacket % 4))
     * pktcnt = N           */
    if (ep->xfer_len == 0)
    {
      deptsiz.b.xfersize = ep->maxpacket;
      deptsiz.b.pktcnt = 1;
    }
    else
    {
      deptsiz.b.pktcnt = (ep->xfer_len + (ep->maxpacket - 1)) / ep->maxpacket;
      deptsiz.b.xfersize = deptsiz.b.pktcnt * ep->maxpacket;
    }

    USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DOUTEPS[0]->DOEPTSIZx, deptsiz.d32);

    /* EP enable */
    depctl.b.cnak = 1;
    depctl.b.epena = 1;
    USB_OTG_WRITE_REG32 (&(USB_OTG_FS_regs.DOUTEPS[0]->DOEPCTLx), depctl.d32);
  }
  return status;
}
/*******************************************************************************
* Function Name  : OTGD_FS_EPSetStall
* Description    : Set the EP STALL
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EPSetStall(USB_OTG_EP *ep)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef depctl; 
  __IO uint32_t *depctl_addr;

  depctl.d32 = 0;
  
  
  if (ep->is_in == 1)
  {
    depctl_addr = &(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

    /* set the disable and stall bits */
    if (depctl.b.epena)
    {
      depctl.b.epdis = 1;
    }
    depctl.b.stall = 1;
    USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  }
  else
  {
    depctl_addr = &(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

    /* set the stall bit */
    depctl.b.stall = 1;
    USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  }
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_EPClearStall
* Description    : Clear the EP STALL
* Input          : None
* Output         : None
* Return         : Status
*******************************************************************************/
USB_OTG_Status OTGD_FS_EPClearStall(USB_OTG_EP *ep)
{
  USB_OTG_Status status = USB_OTG_OK;
  USB_OTG_DEPCTLx_TypeDef depctl;
  __IO uint32_t *depctl_addr;

  
  depctl.d32 = 0;
  
  if (ep->is_in == 1)
  {
    depctl_addr = &(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx);
  }
  else
  {
    depctl_addr = &(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx);
  }

  
  depctl.d32 = USB_OTG_READ_REG32(depctl_addr);
   
  /* clear the stall bits */
  depctl.b.stall = 0;

  if (ep->type == EP_TYPE_INTR || ep->type == EP_TYPE_BULK) 
  {
    depctl.b.setd0pid = 1; /* DATA0 */
  }

  USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  return status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_ReadDevAllOutEp_itr
* Description    : returns the OUT endpoint interrupt bits
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t OTGD_FS_ReadDevAllOutEp_itr(void)
{
  uint32_t v = 0;
  
  v  = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DAINT);
  v &= USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DAINTMSK);
  return ((v & 0xffff0000) >> 16);
}

/*******************************************************************************
* Function Name  : OTGD_FS_ReadDevOutEP_itr
* Description    : returns the Device OUT EP Interrupt register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t OTGD_FS_ReadDevOutEP_itr(USB_OTG_EP *ep)
{
  uint32_t v = 0;
  
  v  = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPINTx);
  v &= USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DOEPMSK);
  return v;
}
/*******************************************************************************
* Function Name  : OTGD_FS_ReadDevAllInEPItr
* Description    : Get int status register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t OTGD_FS_ReadDevAllInEPItr(void)
{
  uint32_t v = 0;
  
  v = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DAINT);
  v &= USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DAINTMSK);
  return (v & 0xffff);
}

/*******************************************************************************
* Function Name  : OTGD_FS_GetEPStatus
* Description    : returns the EP Status 
* Input          : - ep: pointer to the EP structure
* Output         : None
* Return         : status: DEV_EP_TX_STALL, DEV_EP_TX_VALID, DEV_EP_TX_NAK, 
*                  DEV_EP_RX_STALL, DEV_EP_RX_VALID or DEV_EP_RX_NAK,
*******************************************************************************/
uint32_t OTGD_FS_GetEPStatus(USB_OTG_EP *ep)
{
  USB_OTG_DEPCTLx_TypeDef depctl;
  __IO uint32_t *depctl_addr;
  uint32_t Status = 0;
  
  depctl.d32 = 0;

  if (ep->is_in == 1)
  {
    depctl_addr = &(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx);
  }
  else
  {
    depctl_addr = &(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx);
  }

  depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

  /* Process for IN endpoint */
  if (ep->is_in == 1)
  {
    if (depctl.b.stall == 1)  
      Status = DEV_EP_TX_STALL;
    else if (depctl.b.naksts == 1)
      Status = DEV_EP_TX_NAK;
    else 
      Status = DEV_EP_TX_VALID; 
  } 
  /* Process for OUT endpoint */
  else 
  {
    if (depctl.b.stall == 1)  
      Status = DEV_EP_RX_STALL;
    else if (depctl.b.naksts == 1)
      Status = DEV_EP_RX_NAK;
    else 
      Status = DEV_EP_RX_VALID; 
  }
  
  /* Return the current status */
  return Status;
}

/*******************************************************************************
* Function Name  : OTGD_FS_SetEPStatus
* Description    : Sets the EP Status 
* Input          : - ep: pointer to the EP structure
*                  - Status: new status to be set
* Output         : None
* Return         : None
*******************************************************************************/
void OTGD_FS_SetEPStatus(USB_OTG_EP *ep, uint32_t Status)
{
  USB_OTG_DEPCTLx_TypeDef depctl;
  __IO uint32_t *depctl_addr;

  depctl.d32 = 0;
  
  
  if (ep->is_in == 1)
  {
    depctl_addr = &(USB_OTG_FS_regs.DINEPS[ep->num]->DIEPCTLx);
  }
  else
  {
    depctl_addr = &(USB_OTG_FS_regs.DOUTEPS[ep->num]->DOEPCTLx);
  }

  depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

  /* Process for IN endpoint */
  if (ep->is_in == 1)
  {
    if (Status == DEV_EP_TX_STALL)  
    {
      OTGD_FS_EPSetStall(ep); return;
    }
    else if (Status == DEV_EP_TX_NAK)
      depctl.b.snak = 1;
    else if (Status == DEV_EP_TX_VALID)
    {
      if (depctl.b.stall == 1)
      {  
        ep->even_odd_frame = 0;
        OTGD_FS_EPClearStall(ep);
        return;
      }      
      depctl.b.cnak = 1;
      depctl.b.usbactep = 1; 
      depctl.b.epena = 1;
    }
    else if (Status == DEV_EP_TX_DIS)
      depctl.b.usbactep = 0;
  } 
  else /* Process for OUT endpoint */
  {
    if (Status == DEV_EP_RX_STALL)  {
      depctl.b.stall = 1;
    }
    else if (Status == DEV_EP_RX_NAK)
      depctl.b.snak = 1;
    else if (Status == DEV_EP_RX_VALID)
    {
      if (depctl.b.stall == 1)
      {  
        ep->even_odd_frame = 0;
        OTGD_FS_EPClearStall(ep);
        return;
      }  
      depctl.b.cnak = 1;
      depctl.b.usbactep = 1;    
      depctl.b.epena = 1;
    }
    else if (Status == DEV_EP_RX_DIS)
    {
      depctl.b.usbactep = 0;    
    }
  }

  USB_OTG_WRITE_REG32(depctl_addr, depctl.d32); 
}

/*******************************************************************************
* Function Name  : OTGD_FS_SetRemoteWakeup
* Description    : Enable Remote wakeup signaling
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
void OTGD_FS_SetRemoteWakeup()
{
  USB_OTG_DCTL_TypeDef devctl;
  
  devctl.d32 = 0;
  
  devctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DCTL);
  
  /* Enable the Remote Wakeup signal */
  devctl.b.rmtwkupsig = 1;
  
  USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DCTL, devctl.d32);
}

/*******************************************************************************
* Function Name  : OTGD_FS_ResetRemoteWakeup
* Description    : Disable Remote wakeup signaling
* Input          : None
* Output         : None
* Return         : status
*******************************************************************************/
void OTGD_FS_ResetRemoteWakeup()
{
 USB_OTG_DCTL_TypeDef devctl;

 
 devctl.d32 = 0;
 
 devctl.d32 = USB_OTG_READ_REG32(&USB_OTG_FS_regs.DEV->DCTL);
 
 /* Disable the Remote Wakeup signal */
 devctl.b.rmtwkupsig = 0;
 
 USB_OTG_WRITE_REG32(&USB_OTG_FS_regs.DEV->DCTL, devctl.d32);
}
#endif /* STM32F10X_CL */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
