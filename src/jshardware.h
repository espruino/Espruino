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
 * Hardware interface Layer
 * NOTE: The definitions of these functions are inside:
 *                                         targets/{target}/jshardware.c
 * ----------------------------------------------------------------------------
 */

#ifndef JSHARDWARE_H_
#define JSHARDWARE_H_

#include "jsutils.h"
#include "jsvar.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jspininfo.h"

#ifdef LINUX
#include <inttypes.h>
#endif

void jshInit();
void jshReset(); // When 'reset' is called - we try and put peripherals back to their power-on state
void jshKill();
void jshIdle(); // stuff to do on idle

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars);

bool jshIsUSBSERIALConnected(); // is the serial device connected?

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime();
/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime time);
/// Convert a time in Milliseconds to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms);
/// Convert ticks to a time in Milliseconds
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time);

// software IO functions...
void jshInterruptOff();
void jshInterruptOn();
void jshDelayMicroseconds(int microsec);
void jshPinSetValue(Pin pin, bool value);
bool jshPinGetValue(Pin pin);
// ------

typedef enum {
  JSHPINSTATE_UNDEFINED,
  JSHPINSTATE_GPIO_OUT,
  JSHPINSTATE_GPIO_OUT_OPENDRAIN,
  JSHPINSTATE_GPIO_IN,
  JSHPINSTATE_GPIO_IN_PULLUP,
  JSHPINSTATE_GPIO_IN_PULLDOWN,
  JSHPINSTATE_ADC_IN,
  JSHPINSTATE_AF_OUT,
  JSHPINSTATE_AF_OUT_OPENDRAIN,
  JSHPINSTATE_USART_IN,
  JSHPINSTATE_USART_OUT,
  JSHPINSTATE_DAC_OUT,
  JSHPINSTATE_I2C,
  JSHPINSTATE_MASK = NEXT_POWER_2(JSHPINSTATE_I2C)-1,

  JSHPINSTATE_PIN_IS_ON = JSHPINSTATE_MASK+1,
} PACKED_FLAGS JshPinState;

#define JSHPINSTATE_IS_OUTPUT(state) ( \
             (state)==JSHPINSTATE_GPIO_OUT ||               \
             (state)==JSHPINSTATE_GPIO_OUT_OPENDRAIN ||     \
             (state)==JSHPINSTATE_AF_OUT ||                 \
             (state)==JSHPINSTATE_AF_OUT_OPENDRAIN ||       \
             (state)==JSHPINSTATE_USART_OUT ||              \
             (state)==JSHPINSTATE_DAC_OUT ||                \
             (state)==JSHPINSTATE_I2C ||                    \
0)
#define JSHPINSTATE_IS_OPENDRAIN(state) ( \
             (state)==JSHPINSTATE_GPIO_OUT_OPENDRAIN ||     \
             (state)==JSHPINSTATE_AF_OUT_OPENDRAIN ||       \
             (state)==JSHPINSTATE_I2C              ||       \
0)

/// Set the pin state
void jshPinSetState(Pin pin, JshPinState state);
/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin);

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin);
/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin);

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq); // if freq<=0, the default is used
void jshPinPulse(Pin pin, bool value, JsVarFloat time);
bool jshCanWatch(Pin pin); ///< Can the given pin be watched? it may not be possible because of conflicts
IOEventFlags jshPinWatch(Pin pin, bool shouldWatch); // start watching pin - return the EXTI associated with it

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin);

/// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value);

/// Enable watchdog with a timeout in seconds
void jshEnableWatchDog(JsVarFloat timeout);

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device);

bool jshIsEventForPin(IOEvent *event, Pin pin);

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device);


#define DEFAULT_BAUD_RATE               9600
#define DEFAULT_BYTESIZE                8
#define DEFAULT_PARITY                  0
#define DEFAULT_STOPBITS                1

typedef struct {
  int baudRate; // FIXME uint32_t ???
  Pin pinRX;
  Pin pinTX;
  unsigned char bytesize;
  unsigned char parity; // 0=none, 1=odd, 2=even
  unsigned char stopbits;
  bool xOnXOff; // XON XOFF flow control?
} PACKED_FLAGS JshUSARTInfo;

