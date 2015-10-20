/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include "jsutils.h"

#define JS_USB_HID_VAR_NAME "HID"

 // ----------------------------------------------------------------------------
#define CDC_DATA_FS_MAX_PACKET_SIZE                 64  /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE                         8  /* Control Endpoint Packet size */
#define CDC_DATA_FS_IN_PACKET_SIZE                  CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE                 CDC_DATA_FS_MAX_PACKET_SIZE

#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23

// ----------------------------------------------------------------------------

 #define HID_DATA_IN_PACKET_SIZE       0x10 // just in case... mouse=4, kb=8?

 #define HID_DESCRIPTOR_TYPE           0x21
 #define HID_REPORT_DESC               0x22

 #define HID_HS_BINTERVAL               0x07
 #define HID_FS_BINTERVAL               0x0A
 #define HID_POLLING_INTERVAL           0x0A
 #define HID_REQ_SET_PROTOCOL          0x0B
 #define HID_REQ_GET_PROTOCOL          0x03
 #define HID_REQ_SET_IDLE              0x0A
 #define HID_REQ_GET_IDLE              0x02
 #define HID_REQ_SET_REPORT            0x09
 #define HID_REQ_GET_REPORT            0x01

// ----------------------------------------------------------------------------

typedef enum
{
  HID_IDLE = 0,
  HID_BUSY,
} PACKED_FLAGS HID_StateTypeDef;

typedef enum
{
  CDC_IDLE = 0,
  CDC_WRITE_TX_WAIT = 1,
  CDC_READ_WAIT_EMPTY = 2,
} PACKED_FLAGS CDC_StateTypeDef;

typedef struct
{
  // TODO: can 'data' be shorter? IMO it only needs CDC_CMD_PACKET_SIZE bytes
  uint32_t data[CDC_DATA_FS_MAX_PACKET_SIZE/4];      /* Force 32bits alignment */
  uint8_t  cdcRX[CDC_DATA_FS_OUT_PACKET_SIZE];
  uint8_t  cdcTX[CDC_DATA_FS_IN_PACKET_SIZE];
#ifdef USE_USB_HID
  uint32_t hidData[HID_DATA_IN_PACKET_SIZE/4];  /* Force 32bits alignment */
#endif
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;    
  CDC_StateTypeDef     cdcState;
#ifdef USE_USB_HID
  HID_StateTypeDef     hidState;
  uint16_t             hidReportDescSize; // Nonzero if we're doing HID
  uint8_t             *hidReportDesc;
  uint32_t             hidProtocol;
  uint32_t             hidIdleState;
  uint32_t             hidAltSetting;
#endif
} USBD_CDC_HID_HandleTypeDef;

// ----------------------------------------------------------------------------
extern const USBD_ClassTypeDef  USBD_CDC_HID;
// ----------------------------------------------------------------------------
uint8_t USBD_HID_SendReport (uint8_t *report,unsigned int len);
// ----------------------------------------------------------------------------
int USB_IsConnected();
unsigned char *USB_GetHIDReportDesc(unsigned int *len);

#ifdef __cplusplus
}
#endif



#endif
