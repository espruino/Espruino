/*
 * jsdevices.h
 *
 *  Created on: 8 Nov 2012
 *      Author: gw
 */

#ifndef JSDEVICES_H_
#define JSDEVICES_H_

#include "jsutils.h"

#define DEFAULT_BAUD_RATE               9600
#ifdef ARM
 #include "platform_config.h"
#else
 #define IOBUFFERMASK                   255 // (max 255)
 #define TXBUFFERMASK                   255
 #define DEFAULT_CONSOLE_DEVICE         EV_USBSERIAL
 #define DEFAULT_BUSY_PIN_INDICATOR     -1
 #define DEFAULT_SLEEP_PIN_INDICATOR     -1
 #define USARTS 1
#endif

typedef enum {
 // device type
 EV_NONE,
 EV_EXTI0,
 EV_EXTI1,
 EV_EXTI2,
 EV_EXTI3,
 EV_EXTI4,
 EV_EXTI5,
 EV_EXTI6,
 EV_EXTI7,
 EV_EXTI8,
 EV_EXTI9,
 EV_EXTI10,
 EV_EXTI11,
 EV_EXTI12,
 EV_EXTI13,
 EV_EXTI14,
 EV_EXTI15,
 EV_USBSERIAL,
 EV_SERIAL1,
 EV_SERIAL2,
 EV_SERIAL3,
 EV_SERIAL4,
 EV_SERIAL5,
 EV_SERIAL6,

 EV_TYPE_MASK = 31,
 EV_CHARS_MASK = 7 << 5,
} PACKED_FLAGS IOEventFlags;

#define DEVICE_IS_USART(X) (((X)==EV_USBSERIAL) || ((X)==EV_SERIAL1) || ((X)==EV_SERIAL2) || ((X)==EV_SERIAL3) || ((X)==EV_SERIAL4) || ((X)==EV_SERIAL5) || ((X)==EV_SERIAL6))

#define IOEVENTFLAGS_GETTYPE(X) ((X)&EV_TYPE_MASK)
#define IOEVENTFLAGS_GETCHARS(X) ((((X)&EV_CHARS_MASK)>>5)+1)
#define IOEVENTFLAGS_SETCHARS(X,CHARS) ((X)=(((X)&(IOEventFlags)~EV_CHARS_MASK) | (((CHARS)-1)<<5)))
#define IOEVENT_MAXCHARS 8

typedef union {
  JsSysTime time; // time event occurred
  char chars[IOEVENT_MAXCHARS];
} PACKED_FLAGS IOEventData;

// IO Events - these happen when a pin changes
typedef struct IOEvent {
  IOEventFlags flags; // where this came from, and # of chars in it
  IOEventData data;
} PACKED_FLAGS IOEvent;

void jshPushIOEvent(IOEventFlags channel, JsSysTime time);
void jshPushIOCharEvent(IOEventFlags channel, char charData);
bool jshPopIOEvent(IOEvent *result); ///< returns true on success
/// Do we have any events pending? Will jshPopIOEvent return true?
bool jshHasEvents();
/// Do we have enough space for N chanacters?
bool jshHasEventSpaceForChars(int n);

const char *jshGetDeviceString(IOEventFlags device);
IOEventFlags jshFromDeviceString(const char *device);

// ----------------------------------------------------------------------------
//                                                         DATA TRANSMIT BUFFER
// Queue a character for transmission
void jshTransmit(IOEventFlags device, unsigned char data);
/// Wait for transmit to finish
void jshTransmitFlush();
// Clear everything from a device
void jshTransmitClearDevice(IOEventFlags device);
/// Do we have anything we need to send?
bool jshHasTransmitData();
// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device);


#endif /* JSDEVICES_H_ */
