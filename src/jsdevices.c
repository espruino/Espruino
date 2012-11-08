/*
 * jsdevices.c
 *
 *  Created on: 8 Nov 2012
 *      Author: gw
 */

#include "jsdevices.h"
#include "jsparse.h"
#include "jsinteractive.h"

#ifndef ARM
 #include <signal.h>
#endif//ARM

// ----------------------------------------------------------------------------
//                                                                     BUFFERS

// ----------------------------------------------------------------------------
//                                                         DATA TRANSMIT BUFFER
typedef struct {
  IOEventFlags flags; // Where this data should be transmitted
  unsigned char data;         // data to transmit
} PACKED_FLAGS TxBufferItem;

TxBufferItem txBuffer[TXBUFFERMASK+1];
volatile unsigned char txHead=0, txTail=0;
// ----------------------------------------------------------------------------

// Queue a character for transmission
void jshTransmit(IOEventFlags device, unsigned char data) {
#ifdef ARM
#ifdef USB
  if (device==EV_USBSERIAL && !jshIsUSBSERIALConnected()) {
    jshTransmitClearDevice(EV_USBSERIAL); // clear out stuff already waiting
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

#else // if PC, just put to stdout
  fputc(data, stdout);
  fflush(stdout);
#endif
}

#ifdef ARM
// Try and get a character for transmission - could just return -1 if nothing
int jshGetCharToTransmit(IOEventFlags device) {
  unsigned char ptr = txTail;
  while (txHead != ptr) {
    if (IOEVENTFLAGS_GETTYPE(txBuffer[ptr].flags) == device) {
      unsigned char data = txBuffer[ptr].data;
      if (ptr != txTail) { // so we weren't right at the back of the queue
        // we need to work back from ptr (until we hit tail), shifting everything forwards
        unsigned char this = ptr;
        unsigned char last = (this+TXBUFFERMASK)&TXBUFFERMASK;
        while (this!=txTail) { // if this==txTail, then last is before it, so stop here
          txBuffer[this] = txBuffer[last];
          this = last;
          last = (this+TXBUFFERMASK)&TXBUFFERMASK;
        }
      }
      txTail = (txTail+1)&TXBUFFERMASK; // advance the tail
      return data; // return data
    }
    ptr = (ptr+1)&TXBUFFERMASK;
  }
  return -1; // no data :(
}
#endif

void jshTransmitFlush() {
#ifdef ARM
  jsiSetBusy(BUSY_TRANSMIT, true);
  while (jshHasTransmitData()) ; // wait for send to finish
  jsiSetBusy(BUSY_TRANSMIT, false);
#endif
}

// Clear everything from a device
void jshTransmitClearDevice(IOEventFlags device) {
#ifdef ARM
  while (jshGetCharToTransmit(device)>=0);
#endif
}

bool jshHasTransmitData() {
#ifdef ARM
  return txHead != txTail;
#else
  return false;
#endif
}

// ----------------------------------------------------------------------------
//                                                              IO EVENT BUFFER
IOEvent ioBuffer[IOBUFFERMASK+1];
volatile unsigned char ioHead=0, ioTail=0;
// ----------------------------------------------------------------------------


void jshIOEventOverflowed() {
  // TODO: error here?
}


void jshPushIOCharEvent(IOEventFlags channel, char charData) {
  if (charData==3) {
    // Ctrl-C - force interrupt
    // TODO - check if this is our Console port?
#ifndef ARM
    raise(SIGINT);
#endif
    jspSetInterrupted(true);
    return;
  }
  // Check for existing buffer (we must have at least 2 in the queue to avoid dropping chars though!)
  unsigned char nextTail = (unsigned char)((ioTail+1) & IOBUFFERMASK);
  if (ioHead!=ioTail && ioHead!=nextTail) {
    // we can do this because we only read in main loop, and we're in an interrupt here
    unsigned char lastHead = (unsigned char)((ioHead+IOBUFFERMASK) & IOBUFFERMASK);
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[lastHead].flags) == channel &&
        IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags) < IOEVENT_MAXCHARS) {
      // last event was for this event type, and it has chars left
      unsigned char c = (unsigned char)IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags);
      ioBuffer[lastHead].data.chars[c] = charData;
      IOEVENTFLAGS_SETCHARS(ioBuffer[lastHead].flags, c+1);
      return;
    }
  }
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

void jshPushIOEvent(IOEventFlags channel, JsSysTime time) {
  unsigned char nextHead = (unsigned char)((ioHead+1) & IOBUFFERMASK);
  if (ioTail == nextHead) {
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead].flags = channel;
  ioBuffer[ioHead].data.time = time;
  ioHead = nextHead;
}

// returns true on success
bool jshPopIOEvent(IOEvent *result) {
  if (ioHead==ioTail) return false;
  *result = ioBuffer[ioTail];
  ioTail = (unsigned char)((ioTail+1) & IOBUFFERMASK);
  return true;
}

bool jshHasEvents() {
  return ioHead!=ioTail;
}

bool jshHasEventSpaceForChars(int n) {
  int spacesNeeded = 4 + (n/IOEVENT_MAXCHARS); // be sensible - leave a little spare
  int spaceUsed = (ioHead >= ioTail) ? ((int)ioHead-(int)ioTail) : /*or rolled*/((int)ioHead+IOBUFFERMASK+1-(int)ioTail);
  int spaceLeft = IOBUFFERMASK+1-spaceUsed;
  return spaceLeft > spacesNeeded;
}

// ----------------------------------------------------------------------------
//                                                                      DEVICES
const char *jshGetDeviceString(IOEventFlags device) {
  switch (device) {
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
  default: return "";
  }
}

IOEventFlags jshFromDeviceString(const char *device) {
  if (device[0]=='U') {
#ifdef USB
    if (strcmp(device, "USB")==0) return EV_USBSERIAL;
#endif
  }
  if (device[0]=='S') {
    if (strcmp(device, "Serial1")==0) return EV_SERIAL1;
    if (strcmp(device, "Serial2")==0) return EV_SERIAL2;
    if (strcmp(device, "Serial3")==0) return EV_SERIAL3;
#if USARTS>=4
    if (strcmp(device, "Serial4")==0) return EV_SERIAL4;
#endif
#if USARTS>=5
    if (strcmp(device, "Serial5")==0) return EV_SERIAL5;
#endif
#if USARTS>=6
    if (strcmp(device, "Serial6")==0) return EV_SERIAL6;
#endif
  }
  return EV_NONE;
}

