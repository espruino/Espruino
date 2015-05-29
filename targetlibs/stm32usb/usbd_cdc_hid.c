#include "usbd_cdc_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

#include "jshardware.h"
#include "jsinteractive.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
// CDC Buffers -----------------------------------
#define CDC_RX_DATA_SIZE  CDC_DATA_FS_OUT_PACKET_SIZE
#define CDC_TX_DATA_SIZE  CDC_DATA_FS_IN_PACKET_SIZE
uint8_t CDCRxBufferFS[CDC_RX_DATA_SIZE];
uint8_t CDCTxBufferFS[CDC_TX_DATA_SIZE];
// ..
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static void CDC_TxReady(void);
//-------------------------------------------------

static uint8_t  USBD_CDC_HID_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_HID_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_HID_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_CDC_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_HID_EP0_RxReady (USBD_HandleTypeDef *pdev);
static uint8_t  *USBD_CDC_HID_GetCfgDesc (uint16_t *length);
uint8_t  *USBD_CDC_HID_GetDeviceQualifierDescriptor (uint16_t *length);

/* CDC interface class callbacks structure */
const USBD_ClassTypeDef  USBD_CDC_HID =
{
  USBD_CDC_HID_Init,
  USBD_CDC_HID_DeInit,
  USBD_CDC_HID_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_CDC_HID_EP0_RxReady,
  USBD_CDC_HID_DataIn,
  USBD_CDC_HID_DataOut,
  NULL,
  NULL,
  NULL,     
  USBD_CDC_HID_GetCfgDesc,
  USBD_CDC_HID_GetCfgDesc,
  USBD_CDC_HID_GetCfgDesc,
  USBD_CDC_HID_GetDeviceQualifierDescriptor,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static const uint8_t USBD_CDC_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
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

/* USB CDC device Configuration Descriptor */
const __ALIGN_BEGIN uint8_t USBD_CDC_HID_CfgDesc[USB_CDC_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_HID_CONFIG_DESC_SIZ, 0x00,               /* wTotalLength:no of returned bytes */
#ifdef NOHID
  0x02,   /* bNumInterfaces: 2 interface */
#else
  0x03,   /* bNumInterfaces: 3 interface */
#endif
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
#ifndef NOHID
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 0 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  HID_INTERFACE_NUMBER,   /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 9 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE, 0, /*wItemLength: Total length of Report descriptor*/
  /******************** Descriptor of Mouse endpoint ********************/
  /* 18 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_DATA_IN_PACKET_SIZE,0x00, /*wMaxPacketSize: 4 Byte max */
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
  /* 25 */
#endif
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  CDC_INTERFACE_NUMBER,   /* bInterfaceNumber: Number of Interface */
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
  CDC_INTERFACE_NUMBER+1,   /* bDataInterface */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  CDC_INTERFACE_NUMBER,   /* bMasterInterface: Communication class interface */
  CDC_INTERFACE_NUMBER+1,   /* bSlaveInterface0: Data Class Interface */
  
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
  CDC_INTERFACE_NUMBER+1,   /* bInterfaceNumber: Number of Interface */
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
  LOBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_IN_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_IN_PACKET_SIZE),
  0x00,                               /* bInterval: ignore for Bulk transfer */


} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static const uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

__ALIGN_BEGIN static const uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
  0x05,   0x01,
  0x09,   0x02,
  0xA1,   0x01,
  0x09,   0x01,

  0xA1,   0x00,
  0x05,   0x09,
  0x19,   0x01,
  0x29,   0x03,

  0x15,   0x00,
  0x25,   0x01,
  0x95,   0x03,
  0x75,   0x01,

  0x81,   0x02,
  0x95,   0x01,
  0x75,   0x05,
  0x81,   0x01,

  0x05,   0x01,
  0x09,   0x30,
  0x09,   0x31,
  0x09,   0x38,

  0x15,   0x81,
  0x25,   0x7F,
  0x75,   0x08,
  0x95,   0x03,

  0x81,   0x06,
  0xC0,   0x09,
  0x3c,   0x05,
  0xff,   0x09,

  0x01,   0x15,
  0x00,   0x25,
  0x01,   0x75,
  0x01,   0x95,

  0x02,   0xb1,
  0x22,   0x75,
  0x06,   0x95,
  0x01,   0xb1,

  0x01,   0xc0
};

const int UsingHID = 1;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------


/**
  * @brief  USBD_CDC_HID_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_HID_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx)
{

  static USBD_CDC_HID_HandleTypeDef no_malloc_thx;
  pdev->pClassData = &no_malloc_thx;
  USBD_CDC_HID_HandleTypeDef   *hcdc = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;

  NOT_USED(cfgidx);
  uint8_t ret = 0;
  
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
  
  /* Init Xfer states */
  hcdc->cdcState = CDC_IDLE;

  /* Prepare Out endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev,
                         CDC_OUT_EP,
                         CDCRxBufferFS,
                         CDC_RX_DATA_SIZE);

  if (UsingHID) {
    /* Open EP IN */
    USBD_LL_OpenEP(pdev,
                   HID_IN_EP,
                   USBD_EP_TYPE_INTR,
                   HID_DATA_IN_PACKET_SIZE);
    hcdc->hidState = HID_IDLE;
  }
    
  return ret;
}

