/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Common low-level device handling (Events, IO buffers)
 * ----------------------------------------------------------------------------
 */
#include "jsdevices.h"
#include "jsparse.h"
#include "jsinteractive.h"

#ifdef LINUX
 #include <signal.h>
#endif//LINUX
#ifdef USE_TRIGGER
#include "trigger.h"
#endif



// ----------------------------------------------------------------------------
//                                                              WATCH CALLBACKS
JshEventCallbackCallback jshEventCallbacks[EV_EXTI_MAX+1-EV_EXTI0];

// ----------------------------------------------------------------------------
//                                                         DATA TRANSMIT BUFFER
typedef struct {
  IOEventFlags flags; // Where this data should be transmitted
  unsigned char data;         // data to transmit
} PACKED_FLAGS TxBufferItem;

volatile TxBufferItem txBuffer[TXBUFFERMASK+1];
volatile unsigned char txHead=0, txTail=0;

typedef enum {
  SDS_NONE,
  SDS_XOFF_PENDING = 1,
  SDS_XON_PENDING = 2,
  SDS_XOFF_SENT = 4, // sending XON clears this
  SDS_FLOW_CONTROL_XON_XOFF = 8, // flow control enabled
} PACKED_FLAGS JshSerialDeviceState;
JshSerialDeviceState jshSerialDeviceStates[USARTS+1];

// ----------------------------------------------------------------------------
//                                                              IO EVENT BUFFER
volatile IOEvent ioBuffer[IOBUFFERMASK+1];
volatile unsigned char ioHead=0, ioTail=0;

// ----------------------------------------------------------------------------



void jshInitDevices() { // called from jshInit
  int i;
  // setup flow control
  jshSerialDeviceStates[0] = SDS_FLOW_CONTROL_XON_XOFF; // USB
  for (i=1;i<=USARTS;i++)
    jshSerialDeviceStates[i] = SDS_NONE;
  // set up callbacks for events
  for (i=EV_EXTI0;i<=EV_EXTI_MAX;i++)
    jshEventCallbacks[i-EV_EXTI0] = 0;
}

// ----------------------------------------------------------------------------

// Queue a character for transmission
void jshTransmit(IOEventFlags device, unsigned char data) {
  if (device==EV_LOOPBACKA || device==EV_LOOPBACKB) {
    jshPushIOCharEvent(device==EV_LOOPBACKB ? EV_LOOPBACKA : EV_LOOPBACKB, (char)data);
    return;
  }
#ifndef LINUX
#ifdef USB
  if (device==EV_USBSERIAL && !jshIsUSBSERIALConnected()) {
    jshTransmitClearDevice(EV_USBSERIAL); // clear out stuff already waiting
    return;
  }
#endif
#else // if PC, just put to stdout
  if (device==DEFAULT_CONSOLE_DEVICE) {
    fputc(data, stdout);
    fflush(stdout);
    return;
  }
#endif
  if (device==EV_NONE) return;
  unsigned char txHeadNext = (txHead+1)&TXBUFFERMASK;
  if (txHeadNext==txTail) {
    jsiSetBusy(BUSY_TRANSMIT, true);
    while (txHeadNext==txTail) {
      // wait for send to finish as buffer is about to overflow
#ifdef USB
      // just in case USB was unplugged while we were waiting!
      if (!jshIsUSBSERIALConnected()) jshTransmitClearDevice(EV_USBSERIAL);
#endif
    }
    jsiSetBusy(BUSY_TRANSMIT, false);
  }
  txBuffer[txHead].flags = device;
  txBuffer[txHead].data = (char)data;
  txHead = txHeadNext;

  jshUSARTKick(device); // set up interrupts if required
}

// Return the device at the top of the transmit queue (or EV_NONE)
IOEventFlags jshGetDeviceToTransmit() {
  if (!jshHasTransmitData()) return EV_NONE;
  return IOEVENTFLAGS_GETTYPE(txBuffer[txTail].flags);
}

// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device) {
  if (DEVICE_IS_USART(device)) {
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[device-EV_USBSERIAL];
    if ((*deviceState)&SDS_XOFF_PENDING) {
      (*deviceState) = ((*deviceState)&(~SDS_XOFF_PENDING)) | SDS_XOFF_SENT;
      return 19/*XOFF*/;
    }
    if ((*deviceState)&SDS_XON_PENDING) {
      (*deviceState) = ((*deviceState)&(~(SDS_XON_PENDING|SDS_XOFF_SENT)));
      return 17/*XON*/;
    }
  }

  unsigned char ptr = txTail;
  while (txHead != ptr) {
    if (IOEVENTFLAGS_GETTYPE(txBuffer[ptr].flags) == device) {
      unsigned char data = txBuffer[ptr].data;
      if (ptr != txTail) { // so we weren't right at the back of the queue
        // we need to work back from ptr (until we hit tail), shifting everything forwards
        unsigned char this = ptr;
        unsigned char last = (unsigned char)((this+TXBUFFERMASK)&TXBUFFERMASK);
        while (this!=txTail) { // if this==txTail, then last is before it, so stop here
          txBuffer[this] = txBuffer[last];
          this = last;
          last = (unsigned char)((this+TXBUFFERMASK)&TXBUFFERMASK);
        }
      }
      txTail = (unsigned char)((txTail+1)&TXBUFFERMASK); // advance the tail
      return data; // return data
    }
    ptr = (unsigned char)((ptr+1)&TXBUFFERMASK);
  }
  return -1; // no data :(
}

void jshTransmitFlush() {
  jsiSetBusy(BUSY_TRANSMIT, true);
  while (jshHasTransmitData()) ; // wait for send to finish
  jsiSetBusy(BUSY_TRANSMIT, false);
}

// Clear everything from a device
void jshTransmitClearDevice(IOEventFlags device) {
  while (jshGetCharToTransmit(device)>=0);
}

bool jshHasTransmitData() {
  return txHead != txTail;
}


void jshIOEventOverflowed() {
  // Error here - just set flag so we don't dump a load of data out
  jsErrorFlags |= JSERR_RX_FIFO_FULL;
}


void jshPushIOCharEvent(IOEventFlags channel, char charData) {
  if (charData==3 && channel==jsiGetConsoleDevice()) {
    // Ctrl-C - force interrupt
    execInfo.execute |= EXEC_CTRL_C;
    return;
  }
  if (DEVICE_IS_USART(channel) && jshGetEventsUsed() > IOBUFFER_XOFF) 
    jshSetFlowControlXON(channel, false);
  // Check for existing buffer (we must have at least 2 in the queue to avoid dropping chars though!)
  unsigned char nextTail = (unsigned char)((ioTail+1) & IOBUFFERMASK);
#ifndef LINUX // no need for this on linux, and also potentially dodgy when multi-threading
  if (ioHead!=ioTail && ioHead!=nextTail) {
    // we can do this because we only read in main loop, and we're in an interrupt here
    unsigned char lastHead = (unsigned char)((ioHead+IOBUFFERMASK) & IOBUFFERMASK); // one behind head
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[lastHead].flags) == channel &&
        IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags) < IOEVENT_MAXCHARS) {
      // last event was for this event type, and it has chars left
      unsigned char c = (unsigned char)IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags);
      ioBuffer[lastHead].data.chars[c] = charData;
      IOEVENTFLAGS_SETCHARS(ioBuffer[lastHead].flags, c+1);
      return;
    }
  }
#endif
  // Make new buffer
  unsigned char nextHead = (unsigned char)((ioHead+1) & IOBUFFERMASK);
  if (ioTail == nextHead) {
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead].flags = channel;
  IOEVENTFLAGS_SETCHARS(ioBuffer[ioHead].flags, 1);
  ioBuffer[ioHead].data.chars[0] = charData;
  ioHead = nextHead;
}

