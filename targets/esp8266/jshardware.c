/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <gpio.h>
#include <mem.h>
#include <espmissingincludes.h>
#include <uart.h>
#include <i2c_master.h>
#include <pwm.h>
#include <spi.h> // Include the MetalPhreak/ESP8266_SPI_Library headers.

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jshardware.h"
#include "jsutils.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jspininfo.h"
#include "jswrap_esp8266.h"
#include <jswrap_esp8266_network.h>

// The maximum time that we can safely delay/block without risking a watch dog
// timer error or other undesirable WiFi interaction.  The time is measured in
// microseconds.
#define MAX_SLEEP_TIME_US  3000

// Save-to-flash uses 12KB at 0x78000
// The jshFlash functions use memory-mapped reads to access the first 1MB
// of flash and refuse to go beyond that. Writing uses the SDK functions and is also
// limited to the first MB.
#define FLASH_MAX (1024*1024)
#define FLASH_MMAP 0x40200000
#define FLASH_PAGE_SHIFT 12 // 4KB
#define FLASH_PAGE (1<<FLASH_PAGE_SHIFT)

// Address in RTC RAM where we save the time
#define RTC_TIME_ADDR (256/4) // start of "user data" in RTC RAM


static bool g_spiInitialized = false;
static int  g_lastSPIRead = -1;

// Hardware PWM is broken
#ifdef HARDWARE_PWM
struct PWMRecord {
  bool enabled; //!< Has this PWM been enabled previously?
};
static uint32 g_pwmFreq;
static struct PWMRecord g_PWMRecords[JSH_PIN_COUNT];
#else
// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);
#endif

static uint8 g_pinState[JSH_PIN_COUNT];




/**
 * Convert a pin id to the corresponding Pin Event id.
 */
static IOEventFlags pinToEV_EXTI(
    Pin pin // !< The pin to map to the event id.
  ) {
  // Map pin 0 to EV_EXTI0
  // Map pin 1 to EV_EXTI1
  // ...
  // Map pin x to ECEXTIx
  return (IOEventFlags)(EV_EXTI0 + pin);
}

// forward declaration
static void systemTimeInit(void);
static void utilTimerInit(void);
static void intrHandlerCB(uint32 interruptMask, void *arg);

/**
 * Initialize the ESP8266 hardware environment.
 *
 * TODO: we should move stuff from user_main.c here
 */
void jshInit() {
  // A call to jshInitDevices is architected as something we have to do.
  os_printf("> jshInit\n");

  // Initialize the ESP8266 GPIO subsystem.
  gpio_init();

  systemTimeInit();
  utilTimerInit();
  jshInitDevices();

  // sanity check for pin function enum to catch ordering changes
  if (JSHPINSTATE_I2C != 13 || JSHPINSTATE_GPIO_IN_PULLDOWN != 6 || JSHPINSTATE_MASK != 15) {
    //jsError("%d %d %d\n",JSHPINSTATE_I2C, JSHPINSTATE_GPIO_IN_PULLDOWN,JSHPINSTATE_MASK); 
    jsError("JshPinState #defines have changed, please update pinStateToString()");
  }

  // Register a callback function to be called for a GPIO interrupt
  gpio_intr_handler_register(intrHandlerCB, NULL);

  ETS_GPIO_INTR_ENABLE();

#ifndef HARDWARE_PWM
  BITFIELD_CLEAR(jshPinSoftPWM);
#endif

  // Initialize something for each of the possible pins.
  for (int i=0; i<JSH_PIN_COUNT; i++) {
#ifdef HARDWARE_PWM
    // For each of the PWM records, flag the PWM as having been not initialized.
    g_PWMRecords[i].enabled = false;
#endif
    g_pinState[i] = 0;
  }

  os_printf("< jshInit\n");
} // End of jshInit

/**
 * Handle a GPIO interrupt.
 * We have arrived in this callback function because the state of a GPIO pin has changed
 * and it is time to record that change.
 */
static void CALLED_FROM_INTERRUPT intrHandlerCB(
    uint32 interruptMask, //!< A mask indicating which GPIOs have changed.
    void *arg             //!< Optional argument.
  ) {
  // Given the interrupt mask, we as if bit "x" is on.  If it is, then that is defined as meaning
  // that the state of GPIO "x" has changed so we want to raised an event that indicates that this
  // has happened...
  // Once we have handled the interrupt flags, we need to acknowledge the interrupts so
  // that the ESP8266 will once again cause future interrupts to be processed.

  //os_printf_plus(">> intrHandlerCB\n");
  gpio_intr_ack(interruptMask);
  // We have a mask of interrupts that have happened.  Go through each bit in the mask
  // and, if it is on, then an interrupt has occurred on the corresponding pin.
  int pin;
  for (pin=0; pin<JSH_PIN_COUNT; pin++) {
    if ((interruptMask & (1<<pin)) != 0) {
      // Pin has changed so push the event that says pin has changed.
      jshPushIOWatchEvent(pinToEV_EXTI(pin));
      gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_ANYEDGE);
    }
  }
  //os_printf_plus("<< intrHandlerCB\n");
}

/**
 * Reset the Espruino environment.
 */
