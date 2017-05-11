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
#include <stdio.h>
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

/**
 * A single character to be transmitted.
 */
typedef struct {
  IOEventFlags flags; //!< Where this data should be transmitted
  unsigned char data; //!< data to transmit
} PACKED_FLAGS TxBufferItem;

/**
 * An array of items to transmit.
 */
volatile TxBufferItem txBuffer[TXBUFFERMASK+1];

/**
 * The head and tail of the list.
 */
volatile unsigned char txHead=0, txTail=0;

typedef enum {
  SDS_NONE,
  SDS_XOFF_PENDING = 1,
  SDS_XON_PENDING = 2,
  SDS_XOFF_SENT = 4, // sending XON clears this
  SDS_FLOW_CONTROL_XON_XOFF = 8, // flow control enabled
} PACKED_FLAGS JshSerialDeviceState;
JshSerialDeviceState jshSerialDeviceStates[EV_SERIAL1+USART_COUNT-EV_SERIAL_START];
/// Device clear to send hardware flow control pins (PIN_UNDEFINED if not used)
Pin jshSerialDeviceCTSPins[EV_SERIAL1+USART_COUNT-EV_SERIAL_START];
#define TO_SERIAL_DEVICE_STATE(X) ((X)-EV_SERIAL_START)

// ----------------------------------------------------------------------------
//                                                              IO EVENT BUFFER
volatile IOEvent ioBuffer[IOBUFFERMASK+1];
volatile unsigned char ioHead=0, ioTail=0;

// ----------------------------------------------------------------------------


/** Initialize any device-specific structures, like flow control states.
 * Called from jshInit */
void jshInitDevices() {
  jshResetDevices();
}

/** Reset any devices that could have been set up differently by JS code.
 * Called from jshReset */
