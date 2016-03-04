/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific part of Hardware interface Layer
 * ----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.
#include "jswrap_bluetooth.h"

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_temp.h"
#include "nrf_timer.h"
#include "app_uart.h"

#ifdef NRF52
#include "nrf_saadc.h"
#else
#include "nrf_adc.h"
#endif

#include "nrf5x_utils.h"

#define SYSCLK_FREQ 32768 // this really needs to be a bit higher :)

/*  file:///home/gw/Downloads/S110_SoftDevice_Specification_2.0.pdf

  RTC0 not usable
  RTC1 used by app_timer.c
  TIMER1 used by jshardware util timer
  TIMER2 free
  SPI0/1 free

 */

static int init = 0; // Temporary hack to get jsiOneSecAfterStartup() going.
// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

void TIMER1_IRQHandler(void) {
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
  nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);
  jstUtilTimerInterruptHandler();
}


unsigned int getNRFBaud(int baud) {
  switch (baud) {
    case 1200: return UART_BAUDRATE_BAUDRATE_Baud1200;
    case 2400: return UART_BAUDRATE_BAUDRATE_Baud2400;
    case 4800: return UART_BAUDRATE_BAUDRATE_Baud4800;
    case 9600: return UART_BAUDRATE_BAUDRATE_Baud9600;
    case 14400: return UART_BAUDRATE_BAUDRATE_Baud14400;
    case 19200: return UART_BAUDRATE_BAUDRATE_Baud19200;
    case 28800: return UART_BAUDRATE_BAUDRATE_Baud28800;
    case 38400: return UART_BAUDRATE_BAUDRATE_Baud38400;
    case 57600: return UART_BAUDRATE_BAUDRATE_Baud57600;
    case 76800: return UART_BAUDRATE_BAUDRATE_Baud76800;
    case 115200: return UART_BAUDRATE_BAUDRATE_Baud115200;
    case 230400: return UART_BAUDRATE_BAUDRATE_Baud230400;
    case 250000: return UART_BAUDRATE_BAUDRATE_Baud250000;
    case 460800: return UART_BAUDRATE_BAUDRATE_Baud460800;
    case 921600: return UART_BAUDRATE_BAUDRATE_Baud921600;
    case 1000000: return UART_BAUDRATE_BAUDRATE_Baud1M;
    default: return 0; // error
  }
}


void jshInit() {
  jshInitDevices();
  nrf_utils_lfclk_config_and_start();

  BITFIELD_CLEAR(jshPinSoftPWM);
    
  JshUSARTInfo inf;
  jshUSARTInitInfo(&inf);
  inf.pinRX = DEFAULT_CONSOLE_RX_PIN;
  inf.pinTX = DEFAULT_CONSOLE_TX_PIN;
  inf.baudRate = DEFAULT_CONSOLE_BAUDRATE;
  jshUSARTSetup(EV_SERIAL1, &inf); // Initialize UART for communication with Espruino/terminal.
  init = 1;

  // Enable and sort out the timer
  nrf_timer_mode_set(NRF_TIMER1,NRF_TIMER_MODE_TIMER);
  nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_frequency_set(NRF_TIMER1, NRF_TIMER_FREQ_1MHz); // hmm = only a few options here

  // Irq setup
  NVIC_SetPriority(TIMER1_IRQn, 3); // low - don't mess with BLE :)
  NVIC_ClearPendingIRQ(TIMER1_IRQn);
  NVIC_EnableIRQ(TIMER1_IRQn);
  nrf_timer_int_enable(NRF_TIMER1, NRF_TIMER_INT_COMPARE0_MASK );

  // Pin change
  nrf_drv_gpiote_init();
  jswrap_nrf_bluetooth_init();
}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {
  jshResetDevices();
  // TODO: Reset all pins to their power-on state (apart from default UART :)
}

void jshKill() {

}