/**
  * @brief  USBD_CDC_HID_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_HID_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
  NOT_USED(cfgidx);
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

  if (UsingHID) {
    USBD_LL_CloseEP(pdev,
                  HID_IN_EP);
  }
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
    //USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  

  return ret;
}


static uint8_t  USBD_CDC_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  USBD_CDC_HID_HandleTypeDef   *hcdc = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
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
        hcdc->CmdLength = (uint8_t)req->wLength;
        
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

static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  USBD_CDC_HID_HandleTypeDef *hhid = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {


    case HID_REQ_SET_PROTOCOL:
      hhid->hidProtocol = (uint8_t)(req->wValue);
      break;

    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->hidProtocol,
                        1);
      break;

    case HID_REQ_SET_IDLE:
      hhid->hidIdleState = (uint8_t)(req->wValue >> 8);
      break;

    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->hidIdleState,
                        1);
      break;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_HID_Desc;
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }

      USBD_CtlSendData (pdev,
                        pbuf,
                        len);

      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->hidAltSetting,
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      hhid->hidAltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

static uint8_t  USBD_CDC_HID_Setup (USBD_HandleTypeDef *pdev,
                                    USBD_SetupReqTypedef *req) {

  // req->bRequest==USB_REQ_GET_DESCRIPTOR
  if (UsingHID && req->wIndex == HID_INTERFACE_NUMBER)
    return USBD_HID_Setup(pdev, req);
  else
    return USBD_CDC_Setup(pdev, req);
}


/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HID_HandleTypeDef   *hcdc = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
  
  if (epnum == (HID_IN_EP&0x7F)) {
    /* Ensure that the FIFO is empty before a new transfer, this condition could
    be caused by  a new transfer before the end of the previous transfer */
    ((USBD_CDC_HID_HandleTypeDef *)pdev->pClassData)->hidState = HID_IDLE;
  } else {
    // USB CDC
    hcdc->cdcState &= ~CDC_WRITE_TX_WAIT;
    CDC_TxReady();
  }

  return USBD_OK;
}

/**
  * @brief  USBD_CDC_HID_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
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
  * @brief  USBD_CDC_HID_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_HID_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  USBD_CDC_HID_HandleTypeDef   *hcdc = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
  
  if(hcdc->CmdOpCode != 0xFF)
  {
    CDC_Control_FS(hcdc->CmdOpCode, (uint8_t *)hcdc->data, hcdc->CmdLength);
    hcdc->CmdOpCode = 0xFF;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CDC_HID_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_HID_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CDC_HID_CfgDesc);
  return USBD_CDC_HID_CfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CDC_HID_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_CDC_HID_DeviceQualifierDesc);
  return USBD_CDC_HID_DeviceQualifierDesc;
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
      (((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState & CDC_WRITE_TX_WAIT)) // already waiting for send
    return;

  ((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState &= ~CDC_WRITE_DELAY;

  unsigned int len = 0;

  // try and fill the buffer
  int c;
  while (len<CDC_TX_DATA_SIZE-1 && // TODO: send max packet size -1 to ensure data is pushed through
         ((c = jshGetCharToTransmit(EV_USBSERIAL)) >= 0) ) { // get byte to transmit
    CDCTxBufferFS[len++] = (uint8_t)c;
  }

  // send data if we have any...
  if (len) {
    ((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState |= CDC_WRITE_TX_WAIT;

    /* Transmit next packet */
    USBD_LL_Transmit(&hUsbDeviceFS,
                     CDC_IN_EP,
                     CDCTxBufferFS,
                     (uint16_t)len);
  }
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------- PUBLIC Functions
// ----------------------------------------------------------------------------

uint8_t USBD_HID_SendReport     (uint8_t *report,  int len)
{
  USBD_CDC_HID_HandleTypeDef     *hhid = (USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;

  if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED ) {
    if(hhid->hidState == HID_IDLE) {
      hhid->hidState = HID_BUSY;
      memcpy(hhid->hidData, report, len);
      USBD_LL_Transmit (&hUsbDeviceFS,
                        HID_IN_EP,
                        hhid->hidData,
                        (uint16_t)len);
      return USBD_OK;
    }
  }
  return USBD_FAIL;
}


void USB_StartTransmission() {
  ((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState |= CDC_WRITE_DELAY;
}

int USB_IsConnected() {
  return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}

// To be called on SysTick timer
void USB_SysTick() {
  if (!USB_IsConnected()) return;
  if (((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState & CDC_WRITE_DELAY) {
    ((USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData)->cdcState &= ~CDC_WRITE_DELAY;
    CDC_TxReady();
  }
}