void jshPushIOWatchEvent(IOEventFlags channel) {
  bool state = jshGetWatchedPinState(channel);

  if (jshEventCallbacks[channel-EV_EXTI0]) {
    jshEventCallbacks[channel-EV_EXTI0](state);
    return;
  }

  JsSysTime time = jshGetSystemTime();

#ifdef USE_TRIGGER
  // TODO: move to using jshSetEventCallback
  if (trigHandleEXTI(channel | (state?EV_EXTI_IS_HIGH:0), time))
    return;
#endif
 // Otherwise add this event
 jshPushIOEvent(channel | (state?EV_EXTI_IS_HIGH:0), time);
}

void jshPushIOEvent(IOEventFlags channel, JsSysTime time) {
  unsigned char nextHead = (unsigned char)((ioHead+1) & IOBUFFERMASK);
  if (ioTail == nextHead) {
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead].flags = channel;
  ioBuffer[ioHead].data.time = (unsigned int)time;
  ioHead = nextHead;
}

// returns true on success
bool jshPopIOEvent(IOEvent *result) {
  if (ioHead==ioTail) return false;
  *result = ioBuffer[ioTail];
  ioTail = (unsigned char)((ioTail+1) & IOBUFFERMASK);
  return true;
}

// returns true on success
bool jshPopIOEventOfType(IOEventFlags eventType, IOEvent *result) {
  unsigned char i = ioTail;
  while (ioHead!=i) {
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[i].flags) == eventType) {
      *result = ioBuffer[i];
      // work back and shift all items in out queue
      unsigned char n = (unsigned char)((i+IOBUFFERMASK) & IOBUFFERMASK);
      while (n!=ioTail) {
        ioBuffer[i] = ioBuffer[n];
        i = n;
        n = (unsigned char)((n+IOBUFFERMASK) & IOBUFFERMASK);
      }
      // finally update the tail pointer, and return
      ioTail = (unsigned char)((ioTail+1) & IOBUFFERMASK);
      return true;
    }
    i = (unsigned char)((i+1) & IOBUFFERMASK);
  }
  return false;
}

bool jshHasEvents() {
  return ioHead!=ioTail;
}

/// Check if the top event is for the given device
bool jshIsTopEvent(IOEventFlags eventType) {
  if (ioHead==ioTail) return false;
  return IOEVENTFLAGS_GETTYPE(ioBuffer[ioTail].flags) == eventType;
}

int jshGetEventsUsed() {
  int spaceUsed = (ioHead >= ioTail) ? ((int)ioHead-(int)ioTail) : /*or rolled*/((int)ioHead+IOBUFFERMASK+1-(int)ioTail);
  return spaceUsed;
}

bool jshHasEventSpaceForChars(int n) {
  int spacesNeeded = 4 + (n/IOEVENT_MAXCHARS); // be sensible - leave a little spare
  int spaceUsed = jshGetEventsUsed();
  int spaceLeft = IOBUFFERMASK+1-spaceUsed;
  return spaceLeft > spacesNeeded;
}

// ----------------------------------------------------------------------------
//                                                                      DEVICES
const char *jshGetDeviceString(IOEventFlags device) {
  switch (device) {
  case EV_LOOPBACKA: return "LoopbackA";
  case EV_LOOPBACKB: return "LoopbackB";
#ifdef USB
  case EV_USBSERIAL: return "USB";
#endif
  case EV_SERIAL1: return "Serial1";
  case EV_SERIAL2: return "Serial2";
  case EV_SERIAL3: return "Serial3";
#if USARTS>=4
  case EV_SERIAL4: return "Serial4";
#endif
#if USARTS>=5
  case EV_SERIAL5: return "Serial5";
#endif
#if USARTS>=6
  case EV_SERIAL6: return "Serial6";
#endif
#if SPIS>=1
  case EV_SPI1: return "SPI1";
#endif
#if SPIS>=2
  case EV_SPI2: return "SPI2";
#endif
#if SPIS>=3
  case EV_SPI3: return "SPI3";
#endif
#if I2CS>=1
  case EV_I2C1: return "I2C1";
#endif
#if I2CS>=2
  case EV_I2C2: return "I2C2";
#endif
#if I2CS>=3
  case EV_I2C3: return "I2C3";
#endif
  default: return "";
  }
}