void jshReset() {
  jshResetDevices();
  os_printf("> jshReset\n");

  // Set all GPIO pins to be input with pull-up
  jshPinSetState(0, JSHPINSTATE_GPIO_IN_PULLUP);
  //jshPinSetState(2, JSHPINSTATE_GPIO_IN_PULLUP); // used for debug output
  jshPinSetState(4, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(5, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(12, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(13, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(14, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(15, JSHPINSTATE_GPIO_IN_PULLUP);
  g_spiInitialized = false; // Flag the hardware SPI interface as un-initialized.
  g_lastSPIRead = -1;

  extern void user_uart_init(void); // in user_main.c
  user_uart_init();

  jswrap_ESP8266_wifi_reset(); // reset the wifi

  os_printf("< jshReset\n");
}

/**
 * Re-init the esp8266 stuff after a soft-reset
 */
void jshSoftInit() {
  jswrap_ESP8266_wifi_soft_init();
}

/**
 * Handle whatever needs to be done in the idle loop when there's nothing to do.
 *
 * Nothing is needed on the esp8266.
 */
void jshIdle() {
}

// esp8266 chips don't have a serial number but they do have a MAC address
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  assert(maxChars >= 6); // it's 32
  wifi_get_macaddr(0, data);
  return 6;
}

//===== Interrupts and sleeping

void jshInterruptOff() { ets_intr_lock(); }
void jshInterruptOn()  { ets_intr_unlock(); }

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  int time = (int) timeUntilWake;
  //os_printf("jshSleep %lld\n", timeUntilWake);
  // **** TODO: fix this, this is garbage, we need to tell the idle loop to suspend
  //jshDelayMicroseconds(time);
  return true;
} // End of jshSleep

/**
 * Delay (blocking) for the supplied number of microseconds.
 * Note that for the ESP8266 we must NOT CPU block for more than
 * 10 milliseconds or else we may starve the WiFi subsystem.
 */
void jshDelayMicroseconds(int microsec) {
  // Keep things simple and make the user responsible if they sleep for too long...
  if (microsec > 0) {
    //os_printf("Delay %d us\n", microsec);
    os_delay_us(microsec);
  }
} // End of jshDelayMicroseconds

//===== PIN mux =====

static uint32 g_PERIPHS[] = {
  PERIPHS_IO_MUX_GPIO0_U,    // 00
  PERIPHS_IO_MUX_U0TXD_U,    // 01
  PERIPHS_IO_MUX_GPIO2_U,    // 02
  PERIPHS_IO_MUX_U0RXD_U,    // 03
  PERIPHS_IO_MUX_GPIO4_U,    // 04
  PERIPHS_IO_MUX_GPIO5_U,    // 05
  PERIPHS_IO_MUX_SD_CLK_U,   // 06
  PERIPHS_IO_MUX_SD_DATA0_U, // 07
  PERIPHS_IO_MUX_SD_DATA1_U, // 08
  PERIPHS_IO_MUX_SD_DATA2_U, // 09
  PERIPHS_IO_MUX_SD_DATA3_U, // 10
  PERIPHS_IO_MUX_SD_CMD_U,   // 11
  PERIPHS_IO_MUX_MTDI_U,     // 12
  PERIPHS_IO_MUX_MTCK_U,     // 13
  PERIPHS_IO_MUX_MTMS_U,     // 14
  PERIPHS_IO_MUX_MTDO_U      // 15
};

/**
 * Return the function value to select GPIO for a pin
 */
static uint32 g_pinGPIOFunc[] = {
  FUNC_GPIO0,  // 00
  FUNC_GPIO1,  // 01
  FUNC_GPIO2,  // 02
  FUNC_GPIO3,  // 03
  FUNC_GPIO4,  // 04
  FUNC_GPIO5,  // 05
  3,           // 06
  3,           // 07
  3,           // 08
  FUNC_GPIO9,  // 09
  FUNC_GPIO10, // 10
  3,           // 11
  FUNC_GPIO12, // 12
  FUNC_GPIO13, // 13
  FUNC_GPIO14, // 14
  FUNC_GPIO15  // 15
};

/**
 * Return the function value to select Alternate Function for a pin
 */
static uint8 pinAFFunc[] = {
  4 /*CLK_OUT*/, FUNC_U0TXD, FUNC_U1TXD_BK, 0 /*U0RXD*/,
  0 /*NOOP*/, 0 /*NOOP*/, 0, 0,
  0, 0, 0, 0, // protected pins
  2 /*SPI_Q*/, 2 /*SPI_D*/, 2 /*SPI_CLK*/, 2 /*SPI_CS*/,
};

/**
 * Convert a pin state to a string representation.
 * This is used during debugging to log a meaningful value instead of a
 * numeric that would then just have to be decoded.
 */
static char *pinStateToString(JshPinState state) {
  static char *states[] = {
    "UNDEFINED", "GPIO_OUT", "GPIO_OUT_OPENDRAIN",
    "GPIO_OUT_OPENDRAIN_PULLUP",	  
    "GPIO_IN", "GPIO_IN_PULLUP", "GPIO_IN_PULLDOWN",
    "ADC_IN", "AF_OUT", "AF_OUT_OPENDRAIN",
    "USART_IN", "USART_OUT", "DAC_OUT", "I2C",
  };
  return states[state];
}


static void jshDebugPin(Pin pin) {
  os_printf("PIN: %d out=%ld enable=%ld in=%ld\n",
      pin, (GPIO_REG_READ(GPIO_OUT_ADDRESS)>>pin)&1, (GPIO_REG_READ(GPIO_ENABLE_ADDRESS)>>pin)&1,
      (GPIO_REG_READ(GPIO_IN_ADDRESS)>>pin)&1);

  uint32_t gpio_pin = GPIO_REG_READ(GPIO_PIN_ADDR(pin));
  uint32_t mux = READ_PERI_REG(PERIPHS_IO_MUX + 4*pin);
  os_printf("     dr=%s src=%s func=%ld pull-up=%ld oe=%ld\n",
      gpio_pin & 4 ? "open-drain" : "totem-pole",
      gpio_pin & 1 ? "sigma-delta" : "gpio",
      (mux>>2)&1 | (mux&3), (mux>>7)&1, mux&1);
}

/**
 * Set the state of the specific pin.
 *
 * The possible states are:
 *
 * JSHPINSTATE_UNDEFINED
 * JSHPINSTATE_GPIO_OUT
 * JSHPINSTATE_GPIO_OUT_OPENDRAIN
 * JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP
 * JSHPINSTATE_GPIO_IN
 * JSHPINSTATE_GPIO_IN_PULLUP
 * JSHPINSTATE_GPIO_IN_PULLDOWN
 * JSHPINSTATE_ADC_IN
 * JSHPINSTATE_AF_OUT
 * JSHPINSTATE_AF_OUT_OPENDRAIN
 * JSHPINSTATE_USART_IN
 * JSHPINSTATE_USART_OUT
 * JSHPINSTATE_DAC_OUT
 * JSHPINSTATE_I2C
 *
 * This function is exposed indirectly through the exposed global function called
 * `pinMode()`.  For example, `pinMode(pin, "input")` will set the given pin to input.
 */
void jshPinSetState(
    Pin pin,                 //!< The pin to have its state changed.
    JshPinState state        //!< The new desired state of the pin.
  ) {
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }

  //os_printf("> ESP8266: jshPinSetState %d, %s, pup=%d, od=%d\n",
  //    pin, pinStateToString(state), JSHPINSTATE_IS_PULLUP(state), JSHPINSTATE_IS_OPENDRAIN(state));

  assert(pin < JSH_PIN_COUNT);
  if (pin >= 6 && pin <= 11) {
    jsError("Cannot change pins used for flash chip");
    return; // these pins are used for the flash chip
  }

  int periph = g_PERIPHS[pin];

  // set the pin mux function
  switch (state) {
  case JSHPINSTATE_GPIO_OUT:
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
  case JSHPINSTATE_GPIO_IN:
  case JSHPINSTATE_GPIO_IN_PULLUP:
  case JSHPINSTATE_I2C:
    PIN_FUNC_SELECT(periph, g_pinGPIOFunc[pin]); // set the pin mux to GPIO
    break;
  case JSHPINSTATE_AF_OUT:
  case JSHPINSTATE_AF_OUT_OPENDRAIN:
    PIN_FUNC_SELECT(periph, pinAFFunc[pin]); // set the pin to the alternate function
    break;
  case JSHPINSTATE_USART_IN:
  case JSHPINSTATE_USART_OUT:
    if (pin == 1 || pin == 3) PIN_FUNC_SELECT(periph, 0);
    else PIN_FUNC_SELECT(periph, 4); // works for many pins...
    break;
  default:
    jsError("Pin state not supported");
    return;
  }

  // enable/disable pull-up
  if (JSHPINSTATE_IS_PULLUP(state)) {
    PIN_PULLUP_EN(periph);
  } else {
    PIN_PULLUP_DIS(periph);
  }

  // enable/disable output and choose open-drain/totem-pole
  if (!JSHPINSTATE_IS_OUTPUT(state)) {
    GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 1<<pin); // disable output
    GPIO_REG_WRITE(GPIO_PIN_ADDR(pin), GPIO_REG_READ(GPIO_PIN_ADDR(pin)) & ~4); // totem-pole
  } else if (JSHPINSTATE_IS_OPENDRAIN(state)) {
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1<<pin); // enable output
    GPIO_REG_WRITE(GPIO_PIN_ADDR(pin), GPIO_REG_READ(GPIO_PIN_ADDR(pin)) | 4); // open-drain
  } else {
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1<<pin); // enable output
    GPIO_REG_WRITE(GPIO_PIN_ADDR(pin), GPIO_REG_READ(GPIO_PIN_ADDR(pin)) & ~4); // totem-pole
  }

  //jshDebugPin(pin);

  g_pinState[pin] = state; // remember what we set this to...
}