// stuff to do on idle
void jshIdle() {
  if (init == 1) {
    jsiOneSecondAfterStartup(); // Do this the first time we enter jshIdle() after we have called jshInit() and never again.
    init = 0;
  }
}

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars) {
    if (maxChars <= 0)
    {
    	return 0;
    }
	return nrf_utils_get_device_id(data, maxChars);
}

// is the serial device connected?
bool jshIsUSBSERIALConnected() {
  return true;
}

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime() {
  // Use RTC0 (also used by BLE stack) - as app_timer starts/stops RTC1
  return (JsSysTime)NRF_RTC0->COUNTER;
}

/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime time) {

}

/// Convert a time in Milliseconds to one in ticks.
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) ((ms * SYSCLK_FREQ) / 1000);
}

/// Convert ticks to a time in Milliseconds.
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (JsVarFloat) ((time * 1000) / SYSCLK_FREQ);
}

// software IO functions...
void jshInterruptOff() {
  __disable_irq(); // Disabling interrupts is not reasonable when using one of the SoftDevices.
}

void jshInterruptOn() {
  __enable_irq(); // *** This wont be good with SoftDevice!
}

void jshDelayMicroseconds(int microsec) {
  if (microsec <= 0) {
    return;
  }

  nrf_utils_delay_us((uint32_t) microsec);
}

void jshPinSetValue(Pin pin, bool value) {
  nrf_gpio_pin_write((uint32_t)pinInfo[pin].pin, value);
}

bool jshPinGetValue(Pin pin) {
  return (bool)nrf_gpio_pin_read((uint32_t)pinInfo[pin].pin);
}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }

  uint32_t ipin = (uint32_t)pinInfo[pin].pin;
  switch (state) {
    case JSHPINSTATE_UNDEFINED :
      nrf_gpio_cfg_default(ipin);
      break;
    case JSHPINSTATE_GPIO_OUT :
      nrf_gpio_cfg_output(ipin);
      break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN :
      NRF_GPIO->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_GPIO_IN :
      nrf_gpio_cfg_input(ipin, NRF_GPIO_PIN_NOPULL);
      break;
    case JSHPINSTATE_GPIO_IN_PULLUP :
      nrf_gpio_cfg_input(ipin, NRF_GPIO_PIN_PULLUP);
      break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN :
      nrf_gpio_cfg_input(ipin, NRF_GPIO_PIN_PULLDOWN);
      break;
    /*case JSHPINSTATE_ADC_IN :
      break;
    case JSHPINSTATE_AF_OUT :
      break;
    case JSHPINSTATE_AF_OUT_OPENDRAIN :
      break;
    case JSHPINSTATE_USART_IN :
      break;
    case JSHPINSTATE_USART_OUT :
      break;
    case JSHPINSTATE_DAC_OUT :
      break;*/
    case JSHPINSTATE_I2C :
      NRF_GPIO->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
      // may need to be set to GPIO_PIN_CNF_DIR_Output as well depending on I2C state?
      break;
    default : assert(0);
      break;
  }
}

/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin) {
  return (JshPinState) nrf_utils_gpio_pin_get_state((uint32_t)pinInfo[pin].pin);
}

#ifdef NRF52
nrf_saadc_value_t nrf_analog_read() {
  nrf_saadc_value_t result;
  nrf_saadc_buffer_init(&result,1);

  nrf_saadc_task_trigger(NRF_SAADC_TASK_START);

  while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED));
  nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);

  nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);


  while(!nrf_saadc_event_check(NRF_SAADC_EVENT_END));
  nrf_saadc_event_clear(NRF_SAADC_EVENT_END);

  nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
  while(!nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED));
  nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);

  return result;
}
#endif

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin) {
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return NAN;
#ifdef NRF52
  // sanity checks for channel
  assert(NRF_SAADC_INPUT_AIN0 == 1);
  assert(NRF_SAADC_INPUT_AIN1 == 2);
  assert(NRF_SAADC_INPUT_AIN2 == 3);
  nrf_saadc_input_t ain = 1 + (pinInfo[pin].analog & JSH_MASK_ANALOG_CH);

  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_3US;
  config.gain = NRF_SAADC_GAIN1_4; // 1/4 of input volts
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = ain;
  config.pin_n = ain;
  config.reference = NRF_SAADC_REFERENCE_VDD4; // VDD/4 as reference.
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

  // make reading
  nrf_saadc_enable();
  nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_14BIT);
  nrf_saadc_channel_init(0, &config);

  return nrf_analog_read() / 8192.0;
