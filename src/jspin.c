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

#include "jspin.h"
#include "jspininfo.h" // auto-generated
#include "jsinteractive.h"
#include "jshardware.h"

// jshGetDeviceObjectFor
#include "jswrapper.h"

#if defined(PICO) || defined(ESPRUINOWIFI) || defined(NUCLEOF401RE) || defined(NUCLEOF411RE)
#define PIN_NAMES_DIRECT // work out pin names directly from port + pin in pinInfo
#endif

// ----------------------------------------------------------------------------

// Whether a pin's state has been set manually or not
BITFIELD_DECL(jshPinStateIsManual, JSH_PIN_COUNT);

// ----------------------------------------------------------------------------


bool jshIsPinValid(Pin pin) {
  // Note, PIN_UNDEFINED is always > JSH_PIN_COUNT
  return pin < JSH_PIN_COUNT && pinInfo[pin].port != JSH_PORT_NONE;
}

Pin jshGetPinFromString(const char *s) {
  if ((s[0]>='A' && s[0]<='H') && s[1]) {
    int port = JSH_PORTA+s[0]-'A';
    int pin = -1;
    if (s[1]>='0' && s[1]<='9') {
      if (!s[2]) { // D0-D9
        pin = (s[1]-'0');
      } else if (s[2]>='0' && s[2]<='9') {
        if (!s[3]) {
          pin = ((s[1]-'0')*10 + (s[2]-'0'));
        } else if (!s[4] && s[3]>='0' && s[3]<='9') {
          pin = ((s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0'));
        }
      }
    }
    if (pin>=0) {
#ifdef PIN_NAMES_DIRECT
      int i;
      for (i=0;i<JSH_PIN_COUNT;i++)
        if (pinInfo[i].port == port && pinInfo[i].pin==pin)
          return (Pin)i;
#else
      if (port == JSH_PORTA) {
        if (pin<JSH_PORTA_COUNT) return (Pin)(JSH_PORTA_OFFSET + pin);
      } else if (port == JSH_PORTB) {
        if (pin<JSH_PORTB_COUNT) return (Pin)(JSH_PORTB_OFFSET + pin);
      } else if (port == JSH_PORTC) {
        if (pin<JSH_PORTC_COUNT) return (Pin)(JSH_PORTC_OFFSET + pin);
      } else if (port == JSH_PORTD) {
        if (pin<JSH_PORTD_COUNT) return (Pin)(JSH_PORTD_OFFSET + pin);
#if JSH_PORTE_OFFSET!=-1
      } else if (port == JSH_PORTE) {
        if (pin<JSH_PORTE_COUNT) return (Pin)(JSH_PORTE_OFFSET + pin);
#endif
#if JSH_PORTF_OFFSET!=-1
      } else if (port == JSH_PORTF) {
        if (pin<JSH_PORTF_COUNT) return (Pin)(JSH_PORTF_OFFSET + pin);
#endif
#if JSH_PORTG_OFFSET!=-1
      } else if (port == JSH_PORTG) {
        if (pin<JSH_PORTG_COUNT) return (Pin)(JSH_PORTG_OFFSET + pin);
#endif
#if JSH_PORTH_OFFSET!=-1
      } else if (port == JSH_PORTH) {
        if (pin<JSH_PORTH_COUNT) return (Pin)(JSH_PORTH_OFFSET + pin);
#endif
      }
#endif
    }
  }

  return PIN_UNDEFINED;
}

/** Write the pin name to a string. String must have at least 10 characters (to be safe) */
void jshGetPinString(char *result, Pin pin) {
  result[0] = 0; // just in case
#ifdef PIN_NAMES_DIRECT
  if (jshIsPinValid(pin)) {
    result[0] = (char)('A'+pinInfo[pin].port-JSH_PORTA);
    itostr(pinInfo[pin].pin-JSH_PIN0,&result[1],10);
#else
    if (
#if JSH_PORTA_OFFSET!=0
        pin>=JSH_PORTA_OFFSET &&
#endif
        pin<JSH_PORTA_OFFSET+JSH_PORTA_COUNT) {
      result[0]='A';
      itostr(pin-JSH_PORTA_OFFSET,&result[1],10);
    } else if (pin>=JSH_PORTB_OFFSET && pin<JSH_PORTB_OFFSET+JSH_PORTB_COUNT) {
      result[0]='B';
      itostr(pin-JSH_PORTB_OFFSET,&result[1],10);
    } else if (pin>=JSH_PORTC_OFFSET && pin<JSH_PORTC_OFFSET+JSH_PORTC_COUNT) {
      result[0]='C';
      itostr(pin-JSH_PORTC_OFFSET,&result[1],10);
    } else if (pin>=JSH_PORTD_OFFSET && pin<JSH_PORTD_OFFSET+JSH_PORTD_COUNT) {
      result[0]='D';
      itostr(pin-JSH_PORTD_OFFSET,&result[1],10);
#if JSH_PORTE_OFFSET!=-1
    } else if (pin>=JSH_PORTE_OFFSET && pin<JSH_PORTE_OFFSET+JSH_PORTE_COUNT) {
      result[0]='E';
      itostr(pin-JSH_PORTE_OFFSET,&result[1],10);
#endif
#if JSH_PORTF_OFFSET!=-1
    } else if (pin>=JSH_PORTF_OFFSET && pin<JSH_PORTF_OFFSET+JSH_PORTF_COUNT) {
      result[0]='F';
      itostr(pin-JSH_PORTF_OFFSET,&result[1],10);
#endif
#if JSH_PORTG_OFFSET!=-1
    } else if (pin>=JSH_PORTG_OFFSET && pin<JSH_PORTG_OFFSET+JSH_PORTG_COUNT) {
      result[0]='G';
      itostr(pin-JSH_PORTG_OFFSET,&result[1],10);
#endif
#if JSH_PORTH_OFFSET!=-1
    } else if (pin>=JSH_PORTH_OFFSET && pin<JSH_PORTH_OFFSET+JSH_PORTH_COUNT) {
      result[0]='H';
      itostr(pin-JSH_PORTH_OFFSET,&result[1],10);
#endif
#endif
    } else {
      strncpy(result, "undefined", 10);
    }
  }

/**
 * Given a var, convert it to a pin ID (or PIN_UNDEFINED if it doesn't exist). safe for undefined!
 */
Pin jshGetPinFromVar(
    JsVar *pinv //!< The class instance representing a Pin.
  ) {
  if (jsvIsString(pinv) && pinv->varData.str[5]==0/*should never be more than 4 chars!*/) {
    return jshGetPinFromString(&pinv->varData.str[0]);
  } else if (jsvIsInt(pinv) /* This also tests for the Pin datatype */) {
    return (Pin)jsvGetInteger(pinv);
  } else return PIN_UNDEFINED;
}

Pin jshGetPinFromVarAndUnLock(JsVar *pinv) {
  Pin pin = jshGetPinFromVar(pinv);
  jsvUnLock(pinv);
  return pin;
}

  // ----------------------------------------------------------------------------

bool jshGetPinStateIsManual(Pin pin) {
  return BITFIELD_GET(jshPinStateIsManual, pin);
}

void jshSetPinStateIsManual(Pin pin, bool manual) {
  BITFIELD_SET(jshPinStateIsManual, pin, manual);
}

// Reset our list of which pins are set manually - called from jshResetDevices
void jshResetPinStateIsManual() {
  BITFIELD_CLEAR(jshPinStateIsManual);
}


  // ----------------------------------------------------------------------------

/**
 * Get the value of a pin.
 * \return The value of the pin.
 */
bool jshPinInput(
    Pin pin //!< The pin to have the value retrieved.
  ) {
  bool value = false;
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin))
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

    value = jshPinGetValue(pin);
  }
  // Handle pin being invalid.
  else jsExceptionHere(JSET_ERROR, "Invalid pin!");
  return value;
}