/**
 * Return the current state of the selected pin.
 * \return The current state of the selected pin.
 */
JshPinState jshPinGetState(Pin pin) {
  //os_printf("> ESP8266: jshPinGetState %d\n", pin);
  /*
    os_printf("> ESP8266: pin %d, pinState %d, reg_read %d, out_addr: %d input get %d\n", 
      pin, g_pinState[pin], (GPIO_REG_READ(GPIO_OUT_W1TS_ADDRESS)>>pin)&1,
      (GPIO_REG_READ(GPIO_OUT_ADDRESS)>>pin)&1, GPIO_INPUT_GET(pin));
  */
  if ( (GPIO_REG_READ(GPIO_OUT_ADDRESS)>>pin)&1 ) 
    return g_pinState[pin] | JSHPINSTATE_PIN_IS_ON;
  return g_pinState[pin];
}

//===== GPIO and PIN stuff =====

/**
 * Set the value of the corresponding pin.
 */
void jshPinSetValue(
    Pin pin,   //!< The pin to have its value changed.
    bool value //!< The new value of the pin.
  ) {
  //os_printf("> ESP8266: jshPinSetValue pin=%d, value=%d\n", pin, value);
  if (value & 1) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<pin);
  else           GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<pin);
  //jshDebugPin(pin);
}


/**
 * Get the value of the corresponding pin.
 * \return The current value of the pin.
 */
bool CALLED_FROM_INTERRUPT jshPinGetValue( // can be called at interrupt time
    Pin pin //!< The pin to have its value read.
  ) {
  //os_printf("> ESP8266: jshPinGetValue pin=%d, value=%d\n", pin, GPIO_INPUT_GET(pin));
  return GPIO_INPUT_GET(pin);
}


JsVarFloat jshPinAnalog(Pin pin) {
  //os_printf("> ESP8266: jshPinAnalog: pin=%d\n", pin);
  return (JsVarFloat)system_adc_read() / 1023.0;
}

int jshPinAnalogFast(Pin pin) {
  //os_printf("> ESP8266: jshPinAnalogFast: pin=%d\n", pin);
  return (int)system_adc_read() << 6; // left-align to 16 bits
}


/**
 * Set the output PWM value.
 */
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  if (value<0) value=0;
  if (value>1) value=1;
  if (!isfinite(freq)) freq=0;

