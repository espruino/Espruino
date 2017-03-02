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
 * Utilities and definitions for handling Pins
 * ----------------------------------------------------------------------------
 */

#ifndef JSPIN_H
#define JSPIN_H

#include "jsutils.h"
#include "jsvar.h"
#include "jsdevices.h"

typedef unsigned char Pin; ///< for specifying pins for hardware
#define PIN_UNDEFINED ((Pin)0xFF)

typedef enum {
  JSH_PORT_NONE,
  JSH_PORTA=1,
  JSH_PORTB,
  JSH_PORTC,
  JSH_PORTD,
  JSH_PORTE,
  JSH_PORTF,
  JSH_PORTG,
  JSH_PORTH,
} PACKED_FLAGS JsvPinInfoPort;

typedef enum {
  JSH_PIN0 = 0,
  JSH_PIN1,
  JSH_PIN2,
  JSH_PIN3,
  JSH_PIN4,
  JSH_PIN5,
  JSH_PIN6,
  JSH_PIN7,
  JSH_PIN8,
  JSH_PIN9,
  JSH_PIN10,
  JSH_PIN11,
  JSH_PIN12,
  JSH_PIN13,
  JSH_PIN14,
  JSH_PIN15,
  // and there may be more on Linux systems...
} PACKED_FLAGS JsvPinInfoPin;

typedef enum {
  JSH_ANALOG_NONE = 0,
  JSH_ANALOG1     = 32,
  JSH_ANALOG2     = 64,
  JSH_ANALOG3     = 128,
  JSH_ANALOG4     = 256,
  JSH_ANALOG12    = JSH_ANALOG1|JSH_ANALOG2,
  JSH_ANALOG123   = JSH_ANALOG1|JSH_ANALOG2|JSH_ANALOG3,
  JSH_ANALOG34    = JSH_ANALOG3|JSH_ANALOG4,

  JSH_ANALOG_CH0 = 0,
  JSH_ANALOG_CH1,
  JSH_ANALOG_CH2,
  JSH_ANALOG_CH3,
  JSH_ANALOG_CH4,
  JSH_ANALOG_CH5,
  JSH_ANALOG_CH6,
  JSH_ANALOG_CH7,
  JSH_ANALOG_CH8,
  JSH_ANALOG_CH9,
  JSH_ANALOG_CH10,
  JSH_ANALOG_CH11,
  JSH_ANALOG_CH12,
  JSH_ANALOG_CH13,
  JSH_ANALOG_CH14,
  JSH_ANALOG_CH15,
  JSH_ANALOG_CH16,
  JSH_ANALOG_CH17,

  JSH_MASK_ANALOG_CH  = 31,
  JSH_MASK_ANALOG_ADC = JSH_ANALOG1|JSH_ANALOG2|JSH_ANALOG3|JSH_ANALOG4,

} PACKED_FLAGS JsvPinInfoAnalog;

typedef enum {
  // ---------------------------- JSH_MASK_AF
  JSH_AF0 = 0,
  JSH_AF1,
  JSH_AF2,
  JSH_AF3,
  JSH_AF4,
  JSH_AF5,
  JSH_AF6,
  JSH_AF7,
  JSH_AF8,
  JSH_AF9,
  JSH_AF10,
  JSH_AF11,
  JSH_AF12,
  JSH_AF13,
  JSH_AF14,
  JSH_AF15,

  // ---------------------------- JSH_MASK_TYPE
  JSH_NOTHING = 0,
  JSH_TIMER1  = 0x0010,
  JSH_TIMER2  = 0x0020,
  JSH_TIMER3  = 0x0030,
  JSH_TIMER4  = 0x0040,
  JSH_TIMER5  = 0x0050,
  JSH_TIMER6  = 0x0060,
  JSH_TIMER7  = 0x0070,
  JSH_TIMER8  = 0x0080,
  JSH_TIMER9  = 0x0090,
  JSH_TIMER10 = 0x00A0,
  JSH_TIMER11 = 0x00B0,
  JSH_TIMER12 = 0x00C0,
  JSH_TIMER13  = 0x00D0,
  JSH_TIMER14  = 0x00E0,
  JSH_TIMER15  = 0x00F0,
  JSH_TIMER16  = 0x0100,
  JSH_TIMER17  = 0x0110,
  JSH_TIMER18  = 0x0120,
  JSH_TIMERMAX = JSH_TIMER18,
  JSH_DAC      = 0x0180,
  JSH_SPI1     = 0x0200,
  JSH_SPI2     = 0x0210,
  JSH_SPI3     = 0x0220,
  JSH_SPIMAX   = JSH_SPI3,
  JSH_I2C1     = 0x0280,
  JSH_I2C2     = 0x0290,
  JSH_I2C3     = 0x02A0,
  JSH_I2CMAX   = JSH_I2C3,
  JSH_USART1   = 0x0300,
  JSH_USART2   = 0x0310,
  JSH_USART3   = 0x0320,
  JSH_USART4   = 0x0330,
  JSH_USART5   = 0x0340,
  JSH_USART6   = 0x0350,
  JSH_USARTMAX = JSH_USART6,

  // ---------------------------- JSH_MASK_INFO

  JSH_TIMER_CH1 = 0x0000,
  JSH_TIMER_CH2 = 0x1000,
  JSH_TIMER_CH3 = 0x2000,
  JSH_TIMER_CH4 = 0x3000,
  JSH_MASK_TIMER_CH = 0x7000,
  JSH_TIMER_NEGATED = 0x8000,

  JSH_USART_RX = 0x0000,
  JSH_USART_TX = 0x1000,
  JSH_USART_CK = 0x2000,

  JSH_SPI_MISO = 0x0000,
  JSH_SPI_MOSI = 0x1000,
  JSH_SPI_SCK  = 0x2000,

  JSH_I2C_SCL  = 0x0000,
  JSH_I2C_SDA  = 0x1000,

  JSH_DAC_CH1 = 0x0000,
  JSH_DAC_CH2 = 0x1000,

  // ---------------------------- Masks
  JSH_MASK_AF = 0x000F,
  JSH_MASK_TYPE = 0x0FF0,
  JSH_MASK_INFO = 0xF000,
  JSH_SHIFT_TYPE = 4, // amount type is shifted left
  JSH_SHIFT_INFO = 12, // amount info is shifted left
} PACKED_FLAGS JshPinFunction;

