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

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jshardware.h"
#include "jsutils.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsinteractive.h"

// The maximum time that we can safely delay/block without risking a watch dog
// timer error or other undesirable WiFi interaction.  The time is measured in
// microseconds.
#define MAX_SLEEP_TIME_US  3000

// Save-to-flash uses the 16KB of "user params" locates right after the first firmware
// block, see https://github.com/espruino/Espruino/wiki/ESP8266-Design-Notes for memory
// map details. The jshFlash functions use memory-mapped reads to access the first 1MB
// of flash and refuse to go beyond that. Writing uses the SDK functions and is also
// limited to the first MB.
#define FLASH_MAX (1024*1024)
#define FLASH_MMAP 0x40200000
#define FLASH_PAGE_SHIFT 12 // 4KB
#define FLASH_PAGE (1<<FLASH_PAGE_SHIFT)

// Address in RTC RAM where we save the time
#define RTC_TIME_ADDR (256/4) // start of "user data" in RTC RAM

/**
 * Transmit all the characters in the transmit buffer.
 *
 */
void esp8266_uartTransmitAll(IOEventFlags device) {
  // Get the next character to transmit.  We will have reached the end when
  // the value of the character to transmit is -1.
  int c = jshGetCharToTransmit(device);

  while (c >= 0) {
    uart_tx_one_char(0, c);
    c = jshGetCharToTransmit(device);
  } // No more characters to transmit
} // End of esp8266_transmitAll

// ----------------------------------------------------------------------------

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
  if (JSHPINSTATE_I2C != 12 || JSHPINSTATE_GPIO_IN_PULLDOWN != 5 || JSHPINSTATE_MASK != 15) {
    jsError("JshPinState #defines have changed, please update pinStateToString()");
  }

  // Register a callback function to be called for a GPIO interrupt
  gpio_intr_handler_register(intrHandlerCB, NULL);

  ETS_GPIO_INTR_ENABLE();
  os_printf("< jshInit\n");
} // End of jshInit

/**
 * Handle a GPIO interrupt.
 * We have arrived in this callback function because the state of a GPIO pin has changed
 * and it is time to record that change.
 */
static void intrHandlerCB(
    uint32 interruptMask, //!< A mask indicating which GPIOs have changed.
    void *arg             //!< Optional argument.
  ) {
  // Given the interrupt mask, we as if bit "x" is on.  If it is, then that is defined as meaning
  // that the state of GPIO "x" has changed so we want to raised an event that indicates that this
  // has happened...
  // Once we have handled the interrupt flags, we need to acknowledge the interrupts so
  // that the ESP8266 will once again cause future interrupts to be processed.

  os_printf_plus(">> intrHandlerCB\n");
  gpio_intr_ack(interruptMask);
  // We have a mask of interrupts that have happened.  Go through each bit in the mask
  // and, if it is on, then an interrupt has occurred on the corresponding pin.
  int pin;
  for (pin=0; pin<16; pin++) {
    if ((interruptMask & (1<<pin)) != 0) {
      // Pin has changed so push the event that says pin has changed.
      jshPushIOWatchEvent(pinToEV_EXTI(pin));
      gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_ANYEDGE);
    }
  }
  os_printf_plus("<< intrHandlerCB\n");
}

/**
 * Reset the Espruino environment.
 */
void jshReset() {
  //system_restart();
  os_printf("> jshReset\n");
  // Set all GPIO pins to be input.
  /*
  int i;
  for (int i=0; i<16; i++) {
    jshPinSetState(i, JSHPINSTATE_GPIO_IN);
  }
  */
  os_printf("< jshReset\n");
} // End of jshReset

/**
 * Handle whatever needs to be done in the idle loop when there's nothing to do.
 *
 * Nothing is needed on the esp8266. The watchdog timer is taken care of by the SDK.
 */
void jshIdle() {
} // End of jshIdle

// esp8266 chips don't have a serial number but they do have a MAC address
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  uint8_t mac_addr[6];
  wifi_get_macaddr(0, mac_addr); // 0->MAC of STA interface
  char buf[16];
  int len = os_sprintf(buf, MACSTR, MAC2STR(mac_addr));
  strncpy((char *)data, buf, maxChars);
  return len > maxChars ? maxChars : len;
} // End of jshSerialNumber

//===== Interrupts and sleeping

void jshInterruptOff() {
  //os_printf("> jshInterruptOff\n");
  ets_intr_lock();
  //os_printf("< jshInterruptOff\n");
} // End of jshInterruptOff