#ifdef HARDWARE_PWM
  os_printf("> jshPinAnalogOutput - jshPinAnalogOutput: pin=%d, value(x100)=%d, freq=%d\n", pin, (int)(value*100), (int)freq);
  // Check that the value is between 0.0 and 1.0
  if (value < 0 || value > 1.0) {
    return 0;
  }

  // If PWM for the pin has not previously been enabled, enable it now.
  if (g_PWMRecords[pin].enabled == false) {
    g_PWMRecords[pin].enabled = true;
    g_pwmFreq = (uint32)freq;
    // Set the default frequency to 1KHz if no supplied frequency.
    if (g_pwmFreq == 0) {
      g_pwmFreq = 1000;
    }

    // Initialize the PWM subsystem
    uint32 duty = 0;
    uint32 pinInfoList[3] = {g_PERIPHS[pin], g_pinGPIOFunc[pin], pin};
    pwm_init(g_pwmFreq * 1000000, &duty, 1, &pinInfoList);

    // Start the PWM subsystem
    pwm_start();
  }

  // If the period/frequency has changed, update the period.
  if ((uint32)freq != 0 && (uint32)freq != g_pwmFreq) {
    g_pwmFreq = (uint32)freq;
    pwm_set_period(g_pwmFreq * 1000000);
  }

  uint32 duty = value * 1000000 / 0.045 / g_pwmFreq;
  os_printf(" - Duty: %d (units of 45 nsecs)\n", duty);
  pwm_set_duty(duty, 0);

#else
  // Software PWM

  if (jshIsPinValid(pin)/* && (flags&(JSAOF_ALLOW_SOFTWARE|JSAOF_FORCE_SOFTWARE))*/) {
    /* we set the bit field here so that if the user changes the pin state
     * later on, we can get rid of the IRQs */
    if (!jshGetPinStateIsManual(pin)) {
      BITFIELD_SET(jshPinSoftPWM, pin, 0);
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    }
    BITFIELD_SET(jshPinSoftPWM, pin, 1);
    if (freq<=0) freq=50;
    jstPinPWM(freq, value, pin);
    return 0;
  }

  // Otherwise
  //if (jshIsPinValid(pin))
//    jsiConsolePrint("You need to use analogWrite(pin, val, {soft:true}) for Software PWM on this pin\n");
  return 0;
#endif
}


/**
 *
 */
void jshSetOutputValue(JshPinFunction func, int value) {
  os_printf("ESP8266: jshSetOutputValue %d %d - unimplemented\n", func, value);
}


/**
 *
 */
void jshEnableWatchDog(JsVarFloat timeout) {
  os_printf("ESP8266: jshEnableWatchDog %0.3f\n", timeout);
}

// Kick the watchdog
void jshKickWatchDog() {
  os_printf("ESP8266: jshKickWatchDog\n");
}


/**
 * Get the state of the pin associated with the event flag.
 */
bool CALLED_FROM_INTERRUPT jshGetWatchedPinState(IOEventFlags eventFlag) { // can be called at interrupt time
  //os_printf("> jshGetWatchedPinState eventFlag=%d\n", eventFlag);

  if (eventFlag > EV_EXTI_MAX || eventFlag < EV_EXTI0) {
    os_printf(" - Error ... eventFlag out of range\n");
    jsError("eventFlag out of range");
    //os_printf("< jshGetWatchedPinState\n");
    return false;
  }

  bool currentPinValue = jshPinGetValue((Pin)(eventFlag-EV_EXTI0));
  //os_printf("< jshGetWatchedPinState = %d\n", currentPinValue);
  return currentPinValue;
}


/**
 * Set the value of the pin to be the value supplied and then wait for
 * a given period and set the pin value again to be the opposite.
 */
void jshPinPulse(
    Pin pin,              //!< The pin to be pulsed.
    bool pulsePolarity,   //!< The value to be pulsed into the pin.
    JsVarFloat pulseTime  //!< The duration in milliseconds to hold the pin.
) {
#if 0
  // Implementation using the utility timer. This doesn't work well on the esp8266, because the
  // utility timer uses tasks and these are not pre-emptible. So the timer won't actually fire
  // until the main espruino task becomes idle, which messes up timings. It also locks-up things
  // when someone defines a pulse train that is longer than the timer queue, because then the
  // main task busy-waits for the timer queue to drain a bit, which never happens.
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin!");
    return;
  }
  if (pulseTime <= 0) {
    // just wait for everything to complete [??? what does this mean ???]
    jstUtilTimerWaitEmpty();
    return;
  } else {
    // find out if we already had a timer scheduled
    UtilTimerTask task;
    if (!jstGetLastPinTimerTask(pin, &task)) {
      // no timer - just start the pulse now!
      jshPinOutput(pin, pulsePolarity);
      task.time = jshGetSystemTime();
    }
    // Now set the end of the pulse to happen on a timer
    jstPinOutputAtTime(task.time + jshGetTimeFromMilliseconds(pulseTime), &pin, 1, !pulsePolarity);
  }
#endif

#if 1
  // Implementation using busy-waiting. Ugly and if the pulse train exceeds 10ms one risks WDT
  // resets, but it actually works...
  jshPinOutput(pin, pulsePolarity);
  jshDelayMicroseconds(jshGetTimeFromMilliseconds(pulseTime)-6);  // -6 adjustment is for overhead
  jshPinSetValue(pin, !pulsePolarity);
#endif
}


/**
 * Determine whether the pin can be watchable.
 * \return Returns true if the pin is wathchable.
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
  // As of right now, let us assume that all pins on an ESP8266 are watchable.
  os_printf("> jshCanWatch: pin=%d\n", pin);
  os_printf("< jshCanWatch = true\n");
  return true;
}


/**
 * Do what ever is necessary to watch a pin.
 * \return The event flag for this pin.
 */