/**
 * Set the value of a pin.
 */
void jshPinOutput(
    Pin pin,   //!< The pin to set.
    bool value //!< The new value to set on the pin.
  ) {
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin))
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
  }
  // Handle pin being invalid.
  else jsExceptionHere(JSET_ERROR, "Invalid pin!");
}


// ----------------------------------------------------------------------------

// Convert an event type flag into a jshPinFunction for an actual hardware device
JshPinFunction jshGetPinFunctionFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_SERIAL1 : return JSH_USART1;
   case EV_SERIAL2 : return JSH_USART2;
   case EV_SERIAL3 : return JSH_USART3;
   case EV_SERIAL4 : return JSH_USART4;
   case EV_SERIAL5 : return JSH_USART5;
   case EV_SERIAL6 : return JSH_USART6;

   case EV_SPI1    : return JSH_SPI1;
   case EV_SPI2    : return JSH_SPI2;
   case EV_SPI3    : return JSH_SPI3;

   case EV_I2C1    : return JSH_I2C1;
   case EV_I2C2    : return JSH_I2C2;
   case EV_I2C3    : return JSH_I2C3;
   default: return 0;
 }
}

// Convert a jshPinFunction to an event type flag
IOEventFlags jshGetFromDevicePinFunction(JshPinFunction func) {
 switch (func & JSH_MASK_TYPE) {
   case JSH_USART1 : return EV_SERIAL1;
   case JSH_USART2 : return EV_SERIAL2;
   case JSH_USART3 : return EV_SERIAL3;
   case JSH_USART4 : return EV_SERIAL4;
   case JSH_USART5 : return EV_SERIAL5;
   case JSH_USART6 : return EV_SERIAL6;

   case JSH_SPI1    : return EV_SPI1;
   case JSH_SPI2    : return EV_SPI2;
   case JSH_SPI3    : return EV_SPI3;

   case JSH_I2C1    : return EV_I2C1;
   case JSH_I2C2    : return EV_I2C2;
   case JSH_I2C3    : return EV_I2C3;
   default: return 0;
 }
}