#define JSH_PINFUNCTION_IS_TIMER(F) ( \
  (((F)&JSH_MASK_TYPE)>=JSH_TIMER1) && \
  (((F)&JSH_MASK_TYPE)<=JSH_TIMER18))
#define JSH_PINFUNCTION_IS_DAC(F) ( \
  (((F)&JSH_MASK_TYPE)==JSH_DAC))
#define JSH_PINFUNCTION_IS_USART(F) ( \
  (((F)&JSH_MASK_TYPE)>=JSH_USART1) && \
  (((F)&JSH_MASK_TYPE)<=JSH_USART6))
#define JSH_PINFUNCTION_IS_I2C(F) ( \
  (((F)&JSH_MASK_TYPE)>=JSH_I2C1) && \
  (((F)&JSH_MASK_TYPE)<=JSH_I2CMAX))
#define JSH_PINFUNCTION_IS_SPI(F) ( \
  (((F)&JSH_MASK_TYPE)>=JSH_SPI1) && \
  (((F)&JSH_MASK_TYPE)<=JSH_SPIMAX))

bool jshIsPinValid(Pin pin); ///< is the specific pin actually valid?

/// Given a string, convert it to a pin ID (or -1 if it doesn't exist)
Pin jshGetPinFromString(const char *s);
/** Write the pin name to a string. String must have at least 10 characters (to be safe) */
void jshGetPinString(char *result, Pin pin);

/// Given a var, convert it to a pin ID (or -1 if it doesn't exist). safe for undefined!
Pin jshGetPinFromVar(JsVar *pinv);
Pin jshGetPinFromVarAndUnLock(JsVar *pinv);


/// Is the pin state manual (has the user asked us explicitly to change it?)
bool jshGetPinStateIsManual(Pin pin);
/// Set whether the pin state is manual (has the user asked us explicitly to change it?)
void jshSetPinStateIsManual(Pin pin, bool manual);
// Reset our list of which pins are set manually - called from jshResetDevices
void jshResetPinStateIsManual();

bool jshPinInput(Pin pin);
void jshPinOutput(Pin pin, bool value);


// Convert an event type flag into a jshPinFunction for an actual hardware device
JshPinFunction jshGetPinFunctionFromDevice(IOEventFlags device);
// Convert a jshPinFunction to an event type flag
IOEventFlags jshGetFromDevicePinFunction(JshPinFunction func);

/** Try and find a specific type of function for the given pin. Can be given an invalid pin and will return 0. */
JshPinFunction NO_INLINE jshGetPinFunctionForPin(Pin pin, JshPinFunction functionType);

/** Try and find the best pin suitable for the given function. Can return -1. */
Pin NO_INLINE jshFindPinForFunction(JshPinFunction functionType, JshPinFunction functionInfo);
/// Flags for jshPinFunctionToString
typedef enum {
  JSPFTS_DEVICE        = 1, ///< The device itself
  JSPFTS_DEVICE_NUMBER = 2, ///< The device's number
  JSPFTS_SPACE         = 4, ///< A space between device and pin type
  JSPFTS_TYPE          = 8,  ///< The pin type (RX/TX/etc)
  JSPFTS_JS_NAMES      = 16 //< Use JS names - eg. Serial, not USART
} JshPinFunctionToStringFlags;

/// Given a full pin function, return a string describing it depending of what's in the flags enum
void jshPinFunctionToString(JshPinFunction pinFunc, JshPinFunctionToStringFlags flags, char *buf, size_t bufSize);

/** Prints a list of capable pins, eg:
 jshPrintCapablePins(..., "PWM", JSH_TIMER1, JSH_TIMERMAX, 0,0, false)
 jshPrintCapablePins(..., "SPI", JSH_SPI1, JSH_SPIMAX, JSH_MASK_INFO,JSH_SPI_SCK, false)
 jshPrintCapablePins(..., "Analog Input", 0,0,0,0, true) - for analogs */
void NO_INLINE jshPrintCapablePins(Pin existingPin, const char *functionName, JshPinFunction typeMin, JshPinFunction typeMax, JshPinFunction pMask, JshPinFunction pData, bool printAnalogs);

/** Find a device of the given type that works on the given pin. For instance:
 * `jshGetDeviceFor(JSH_SPI1, JSH_SPIMAX, pin);
 */
JshPinFunction jshGetDeviceFor(JshPinFunction deviceMin, JshPinFunction deviceMax, Pin pin);

/** Like jshGetDeviceFor, but returns an actual Object (eg. SPI) if one can be found. */
JsVar *jshGetDeviceObjectFor(JshPinFunction deviceMin, JshPinFunction deviceMax, Pin pin);

#endif //JSPIN_H