IOEventFlags jshPinWatch(
    Pin pin,         //!< The pin to be watched.
    bool shouldWatch //!< True for watching and false for unwatching.
  ) {
  //os_printf("> jshPinWatch: pin=%d, shouldWatch=%d\n", pin, shouldWatch);
  if (jshIsPinValid(pin)) {
    ETS_GPIO_INTR_DISABLE();
    if (shouldWatch) {
      // Start watching the given pin ...  First we ask ourselves if the
      // pin state has been set manually. If it has not been set manually,
      // then set the pin state to input.
      if (jshGetPinStateIsManual(pin) == false) {
        jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
      }
      gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_ANYEDGE);
    } else {
      // Stop watching the given pin
      gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_DISABLE);
    }
    ETS_GPIO_INTR_ENABLE();
  } else {
    jsError("Invalid pin");
    //os_printf("< jshPinWatch: Invalid pin\n");
    return EV_NONE;
  }
  //os_printf("< jshPinWatch\n");
  return pinToEV_EXTI(pin);
}


/**
 *
 */
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  //os_printf("jshGetCurrentPinFunction %d\n", pin);
  return JSH_NOTHING;
}

/**
 * Determine if a given event is associated with a given pin.
 * \return True if the event is associated with the pin and false otherwise.
 */
bool jshIsEventForPin(
    IOEvent *event, //!< The event that has been detected.
    Pin pin         //!< The identity of a pin.
  ) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEV_EXTI(pin);
}

//===== USART and Serial =====



void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  uint8 uart_no;
  if (device == EV_SERIAL1) uart_no = UART0;
  else if (device == EV_SERIAL2) uart_no = UART1;
  else {
    jsError("jshUSARTSetup Unknown device %d\n", device);
    return;
  }

  extern void uart_config(uint8 uart_no);
  extern UartDevice    UartDev;

  UartDev.baut_rate = inf->baudRate;
  if (inf->bytesize == 5)
    UartDev.data_bits = FIVE_BITS;
  else if (inf->bytesize == 6)
    UartDev.data_bits = SIX_BITS;
  else if (inf->bytesize == 7)
    UartDev.data_bits = SEVEN_BITS;
  else if (inf->bytesize == 8)
    UartDev.data_bits = EIGHT_BITS;
  else assert(0);
  if (inf->parity == 1) {
    UartDev.exist_parity = STICK_PARITY_EN;
    UartDev.parity = EVEN_BITS;
  } else  if (inf->parity == 2) {
    UartDev.exist_parity = STICK_PARITY_EN;
    UartDev.parity = ODD_BITS;
  } else {
    UartDev.exist_parity = STICK_PARITY_DIS;
    UartDev.parity = NONE_BITS;
  }
  if (inf->stopbits == 1)
    UartDev.stop_bits = ONE_STOP_BIT;
  else if (inf->stopbits == 2)
    UartDev.stop_bits = TWO_STOP_BIT;
  else assert(0);
  UartDev.flow_ctrl = NONE_CTRL;

  uart_config(uart_no);
}

bool jshIsUSBSERIALConnected() {
  return false; // "On non-USB boards this just returns false"
}

/**
 * Kick a device into action (if required).
 *
 * For instance we may need
 * to set up interrupts.  In this ESP8266 implementation, we transmit all the
 * data that can be found associated with the device.
 */
void jshUSARTKick(
    IOEventFlags device //!< The device to be kicked.
) {
  // transmit anything that is sitting in the uart output buffers
  if (device == EV_SERIAL1 || device == EV_SERIAL2) {
    // Get the next character to transmit.  We will have reached the end when
    // the value of the character to transmit is -1.
    int c = jshGetCharToTransmit(device);
    uint8_t uart = device == EV_SERIAL1 ? 0 : 1;

    while (c >= 0) {
      uart_tx_one_char(uart, c);
      c = jshGetCharToTransmit(device);
    }
  }
}


//===== SPI =====

/**
 * Initialize the hardware SPI device.
 * On the ESP8266, hardware SPI is implemented via a set of pins defined
 * as follows:
 *
 * | GPIO   | NodeMCU | Name  | Function |
 * |--------|---------|-------|----------|
 * | GPIO12 | D6      | HMISO | MISO     |
 * | GPIO13 | D7      | HMOSI | MOSI     |
 * | GPIO14 | D5      | HSCLK | CLK      |
 * | GPIO15 | D8      | HCS   | CS       |
 *
 */
void jshSPISetup(
    IOEventFlags device, //!< The identity of the SPI device being initialized.
    JshSPIInfo *inf      //!< Flags for the SPI device.
) {
  if (device != EV_SPI1) {
    jsExceptionHere(JSET_ERROR, "Only SPI1 supported");
    return;
  }

  //os_printf("jshSPISetup - jshSPISetup: device=%d\n", device);
  spi_init(HSPI, inf->baudRate); // Initialize the hardware SPI components.
  g_spiInitialized = true;
  g_lastSPIRead = -1;

  if (inf != NULL) {
    os_printf("baudRate=%d, baudRateSpec=%d, pinSCK=%d, pinMISO=%d, pinMOSI=%d, spiMode=%d, spiMSB=%d\n",
        inf->baudRate, inf->baudRateSpec, inf->pinSCK, inf->pinMISO, inf->pinMOSI, inf->spiMode, inf->spiMSB);
  }
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device, //!< The identity of the SPI device through which data is being sent.
    int data             //!< The data to be sent or an indication that no data is to be sent.
) {
  if (device != EV_SPI1) {
    return -1;
  }
  //os_printf("> jshSPISend - device=%d, data=%x\n", device, data);
  int retData = g_lastSPIRead;
  if (data >=0) {
    g_lastSPIRead = spi_transaction(HSPI, 8, (uint32)data);
  } else {
    g_lastSPIRead = -1;
  }
  //os_printf("< jshSPISend\n");
  return retData;
}


/**
 * Send 16 bit data through the given SPI device.
 */
void jshSPISend16(
    IOEventFlags device, //!< Unknown
    int data             //!< Unknown
) {
  //os_printf("> jshSPISend16 - device=%d, data=%x\n", device, data);
  //jshSPISend(device, data >> 8);
  //jshSPISend(device, data & 255);
  if (device != EV_SPI1) {
    return;
  }

  spi_transaction(HSPI, 16, (uint32)data);
  //os_printf("< jshSPISend16\n");
}


