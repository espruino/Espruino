/*
 * jshardware.h
 *
 *  Created on: 8 Aug 2012
 *      Author: gw
 */

#ifndef JSHARDWARE_H_
#define JSHARDWARE_H_

#define DEFAULT_BAUD_RATE               9600

#ifdef ARM
 #include "platform_config.h"
#else
 #define IOBUFFERMASK                   255 // (max 255)
 #define DEFAULT_CONSOLE_DEVICE         EV_USBSERIAL
 #define DEFAULT_BUSY_PIN_INDICATOR     -1
 #define DEFAULT_SLEEP_PIN_INDICATOR     -1
#endif
#include "jsutils.h"
#include "jsvar.h"

void jshInit();
void jshKill();
void jshIdle(); // stuff to do on idle

bool jshIsUSBSERIALConnected(); // is the serial device connected?

#define JSSYSTIME_MAX 0x7FFFFFFFFFFFFFFFLL
typedef long long JsSysTime;
/// Get the system time (in ticks)
JsSysTime jshGetSystemTime();
/// Convert a time in Milliseconds to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms);
/// Convert ticks to a time in Milliseconds
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time);

typedef enum {
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
 EV_USART1,
 EV_USART2,  
 EV_USART3,  
 EV_USART4,    

 EV_TYPE_MASK = 31,
 EV_CHARS_MASK = 7 << 5,
} PACKED_FLAGS IOEventFlags;

#define DEVICE_IS_USART(X) (((X)==EV_USBSERIAL) || ((X)==EV_USART1) || ((X)==EV_USART2) || ((X)==EV_USART3) || ((X)==EV_USART4))

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


/// Given a string, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromString(const char *s);
/// Given a var, convert it to a pin ID (or -1 if it doesn't exist)
int jshGetPinFromVar(JsVar *pinv);
bool jshPinInput(int pin);
JsVarFloat jshPinAnalog(int pin);
void jshPinOutput(int pin, bool value);
void jshPinAnalogOutput(int pin, JsVarFloat value);
void jshPinPulse(int pin, bool value, JsVarFloat time);
void jshPinWatch(int pin, bool shouldWatch);
bool jshIsEventForPin(IOEvent *event, int pin);

void jshUSARTSetup(IOEventFlags device, int baudRate);

/// Save contents of JsVars into Flash
void jshSaveToFlash();
/// Load contents of JsVars from Flash
void jshLoadFromFlash();
/// Returns true if flash contains something useful
bool jshFlashContainsCode();


/// Enter simple sleep mode (can be woken up by interrupts)
void jshSleep();

// ---------------------------------------------- LOW LEVEL

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

#ifdef ARM
// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device);

// ----------------------------------------------------------------------------
//                                                                      SYSTICK
// On SYSTick interrupt, call this
void jshDoSysTick();
#ifdef USB
// Kick the USB SysTick watchdog - we need this to see if we have disconnected or not
void jshKickUSBWatchdog();
#endif

#endif // ARM

#endif /* JSHARDWARE_H_ */