IOEventFlags jshFromDeviceString(const char *device) {
  if (device[0]=='L') {
    if (strcmp(&device[1], "oopbackA")==0) return EV_LOOPBACKA;
    if (strcmp(&device[1], "oopbackB")==0) return EV_LOOPBACKB;
  }
#ifdef USB
  if (device[0]=='U' && device[1]=='S' && device[2]=='B' && device[3]==0) {
    return EV_USBSERIAL;
  }
#endif
  else if (device[0]=='S') {
    if (device[1]=='e' && device[2]=='r' && device[3]=='i' && device[4]=='a' && device[5]=='l' && device[6]!=0 && device[7]==0) {
      if (device[6]=='1') return EV_SERIAL1;
      if (device[6]=='2') return EV_SERIAL2;
      if (device[6]=='3') return EV_SERIAL3;
#if USARTS>=4
      if (device[6]=='4') return EV_SERIAL4;
#endif
#if USARTS>=5
      if (device[6]=='5') return EV_SERIAL5;
#endif
#if USARTS>=6
      if (device[6]=='6') return EV_SERIAL6;
#endif
    }
    if (device[1]=='P' && device[2]=='I' && device[3]!=0 && device[4]==0) {
#if SPIS>=1
      if (device[3]=='1') return EV_SPI1;
#endif
#if SPIS>=2
      if (device[3]=='2') return EV_SPI2;
#endif
#if SPIS>=3
      if (device[3]=='3') return EV_SPI3;
#endif
    }
  }
  else if (device[0]=='I' && device[1]=='2' && device[2]=='C' && device[3]!=0 && device[4]==0) {
#if I2CS>=1
    if (device[3]=='1') return EV_I2C1;
#endif
#if I2CS>=2
    if (device[3]=='2') return EV_I2C2;
#endif
#if I2CS>=3
    if (device[3]=='3') return EV_I2C3;
#endif
  }
  return EV_NONE;
}

/// Set whether the host should transmit or not
void jshSetFlowControlXON(IOEventFlags device, bool hostShouldTransmit) {
  if (DEVICE_IS_USART(device)) {
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[device-EV_USBSERIAL];
    if ((*deviceState) & SDS_FLOW_CONTROL_XON_XOFF) {
      if (hostShouldTransmit) {
        if (((*deviceState)&(SDS_XOFF_SENT|SDS_XON_PENDING)) == SDS_XOFF_SENT) {
          jshInterruptOff();
          (*deviceState) |= SDS_XON_PENDING;
          jshInterruptOn();
          jshUSARTKick(device);
        }
      } else { // !hostShouldTransmit
        if (((*deviceState)&(SDS_XOFF_SENT|SDS_XOFF_PENDING)) == 0) {
          jshInterruptOff();
          (*deviceState) |= SDS_XOFF_PENDING;
          jshInterruptOn();
          jshUSARTKick(device);
        }
      }
    }
  }
}

/// Gets a device's object from a device, or return 0 if it doesn't exist
JsVar *jshGetDeviceObject(IOEventFlags device) {
  const char *deviceStr = jshGetDeviceString(device);
  if (!deviceStr) return 0;
  return jsvObjectGetChild(execInfo.root, deviceStr, 0);
}

/// Set whether to use flow control on the given device or not
void jshSetFlowControlEnabled(IOEventFlags device, bool xOnXOff) {
  if (!DEVICE_IS_USART(device)) return;
  JshSerialDeviceState *deviceState = &jshSerialDeviceStates[device-EV_USBSERIAL];
  if (xOnXOff)
    (*deviceState) |= SDS_FLOW_CONTROL_XON_XOFF;
  else
    (*deviceState) &= ~SDS_FLOW_CONTROL_XON_XOFF;
}

/// Set a callback function to be called when an event occurs
void jshSetEventCallback(IOEventFlags channel, JshEventCallbackCallback callback) {
  assert(channel>=EV_EXTI0 && channel<=EV_EXTI_MAX);
  jshEventCallbacks[channel-EV_EXTI0] = callback;
}
