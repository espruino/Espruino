#include "usbd_cdc_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

#include "jshardware.h"
#include "jsinteractive.h"

#ifdef STM32F1
// don't know why :( probably setup in usdb_conf
#define CDC_IN_EP                     0x81  /* EP1 for data IN */
#define CDC_OUT_EP                    0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                    0x82  /* EP2 for CDC commands */
#else
#define CDC_IN_EP                     0x83  /* EP1 for data IN */
#define CDC_OUT_EP                    0x03  /* EP1 for data OUT */
#define CDC_CMD_EP                    0x82  /* EP2 for CDC commands */

#define HID_IN_EP                     0x81
#define HID_INTERFACE_NUMBER          2
#endif

extern USBD_HandleTypeDef hUsbDeviceFS;
//-------------------------------------------------
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
//-------------------------------------------------

static uint8_t  USBD_CDC_HID_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_HID_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_HID_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_CDC_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_HID_SOF (struct _USBD_HandleTypeDef *pdev);
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
  USBD_CDC_HID_SOF,
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
  USB_LEN_DEV_QUALIFIER_DESC,    // bLength
  USB_DESC_TYPE_DEVICE_QUALIFIER,// bDescriptorType
  0x00,  // bcdUSB lo
  0x02,  // bcdUSB hi
  0x00,  // bDeviceClass
  0x00,  // bDeviceSubClass
  0x00,  // bDeviceProtocol
  0x40,  // bMaxPacketSize0
  0x01,  // bNumConfigurations
  0x00,  // bReserved
};


/* USB CDC device Configuration Descriptor
 * ============================================================================
 *
 * No HID
 * CDC on Interfaces 0 and 1
 */
#define USBD_CDC_CFGDESC_SIZE              (67+8)
const __ALIGN_BEGIN uint8_t USBD_CDC_CfgDesc[USBD_CDC_CFGDESC_SIZE] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USBD_CDC_CFGDESC_SIZE, 0x00,               /* wTotalLength:no of returned bytes */
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /* Interface Association Descriptor */
  0x08, /* bLength: IAD descriptor size */
  0x0B, /* bDescriptorType: IAD */
  0x00, /* bFirstInterface: Number if first interface for grouping */
  0x02, /* bInterfaceCount: Interfaces count for grouping */
  0x02, /* bFunctionClass: The same as bInterfaceClass below */
  0x02, /* bFunctionSubClass: The same as bInterfaceSubClass below */
  0x01, /* bFunctionProtocol: The same as bInterfaceProtocol below */
  0x00, /* iFunction */
  
  // -----------------------------------------------------------------------
  /*CDC Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0,   /* bInterfaceNumber: Number of Interface */
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
  1,   /* bDataInterface */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0,   /* bMasterInterface: Communication class interface */
  1,   /* bSlaveInterface0: Data Class Interface */
  
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
  1,   /* bInterfaceNumber: Number of Interface */
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

#ifdef USE_USB_HID
/* USB HID + CDC device Configuration Descriptor
 * ============================================================================
 *
 * HID on Interface 0
 * CDC on Interfaces 1 and 2
 */
#define USBD_CDC_HID_CFGDESC_SIZE              (67+25+8)
#define USBD_CDC_HID_CFGDESC_REPORT_SIZE_IDX   91
// NOT CONST - descriptor size needs updating as this is sent out
__ALIGN_BEGIN uint8_t USBD_CDC_HID_CfgDesc[USBD_CDC_HID_CFGDESC_SIZE] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USBD_CDC_HID_CFGDESC_SIZE, 0x00,               /* wTotalLength:no of returned bytes */
  0x03,   /* bNumInterfaces: 3 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */

  /* Interface Association Descriptor */
  0x08, /* bLength: IAD descriptor size */
  0x0B, /* bDescriptorType: IAD */
  0x00, /* bFirstInterface: Number if first interface for grouping */
  0x02, /* bInterfaceCount: Interfaces count for grouping */
  0x02, /* bFunctionClass: The same as bInterfaceClass below */
  0x02, /* bFunctionSubClass: The same as bInterfaceSubClass below */
  0x01, /* bFunctionProtocol: The same as bInterfaceProtocol below */
  0x00, /* iFunction */
  
  // -----------------------------------------------------------------------
  /*CDC Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0,   /* bInterfaceNumber: Number of Interface */
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
  1,   /* bDataInterface */

  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0,   /* bMasterInterface: Communication class interface */
  1,   /* bSlaveInterface0: Data Class Interface */

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
  1,   /* bInterfaceNumber: Number of Interface */
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
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 9 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  HID_INTERFACE_NUMBER,   /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0,            /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  0/*HID_REPORT_DESC_SIZE*/, 0, /*wItemLength: Total length of Report descriptor*/
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_DATA_IN_PACKET_SIZE,0x00, /*wMaxPacketSize: 4 Byte max */
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
} ;