#else
  const nrf_adc_config_t nrf_adc_config =  {
      NRF_ADC_CONFIG_RES_10BIT,
      NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
      NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) / 1024.0;
#endif
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return 0;

#ifdef NRF52
  // sanity checks for channel
  assert(NRF_SAADC_INPUT_AIN0 == 1);
  assert(NRF_SAADC_INPUT_AIN1 == 2);
  assert(NRF_SAADC_INPUT_AIN2 == 3);
  nrf_saadc_input_t ain = 1 + (pinInfo[pin].analog & JSH_MASK_ANALOG_CH);

  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_3US;
  config.gain = NRF_SAADC_GAIN1_4; // 1/4 of input volts
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = ain;
  config.pin_n = ain;
  config.reference = NRF_SAADC_REFERENCE_VDD4; // VDD/4 as reference.
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

  // make reading
  nrf_saadc_enable();
  nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_8BIT);
  nrf_saadc_channel_init(0, &config);

  return nrf_analog_read() << 8;
#else
  const nrf_adc_config_t nrf_adc_config =  {
        NRF_ADC_CONFIG_RES_8BIT, // 8 bit for speed (hopefully!)
        NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
        NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) << 8;
#endif
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  /* we set the bit field here so that if the user changes the pin state
   * later on, we can get rid of the IRQs */
  if (!jshGetPinStateIsManual(pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  }
  BITFIELD_SET(jshPinSoftPWM, pin, 1);
  if (freq<=0) freq=50;
  jstPinPWM(freq, value, pin);
  return JSH_NOTHING;
} // if freq<=0, the default is used

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {
  // ---- USE TIMER FOR PULSE
  if (!jshIsPinValid(pin)) {
       jsExceptionHere(JSET_ERROR, "Invalid pin!");
       return;
  }
  if (pulseTime<=0) {
    // just wait for everything to complete
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
}


static IOEventFlags jshGetEventFlagsForWatchedPin(nrf_drv_gpiote_pin_t pin) {
  uint32_t addr = nrf_drv_gpiote_in_event_addr_get(pin);

  // sigh. all because the right stuff isn't exported. All we wanted was channel_port_get
  int i;
  for (i=0;i<NUMBER_OF_GPIO_TE;i++)
    if (addr == nrf_gpiote_event_addr_get((nrf_gpiote_events_t)((uint32_t)NRF_GPIOTE_EVENTS_IN_0+(sizeof(uint32_t)*i))))
      return EV_EXTI0+i;
  return EV_NONE;
}

bool lastHandledPinState; ///< bit of a hack, this... Ideally get rid of WatchedPinState completely and add to jshPushIOWatchEvent
static void jsvPinWatchHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  lastHandledPinState = (bool)nrf_gpio_pin_read(pin);
  IOEventFlags evt = jshGetEventFlagsForWatchedPin(pin);
  jshPushIOWatchEvent(evt);
}


///< Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  return true;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  if (!jshIsPinValid(pin)) return EV_NONE;
  uint32_t p = (uint32_t)pinInfo[pin].pin;
  if (shouldWatch) {
    nrf_drv_gpiote_in_config_t cls_1_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    nrf_drv_gpiote_in_init(p, &cls_1_config, jsvPinWatchHandler);
    nrf_drv_gpiote_in_event_enable(p, true);
    return jshGetEventFlagsForWatchedPin(p);
  } else {
    nrf_drv_gpiote_in_event_disable(p);
    return EV_NONE;
  }
} // start watching pin - return the EXTI associated with it

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  return JSH_NOTHING;
}

/// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value) {
}