void jshInterruptOn() {
  //os_printf("> jshInterruptOn\n");
  ets_intr_unlock();
  //os_printf("< jshInterruptOn\n");
} // End of jshInterruptOn

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
    os_printf("Delay %d us\n", microsec);
    os_delay_us(microsec);
  }
} // End of jshDelayMicroseconds

//===== PIN mux =====

static uint8_t PERIPHS[] = {
PERIPHS_IO_MUX_GPIO0_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_U0TXD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO2_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_U0RXD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO4_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_GPIO5_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_CLK_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA0_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA1_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA2_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_DATA3_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_SD_CMD_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTDI_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTCK_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTMS_U - PERIPHS_IO_MUX,
PERIPHS_IO_MUX_MTDO_U - PERIPHS_IO_MUX };

/**
 * Return the function value to select GPIO for a pin
 */
static uint8 pinGPIOFunc[] = {
  FUNC_GPIO0, FUNC_GPIO1, FUNC_GPIO2, FUNC_GPIO3,
  FUNC_GPIO4, FUNC_GPIO5, 3, 3,
  3, FUNC_GPIO9, FUNC_GPIO10, 3,
  FUNC_GPIO12, FUNC_GPIO13, FUNC_GPIO14, FUNC_GPIO15,
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
    "GPIO_IN", "GPIO_IN_PULLUP", "GPIO_IN_PULLDOWN",
    "ADC_IN", "AF_OUT", "AF_OUT_OPENDRAIN",
    "USART_IN", "USART_OUT", "DAC_OUT", "I2C",
  };
  return states[state];
}

static uint8 pinState[16] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // all JSHPINSTATE_UNDEFINED until we set them
};

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
  os_printf("> ESP8266: jshPinSetState %d, %s, pup=%d, od=%d\n",
      pin, pinStateToString(state), JSHPINSTATE_IS_PULLUP(state), JSHPINSTATE_IS_OPENDRAIN(state));

  assert(pin < 16);
  if (pin >= 6 && pin <= 11) {
    jsError("Cannot change pins used for flash chip");
    return; // these pins are used for the flash chip
  }

  int periph = PERIPHS_IO_MUX + PERIPHS[pin];

  // set the pin mux function
  switch (state) {
  case JSHPINSTATE_GPIO_OUT:
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
  case JSHPINSTATE_GPIO_IN:
  case JSHPINSTATE_GPIO_IN_PULLUP:
  case JSHPINSTATE_I2C:
    PIN_FUNC_SELECT(periph, pinGPIOFunc[pin]); // set the pin mux to GPIO
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

  pinState[pin] = state; // remember what we set this to...
}


/**
 * Return the current state of the selected pin.
 * \return The current state of the selected pin.
 */
JshPinState jshPinGetState(Pin pin) {
  os_printf("> ESP8266: jshPinGetState %d\n", pin);
  return pinState[pin];
}

//===== GPIO and PIN stuff =====

/**
 * Set the value of the corresponding pin.
 */
void jshPinSetValue(
    Pin pin,   //!< The pin to have its value changed.
    bool value //!< The new value of the pin.
  ) {
  os_printf("> ESP8266: jshPinSetValue %d, %d\n", pin, value);
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, (value&1)<<pin);
  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, (!value)<<pin);
  //jshDebugPin(pin);
}


/**
 * Get the value of the corresponding pin.
 * \return The current value of the pin.
 */
bool jshPinGetValue(
    Pin pin //!< The pin to have its value read.
  ) {
  os_printf("> ESP8266: jshPinGetValue %d, %d\n", pin, GPIO_INPUT_GET(pin));
  return GPIO_INPUT_GET(pin);
}


/**
 *
 */
JsVarFloat jshPinAnalog(Pin pin) {
  os_printf("> ESP8266: jshPinAnalog: %d\n", pin);
  return (JsVarFloat) system_adc_read();
}


/**
 *
 */
int jshPinAnalogFast(Pin pin) {
  os_printf("> ESP8266: jshPinAnalogFast: %d\n", pin);
  return (JsVarFloat) system_adc_read();
}


/**
 *
 */
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  os_printf("ESP8266: jshPinAnalogOutput: %d, %d, %d\n", pin, (int)value, (int)freq);
  jsError("No DAC");
  return 0;
}


/**
 *
 */
void jshSetOutputValue(JshPinFunction func, int value) {
  os_printf("ESP8266: jshSetOutputValue %d %d\n", func, value);
  jsError("No DAC");
}


/**
 *
 */
void jshEnableWatchDog(JsVarFloat timeout) {
  os_printf("ESP8266: jshEnableWatchDog %0.3f\n", timeout);
}