/**
 * Set whether to send 16 bits or 8 over SPI.
 */
void jshSPISet16(
    IOEventFlags device, //!< Unknown
    bool is16            //!< Unknown
) {
  //os_printf("> jshSPISet16 - device=%d, is16=%d\n", device, is16);
  //os_printf("< jshSPISet16\n");
}


/**
 * Wait until SPI send is finished.
 */
void jshSPIWait(
    IOEventFlags device //!< Unknown
) {
  //os_printf("> jshSPIWait - device=%d\n", device);
  while(spi_busy(HSPI)) ;
  //os_printf("< jshSPIWait\n");
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  //os_printf("> jshSPISetReceive - device=%d, isReceive=%d\n", device, isReceive);
  //os_printf("< jshSPISetReceive\n");
}

//===== I2C =====

/** Set-up I2C master for ESP8266, default pins are SCL:12, SDA:13. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  //os_printf("> jshI2CSetup: SCL=%d SDA=%d bitrate=%d\n",
  //    info->pinSCL, info->pinSDA, info->bitrate);
  if (device != EV_I2C1) {
    jsError("Only I2C1 supported"); return; }
  Pin scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 12;
  Pin sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 13;

  jshPinSetState(scl, JSHPINSTATE_I2C);
  jshPinSetState(sda, JSHPINSTATE_I2C);

  i2c_master_gpio_init(scl, sda, info->bitrate);
  //os_printf("< jshI2CSetup\n");
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes,
    const unsigned char *data, bool sendStop) {
  //os_printf("ESP8266: jshI2CWrite 0x%x %dbytes %s\n", address, nBytes, sendStop?"stop":"");
  if (device != EV_I2C1) return;     // we only support one i2c device

  uint8 ack;

  i2c_master_start();                   // start the transaction
  i2c_master_writeByte((address<<1)|0); // send address and r/w
  ack = i2c_master_getAck();            // get ack bit from slave
  //os_printf("I2C: ack=%d\n", ack);
  if (!ack) goto error;
  while (nBytes--) {
    i2c_master_writeByte(*data++);      // send data byte
    ack = i2c_master_getAck();          // get ack bit from slave
    if (!ack) goto error;
  }
  if (sendStop) i2c_master_stop();
  return;
error:
  i2c_master_stop();
  jsExceptionHere(JSET_INTERNALERROR, "I2CWrite: No ACK %d\n", ack);
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes,
    unsigned char *data, bool sendStop) {
  //os_printf("ESP8266: jshI2CRead 0x%x %dbytes %s\n", address, nBytes, sendStop?"stop":"");
  if (device != EV_I2C1) return;     // we only support one i2c device

  uint8 ack;

  i2c_master_start();                   // start the transaction
  i2c_master_writeByte((address<<1)|1); // send address and r/w
  ack = i2c_master_getAck();            // get ack bit from slave
  if (!ack) goto error;
  while (nBytes--) {
    *data++ = i2c_master_readByte();    // recv data byte
    i2c_master_setAck(nBytes == 0);     // send ack or no-ack for last byte
  }
  if (sendStop) i2c_master_stop();
  return;
error:
  i2c_master_stop();
  jsExceptionHere(JSET_INTERNALERROR, "I2CRead: No ACK %d\n", ack);
}

//===== System time stuff =====

/* The esp8266 has two notions of system time implemented in the SDK by system_get_time()
 * and system_get_rtc_time(). The former has 1us granularity and comes off the CPU cycle
 * counter, the latter has approx 57us granularity (need to check) and comes off the RTC
 * clock. Both are 32-bit counters and thus need some form of roll-over handling in software
 * to produce a JsSysTime.
 *
 * It seems pretty clear from the API and the calibration concepts that the RTC runs off an
 * internal RC oscillator or something similar and the SDK provides functions to calibrate
 * it WRT the crystal oscillator, i.e., to get the current clock ratio. The only benefit of
 * RTC timer is that it keeps running when in light sleep mode. (It also keeps running in
 * deep sleep mode since it can be used to exit deep sleep but some brilliant engineer at
 * espressif decided to reset the RTC timer when coming out of deep sleep so the time is
 * actually lost!)
 *
 * It seems that the best course of action is to use the system timer for jshGetSystemTime()
 * and related functions and to use the rtc timer only to preserve time during light sleep.
 */

/**
 * Given a time in milliseconds as float, get us the value in microsecond
 */
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  //  os_printf("jshGetTimeFromMilliseconds %d, %f\n", (JsSysTime)(ms * 1000.0), ms);
  return (JsSysTime) (ms * 1000.0 + 0.5);
}

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  //  os_printf("jshGetMillisecondsFromTime %d, %f\n", time, (JsVarFloat)time / 1000.0);
  return (JsVarFloat) time / 1000.0;
}

// Structure to hold a timestamp in us since the epoch, plus the system timer value at that time
// stamp. The crc field is used when saving this to RTC RAM
typedef struct {
  JsSysTime timeStamp;  // UTC time at time stamp
  uint32_t hwTimeStamp; // time in hw register at time stamp
  uint32_t cksum;       // checksum to check validity when loading from RTC RAM
} espTimeStamp;
static espTimeStamp sysTimeStamp; // last time stamp off system_get_time()
static espTimeStamp rtcTimeStamp; // last time stamp off system_get_rtc_time()

// Given a time stamp and a new value for the HW clock calculate the new time and update accordingly
static void updateTime(espTimeStamp *stamp, uint32_t clock) {
  uint32_t delta = clock - stamp->hwTimeStamp;
  stamp->timeStamp += (JsSysTime)delta;
  stamp->hwTimeStamp = clock;
}