static inline void jshUSARTInitInfo(JshUSARTInfo *inf) {
  inf->baudRate = DEFAULT_BAUD_RATE;
  inf->pinRX    = PIN_UNDEFINED;
  inf->pinTX    = PIN_UNDEFINED;
  inf->bytesize = DEFAULT_BYTESIZE;
  inf->parity   = DEFAULT_PARITY; // PARITY_NONE = 0, PARITY_ODD = 1, PARITY_EVEN = 2 FIXME: enum?
  inf->stopbits = DEFAULT_STOPBITS;
  inf->xOnXOff = false;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf);
/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device);

typedef enum {
  SPIF_CPHA = 1,
  SPIF_CPOL = 2,
  SPIF_SPI_MODE_0 = 0,
  SPIF_SPI_MODE_1 = SPIF_CPHA,
  SPIF_SPI_MODE_2 = SPIF_CPOL,
  SPIF_SPI_MODE_3 = SPIF_CPHA | SPIF_CPOL,
  /* Mode   CPOL  CPHA
        0   0     0
        1   0     1
        2   1     0
        3   1     1
    */

} PACKED_FLAGS JshSPIFlags;

typedef enum {
  SPIB_DEFAULT,
  SPIB_MAXIMUM, // baudRate is the maximum we'll choose
  SPIB_MINIMUM,// baudRate is the minimum we'll choose
} PACKED_FLAGS JshBaudFlags;

typedef struct {
  int baudRate;
  JshBaudFlags baudRateSpec;
  Pin pinSCK;
  Pin pinMISO;
  Pin pinMOSI;
  unsigned char spiMode;
  bool spiMSB; // MSB first?
} PACKED_FLAGS JshSPIInfo;
static inline void jshSPIInitInfo(JshSPIInfo *inf) {
  inf->baudRate = 100000;
  inf->baudRateSpec = 0;
  inf->pinSCK = PIN_UNDEFINED;
  inf->pinMISO = PIN_UNDEFINED;
  inf->pinMOSI = PIN_UNDEFINED;
  inf->spiMode = SPIF_SPI_MODE_0;
  inf->spiMSB = true; // MSB first is default
}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf);
/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data);
/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data);
/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16);
/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device);

typedef struct {
  Pin pinSCL;
  Pin pinSDA;
  char slaveAddr; // or -1 if it is master!
  int bitrate;
  // timeout?
} PACKED_FLAGS JshI2CInfo;
static inline void jshI2CInitInfo(JshI2CInfo *inf) {
  inf->pinSCL = PIN_UNDEFINED;
  inf->pinSDA = PIN_UNDEFINED;
  inf->slaveAddr = (char)-1; // master
  inf->bitrate = 50000; // Is what we used - shouldn't it be 100k?
}
/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf);
/** Addresses are 7 bit - that is, between 0 and 0x7F. sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop);
void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop);


/// Save contents of JsVars into Flash
void jshSaveToFlash();
/// Load contents of JsVars from Flash
void jshLoadFromFlash();
/// Returns true if flash contains something useful
bool jshFlashContainsCode();

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake);

/// Utility timer handling functions ------------------------------

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period);
/// Reschedult the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period);
/// Stop the timer
void jshUtilTimerDisable();

// ---------------------------------------------- LOW LEVEL
#ifdef ARM
// ----------------------------------------------------------------------------
//                                                                      SYSTICK
// On SYSTick interrupt, call this
void jshDoSysTick();
#ifdef USB
// Kick the USB SysTick watchdog - we need this to see if we have disconnected or not
void jshKickUSBWatchdog();
#endif

#endif // ARM

#ifdef STM32
// push a byte into SPI buffers
void jshSPIPush(IOEventFlags device, uint16_t data);
#endif

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature();
// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef();

#ifdef STM32F3
#define SPI_I2S_SendData SPI_I2S_SendData16
#define SPI_I2S_ReceiveData SPI_I2S_ReceiveData16
#endif

#ifdef STM32F4
#define WAIT_UNTIL_N_CYCLES 10000000
#else
#define WAIT_UNTIL_N_CYCLES 2000000
#endif
#define WAIT_UNTIL(CONDITION, REASON) { \
    int timeout = WAIT_UNTIL_N_CYCLES;                                              \
    while (!(CONDITION) && !jspIsInterrupted() && (timeout--)>0);                  \
    if (timeout<=0 || jspIsInterrupted()) { jsExceptionHere(JSET_INTERNALERROR, "Timeout on "REASON); }  \
}

#endif /* JSHARDWARE_H_ */
