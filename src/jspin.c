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
#include "jshardware.h"

#if defined(PICO) || defined(NUCLEOF401RE) || defined(NUCLEOF411RE)
#define PIN_NAMES_DIRECT // work out pin names directly from port + pin in pinInfo
#endif

bool jshIsPinValid(Pin pin) {
  // Note, PIN_UNDEFINED is always > JSH_PIN_COUNT
  return pin < JSH_PIN_COUNT && pinInfo[pin].port!=JSH_PORT_NONE;
}


Pin jshGetPinFromString(const char *s) {
  // built in constants

  if (s[0]=='B' && s[1]=='T' && s[2]=='N') {
#ifdef BTN1_PININDEX
    if (!s[3]) return BTN1_PININDEX;
    if (s[3]=='1' && !s[4]) return BTN1_PININDEX;
#endif
#ifdef BTN2_PININDEX
    if (s[3]=='2' && !s[4]) return BTN2_PININDEX;
#endif
#ifdef BTN3_PININDEX
    if (s[3]=='3' && !s[4]) return BTN3_PININDEX;
#endif
#ifdef BTN4_PININDEX
    if (s[3]=='4' && !s[4]) return BTN4_PININDEX;
#endif
  }
  if (s[0]=='L' && s[1]=='E' && s[2]=='D') {
#ifdef LED1_PININDEX
    if (!s[3]) return LED1_PININDEX;
    if (s[3]=='1' && !s[4]) return LED1_PININDEX;
#endif
#ifdef LED2_PININDEX
    if (s[3]=='2' && !s[4]) return LED2_PININDEX;
#endif
#ifdef LED3_PININDEX
    if (s[3]=='3' && !s[4]) return LED3_PININDEX;
#endif
#ifdef LED4_PININDEX
    if (s[3]=='4' && !s[4]) return LED4_PININDEX;
#endif
#ifdef LED5_PININDEX
    if (s[3]=='5' && !s[4]) return LED5_PININDEX;
#endif
#ifdef LED6_PININDEX
    if (s[3]=='6' && !s[4]) return LED6_PININDEX;
#endif
#ifdef LED7_PININDEX
    if (s[3]=='7' && !s[4]) return LED7_PININDEX;
#endif
#ifdef LED8_PININDEX
    if (s[3]=='8' && !s[4]) return LED8_PININDEX;
#endif
  }

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
          return i;
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

/** Write the pin name to a string. String must have at least 8 characters (to be safe) */
void jshGetPinString(char *result, Pin pin) {
  result[0] = 0; // just in case
#ifdef PIN_NAMES_DIRECT
  if (jshIsPinValid(pin)) {
    result[0]='A'+pinInfo[pin].port-JSH_PORTA;
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
    strncpy(result, "UNKNOWN", 8);
  }
}

/// Given a var, convert it to a pin ID (or -1 if it doesn't exist). safe for undefined!
Pin jshGetPinFromVar(JsVar *pinv) {
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

BITFIELD_DECL(jshPinStateIsManual, JSH_PIN_COUNT);

bool jshGetPinStateIsManual(Pin pin) {
  return BITFIELD_GET(jshPinStateIsManual, pin);
}

void jshSetPinStateIsManual(Pin pin, bool manual) {
  BITFIELD_SET(jshPinStateIsManual, pin, manual);
}

// ----------------------------------------------------------------------------

bool jshPinInput(Pin pin) {
  bool value = false;
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin))
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

    value = jshPinGetValue(pin);
  } else jsExceptionHere(JSET_ERROR, "Invalid pin!");
  return value;
}


void jshPinOutput(Pin pin, bool value) {
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin))
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
  } else jsExceptionHere(JSET_ERROR, "Invalid pin!");
}