#define USB_HID_DESC_SIZ              9
#define USBD_HID_DESC_REPORT_SIZE_IDX 7
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  0/*HID_REPORT_DESC_SIZE*/, 0x00, /*wItemLength: Total length of Report descriptor*/
};
#endif
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------


USBD_CDC_HID_HandleTypeDef cdc_hid;

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

  pdev->pClassData = &cdc_hid;
  USBD_CDC_HID_HandleTypeDef   *handle = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;

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
  handle->cdcState = CDC_IDLE;

  /* Prepare Out endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev,
                         CDC_OUT_EP,
                         handle->cdcRX,
                         CDC_DATA_FS_OUT_PACKET_SIZE);

#ifdef USE_USB_HID
  unsigned int reportSize = 0;
  handle->hidReportDesc = USB_GetHIDReportDesc(&reportSize);
  handle->hidReportDescSize = (uint16_t)reportSize;

  /* Open HID EP IN - even if we're not using it */
  USBD_LL_OpenEP(pdev,
                 HID_IN_EP,
                 USBD_EP_TYPE_INTR,
                 HID_DATA_IN_PACKET_SIZE);

  handle->hidState = HID_IDLE;
#endif

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
  USBD_CDC_HID_HandleTypeDef   *handle = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
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
#ifdef USE_USB_HID
  USBD_LL_CloseEP(pdev,
                HID_IN_EP);