/**
 * Get the state of the pin associated with the event flag.
 */
bool jshGetWatchedPinState(IOEventFlags eventFlag) {
  os_printf("> jshGetWatchedPinState eventFlag=%d\n", eventFlag);

  if (eventFlag > EV_EXTI_MAX || eventFlag < EV_EXTI0) {
    os_printf(" - Error ... eventFlag out of range\n");
    os_printf("< jshGetWatchedPinState\n");
    return false;
  }

  bool currentPinValue = jshPinGetValue((Pin)(eventFlag-EV_EXTI0));
  os_printf("< jshGetWatchedPinState = %d\n", currentPinValue);
  return currentPinValue;
}


/**
 * Set the value of the pin to be the value supplied and then wait for
 * a given period and set the pin value again to be the opposite.
 */
void jshPinPulse(
    Pin pin,        //!< The pin to be pulsed.
    bool value,     //!< The value to be pulsed into the pin.
    JsVarFloat time //!< The period in milliseconds to hold the pin.
  ) {
  if (jshIsPinValid(pin)) {
    //jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
    jshDelayMicroseconds(jshGetTimeFromMilliseconds(time));
    jshPinSetValue(pin, !value);
  } else
    jsError("Invalid pin!");
}


/**
 * Determine whether the pin can be watchable.
 * \return Returns true if the pin is wathchable.
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
  // As of right now, let us assume that all pins on an ESP8266 are watchable.
  os_printf("> jshCanWatch: %d\n", pin);
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
  os_printf("> jshPinWatch: pin=%d, shouldWatch=%d\n", pin, shouldWatch);
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
    jsError("Invalid pin (ESP8266)");
    os_printf("< jshPinWatch: Invalid pin\n");
    return EV_NONE;
  }
  os_printf("< jshPinWatch\n");
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

/**
 *
 */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
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
  esp8266_uartTransmitAll(device);
}


//===== SPI =====

/**
 * Unknown
 *
 *
 */
void jshSPISetup(
    IOEventFlags device, //!< Unknown
    JshSPIInfo *inf      //!< Unknown
  ) {
  os_printf("ESP8266: jshSPISetup: device=%d, inf=0x%x\n", device, (int)inf);
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device, //!< Unknown
    int data             //!< Unknown
  ) {
  os_printf("ESP8266: jshSPISend\n");
  return NAN;
}


/**
 * Send 16 bit data through the given SPI device.
 */
void jshSPISend16(
    IOEventFlags device, //!< Unknown
    int data             //!< Unknown
  ) {
  os_printf("ESP8266: jshSPISend16\n");
  jshSPISend(device, data >> 8);
  jshSPISend(device, data & 255);
}


/**
 * Set whether to send 16 bits or 8 over SPI.
 */
void jshSPISet16(
    IOEventFlags device, //!< Unknown
    bool is16            //!< Unknown
  ) {
  os_printf("ESP8266: jshSPISet16\n");
}


/**
 * Wait until SPI send is finished.
 */
void jshSPIWait(
    IOEventFlags device //!< Unknown
  ) {
  os_printf("ESP8266: jshSPIWait\n");
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

//===== I2C =====

/** Set-up I2C master for ESP8266, default pins are SCL:14, SDA:2. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  os_printf("ESP8266: jshI2CSetup SCL=%d SDA=%d bitrate=%d\n",
      info->pinSCL, info->pinSDA, info->bitrate);
  if (device != EV_I2C1) {
    jsError("Only I2C1 supported");
    return;
  }

  Pin scl = info->pinSCL >= 0 ? info->pinSCL : 14;
  Pin sda = info->pinSDA >= 0 ? info->pinSDA : 2;

  jshPinSetState(info->pinSCL, JSHPINSTATE_I2C);
  jshPinSetState(info->pinSDA, JSHPINSTATE_I2C);

  i2c_master_gpio_init(scl, sda, info->bitrate);
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
  jsError("No ACK");
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
  jsError("No ACK");
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
 * it WRT the crystal oscillator, i.e., to get the current clock ratio.
 *
 * The RTC timer is preserved when the chip goes into sleep mode, including deep sleep, as
 * well when it is reset (but not if reset using the ch_pd pin).
 *
 * It seems that the best course of action is to use the system timer for jshGetSystemTime()
 * and related functions and to use the rtc timer only at start-up to initialize the system
 * timer to the best guess available for the current date-time.
 */

/**
 * Given a time in milliseconds as float, get us the value in microsecond
 */
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
//  os_printf("jshGetTimeFromMilliseconds %d, %f\n", (JsSysTime)(ms * 1000.0), ms);
  return (JsSysTime) (ms * 1000.0 + 0.5);
} // End of jshGetTimeFromMilliseconds

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
//  os_printf("jshGetMillisecondsFromTime %d, %f\n", time, (JsVarFloat)time / 1000.0);
  return (JsVarFloat) time / 1000.0;
} // End of jshGetMillisecondsFromTime

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
JsSysTime jshGetSystemTime() { // in us
  return sysTimeStamp.timeStamp + (JsSysTime)(system_get_time() - sysTimeStamp.hwTimeStamp);
} // End of jshGetSystemTime


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
} // End of jshSetSystemTime

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