/** Try and find a specific type of function for the given pin. Can be given an invalid pin and will return 0. */
JshPinFunction NO_INLINE jshGetPinFunctionForPin(Pin pin, JshPinFunction functionType) {
  if (!jshIsPinValid(pin)) return 0;
  int i;
  for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
    if ((pinInfo[pin].functions[i]&JSH_MASK_TYPE) == functionType)
      return pinInfo[pin].functions[i];
  }
  return 0;
}

/** Try and find the best pin suitable for the given function. Can return -1. */
Pin NO_INLINE jshFindPinForFunction(JshPinFunction functionType, JshPinFunction functionInfo) {
#ifdef OLIMEXINO_STM32
  /** Hack, as you can't mix AFs on the STM32F1, and Olimexino reordered the pins
   * such that D4(AF1) is before D11(AF0) - and there are no SCK/MISO for AF1! */
  if (functionType == JSH_SPI1 && functionInfo==JSH_SPI_MOSI) return JSH_PORTD_OFFSET+11;
#endif
#ifdef PICO
  /* On the Pico, A9 is used for sensing when USB power is applied. Is someone types in
   * Serial1.setup(9600) it'll get chosen as it's the first pin, but setting it to an output
   * totally messes up the STM32 as it's fed with 5V. This ensures that it won't get chosen
   * UNLESS it is explicitly selected.
   *
   * TODO: better way of doing this? A JSH_DONT_DEFAULT flag for pin functions? */
  if (functionType == JSH_USART1) {
    if (functionInfo==JSH_USART_TX) return JSH_PORTB_OFFSET+6;
    if (functionInfo==JSH_USART_RX) return JSH_PORTB_OFFSET+7;
  }
#endif
  Pin i;
  int j;
  // first, try and find the pin with an AF of 0 - this is usually the 'default'
  for (i=0;i<JSH_PIN_COUNT;i++)
    for (j=0;j<JSH_PININFO_FUNCTIONS;j++)
      if ((pinInfo[i].functions[j]&JSH_MASK_AF) == JSH_AF0 &&
          (pinInfo[i].functions[j]&JSH_MASK_TYPE) == functionType &&
          (pinInfo[i].functions[j]&JSH_MASK_INFO) == functionInfo)
        return i;
  // otherwise just try and find anything
  for (i=0;i<JSH_PIN_COUNT;i++)
    for (j=0;j<JSH_PININFO_FUNCTIONS;j++)
      if ((pinInfo[i].functions[j]&JSH_MASK_TYPE) == functionType &&
          (pinInfo[i].functions[j]&JSH_MASK_INFO) == functionInfo)
        return i;
  return PIN_UNDEFINED;
}

/// Given a full pin function, return a string describing it depending of what's in the flags enum
void jshPinFunctionToString(JshPinFunction pinFunc, JshPinFunctionToStringFlags flags, char *buf, size_t bufSize) {
  const char *devStr = "";
  JshPinFunction info = JSH_MASK_INFO & pinFunc;
  JshPinFunction firstDevice = 0;
  const char *infoStr = 0;
  buf[0]=0;
  if (JSH_PINFUNCTION_IS_USART(pinFunc)) {
    devStr=(flags&JSPFTS_JS_NAMES)?"Serial":"USART";
    firstDevice=JSH_USART1;
    if (info==JSH_USART_RX) infoStr="RX";
    else if (info==JSH_USART_TX) infoStr="TX";
    else if (info==JSH_USART_CK) infoStr="CK";
  } else if (JSH_PINFUNCTION_IS_SPI(pinFunc)) {
    devStr="SPI";
    firstDevice=JSH_SPI1;
    if (info==JSH_SPI_MISO) infoStr="MISO";
    else if (info==JSH_SPI_MOSI) infoStr="MOSI";
    else if (info==JSH_SPI_SCK) infoStr="SCK";
  } else if (JSH_PINFUNCTION_IS_I2C(pinFunc)) {
    devStr="I2C";
    firstDevice=JSH_I2C1;
    if (info==JSH_I2C_SCL) infoStr="SCL";
    else if (info==JSH_I2C_SDA) infoStr="SDA";
  } else if (JSH_PINFUNCTION_IS_DAC(pinFunc)) {
     devStr="DAC";
     firstDevice=JSH_DAC;
     if (info==JSH_DAC_CH1) infoStr="CH1";
     else if (info==JSH_DAC_CH2) infoStr="CH2";
  } else if (JSH_PINFUNCTION_IS_TIMER(pinFunc)) {
     devStr="TIM";
     firstDevice=JSH_TIMER1;
     char infoStrBuf[5];
     infoStr = &infoStrBuf[0];
     infoStrBuf[0] = 'C';
     infoStrBuf[1] = 'H';
     infoStrBuf[2] = (char)('1' + ((info&JSH_MASK_TIMER_CH)>>JSH_SHIFT_INFO));
     if (info & JSH_TIMER_NEGATED) {
       infoStrBuf[3]='N';
       infoStrBuf[4] = 0;
     } else {
       infoStrBuf[3] = 0;
     }
   }
  int devIdx = 1 + ((((pinFunc&JSH_MASK_TYPE) - firstDevice) >> JSH_SHIFT_TYPE));

  if (!devStr) {
    jsiConsolePrintf("Couldn't convert pin function %d\n", pinFunc);
    return;
  }
  if (flags & JSPFTS_DEVICE) strncat(buf, devStr, bufSize);
  if (flags & JSPFTS_DEVICE_NUMBER) itostr(devIdx, &buf[strlen(buf)], 10);
  if (flags & JSPFTS_SPACE) strncat(buf, " ", bufSize);
  if (infoStr && (flags & JSPFTS_TYPE)) strncat(buf, infoStr, bufSize);
}