#endif
  
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
  USBD_CDC_HID_HandleTypeDef   *handle = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
  static uint8_t ifalt = 0;
    
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    if (req->wLength)
    {
      if (req->bmRequest & 0x80)
      {
        CDC_Control_FS(req->bRequest, (uint8_t *)handle->data, req->wLength);
        USBD_CtlSendData (pdev,
                            (uint8_t *)handle->data,
                            req->wLength);
      }
      else
      {
        handle->CmdOpCode = req->bRequest;
        handle->CmdLength = (uint8_t)req->wLength;
        
        USBD_CtlPrepareRx (pdev, 
                           (uint8_t *)handle->data,
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

#ifdef USE_USB_HID
static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  int len = 0;
  uint8_t  *pbuf = NULL;
  USBD_CDC_HID_HandleTypeDef *handle = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {


    case HID_REQ_SET_PROTOCOL:
      handle->hidProtocol = (uint8_t)(req->wValue);
      break;

    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&handle->hidProtocol,
                        1);
      break;

    case HID_REQ_SET_IDLE:
      handle->hidIdleState = (uint8_t)(req->wValue >> 8);
      break;

    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&handle->hidIdleState,
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
        pbuf = handle->hidReportDesc;
        len = MIN(handle->hidReportDescSize, req->wLength);
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        USBD_HID_Desc[USBD_HID_DESC_REPORT_SIZE_IDX] = (uint8_t)handle->hidReportDescSize;
        USBD_HID_Desc[USBD_HID_DESC_REPORT_SIZE_IDX+1] = (uint8_t)(handle->hidReportDescSize>>8);
        pbuf = USBD_HID_Desc;
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }

      USBD_CtlSendData (pdev,
                        pbuf,
                        (uint16_t)len);

      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&handle->hidAltSetting,
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      handle->hidAltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}
#endif

static uint8_t  USBD_CDC_HID_Setup (USBD_HandleTypeDef *pdev,
                                    USBD_SetupReqTypedef *req) {
  USBD_CDC_HID_HandleTypeDef *handle = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;
#ifdef USE_USB_HID
  if (handle->hidReportDescSize && req->wIndex == HID_INTERFACE_NUMBER)
    return USBD_HID_Setup(pdev, req);
  else
#endif
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
  USBD_CDC_HID_HandleTypeDef *handle = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;

#ifdef USE_USB_HID
  if (epnum == (HID_IN_EP&0x7F)) {
    /* Ensure that the FIFO is empty before a new transfer, this condition could
    be caused by  a new transfer before the end of the previous transfer */
    handle->hidState = HID_IDLE;
  } else
#endif
  {
    // USB CDC
    handle->cdcState &= ~CDC_WRITE_TX_WAIT;
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
  USBD_CDC_HID_HandleTypeDef *handle = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;

  /* Get the received data length */
  unsigned int rxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  
  if (handle) {
    // Process data
    jshPushIOCharEvents(EV_USBSERIAL, (char*)handle->cdcRX, rxLength);

    // Set CDC_READ_WAIT_EMPTY flag - we'll re-enable USB RX using
    // USBD_LL_PrepareReceive ONLY when we have enough space
    handle->cdcState |= CDC_READ_WAIT_EMPTY;

    return USBD_OK;
  } else {
    return USBD_FAIL;
  }
}

static uint8_t  USBD_CDC_HID_SOF (struct _USBD_HandleTypeDef *pdev) {
  USBD_CDC_HID_HandleTypeDef *handle = (USBD_CDC_HID_HandleTypeDef*)pdev->pClassData;
  if (!handle ||
      pdev->dev_state != USBD_STATE_CONFIGURED)
    return USBD_OK;

  if ((handle->cdcState & CDC_READ_WAIT_EMPTY) &&
      jshGetEventsUsed() < IOBUFFER_XOFF) {
    handle->cdcState &= ~CDC_READ_WAIT_EMPTY;
    USBD_LL_PrepareReceive(pdev,
                           CDC_OUT_EP,
                           handle->cdcRX,
                           CDC_DATA_FS_OUT_PACKET_SIZE);
  }

  if (!handle->cdcState & CDC_WRITE_TX_WAIT) {
    // try and fill the buffer
    unsigned int len = 0;
    int c;
    while (len<CDC_DATA_FS_IN_PACKET_SIZE-1 && // TODO: send max packet size -1 to ensure data is pushed through
           ((c = jshGetCharToTransmit(EV_USBSERIAL)) >= 0) ) { // get byte to transmit
      handle->cdcTX[len++] = (uint8_t)c;
    }

    // send data if we have any...
    if (len) {
      /* Transmit next packet */
      handle->cdcState |= CDC_WRITE_TX_WAIT;
      USBD_LL_Transmit(pdev,
                       CDC_IN_EP,
                       handle->cdcTX,
                       (uint16_t)len);
    }
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
static uint8_t  USBD_CDC_HID_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  USBD_CDC_HID_HandleTypeDef   *handle = (USBD_CDC_HID_HandleTypeDef*) pdev->pClassData;
  
  if(handle->CmdOpCode != 0xFF)
  {
    CDC_Control_FS(handle->CmdOpCode, (uint8_t *)handle->data, handle->CmdLength);
    handle->CmdOpCode = 0xFF;
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
#ifdef USE_USB_HID
  // Check if we have a HID report here - we do it now so we can actually allow
  // Espruino to be plugged and unplugged WITHOUT A RESET
  unsigned int reportSize = 0;
  USB_GetHIDReportDesc(&reportSize);

  if (reportSize) {
    USBD_CDC_HID_CfgDesc[USBD_CDC_HID_CFGDESC_REPORT_SIZE_IDX] = (uint8_t)reportSize;
    USBD_CDC_HID_CfgDesc[USBD_CDC_HID_CFGDESC_REPORT_SIZE_IDX+1] = (uint8_t)(reportSize>>8);
    *length = USBD_CDC_HID_CFGDESC_SIZE;
    return USBD_CDC_HID_CfgDesc;
  } else
#endif
  {
    *length = USBD_CDC_CFGDESC_SIZE;
    return USBD_CDC_CfgDesc;
  }
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
  NOT_USED(pbuf);
  NOT_USED(length);
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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


#ifdef USE_USB_HID
unsigned char *USB_GetHIDReportDesc(unsigned int *len) {
  if (execInfo.hiddenRoot) {
    JsVar *v = jsvObjectGetChild(execInfo.hiddenRoot, JS_USB_HID_VAR_NAME, 0);
    if (jsvIsFlatString(v)) {
      if (len) *len = jsvGetStringLength(v);
      unsigned char *p = jsvGetFlatStringPointer(v);
      jsvUnLock(v);
      return p;
    }
  }

  if (len) *len = 0;
  return 0;
}
#endif

// ----------------------------------------------------------------------------
// --------------------------------------------------------- PUBLIC Functions
// ----------------------------------------------------------------------------

#ifdef USE_USB_HID
uint8_t USBD_HID_SendReport     (uint8_t *report,  unsigned int len)
{
  USBD_CDC_HID_HandleTypeDef     *handle = (USBD_CDC_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;

  if (USB_IsConnected() &&
      handle->hidReportDescSize && // HID enabled?
      handle->hidState == HID_IDLE) { // busy?
    handle->hidState = HID_BUSY;
    memcpy(handle->hidData, report, len);
    USBD_LL_Transmit (&hUsbDeviceFS,
                      HID_IN_EP,
                      (uint8_t*)handle->hidData,
                      (uint16_t)len);
    return USBD_OK;
  }
  return USBD_FAIL;
}
#endif


int USB_IsConnected() {
  return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}