// There are two versions here. One uses the SDK timer in microsecond mode and the other uses the
// hw_timer using the driver provided by Espressif. The hw_timer is not working and causes the
// system to crash for unknown reasons. Thus using the SDK timer for now...
// If we're happy with the SDK timer then the other code can be deleted as well as the ht_timer.[hc]
// files...

#ifndef USE_HW_TIMER
os_timer_t utilTimer;

static void utilTimerInit(void) {
  os_printf("UStimer init\n");
  os_timer_disarm(&utilTimer);
  os_timer_setfn(&utilTimer, jstUtilTimerInterruptHandler, NULL);
}

void jshUtilTimerDisable() {
  os_printf("UStimer disarm\n");
  os_timer_disarm(&utilTimer);
}

void jshUtilTimerStart(JsSysTime period) {
  os_printf("UStimer arm\n");
  os_timer_arm_us(&utilTimer, (uint32_t)period, 0);
}

#else
os_event_t utilTimerQ[2];

static void utilTimerTask(os_event_t *e) {
  os_printf("HW timer task\n");
  jstUtilTimerInterruptHandler();
}

static void ICACHE_RAM_ATTR utilTimerCb(void) {
  os_printf_plus("HW timer CB\n");
  system_os_post(2, NULL, 0);
}

static void utilTimerInit(void) {
  system_os_task(utilTimerTask, 2, utilTimerQ, 2);
  hw_timer_init(FRC1_SOURCE, 0); // 0->one-time
  hw_timer_set_func(utilTimerCb);
}

void jshUtilTimerDisable() {
  hw_timer_set_func(NULL);
}

void jshUtilTimerStart(JsSysTime period) {
  os_printf("HW Timer: %ldus\n", (uint32_t)period);
  if (period < 50) {
    // the hardware timer can't do a delay of less than 100us
    os_delay_us((uint32_t)period);
    system_os_post(2, NULL, NULL);
    return;
  }
  if (period > 0x7fffffLL) {
    // the hardware timer can't do a delay of more than 0x7fffffus
    period = 0x7fffffLL; // FIXME !!!
  }
  hw_timer_arm((uint32_t)period);
}
#endif

void jshUtilTimerReschedule(JsSysTime period) {
  jshUtilTimerDisable();
  jshUtilTimerStart(period);
}

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
  os_printf("ESP8266: jshIsDeviceInitialised %d\n", device);
  return true;
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
  //os_printf("ESP8266: jshFlashRead: dest=%p for len=%ld from flash addr=0x%lx max=%ld\n",
  //    buf, len, addr, FLASH_MAX);

  // make sure we stay with the flash address space
  if (addr >= FLASH_MAX) return;
  if (addr + len > FLASH_MAX) len = FLASH_MAX - addr;
  addr += FLASH_MMAP;

  // copy the bytes reading a word from flash at a time
  uint8_t *dest = buf;
  uint32_t bytes = *(uint32_t*)(addr & ~3);
  while (len-- > 0) {
    if (addr & 3 == 0) bytes = *(uint32_t*)addr;
    *dest++ = ((uint8_t*)&bytes)[(uint32)addr++ & 3];
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
  //os_printf("ESP8266: jshFlashWrite: src=%p for len=%ld into flash addr=0x%lx\n",
  //    buf, len, addr);

  // make sure we stay with the flash address space
  if (addr >= FLASH_MAX) return;
  if (addr + len > FLASH_MAX) len = FLASH_MAX - addr;

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

  if (addr >= FLASH_MAX) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
  //os_printf("ESP8266: jshFlashErasePage: addr=0x%lx\n", addr);

  SpiFlashOpResult res;
  res = spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
  if (res != SPI_FLASH_RESULT_OK)
    os_printf("ESP8266: jshFlashErase%s\n",
      res == SPI_FLASH_RESULT_ERR ? "error" : "timeout");
}


/**
 * Callback for end of runtime.  This should never be called and has been
 * added to satisfy the linker.
 */
void _exit(int status) {
}