// Save the current RTC timestamp to RTC RAM so we don't loose track of time during a reset
// or sleep
static void saveTime() {
  // calculate checksum
  rtcTimeStamp.cksum = 0xdeadbeef ^ rtcTimeStamp.hwTimeStamp ^
    (uint32_t)(rtcTimeStamp.timeStamp & 0xffffffff) ^
    (uint32_t)(rtcTimeStamp.timeStamp >> 32);
  system_rtc_mem_write(RTC_TIME_ADDR, &rtcTimeStamp, sizeof(rtcTimeStamp));
  // Debug
  //os_printf("RTC write: %lu %lu 0x%08x\n", (uint32_t)(rtcTimeStamp.timeStamp/1000000),
  //  rtcTimeStamp.hwTimeStamp, (int)rtcTimeStamp.cksum);
}

/**
 * Return the current time in microseconds.
 */
JsSysTime CALLED_FROM_INTERRUPT jshGetSystemTime() { // in us -- can be called at interrupt time
  return sysTimeStamp.timeStamp + (JsSysTime)(system_get_time() - sysTimeStamp.hwTimeStamp);
}


/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime newTime) {
  //os_printf("ESP8266: jshSetSystemTime: %d\n", time);
  uint32_t sysTime = system_get_time();
  uint32_t rtcTime = system_get_rtc_time();

  sysTimeStamp.timeStamp = newTime;
  sysTimeStamp.hwTimeStamp = sysTime;
  rtcTimeStamp.timeStamp = newTime;
  rtcTimeStamp.hwTimeStamp = rtcTime;
  saveTime(&rtcTimeStamp);
}

/**
 * Periodic system timer to update the time structure and save it to RTC RAM so we don't loose
 * track of it and it doesn't roll-over unnoticed
 */
#define SYSTEM_TIME_QUANTUM 0x1000000 // time period in us for system timer callback
static ETSTimer systemTimeTimer;

// callback for periodic system timer update and saving
static void systemTimeCb(void *arg) {
  uint32_t sysTime = system_get_time();
  uint32_t rtc = system_get_rtc_time();
  __asm__ __volatile__("memw" : : : "memory"); // memory barrier to enforce above happen
  updateTime(&sysTimeStamp, sysTime);
  rtcTimeStamp.timeStamp = sysTimeStamp.timeStamp;
  rtcTimeStamp.hwTimeStamp = rtc;
  // Debug
  // os_printf("RTC sys=%lu rtc=%lu\n", sysTime, rtc);

  saveTime(&rtcTimeStamp);
}

// Initialize the system time, trying to rescue what we know from RTC RAM. We can continue
// running the RTC clock if two conditions are met: we can read the old time from RTC RAM and
// the RTC clock hasn't been reset. The latter is the case for reset reasons 1 thru 4 (wdt reset,
// exception, soft wdt, and restart), the RTC clock is reset on power-on, on reset pin input, and
// on deep sleep (which is left using a reset pin input).
static void systemTimeInit(void) {
  // kick off the system timer
  os_timer_disarm(&systemTimeTimer);
  os_timer_setfn(&systemTimeTimer, systemTimeCb, NULL);
  //os_timer_arm(&systemTimeTimer, 0x1000000, 1);
  os_timer_arm(&systemTimeTimer, 0x10000, 1);

  // load the reset cause
  uint32 reason = system_get_rst_info()->reason;

  // load time from RTC RAM
  system_rtc_mem_read(RTC_TIME_ADDR, &rtcTimeStamp, sizeof(rtcTimeStamp));
  uint32_t cksum = rtcTimeStamp.cksum ^ rtcTimeStamp.hwTimeStamp ^
    (uint32_t)(rtcTimeStamp.timeStamp & 0xffffffff) ^
    (uint32_t)(rtcTimeStamp.timeStamp >> 32);
  os_printf("RTC read: %d %d 0x%08x (0x%08x)\n", (int)(rtcTimeStamp.timeStamp/1000000),
    (int)rtcTimeStamp.hwTimeStamp, (unsigned int)rtcTimeStamp.cksum, (unsigned int)cksum);
  if (reason < 1 || reason > 4 || cksum != 0xdeadbeef) {
    // we lost track of time, start at zero
    os_printf("RTC: cannot restore time\n");
    memset(&rtcTimeStamp, 0, sizeof(rtcTimeStamp));
    memset(&sysTimeStamp, 0, sizeof(sysTimeStamp));
    return;
  }
  // calculate current time based on RTC clock delta; the system_rtc_clock_cali_proc() tells
  // us how many us there are per RTC tick, the value is fixed-point decimal with 12
  // decimal bits, hence the shift by 12 below
  uint32_t sysTime = system_get_time();
  uint32_t rtcTime = system_get_rtc_time();
  uint32_t cal = system_rtc_clock_cali_proc(); // us per rtc tick as fixed point
  __asm__ __volatile__("memw" : : : "memory"); // memory barrier to enforce above happen
  uint64_t delta = (uint64_t)(rtcTime - rtcTimeStamp.hwTimeStamp);
  rtcTimeStamp.timeStamp += (delta * (uint64_t)cal) >> 12;
  rtcTimeStamp.hwTimeStamp = rtcTime;
  sysTimeStamp.timeStamp = rtcTimeStamp.timeStamp;
  sysTimeStamp.hwTimeStamp = sysTime;
  os_printf("RTC: restore sys=%lu rtc=%lu\n", sysTime, rtcTime);
  os_printf("RTC: restored time: %lu (delta=%lu cal=%luus)\n",
      (uint32_t)(rtcTimeStamp.timeStamp/1000000),
      (uint32_t)delta, (cal*1000)>>12);
  saveTime(&rtcTimeStamp);
}

//===== Utility timer =====

// The utility timer uses the SDK timer in microsecond mode.

os_timer_t utilTimer;

static void utilTimerInit(void) {
  //os_printf("UStimer init\n");
  os_timer_disarm(&utilTimer);
  os_timer_setfn(&utilTimer, jstUtilTimerInterruptHandler, NULL);
}

void jshUtilTimerDisable() {
  //os_printf("UStimer disarm\n");
  os_timer_disarm(&utilTimer);
}