/** Prints a list of capable pins, eg:
 jshPrintCapablePins(..., "PWM", JSH_TIMER1, JSH_TIMERMAX, 0,0, false)
 jshPrintCapablePins(..., "SPI", JSH_SPI1, JSH_SPIMAX, JSH_MASK_INFO,JSH_SPI_SCK, false)
 jshPrintCapablePins(..., "Analog Input", 0,0,0,0, true) - for analogs */
void NO_INLINE jshPrintCapablePins(Pin existingPin, const char *functionName, JshPinFunction typeMin, JshPinFunction typeMax, JshPinFunction pMask, JshPinFunction pData, bool printAnalogs) {
  if (functionName) {
    jsError("Pin %p is not capable of %s\nSuitable pins are:", existingPin, functionName);
  }

  Pin pin;
  int i,n=0;
  for (pin=0;pin<JSH_PIN_COUNT;pin++) {
    bool has = false;
#ifdef STM32F1
    int af = 0;
#endif
    if (printAnalogs) {
      has = pinInfo[pin].analog!=JSH_ANALOG_NONE;
    } else {
      for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
        JshPinFunction type = pinInfo[pin].functions[i] & JSH_MASK_TYPE;
        if (type>=typeMin && type<=typeMax && ((pinInfo[pin].functions[i]&pMask)==pData)) {
          has = true;
#ifdef STM32F1
          af = pinInfo[pin].functions[i] & JSH_MASK_AF;
#endif
        }
      }
    }
    if (has) {
      jsiConsolePrintf("%p",pin);
#ifdef STM32F1
      if (af!=JSH_AF0) jsiConsolePrint("(AF)");
#endif
      jsiConsolePrint(" ");
      if (n++==8) { n=0; jsiConsolePrint("\n"); }
    }
  }
  jsiConsolePrint("\n");
}

/** Find a device of the given type that works on the given pin. For instance:
 * `jshGetDeviceFor(JSH_SPI1, JSH_SPIMAX, pin);
 */
JshPinFunction jshGetDeviceFor(JshPinFunction deviceMin, JshPinFunction deviceMax, Pin pin) {
  if (!jshIsPinValid(pin)) return JSH_NOTHING;
  int i;
  for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
    JshPinFunction f = pinInfo[pin].functions[i];
    if ((f&JSH_MASK_TYPE) >= deviceMin &&
        (f&JSH_MASK_TYPE) <= deviceMax)
      return f;
  }
  return JSH_NOTHING;
}

/** Like jshGetDeviceFor, but returns an actual Object (eg. SPI) if one can be found. */
JsVar *jshGetDeviceObjectFor(JshPinFunction deviceMin, JshPinFunction deviceMax, Pin pin) {
  JshPinFunction dev = jshGetDeviceFor(deviceMin, deviceMax, pin);
  if (dev==JSH_NOTHING) return 0;
  char devName[16];
  jshPinFunctionToString(dev, JSPFTS_DEVICE|JSPFTS_DEVICE_NUMBER|JSPFTS_JS_NAMES, devName, sizeof(devName));
  JsVar *devVar = jsvObjectGetChild(execInfo.root, devName, 0);
  if (devVar) return devVar;
  return jswFindBuiltInFunction(0, devName);
}
