#include "usbd_cdc.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

#include "jshardware.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
// CDC Buffers-----------------------------------
#define CDC_RX_DATA_SIZE  4
#define CDC_TX_DATA_SIZE  4
uint8_t CDCRxBufferFS[CDC_RX_DATA_SIZE];
uint8_t CDCTxBufferFS[CDC_TX_DATA_SIZE];
// ..
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static void CDC_TxReady(void);

static uint8_t  USBD_CDC_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_CDC_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_CDC_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_CDC_DataOut (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_CDC_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  *USBD_CDC_GetCfgDesc (uint16_t *length);

uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor (uint16_t *length);

/* USB Standard Device Descriptor */
const __ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_CDC_Private_Variables
  * @{
  */ 


/* CDC interface class callbacks structure */
const USBD_ClassTypeDef  USBD_CDC =
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_CDC_EP0_RxReady,
  USBD_CDC_DataIn,
  USBD_CDC_DataOut,
  NULL,
  NULL,
  NULL,     
  USBD_CDC_GetCfgDesc,
  USBD_CDC_GetCfgDesc,
  USBD_CDC_GetCfgDesc,
  USBD_CDC_GetDeviceQualifierDescriptor,
};


/* USB CDC device Configuration Descriptor */
const __ALIGN_BEGIN uint8_t USBD_CDC_CfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0x10,                           /* bInterval: */ 
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                               /* bInterval: ignore for Bulk transfer */
} ;

/**
  * @}
  */ 

/** @defgroup USBD_CDC_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t ret = 0;
  USBD_CDC_HandleTypeDef   *hcdc;
  
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 CDC_IN_EP,
                 USBD_EP_TYPE_BULK,
                 CDC_DATA_FS_IN_PACKET_SIZE);

  /* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 CDC_OUT_EP,
                 USBD_EP_TYPE_BULK,
                 CDC_DATA_FS_OUT_PACKET_SIZE);
  /* Open Command IN EP */
  USBD_LL_OpenEP(pdev,
                 CDC_CMD_EP,
                 USBD_EP_TYPE_INTR,
                 CDC_CMD_PACKET_SIZE);
  
    
  //pdev->pClassData = USBD_malloc(sizeof (USBD_CDC_HandleTypeDef));
  static USBD_CDC_HandleTypeDef no_malloc_thx;
  pdev->pClassData = &no_malloc_thx;

  hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

  /* Init Xfer states */
  hcdc->TxState = 0;

  /* Prepare Out endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev,
                         CDC_OUT_EP,
                         CDCRxBufferFS,
                         CDC_RX_DATA_SIZE);
    
  return ret;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Open EP IN */
  USBD_LL_CloseEP(pdev,
              CDC_IN_EP);
  
  /* Open EP OUT */
  USBD_LL_CloseEP(pdev,
              CDC_OUT_EP);
  
  /* Open Command IN EP */
  USBD_LL_CloseEP(pdev,
              CDC_CMD_EP);
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
    //USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  

  return ret;
}

/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CDC_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
  static uint8_t ifalt = 0;
    
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    if (req->wLength)
    {
      if (req->bmRequest & 0x80)
      {
        CDC_Control_FS(req->bRequest, (uint8_t *)hcdc->data, req->wLength);
        USBD_CtlSendData (pdev,
                            (uint8_t *)hcdc->data,
                            req->wLength);
      }
      else
      {
        hcdc->CmdOpCode = req->bRequest;
        hcdc->CmdLength = req->wLength;
        
        USBD_CtlPrepareRx (pdev, 
                           (uint8_t *)hcdc->data,
                           req->wLength);
      }
      
    }
    else
    {
      CDC_Control_FS(req->bRequest, (uint8_t*)req, 0);
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        &ifalt,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      break;
    }
 
  default: 
    break;
  }
  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    
    hcdc->TxState = 0;
    CDC_TxReady();
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
  
  /* Get the received data length */
  unsigned int RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the application Xfer */
  if(pdev->pClassData != NULL)
  {
    jshPushIOCharEvents(EV_USBSERIAL, (char*)CDCRxBufferFS, RxLength);

    USBD_LL_PrepareReceive(pdev,
                           CDC_OUT_EP,
                           CDCRxBufferFS,
                           CDC_DATA_FS_OUT_PACKET_SIZE);

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}


/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
  
  if(hcdc->CmdOpCode != 0xFF)
  {
    CDC_Control_FS(hcdc->CmdOpCode, (uint8_t *)hcdc->data, hcdc->CmdLength);
      hcdc->CmdOpCode = 0xFF; 
      
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CDC_GetFSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CDC_CfgDesc);
  return USBD_CDC_CfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_CDC_DeviceQualifierDesc);
  return USBD_CDC_DeviceQualifierDesc;
}

//----------------------------------------------------------------------------

static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case CDC_SET_LINE_CODING:
    // called when plugged in and app connects
    break;

  case CDC_GET_LINE_CODING:

    break;

  case CDC_SET_CONTROL_LINE_STATE:
    // called on connect/disconnect by app
    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
}

// USB transmit is ready for more data
static void CDC_TxReady(void)
{
  if (!USB_IsConnected() || // not connected
      (((USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->TxState!=0)) // already waiting for send
    return;
  jshPinOutput(LED1_PININDEX, 1);

  int len = 0;

  // try and fill the buffer
  int c;
  while (len<CDC_TX_DATA_SIZE &&
         ((c = jshGetCharToTransmit(EV_USBSERIAL)) >= 0) ) { // get byte to transmit
    CDCTxBufferFS[len++] = (uint8_t)c;
  }

  // send data if we have any...
  if (len) {
    ((USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->TxState = 1;

    /* Transmit next packet */
    USBD_LL_Transmit(&hUsbDeviceFS,
                     CDC_IN_EP,
                     CDCTxBufferFS,
                     len);
  }
  jshPinOutput(LED1_PININDEX, 0);
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------- PUBLIC Functions
// ----------------------------------------------------------------------------
void USB_StartTransmission() {
  CDC_TxReady();
}

int USB_IsConnected() {
  return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}