/// Enable watchdog with a timeout in seconds
void jshEnableWatchDog(JsVarFloat timeout) {
}

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  return lastHandledPinState;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == jshGetEventFlagsForWatchedPin((uint32_t)pinInfo[pin].pin);
}

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device) {
  return false;
}

bool uartIsSending = false;
bool uartInitialised = false;

void uart0_event_handle(app_uart_evt_t * p_event) {
  if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
    jshPushIOEvent(IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(EV_SERIAL1) | EV_SERIAL_STATUS_FRAMING_ERR, 0);
  } else if (p_event->evt_type == APP_UART_TX_EMPTY) {
    int ch = jshGetCharToTransmit(EV_SERIAL1);
    if (ch >= 0) {
      uartIsSending = true;
      while (app_uart_put((uint8_t)ch) != NRF_SUCCESS);
    } else
      uartIsSending = false;
  } else if (p_event->evt_type == APP_UART_DATA) {
    uint8_t character;
    while (app_uart_get(&character) != NRF_SUCCESS);
    jshPushIOCharEvent(EV_SERIAL1, (char) character);
  }
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  if (device != EV_SERIAL1)
    return;

  int baud = getNRFBaud(inf->baudRate);
  if (baud==0)
    return jsError("Invalid baud rate %d", inf->baudRate);
  if (!jshIsPinValid(inf->pinRX) || !jshIsPinValid(inf->pinTX))
    return jsError("Invalid RX or TX pins");

  uint32_t err_code;
  const app_uart_comm_params_t comm_params = {
      pinInfo[inf->pinRX].pin,
      pinInfo[inf->pinTX].pin,
      (uint8_t)UART_PIN_DISCONNECTED,
      (uint8_t)UART_PIN_DISCONNECTED,
      APP_UART_FLOW_CONTROL_DISABLED,
      inf->parity!=0, // TODO: ODD or EVEN parity?
      baud
  };

  APP_UART_INIT(&comm_params,
                uart0_event_handle,
                APP_IRQ_PRIORITY_HIGH,
                err_code);
  APP_ERROR_CHECK(err_code);
}

/** Kick a device into action (if required). For instance we may need to set up interrupts */
void jshUSARTKick(IOEventFlags device) {

  if (device == EV_BLUETOOTH) {
    /* For bluetooth, start transmit after one character.
      The BLE_EVT_TX_COMPLETE event will get triggered and
      will auto-reload whatever needs sending. */
    bool jswrap_nrf_transmit_string();
    jswrap_nrf_transmit_string();
  }
  
  if (device == EV_SERIAL1 && !uartIsSending) {
    jshInterruptOff();
    int ch = jshGetCharToTransmit(EV_SERIAL1);
    if (ch >= 0) {
      // put data - this will kick off the USART
      while (app_uart_put((uint8_t)ch) != NRF_SUCCESS);
      uartIsSending = true;
    }
    jshInterruptOn();
  }
}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {

}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
  return -1;
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {

}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {

}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {

}

/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device) {

}

const nrf_drv_twi_t TWI1 = NRF_DRV_TWI_INSTANCE(1);
bool twi1Initialised = false;

const nrf_drv_twi_t *jshGetTWI(IOEventFlags device) {
  if (device == EV_I2C1) return &TWI1;
  return 0;
}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  if (!jshIsPinValid(inf->pinSCL) || !jshIsPinValid(inf->pinSDA)) {
    jsError("SDA and SCL pins must be valid, got %d and %d\n", inf->pinSDA, inf->pinSCL);
    return;
  }
  const nrf_drv_twi_t *twi = jshGetTWI(device);
  if (!twi) return;
  // http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk51.v9.0.0%2Fhardware_driver_twi.html&cp=4_1_0_2_10
  nrf_drv_twi_config_t    p_twi_config;
  p_twi_config.scl = (uint32_t)pinInfo[inf->pinSCL].pin;
  p_twi_config.sda = (uint32_t)pinInfo[inf->pinSDA].pin;
  p_twi_config.frequency = (inf->bitrate<175000) ? NRF_TWI_FREQ_100K : ((inf->bitrate<325000) ? NRF_TWI_FREQ_250K : NRF_TWI_FREQ_400K);
  p_twi_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;
  if (twi1Initialised) nrf_drv_twi_uninit(twi);
  twi1Initialised = true;
  uint32_t err_code = nrf_drv_twi_init(twi, &p_twi_config, NULL, NULL);
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "I2C Initialisation Error %d\n", err_code);
  else
    nrf_drv_twi_enable(twi);
}

