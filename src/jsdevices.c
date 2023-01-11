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
#include "jswrapper.h"
#ifdef BLUETOOTH
#include "bluetooth.h"
#endif

#ifdef LINUX
#include <stdio.h>
#include <signal.h>
#endif//LINUX
#ifdef USE_TRIGGER
#include "trigger.h"
#endif

// ----------------------------------------------------------------------------
//                                                              WATCH CALLBACKS
#define JSEVENTCALLBACK_PIN_MASK 0xFFFFFF00
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
  SDS_ERROR_HANDLING = 16
} PACKED_FLAGS JshSerialDeviceState;
#define JSHSERIALDEVICESTATUSES (1+EV_SERIAL_MAX-EV_SERIAL_DEVICE_STATE_START)

/// Was flow control ever set? Allows us to save time if it wasn't
bool jshSerialFlowControlWasSet;
/// Info about the current device - eg. is flow control enabled?
volatile JshSerialDeviceState jshSerialDeviceStates[JSHSERIALDEVICESTATUSES];
/// Device clear to send hardware flow control pins (PIN_UNDEFINED if not used)
Pin jshSerialDeviceCTSPins[JSHSERIALDEVICESTATUSES];


// ----------------------------------------------------------------------------
//                                                              IO EVENT BUFFER
#if IOBUFFERMASK<256
typedef uint8_t IOBufferIdx;
#else
typedef uint16_t IOBufferIdx;
#endif

volatile IOEvent ioBuffer[IOBUFFERMASK+1];
volatile IOBufferIdx ioHead=0, ioTail=0;

// ----------------------------------------------------------------------------


/** Initialize any device-specific structures, like flow control states.
 * Called from jshInit */
void jshInitDevices() {
  DEVICE_SANITY_CHECK();
  // Setup USB/Bluetooth flow control separately so
  // we don't reset it for every call to reset()
#ifdef USB
  assert(EV_USBSERIAL>=EV_SERIAL_DEVICE_STATE_START);
  jshSerialDeviceStates[TO_SERIAL_DEVICE_STATE(EV_USBSERIAL)] = SDS_FLOW_CONTROL_XON_XOFF;
#endif
#ifdef BLUETOOTH
  jshSerialDeviceStates[TO_SERIAL_DEVICE_STATE(EV_BLUETOOTH)] = SDS_FLOW_CONTROL_XON_XOFF;
#endif
  // reset everything else...
  jshResetDevices();
}

/** Reset any devices that could have been set up differently by JS code.
 * Called from jshReset */
