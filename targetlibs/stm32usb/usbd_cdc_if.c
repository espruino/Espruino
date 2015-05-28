#include "usbd_cdc_if.h"
#include "jsdevices.h"
#include "jshardware.h"

/* Define size for the receive and transmit buffer over CDC */
#define APP_RX_DATA_SIZE  4
#define APP_TX_DATA_SIZE  4

/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t CDC_Init_FS     (void);
static int8_t CDC_DeInit_FS   (void);
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS  (uint8_t* pbuf, uint32_t *Len);
static void   CDC_TxReady_FS  (void);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = 
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,  
  CDC_Receive_FS,
  CDC_TxReady_FS,
};

static int8_t CDC_Init_FS(void)
{
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
}

static int8_t CDC_DeInit_FS(void)
{
  return (USBD_OK);
}

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

static int8_t CDC_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
  jshPushIOCharEvents(EV_USBSERIAL, (char*)Buf, *Len);
  *Len = 0;
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}

// USB transmit is ready for more data
static void CDC_TxReady_FS(void)
{
  if (!USB_IsConnected() || // not connected
      (((USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->TxState!=0)) // already waiting for send
    return;
  jshPinOutput(LED1_PININDEX, 1);

  int len = 0;

  // try and fill the buffer
  int c;
  while (len<APP_TX_DATA_SIZE &&
         ((c = jshGetCharToTransmit(EV_USBSERIAL)) >= 0) ) { // get byte to transmit
    UserTxBufferFS[len++] = (uint8_t)c;
  }

  // send data if we have any...
  if (len) {
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, (uint16_t)len);
    USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  }
  jshPinOutput(LED1_PININDEX, 0);
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------- PUBLIC Functions
// ----------------------------------------------------------------------------
void USB_StartTransmission() {
  CDC_TxReady_FS();
}

int USB_IsConnected() {
  return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}