/** Addresses are 7 bit - that is, between 0 and 0x7F. sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  const  nrf_drv_twi_t *twi = jshGetTWI(device);
  if (!twi) return;
  uint32_t err_code = nrf_drv_twi_tx(twi, address, data, nBytes, !sendStop);
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "I2C Write Error %d\n", err_code);
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  const nrf_drv_twi_t *twi = jshGetTWI(device);
  if (!twi) return;
  uint32_t err_code = nrf_drv_twi_rx(twi, address, data, nBytes);
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "I2C Read Error %d\n", err_code);
}

/// Return start address and size of the flash page the given address resides in. Returns false if no page.
bool jshFlashGetPage(uint32_t addr, uint32_t * startAddr, uint32_t * pageSize)
{
  if (!nrf_utils_get_page(addr, startAddr, pageSize))
  {
	  return false;
  }
  return true;
}

/// Erase the flash page containing the address.
void jshFlashErasePage(uint32_t addr)
{
  uint32_t startAddr;
  uint32_t pageSize;
  if (!jshFlashGetPage(addr, &startAddr, &pageSize))
    return;
  nrf_utils_erase_flash_page(startAddr);
}

/**
 * Reads a byte from memory. Addr doesn't need to be word aligned and len doesn't need to be a multiple of 4.
 */
void jshFlashRead(void * buf, uint32_t addr, uint32_t len)
{
  uint8_t * read_buf = buf;
  nrf_utils_read_flash_bytes(read_buf, addr, len);
}

/**
 * Writes an array of bytes to memory. Addr must be word aligned and len must be a multiple of 4.
 */
void jshFlashWrite(void * buf, uint32_t addr, uint32_t len)
{
  uint8_t * write_buf = buf;
  if ((addr & (3UL)) != 0 || (len % 4) != 0)
  {
	  return;
  }
  nrf_utils_write_flash_bytes(addr, write_buf, len);
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  jstSetWakeUp(timeUntilWake);
  sd_app_evt_wait(); // Go to sleep, wait to be woken up

  return true;
}

/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {
  period = period * 1000000 / SYSCLK_FREQ;
  if (period < 2) period=2;
  if (period > 0xFFFFFFFF) period=0xFFFFFFFF;
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
  nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, (uint32_t)period);
}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {
  jshUtilTimerReschedule(period);
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
}

/// Stop the timer
void jshUtilTimerDisable() {
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_STOP);
}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature() {
  nrf_temp_init();

  return nrf_temp_read() / 4.0;
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef() {
#ifdef NRF52
  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_3US;
  config.gain = NRF_SAADC_GAIN1_6; // 1/6 of input volts
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = NRF_SAADC_INPUT_VDD;
  config.pin_n = NRF_SAADC_INPUT_VDD;
  config.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6v reference.
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

  // make reading
  nrf_saadc_enable();
  nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_14BIT);
  nrf_saadc_channel_init(0, &config);

  return 6.0 * (nrf_analog_read() * 0.6 / 8192.0);
#else
  const nrf_adc_config_t nrf_adc_config =  {
       NRF_ADC_CONFIG_RES_10BIT,
       NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
       NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  return 1.2 / nrf_adc_convert_single(ADC_CONFIG_PSEL_AnalogInput0);
#endif
}

/**
 * Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()`
 */
unsigned int jshGetRandomNumber() {
  return (unsigned int) nrf_utils_get_random_number();
}