void jshResetDevices() {
  unsigned int i;
  // Reset list of pins that were set manually
  jshResetPinStateIsManual();
  // setup flow control
  for (i=0;i<JSHSERIALDEVICESTATUSES;i++) {
#ifdef USB
    if (i==TO_SERIAL_DEVICE_STATE(EV_USBSERIAL)) break; // don't update USB status
#endif
#ifdef BLUETOOTH
    if (i==TO_SERIAL_DEVICE_STATE(EV_BLUETOOTH)) break; // don't update Bluetooth status
#endif
    jshSerialDeviceStates[i] = SDS_NONE;
    jshSerialDeviceCTSPins[i] = PIN_UNDEFINED;
  }
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
#ifdef BTN2_PININDEX
#ifdef BTN2_PINSTATE
  jshSetPinStateIsManual(BTN2_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN2_PININDEX, BTN2_PINSTATE);
#else
  jshPinSetState(BTN2_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
#ifdef BTN3_PININDEX
#ifdef BTN3_PINSTATE
  jshSetPinStateIsManual(BTN3_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN3_PININDEX, BTN3_PINSTATE);
#else
  jshPinSetState(BTN3_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
#ifdef BTN4_PININDEX
#ifdef BTN4_PINSTATE
  jshSetPinStateIsManual(BTN4_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN4_PININDEX, BTN4_PINSTATE);
#else
  jshPinSetState(BTN4_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
#ifdef BTN5_PININDEX
#ifdef BTN5_PINSTATE
  jshSetPinStateIsManual(BTN5_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN5_PININDEX, BTN5_PINSTATE);
#else
  jshPinSetState(BTN5_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
#ifdef BTN6_PININDEX
#ifdef BTN6_PINSTATE
  jshSetPinStateIsManual(BTN6_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN6_PININDEX, BTN6_PINSTATE);
#else
  jshPinSetState(BTN6_PININDEX, JSHPINSTATE_GPIO_IN);
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
    telnetSendChar((char)data);
    return;
  }
#endif
#ifdef USE_TERMINAL
  if (device==EV_TERMINAL) {
    extern void terminalSendChar(char c);
    terminalSendChar((char)data);
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
#ifdef BLUETOOTH
  if (device==EV_BLUETOOTH && !jsble_has_peripheral_connection()) {
    jshTransmitClearDevice(EV_BLUETOOTH); // clear out stuff already waiting
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
      if (jshIsInInterrupt()) {
        // if we're printing from an IRQ, don't wait - it's unlikely TX will ever finish
        jsErrorFlags |= JSERR_BUFFER_FULL;
        return;
      }
      jshBusyIdle();
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

static void jshTransmitPrintfCallback(const char *str, void *user_data) {
  IOEventFlags device = (IOEventFlags)user_data;
  while (*str) jshTransmit(device, (unsigned char)*(str++));
}

void jshTransmitPrintf(IOEventFlags device, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vcbprintf(jshTransmitPrintfCallback,(void *)device, fmt, argp);
  va_end(argp);
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
  if (DEVICE_HAS_DEVICE_STATE(device)) {
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

/// Push an IO event into the ioBuffer (designed to be called from IRQ)
void CALLED_FROM_INTERRUPT jshPushEvent(IOEvent *evt) {
  /* Make new buffer
   *
   * We're disabling IRQs for this bit because it's actually quite likely for
   * USB and USART data to be coming in at the same time, and it can trip
   * things up if one IRQ interrupts another. */
  jshInterruptOff();
  IOBufferIdx nextHead = (IOBufferIdx)((ioHead+1) & IOBUFFERMASK);
  if (ioTail == nextHead) {
    jshInterruptOn();
    jshIOEventOverflowed();
    return; // queue full - dump this event!
  }
  ioBuffer[ioHead] = *evt;
  ioHead = nextHead;
  jshInterruptOn();
}

/// Attempt to push characters onto an existing event
static bool jshPushIOCharEventAppend(IOEventFlags channel, char charData) {
  IOBufferIdx lastHead = (IOBufferIdx)((ioHead+IOBUFFERMASK) & IOBUFFERMASK); // one behind head
  if (ioHead!=ioTail && lastHead!=ioTail) {
    // we can do this because we only read in main loop, and we're in an interrupt here
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[lastHead].flags) == channel) {
      unsigned char c = (unsigned char)IOEVENTFLAGS_GETCHARS(ioBuffer[lastHead].flags);
      if (c < IOEVENT_MAXCHARS) {
        // last event was for this event type, and it has chars left
        ioBuffer[lastHead].data.chars[c] = charData;
        IOEVENTFLAGS_SETCHARS(ioBuffer[lastHead].flags, c+1);
        return true; // char added, job done
      }
    }
  }
  return false;
}

/// Try and handle events in the IRQ itself. true if handled and shouldn't go in queue
static bool jshPushIOCharEventHandler(IOEventFlags channel, char charData) {
  // Check for a CTRL+C
  if (charData==3 && channel==jsiGetConsoleDevice()) {
    jsiCtrlC(); // Ctrl-C - force interrupt of execution
    return true;
  }
  return jswOnCharEvent(channel, charData);
}


// Set flow control (as we're going to use more data)
static void jshPushIOCharEventFlowControl(IOEventFlags channel) {
  if (DEVICE_HAS_DEVICE_STATE(channel) && jshGetEventsUsed() > IOBUFFER_XOFF)
    jshSetFlowControlXON(channel, false);
}

/// Send a character to the specified device.
void jshPushIOCharEvent(
    IOEventFlags channel, // !< The device to target for output.
    char charData         // !< The character to send to the device.
  ) {
  // See if we need to handle this in the IRQ
  if (jshPushIOCharEventHandler(channel, charData)) return;
  // Check if we can push into existing buffer (we must have at least 2 in the queue to avoid dropping chars though!)
  if (jshPushIOCharEventAppend(channel, charData)) return;

  IOEvent evt;
  evt.flags = channel;
  IOEVENTFLAGS_SETCHARS(evt.flags, 1);
  evt.data.chars[0] = charData;
  jshPushEvent(&evt);
  // Set flow control (as we're going to use more data)
  jshPushIOCharEventFlowControl(channel);
}

void jshPushIOCharEvents(IOEventFlags channel, char *data, unsigned int count) {
  // TODO: optimise me!
  unsigned int i;
  for (i=0;i<count;i++) jshPushIOCharEvent(channel, data[i]);
}

/* Signal an IO watch event as having happened.
On the esp8266 we need this to be loaded into static RAM because it can run at interrupt time */
void CALLED_FROM_INTERRUPT jshPushIOWatchEvent(
    IOEventFlags channel //!< The channel on which the IO watch event has happened.
  ) {
  assert(channel >= EV_EXTI0 && channel <= EV_EXTI_MAX);

  bool state = jshGetWatchedPinState(channel);

  // If there is a callback or pin associated with this GPIO event
  // the handle it
  int evt = channel-EV_EXTI0;
  if (jshEventCallbacks[evt]) {
    if (((uint32_t)jshEventCallbacks[evt] & JSEVENTCALLBACK_PIN_MASK)==JSEVENTCALLBACK_PIN_MASK) {
      // It's a pin, read the value and store it in the event channel
      Pin pin = (Pin)((uint32_t)jshEventCallbacks[evt] &~ JSEVENTCALLBACK_PIN_MASK);
      if (jshPinGetValue(pin)) channel |= EV_EXTI_DATA_PIN_HIGH;
    } else {
      // It's a callback - invoke and return
      jshEventCallbacks[evt](state, channel);
      return;
    }
  }

  if (state) channel |= EV_EXTI_IS_HIGH;

  JsSysTime time = jshGetSystemTime();

#ifdef USE_TRIGGER
  // TODO: move to using jshSetEventCallback
  if (trigHandleEXTI(channel, time))
    return;
#endif
  // Otherwise add this event
  jshPushIOEvent(channel, time);
}

/// Add this IO event to the IO event queue.
void CALLED_FROM_INTERRUPT jshPushIOEvent(
    IOEventFlags channel, //!< The event to add to the queue.
    JsSysTime time        //!< The time that the event is thought to have happened.
  ) {
  IOEvent evt;
  evt.flags = channel;
  evt.data.time = (unsigned int)time;
  jshPushEvent(&evt);
}

// returns true on success
bool jshPopIOEvent(IOEvent *result) {
  if (ioHead==ioTail) return false;
  *result = ioBuffer[ioTail];
  ioTail = (IOBufferIdx)((ioTail+1) & IOBUFFERMASK);
  return true;
}

// returns true on success
bool jshPopIOEventOfType(IOEventFlags eventType, IOEvent *result) {
  // Special case for top - it's easier!
  if (IOEVENTFLAGS_GETTYPE(ioBuffer[ioTail].flags) == eventType)
    return jshPopIOEvent(result);
  // Now check non-top
  IOBufferIdx i = ioTail;
  while (ioHead!=i) {
    if (IOEVENTFLAGS_GETTYPE(ioBuffer[i].flags) == eventType) {
      /* We need IRQ off for this, because if we get data it's possible
      that the IRQ will push data and will try and add characters to this
      exact position in the buffer */
      jshInterruptOff();
      *result = ioBuffer[i];
      // work back and shift all items in out queue
      IOBufferIdx n = (IOBufferIdx)((i+IOBUFFERMASK) & IOBUFFERMASK);
      while (n!=ioTail) {
        ioBuffer[i] = ioBuffer[n];
        i = n;
        n = (IOBufferIdx)((n+IOBUFFERMASK) & IOBUFFERMASK);
      }
      // finally update the tail pointer, and return
      ioTail = (IOBufferIdx)((ioTail+1) & IOBUFFERMASK);
      jshInterruptOn();
      return true;
    }
    i = (IOBufferIdx)((i+1) & IOBUFFERMASK);
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
  case EV_NONE: return "null";
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
#ifdef USE_TERMINAL
  case EV_TERMINAL: return "Terminal";
#endif
#if USART_COUNT>=1
  case EV_SERIAL1: return "Serial1";
#endif
#if USART_COUNT>=2
  case EV_SERIAL2: return "Serial2";
#endif
#if USART_COUNT>=3
  case EV_SERIAL3: return "Serial3";
#endif
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
  if (device[0]=='T') {
#ifdef USE_TELNET
     if (strcmp(&device[1], "elnet")==0) return EV_TELNET;
#endif
#ifdef USE_TERMINAL
     if (strcmp(&device[1], "erminal")==0) return EV_TERMINAL;
#endif
  }
  else if (device[0]=='S') {
#if USART_COUNT>0
  if (device[1]=='e' && device[2]=='r' && device[3]=='i' && device[4]=='a' && device[5]=='l' &&
      device[6]>='1' && (device[6]-'1')<USART_COUNT &&
      device[7]==0)
    return EV_SERIAL1+device[6]-'1';
#endif
#if SPI_COUNT>0
  if (device[1]=='P' && device[2]=='I' &&
      device[3]>='1' && (device[3]-'1')<SPI_COUNT &&
      device[4]==0)
    return EV_SPI1+device[3]-'1';
#endif
  }
#if I2C_COUNT>0
  else if (device[0]=='I' && device[1]=='2' && device[2]=='C' &&
           device[3]>='1' && (device[3]-'1')<I2C_COUNT &&
           device[4]==0)
    return EV_I2C1+device[3]-'1';
#endif
  return EV_NONE;
}

/// Set whether the host should transmit or not
void jshSetFlowControlXON(IOEventFlags device, bool hostShouldTransmit) {
  if (DEVICE_HAS_DEVICE_STATE(device)) {
    if (!hostShouldTransmit)
      jshSerialFlowControlWasSet = true;
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

/// To be called on idle when the input queue has enough space
void jshSetFlowControlAllReady() {
  if (!jshSerialFlowControlWasSet)
    return; // nothing to do!
  for (int i=0;i<JSHSERIALDEVICESTATUSES;i++)
    jshSetFlowControlXON(EV_SERIAL_DEVICE_STATE_START+i, true);
  jshSerialFlowControlWasSet = false;
}

/// Gets a device's object from a device, or return 0 if it doesn't exist
JsVar *jshGetDeviceObject(IOEventFlags device) {
  const char *deviceStr = jshGetDeviceString(device);
  if (!deviceStr) return 0;
  return jsvObjectGetChild(execInfo.root, deviceStr, 0);
}

/// Set whether to use flow control on the given device or not. CTS is low when ready, high when not.
void jshSetFlowControlEnabled(IOEventFlags device, bool software, Pin pinCTS) {
  if (DEVICE_HAS_DEVICE_STATE(device)) {
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
}

/// Set a callback function to be called when an event occurs. Shares same storage as jshSetEventDataPin
void jshSetEventCallback(IOEventFlags channel, JshEventCallbackCallback callback) {
  assert(channel>=EV_EXTI0 && channel<=EV_EXTI_MAX);
  jshEventCallbacks[channel-EV_EXTI0] = callback;
}

/// Set a data pin to be read when an event occurs. Shares same storage as jshSetEventCallback
void jshSetEventDataPin(IOEventFlags channel, Pin pin) {
  assert(channel>=EV_EXTI0 && channel<=EV_EXTI_MAX);
  jshEventCallbacks[channel-EV_EXTI0] = (void*)(uint32_t)(JSEVENTCALLBACK_PIN_MASK | pin);
}

/// Get a data pin to be read when an event occurs
Pin jshGetEventDataPin(IOEventFlags channel) {
  assert(channel>=EV_EXTI0 && channel<=EV_EXTI_MAX);
  int evt = channel-EV_EXTI0;
  if (((uint32_t)jshEventCallbacks[evt] & JSEVENTCALLBACK_PIN_MASK) == JSEVENTCALLBACK_PIN_MASK)
    return (Pin)((uint32_t)jshEventCallbacks[evt] & ~JSEVENTCALLBACK_PIN_MASK);
  return PIN_UNDEFINED;
}

void jshSetErrorHandlingEnabled(IOEventFlags device, bool errorHandling) {
  if (DEVICE_HAS_DEVICE_STATE(device)) {
    int devIdx = TO_SERIAL_DEVICE_STATE(device);
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[devIdx];
    if (errorHandling)
      (*deviceState) |= SDS_ERROR_HANDLING;
    else
      (*deviceState) &= ~SDS_ERROR_HANDLING;
  }
}

bool jshGetErrorHandlingEnabled(IOEventFlags device) {
  if (DEVICE_HAS_DEVICE_STATE(device)) {
    int devIdx = TO_SERIAL_DEVICE_STATE(device);
    JshSerialDeviceState *deviceState = &jshSerialDeviceStates[devIdx];
    return (SDS_ERROR_HANDLING & *deviceState)!=0;
  } else
    return false;
}