void jshUtilTimerStart(JsSysTime period) {
  //if (period < 100.0 || period > 10000) os_printf("UStimer arm %ldus\n", (uint32_t)period);
  if (period<1) period=1;
  os_timer_arm_us(&utilTimer, (uint32_t)period, 0);
}

void jshUtilTimerReschedule(JsSysTime period) {
  jshUtilTimerDisable();
  jshUtilTimerStart(period);
}

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
  bool retVal = true;
  switch(device) {
  case EV_SPI1:
    retVal = g_spiInitialized;
    break;
  default:
    break;
  }
  os_printf("jshIsDeviceInitialised: dev %d, ret %d\n", device, retVal);
  return retVal;
} // End of jshIsDeviceInitialised

// the esp8266 doesn't have any temperature sensor
JsVarFloat jshReadTemperature() {
  return NAN;
}

// the esp8266 can read the VRef but then there's no analog input, so we don't support this
JsVarFloat jshReadVRef() {
  return NAN;
}

unsigned int jshGetRandomNumber() {
  return rand();
}

//===== Read-write flash =====

/**
 * Determine available flash depending on EEprom size
 *
 */
uint32_t jshFlashMax() {
  extern uint16_t espFlashKB;
  return 1024*espFlashKB;
}

/**
 * Read data from flash memory into the buffer.
 *
 * This reads from flash using memory-mapped reads. Only works for the first 1MB and
 * requires 4-byte aligned reads.
 *
 */
void jshFlashRead(
    void *buf,     //!< buffer to read into
    uint32_t addr, //!< Flash address to read from
    uint32_t len   //!< Length of data to read
  ) {
  //os_printf("jshFlashRead: dest=%p, len=%ld flash=0x%lx\n", buf, len, addr);

  // make sure we stay with the flash address space
  uint32_t flash_max=jshFlashMax();
  if (addr >= flash_max) return;
  if (addr + len > flash_max) len = flash_max - addr;

  if (addr < FLASH_MAX) {
	  addr += FLASH_MMAP; // Direct fast read if < 1Mb
	  // copy the bytes reading a word from flash at a time
	  uint8_t *dest = buf;
	  uint32_t bytes = *(uint32_t*)(addr & ~3);
	  while (len-- > 0) {
		if ((addr & 3) == 0) bytes = *(uint32_t*)addr;
		*dest++ = ((uint8_t*)&bytes)[addr++ & 3];
	  }
   } else { // Above 1Mb read...
	//os_printf("jshFlashRead: above 1mb!");
	SpiFlashOpResult res;
	res = spi_flash_read(addr, buf, len);
    if (res != SPI_FLASH_RESULT_OK)
      os_printf("ESP8266: jshFlashRead %s\n",
    res == SPI_FLASH_RESULT_ERR ? "error" : "timeout");
   }
}


/**
 * Write data to flash memory from the buffer.
 *
 * This is called from jswrap_flash_write and ... which guarantee that addr is 4-byte aligned
 * and len is a multiple of 4.
 */
void jshFlashWrite(
    void *buf,     //!< Buffer to write from
    uint32_t addr, //!< Flash address to write into
    uint32_t len   //!< Length of data to write
  ) {
  //os_printf("jshFlashWrite: src=%p, len=%ld flash=0x%lx\n", buf, len, addr);

  // make sure we stay with the flash address space
  uint32_t flash_max=jshFlashMax();
  if (addr >= flash_max) return;
  if (addr + len > flash_max) len = flash_max - addr;

  // since things are guaranteed to be aligned we can just call the SDK :-)
  SpiFlashOpResult res;
  res = spi_flash_write(addr, buf, len);
  if (res != SPI_FLASH_RESULT_OK)
    os_printf("ESP8266: jshFlashWrite %s\n",
      res == SPI_FLASH_RESULT_ERR ? "error" : "timeout");
}


/**
 * Return start address and size of the flash page the given address resides in.
 * Returns false if no page.
 */
bool jshFlashGetPage(
    uint32_t addr,       //!<
    uint32_t *startAddr, //!<
    uint32_t *pageSize   //!<
  ) {
  //os_printf("ESP8266: jshFlashGetPage: addr=0x%lx, startAddr=%p, pageSize=%p\n", addr, startAddr, pageSize);

  if (addr >= jshFlashMax()) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true;
}

static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  // Area reserved for EEPROM
  addFlashArea(jsFreeFlash, 0x77000, 0x1000);

  // need 1MB of flash to have more space...
  extern uint16_t espFlashKB; // in user_main,c
  if (espFlashKB > 512) {
    addFlashArea(jsFreeFlash, 0x80000, 0x1000);
    if (espFlashKB > 1024) {
      addFlashArea(jsFreeFlash, 0xf7000, 0x9000);
	} else {
	  addFlashArea(jsFreeFlash, 0xf7000, 0x5000);
    }
	if (espFlashKB == 2048) {
	  addFlashArea(jsFreeFlash, 0x100000, 0x100000-0x4000);
    }
	if (espFlashKB == 4096) {
	  addFlashArea(jsFreeFlash, 0x100000, 0x300000-0x4000);
    }
  }

  return jsFreeFlash;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
  //os_printf("jshFlashErasePage: addr=0x%lx\n", addr);

  SpiFlashOpResult res;
  res = spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
  if (res != SPI_FLASH_RESULT_OK)
    os_printf("ESP8266: jshFlashErase%s\n",
      res == SPI_FLASH_RESULT_ERR ? "error" : "timeout");
}

unsigned int jshSetSystemClock(JsVar *options) {
  int newFreq = jsvGetInteger(options);
  if (newFreq != 80 && newFreq != 160) {
    jsExceptionHere(JSET_ERROR, "Invalid frequency value, must be 80 or 160.");
    return 0;
  }
  system_update_cpu_freq(newFreq);
  return system_get_cpu_freq()*1000000;
}


/**
 * Callback for end of runtime.  This should never be called and has been
 * added to satisfy the linker.
 */
void _exit(int status) {
}

