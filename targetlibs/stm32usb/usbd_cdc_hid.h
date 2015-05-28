/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#ifdef NOHID
#define USB_CDC_HID_CONFIG_DESC_SIZ                     67
#else
#define USB_CDC_HID_CONFIG_DESC_SIZ                     (67+25)
#endif


 // ----------------------------------------------------------------------------
#define CDC_DATA_FS_MAX_PACKET_SIZE                 16  /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE                         8  /* Control Endpoint Packet size */
#define CDC_DATA_FS_IN_PACKET_SIZE                  CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE                 CDC_DATA_FS_MAX_PACKET_SIZE

#define CDC_IN_EP                                   0x81  /* EP1 for data IN */
#define CDC_OUT_EP                                  0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                                  0x82  /* EP2 for CDC commands */


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

 #define HID_IN_EP                     0x83
 #define HID_DATA_IN_PACKET_SIZE       0x04 // higher for keyboard
 #define HID_INTERFACE_NUMBER          2

 #define USB_HID_CONFIG_DESC_SIZ       34
 #define USB_HID_DESC_SIZ              9
 #define HID_MOUSE_REPORT_DESC_SIZE    74

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
} HID_StateTypeDef;

typedef struct
{
  uint32_t data[CDC_CMD_PACKET_SIZE/4];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;    
  uint8_t cdcTxState;

  uint32_t hidData[HID_DATA_IN_PACKET_SIZE/4];  /* Force 32bits alignment */
  uint32_t             hidProtocol;
  uint32_t             hidIdleState;
  uint32_t             hidAltSetting;
  HID_StateTypeDef     hidState;
} USBD_CDC_HID_HandleTypeDef;

// ----------------------------------------------------------------------------
extern const USBD_ClassTypeDef  USBD_CDC_HID;
// ----------------------------------------------------------------------------
uint8_t USBD_HID_SendReport (uint8_t *report, int len);
// ----------------------------------------------------------------------------
void USB_StartTransmission();
int USB_IsConnected();


#ifdef __cplusplus
}
#endif



#endif