void jshResetDevices() {
  unsigned int i;
  // Reset list of pins that were set manually
  jshResetPinStateIsManual();
  // setup flow control
  for (i=0;i<sizeof(jshSerialDeviceStates) / sizeof(JshSerialDeviceState);i++) {
    jshSerialDeviceStates[i] = SDS_NONE;
    jshSerialDeviceCTSPins[i] = PIN_UNDEFINED;
  }
  jshSerialDeviceStates[TO_SERIAL_DEVICE_STATE(EV_USBSERIAL)] = SDS_FLOW_CONTROL_XON_XOFF;
  // reset callbacks for events
  for (i=EV_EXTI0;i<=EV_EXTI_MAX;i++)
    jshEventCallbacks[i-EV_EXTI0] = 0;
  // Reset pin state for button
#ifdef BTN1_PININDEX
#ifdef BTN1_PINSTATE
  jshSetPinStateIsManual(BTN1_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);
#else
  jshPinSetState(BTN1_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
}

// ----------------------------------------------------------------------------

/**
 * Queue a character for transmission.
 */
void jshTransmit(
    IOEventFlags device, //!< The device to be used for transmission.
    unsigned char data   //!< The character to transmit.
  ) {
  if (device==EV_LOOPBACKA || device==EV_LOOPBACKB) {
    jshPushIOCharEvent(device==EV_LOOPBACKB ? EV_LOOPBACKA : EV_LOOPBACKB, (char)data);
    return;
  }
#ifdef USE_TELNET
  if (device == EV_TELNET) {
    // gross hack to avoid deadlocking on the network here
    extern void telnetSendChar(char c);
    telnetSendChar(data);
    return;
  }
#endif
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
  // If the device is EV_NONE then there is nowhere to send the data.
  if (device==EV_NONE) return;

  // The txHead global points to the current item in the txBuffer.  Since we are adding a new
  // character, we increment the head pointer.   If it has caught up with the tail, then that means
  // we have filled the array backing the list.  What we do next is to wait for space to free up.
  unsigned char txHeadNext = (unsigned char)((txHead+1)&TXBUFFERMASK);
  if (txHeadNext==txTail) {
    jsiSetBusy(BUSY_TRANSMIT, true);
    bool wasConsoleLimbo = device==EV_LIMBO && jsiGetConsoleDevice()==EV_LIMBO;
    while (txHeadNext==txTail) {
      // wait for send to finish as buffer is about to overflow
#ifdef USB
      // just in case USB was unplugged while we were waiting!
      if (!jshIsUSBSERIALConnected()) jshTransmitClearDevice(EV_USBSERIAL);
#endif
    }
    if (wasConsoleLimbo && jsiGetConsoleDevice()!=EV_LIMBO) {
      /* It was 'Limbo', but now it's not - see jsiOneSecondAfterStartup.
      Basically we must have printed a bunch of stuff to LIMBO and blocked
      with our output buffer full. But then jsiOneSecondAfterStartup
      switches to the right console device and swaps everything we wrote
      over to that device too. Only we're now here, still writing to the
      old device when really we should be writing to the new one. */
      device = jsiGetConsoleDevice();
    }
    jsiSetBusy(BUSY_TRANSMIT, false);
  }
  // Save the device and data for the new character to be transmitted.
  txBuffer[txHead].flags = device;
  txBuffer[txHead].data = data;
  txHead = txHeadNext;

  jshUSARTKick(device); // set up interrupts if required
}

// Return the device at the top of the transmit queue (or EV_NONE)
IOEventFlags jshGetDeviceToTransmit() {
  if (!jshHasTransmitData()) return EV_NONE;
  return IOEVENTFLAGS_GETTYPE(txBuffer[txTail].flags);
}

/**
 * Try and get a character for transmission.
 * \return The next byte to transmit or -1 if there is none.
 */
int jshGetCharToTransmit(
    IOEventFlags device // The device being looked at for a transmission.
  ) {
  if (DEVICE_IS_USART(device)) {
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[TO_SERIAL_DEVICE_STATE(device)];
    if ((*deviceState)&SDS_XOFF_PENDING) {
      (*deviceState) = ((*deviceState)&(~SDS_XOFF_PENDING)) | SDS_XOFF_SENT;
      return 19/*XOFF*/;
    }
    if ((*deviceState)&SDS_XON_PENDING) {
      (*deviceState) = ((*deviceState)&(~(SDS_XON_PENDING|SDS_XOFF_SENT)));
      return 17/*XON*/;
    }
  }

  unsigned char tempTail = txTail;
  while (txHead != tempTail) {
    if (IOEVENTFLAGS_GETTYPE(txBuffer[tempTail].flags) == device) {
      unsigned char data = txBuffer[tempTail].data;
      if (tempTail != txTail) { // so we weren't right at the back of the queue
        // we need to work back from tempTail (until we hit tail), shifting everything forwards
        unsigned char this = tempTail;
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
    tempTail = (unsigned char)((tempTail+1)&TXBUFFERMASK);
  }
  return -1; // no data :(
}

void jshTransmitFlush() {
  jsiSetBusy(BUSY_TRANSMIT, true);
  while (jshHasTransmitData()) ; // wait for send to finish
  jsiSetBusy(BUSY_TRANSMIT, false);
}

/**
 * Discard all the data waiting for transmission.
 */
void jshTransmitClearDevice(
    IOEventFlags device //!< The device to be cleared.
  ) {
  // Keep requesting a character to transmit until there are no further characters.
  while (jshGetCharToTransmit(device)>=0);
}

/// Move all output from one device to another
void jshTransmitMove(IOEventFlags from, IOEventFlags to) {
  if (to==EV_LOOPBACKA || to==EV_LOOPBACKB) {
    // Loopback is special :(
    IOEventFlags device = (to==EV_LOOPBACKB) ? EV_LOOPBACKA : EV_LOOPBACKB;
    int c = jshGetCharToTransmit(from);
    while (c>=0) {
      jshPushIOCharEvent(device, (char)c);
      c = jshGetCharToTransmit(from);
    }
  } else {
    // Otherwise just rename the contents of the buffer
    jshInterruptOff();
    unsigned char tempTail = txTail;
    while (tempTail != txHead) {
      if (IOEVENTFLAGS_GETTYPE(txBuffer[tempTail].flags) == from) {
        txBuffer[tempTail].flags = (txBuffer[tempTail].flags&~EV_TYPE_MASK) | to;
      }
      tempTail = (unsigned char)((tempTail+1)&TXBUFFERMASK);
    }
    jshInterruptOn();
  }
}

/**
 * Determine if we have data to be transmitted.
 * \return True if we have data to transmit and false otherwise.
 */
bool jshHasTransmitData() {
  return txHead != txTail;
}

/**
 * flag that the buffer has overflowed.
 */
void CALLED_FROM_INTERRUPT jshIOEventOverflowed() {
  // Error here - just set flag so we don't dump a load of data out
  jsErrorFlags |= JSERR_RX_FIFO_FULL;
}


/**
 * Send a character to the specified device.
 */
void jshPushIOCharEvent(
    IOEventFlags channel, // !< The device to target for output.
    char charData         // !< The character to send to the device.
  ) {
  // Check for a CTRL+C
  if (charData==3 && channel==jsiGetConsoleDevice()) {
    jsiCtrlC(); // Ctrl-C - force interrupt of execution
    return;
  }
  // Check for existing buffer (we must have at least 2 in the queue to avoid dropping chars though!)
#ifndef LINUX // no need for this on linux, and also potentially dodgy when multi-threading
  unsigned char lastHead = (unsigned char)((ioHead+IOBUFFERMASK) & IOBUFFERMASK); // one behind head
  if (ioHead!=ioTail && lastHead!=ioTail) {
    // we can do this because we only read in main loop, and we're in an interrupt here
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[lastHead].flags) == channel) {
      unsigned char c = (unsigned char)IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags);
      if (c < IOEVENT_MAXCHARS) {
        // last event was for this event type, and it has chars left
        ioBuffer[lastHead].data.chars[c] = charData;
        IOEVENTFLAGS_SETCHARS(ioBuffer[lastHead].flags, c+1);
        return; // char added, job done
      }
    }
  }
#endif
  // Set flow control (as we're going to use more data)
  if (DEVICE_IS_USART(channel) && jshGetEventsUsed() > IOBUFFER_XOFF)
    jshSetFlowControlXON(channel, false);

  /* Make new buffer
   *
   * We're disabling IRQs for this bit because it's actually quite likely for
   * USB and USART data to be coming in at the same time, and it can trip
   * things up if one IRQ interrupts another. */
  jshInterruptOff();
  unsigned char nextHead = (unsigned char)((ioHead+1) & IOBUFFERMASK);
  if (ioTail == nextHead) {
    jshInterruptOn();
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  unsigned char oldHead = ioHead;
  ioHead = nextHead;
  ioBuffer[oldHead].flags = channel;
  // once channel is set we're safe - another IRQ won't touch this
  jshInterruptOn();
  IOEVENTFLAGS_SETCHARS(ioBuffer[oldHead].flags, 1);
  ioBuffer[oldHead].data.chars[0] = charData;
}

/**
 * Signal an IO watch event as having happened.
 */
// on the esp8266 we need this to be loaded into static RAM because it can run at interrupt time
void CALLED_FROM_INTERRUPT jshPushIOWatchEvent(
    IOEventFlags channel //!< The channel on which the IO watch event has happened.
  ) {
  assert(channel >= EV_EXTI0 && channel <= EV_EXTI_MAX);

  bool state = jshGetWatchedPinState(channel);

  // If there is a callback associated with this GPIO event then invoke
  // it and we are done.
  if (jshEventCallbacks[channel-EV_EXTI0]) {
    jshEventCallbacks[channel-EV_EXTI0](state, channel);
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

/**
 * Add this IO event to the IO event queue.
 */
void CALLED_FROM_INTERRUPT jshPushIOEvent(
    IOEventFlags channel, //!< The event to add to the queue.
    JsSysTime time        //!< The time that the event is thought to have happened.
  ) {
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
  // Special case for top - it's easier!
  if (IOEVENTFLAGS_GETTYPE(ioBuffer[ioTail].flags) == eventType)
    return jshPopIOEvent(result);
  // Now check non-top
  unsigned char i = ioTail;
  while (ioHead!=i) {
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[i].flags) == eventType) {
      /* We need IRQ off for this, because if we get data it's possible
      that the IRQ will push data and will try and add characters to this
      exact position in the buffer */
      jshInterruptOff();
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
      jshInterruptOn();
      return true;
    }
    i = (unsigned char)((i+1) & IOBUFFERMASK);
  }
  return false;
}

/**
 * Determine if we have I/O events to process.
 * \return True if there are I/O events to be processed.
 */
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

/**
 * Get a string representation of a device.
 * \return A string representation of a device.
 */
const char *jshGetDeviceString(
    IOEventFlags device //!< The device to be examined.
  ) {
  switch (device) {
  case EV_LOOPBACKA: return "LoopbackA";
  case EV_LOOPBACKB: return "LoopbackB";
  case EV_LIMBO: return "Limbo";
#ifdef USB
  case EV_USBSERIAL: return "USB";
#endif
#ifdef BLUETOOTH
  case EV_BLUETOOTH: return "Bluetooth";
#endif
#ifdef USE_TELNET
  case EV_TELNET: return "Telnet";
#endif
  case EV_SERIAL1: return "Serial1";
  case EV_SERIAL2: return "Serial2";
  case EV_SERIAL3: return "Serial3";
#if USART_COUNT>=4
  case EV_SERIAL4: return "Serial4";
#endif
#if USART_COUNT>=5
  case EV_SERIAL5: return "Serial5";
#endif
#if USART_COUNT>=6
  case EV_SERIAL6: return "Serial6";
#endif
#if SPI_COUNT>=1
  case EV_SPI1: return "SPI1";
#endif
#if SPI_COUNT>=2
  case EV_SPI2: return "SPI2";
#endif
#if SPI_COUNT>=3
  case EV_SPI3: return "SPI3";
#endif
#if I2C_COUNT>=1
  case EV_I2C1: return "I2C1";
#endif
#if I2C_COUNT>=2
  case EV_I2C2: return "I2C2";
#endif
#if I2C_COUNT>=3
  case EV_I2C3: return "I2C3";
#endif
  default: return "";
  }
}

/**
 * Get a device identity from a string.
 * \return A device identity.
 */
IOEventFlags jshFromDeviceString(
    const char *device //!< A string representation of a device.
  ) {
  if (device[0]=='L') {
    if (strcmp(&device[1], "oopbackA")==0) return EV_LOOPBACKA;
    if (strcmp(&device[1], "oopbackB")==0) return EV_LOOPBACKB;
  }
#ifdef USB
  if (device[0]=='U' && device[1]=='S' && device[2]=='B' && device[3]==0) {
    return EV_USBSERIAL;
  }
#endif
#ifdef BLUETOOTH
  if (device[0]=='B') {
     if (strcmp(&device[1], "luetooth")==0) return EV_BLUETOOTH;
  }
#endif
#ifdef USE_TELNET
  if (device[0]=='T') {
     if (strcmp(&device[1], "elnet")==0) return EV_TELNET;
  }
#endif
  else if (device[0]=='S') {
    if (device[1]=='e' && device[2]=='r' && device[3]=='i' && device[4]=='a' && device[5]=='l' && device[6]!=0 && device[7]==0) {
      if (device[6]=='1') return EV_SERIAL1;
      if (device[6]=='2') return EV_SERIAL2;
      if (device[6]=='3') return EV_SERIAL3;
#if USART_COUNT>=4
      if (device[6]=='4') return EV_SERIAL4;
#endif
#if USART_COUNT>=5
      if (device[6]=='5') return EV_SERIAL5;
#endif
#if USART_COUNT>=6
      if (device[6]=='6') return EV_SERIAL6;
#endif
    }
    if (device[1]=='P' && device[2]=='I' && device[3]!=0 && device[4]==0) {
#if SPI_COUNT>=1
      if (device[3]=='1') return EV_SPI1;
#endif
#if SPI_COUNT>=2
      if (device[3]=='2') return EV_SPI2;
#endif
#if SPI_COUNT>=3
      if (device[3]=='3') return EV_SPI3;
#endif
    }
  }
  else if (device[0]=='I' && device[1]=='2' && device[2]=='C' && device[3]!=0 && device[4]==0) {
#if I2C_COUNT>=1
    if (device[3]=='1') return EV_I2C1;
#endif
#if I2C_COUNT>=2
    if (device[3]=='2') return EV_I2C2;
#endif
#if I2C_COUNT>=3
    if (device[3]=='3') return EV_I2C3;
#endif
  }
  return EV_NONE;
}

/// Set whether the host should transmit or not
void jshSetFlowControlXON(IOEventFlags device, bool hostShouldTransmit) {
  if (DEVICE_IS_USART(device)) {
    int devIdx = TO_SERIAL_DEVICE_STATE(device);
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[devIdx];
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
    Pin flowControlPin = jshSerialDeviceCTSPins[devIdx];
    if (flowControlPin != PIN_UNDEFINED)
      jshPinSetValue(flowControlPin, !hostShouldTransmit);
  }
}

/// Gets a device's object from a device, or return 0 if it doesn't exist
JsVar *jshGetDeviceObject(IOEventFlags device) {
  const char *deviceStr = jshGetDeviceString(device);
  if (!deviceStr) return 0;
  return jsvObjectGetChild(execInfo.root, deviceStr, 0);
}

/// Set whether to use flow control on the given device or not. CTS is low when ready, high when not.
void jshSetFlowControlEnabled(IOEventFlags device, bool software, Pin pinCTS) {
  if (!DEVICE_IS_USART(device)) return;
  int devIdx = TO_SERIAL_DEVICE_STATE(device);
  JshSerialDeviceState *deviceState = &jshSerialDeviceStates[devIdx];
  if (software)
    (*deviceState) |= SDS_FLOW_CONTROL_XON_XOFF;
  else
    (*deviceState) &= ~SDS_FLOW_CONTROL_XON_XOFF;

  jshSerialDeviceCTSPins[devIdx] = PIN_UNDEFINED;
  if (jshIsPinValid(pinCTS)) {
    jshPinSetState(pinCTS, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pinCTS, 0); // CTS ready
    jshSerialDeviceCTSPins[devIdx] = pinCTS;
  }
}

/// Set a callback function to be called when an event occurs
void jshSetEventCallback(
    IOEventFlags channel,             //!< The event that fires the callback.
    JshEventCallbackCallback callback //!< The callback to be invoked.
  ) {
  // Save the callback function for this event channel.
  assert(channel>=EV_EXTI0 && channel<=EV_EXTI_MAX);
  jshEventCallbacks[channel-EV_EXTI0] = callback;
}
