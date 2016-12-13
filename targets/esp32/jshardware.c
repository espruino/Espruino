/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions.
 * ----------------------------------------------------------------------------
 */


/**
 * The ESP32 must implement its part of the Espruino contract.  This file
 * provides implementations for interfaces that are expected to be provided
 * by an Espruino board.  The signatures of the exposed functions are part
 * of the Espruino environment and can not be changed without express
 * approval from all the stakeholders.  In addition, the semantics of the
 * functions should follow the expected conventions.
 */
#include <stdio.h>

#include "jshardware.h"
#include "jshardwareAnalog.h"
#include "jsutils.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jspininfo.h"

#include "jswrap_esp32_network.h"

#include "esp_attr.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "rom/uart.h"
#include "driver/gpio.h"

#include "esp32-hal-spi.h"
#include "esp32-hal-i2c.h"

#define FLASH_MAX (4*1024*1024) //4MB
#define FLASH_PAGE_SHIFT 12 // Shift is much faster than division by 4096 (size of page)
#define FLASH_PAGE ((uint32_t)1<<FLASH_PAGE_SHIFT)  //4KB

#define UNUSED(x) (void)(x)

// The logging tag used for log messages issued by this file.
static char *tag = "jshardware";
static char *tagGPIO = "jshardware(GPIO)";

static spi_t *VSPI_spi;
static spi_t *HSPI_spi;
static uint32_t  g_lastSPIRead = (uint32_t)-1;

// Convert an Espruino pin to an ESP32 pin number.
static gpio_num_t pinToESP32Pin(Pin pin);
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

static uint8_t g_pinState[JSH_PIN_COUNT];

/**
* interrupt handler for gpio interrupts
*/
void IRAM_ATTR gpio_intr_test(void* arg){
  //GPIO intr process. Mainly copied from esp-idf
  UNUSED(arg);
  IOEventFlags exti;
  Pin gpio_num = 0;
  uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31
  uint32_t gpio_intr_status_h = READ_PERI_REG(GPIO_STATUS1_REG);//read status1 to get interrupt status for GPIO32-39
  SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);    //Clear intr for gpio0-gpio31
  SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status_h); //Clear intr for gpio32-39
  do {
    if(gpio_num < 32) {
      if(gpio_intr_status & BIT(gpio_num)) { //gpio0-gpio31
		exti = pinToEV_EXTI(gpio_num);
		jshPushIOWatchEvent(exti);
      }
    } else {
      if(gpio_intr_status_h & BIT(gpio_num - 32)) {
		exti = pinToEV_EXTI(gpio_num);
		jshPushIOWatchEvent(exti);
      }
    }
  } while(++gpio_num < GPIO_PIN_COUNT);
}

/**
 * Initialize the JavaScript hardware interface.
 */
void jshInit() {
  ESP_LOGD(tag,">> jshInit");
  uint32_t freeHeapSize = esp_get_free_heap_size();
  ESP_LOGD(tag, "Free heap size: %d", freeHeapSize);
  spi_flash_init();
  esp32_wifi_init();
  jshInitDevices();
  gpio_isr_register(gpio_intr_test,NULL,0,NULL);  //changed to automatic assign of interrupt
   // Initialize something for each of the possible pins.
  for (int i=0; i<JSH_PIN_COUNT; i++) {
    g_pinState[i] = 0;
  }
  ESP_LOGD(tag,"<< jshInit");
} // End of jshInit


/**
 * Reset the Espruino environment.
 */
void jshReset() {
  ESP_LOGD(tag,">> jshReset");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshReset");
}

/**
 * Re-init the ESP32 after a soft-reset
 */
void jshSoftInit() {
  ESP_LOGD(tag,">> jshSoftInit");
  jswrap_ESP32_wifi_soft_init();
  ESP_LOGD(tag,"<< jshSoftInit");
}

/**
 * Handle whatever needs to be done in the idle loop when there's nothing to do.
 *
 * Nothing is needed on the ESP32.
 */
void jshIdle() {
  //ESP_LOGD(tag,">> jshIdle");  // Can't debug log as called too often.
  // Here we poll the serial input looking for a new character which, if we
  // find, we add to the input queue of input events.  This is going to be
  // wrong for a variety of reasons including:
  //
  // * What if we want to use the serial for data input?
  // * Busy polling is never good ... we should eventually use an interrupt
  //   driven mechanism.
  //
  //char rxChar;
  //STATUS status = uart_rx_one_char((uint8_t *)&rxChar);
  //if (status == OK) {
  //  jshPushIOCharEvents(EV_SERIAL1, &rxChar, 1);
  // }
  //ESP_LOGD(tag,"<< jshIdle");  // Can't debug log as called too often.
}

// ESP32 chips don't have a serial number but they do have a MAC address
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  ESP_LOGD(tag,">> jshGetSerialNumber");
  assert(maxChars >= 6); // it's 32
  esp_wifi_get_mac(WIFI_IF_STA, data);
  ESP_LOGD(tag,"<< jshGetSerialNumber %.2x%.2x%.2x%.2x%.2x%.2x",
      data[0], data[1], data[2], data[3], data[4], data[5]);
  return 6;
}

//===== Interrupts and sleeping
//Mux to protect the JshInterrupt status
//static portMUX_TYPE xJshInterrupt = portMUX_INITIALIZER_UNLOCKED;
void jshInterruptOff() { 
	//xTaskResumeAll();
  //taskEXIT_CRITICAL(&xJshInterrupt);
  taskDISABLE_INTERRUPTS();
}

void jshInterruptOn()  {
  //taskENTER_CRITICAL(&xJshInterrupt);
  taskENABLE_INTERRUPTS();
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  UNUSED(timeUntilWake);
  //ESP_LOGD(tag,">> jshSleep");  // Can't debug log as called too often.
  //ESP_LOGD(tag,"<< jshSleep");  // Can't debug log as called too often.
   return true;
} // End of jshSleep


/**
 * Delay (blocking) for the supplied number of microseconds.
 */
void jshDelayMicroseconds(int microsec) {
  TickType_t ticks = (TickType_t)microsec / (1000 * portTICK_PERIOD_MS);
  vTaskDelay(ticks);
} // End of jshDelayMicroseconds



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
  gpio_mode_t mode;
  gpio_pull_mode_t pull_mode=GPIO_FLOATING;
  switch(state) {
  case JSHPINSTATE_GPIO_OUT:
    mode = GPIO_MODE_OUTPUT;
    break;
  case JSHPINSTATE_GPIO_IN:
    mode = GPIO_MODE_INPUT;
    break;
  case JSHPINSTATE_GPIO_IN_PULLUP:
    mode = GPIO_MODE_INPUT;
	pull_mode=GPIO_PULLUP_ONLY;	
    break;
  case JSHPINSTATE_GPIO_IN_PULLDOWN:
    mode = GPIO_MODE_INPUT;
	pull_mode=GPIO_PULLDOWN_ONLY;	
    break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
    mode = GPIO_MODE_OUTPUT_OD;
    break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
    mode = GPIO_MODE_OUTPUT_OD;
	pull_mode=GPIO_PULLUP_ONLY;
    break;
  default:
    ESP_LOGE(tag, "jshPinSetState: Unexpected state: %d", state);
  return;
  }
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  gpio_set_direction(gpioNum, mode);
  gpio_set_pull_mode(gpioNum, pull_mode);
  gpio_pad_select_gpio(gpioNum);
  g_pinState[pin] = state; // remember what we set this to...
}


/**
 * Return the current state of the selected pin.
 * \return The current state of the selected pin.
 */
JshPinState jshPinGetState(Pin pin) {
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
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  gpio_set_level(gpioNum, (uint32_t)value);
}


/**
 * Get the value of the corresponding pin.
 * \return The current value of the pin.
 */
bool CALLED_FROM_INTERRUPT jshPinGetValue( // can be called at interrupt time
    Pin pin //!< The pin to have its value read.
  ) {
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  bool level = gpio_get_level(gpioNum);
  return level;
}


JsVarFloat jshPinAnalog(Pin pin) {
  //ESP_LOGD(tag,">> jshPinAnalog: pin=%d", pin);
  //ESP_LOGD(tag, "Not implemented");
  //ESP_LOGD(tag,"<< jshPinAnalog");
  return (JsVarFloat) readADC(pin) / 4096;
}


int jshPinAnalogFast(Pin pin) {
  //ESP_LOGD(tag,">> jshPinAnalogFast: pin=%d", pin);
  //ESP_LOGD(tag, "Not implemented");
  //ESP_LOGD(tag,"<< jshPinAnalogFast");
  return readADC(pin) << 4;
}


/**
 * Set the output PWM value.
 */
JshPinFunction jshPinAnalogOutput(Pin pin,
    JsVarFloat value,
    JsVarFloat freq,
    JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  UNUSED(freq);
  UNUSED(flags);
  value = (value * 256);
  uint8_t val = value;
  if(pin == 25 || pin == 26){
	writeDAC(pin,val);
  }
  return 0;
}


/**
 *
 */
void jshSetOutputValue(JshPinFunction func, int value) {
  UNUSED(func);
  UNUSED(value);
  ESP_LOGD(tag,">> JshPinFunction");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< JshPinFunction");
}


/**
 *
 */
void jshEnableWatchDog(JsVarFloat timeout) {
  UNUSED(timeout);
  ESP_LOGD(tag,">> jshEnableWatchDog");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshEnableWatchDog");
}


// Kick the watchdog
void jshKickWatchDog() {
  ESP_LOGD(tag,">> jshKickWatchDog");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshKickWatchDog");
}


/**
 * Get the state of the pin associated with the event flag.
 */
bool CALLED_FROM_INTERRUPT jshGetWatchedPinState(IOEventFlags eventFlag) { // can be called at interrupt time
  gpio_num_t gpioNum = pinToESP32Pin((Pin)(eventFlag-EV_EXTI0));
  bool level = gpio_get_level(gpioNum);
  return level;
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
  UNUSED(pulseTime);
  ESP_LOGD(tag,">> jshPinPulse: pin=%d, polarity=%d", pin, pulsePolarity);
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshPinPulse");
}


/**
 * Determine whether the pin can be watchable.
 * \return Returns true if the pin is wathchable.
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
  UNUSED(pin);
  return true; //lets assume all pins will do
}


/**
 * Do what ever is necessary to watch a pin.
 * \return The event flag for this pin.
 */
IOEventFlags jshPinWatch(
    Pin pin,         //!< The pin to be watched.
    bool shouldWatch //!< True for watching and false for unwatching.
  ) {
      gpio_num_t gpioNum = pinToESP32Pin(pin);
	  if(shouldWatch){
		gpio_set_intr_type(gpioNum,GPIO_INTR_ANYEDGE);             //set posedge interrupt
		gpio_set_direction(gpioNum,GPIO_MODE_INPUT);               //set as input
		gpio_set_pull_mode(gpioNum,GPIO_PULLUP_ONLY);              //enable pull-up mode
		gpio_intr_enable(gpioNum);                                 //enable interrupt
	  }
	  else{
		if(gpio_intr_disable(gpioNum) == ESP_ERR_INVALID_ARG){     //disable interrupt
			ESP_LOGE(tagGPIO,"*** jshPinWatch error");
		}
	  }
      return pin;
}


/**
 *
 */
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  ESP_LOGD(tag,">> jshGetCurrentPinFunction: pin=%d", pin);
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshGetCurrentPinFunction");
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
  UNUSED(device);
  UNUSED(inf);
  ESP_LOGD(tag,">> jshUSARTSetup");
  ESP_LOGD(tag,"<< jshUSARTSetup");
}

bool jshIsUSBSERIALConnected() {
  ESP_LOGD(tag,">> jshIsUSBSERIALConnected");
  ESP_LOGD(tag,"<< jshIsUSBSERIALConnected");
  return false; // "On non-USB boards this just returns false"
}

/**
 * Kick a device into action (if required).
 *
 */
void jshUSARTKick(
    IOEventFlags device //!< The device to be kicked.
) {
    //ESP_LOGD(tag,">> jshUSARTKick");
  int c = jshGetCharToTransmit(device);
  while(c >= 0) {
    uart_tx_one_char((uint8_t)c);
    c = jshGetCharToTransmit(device);
  }
  //ESP_LOGD(tag,"<< jshUSARTKick");
}

//===== SPI =====

/**
 * Initialize the hardware SPI device.
 * On the ESP32, hardware SPI is implemented via a set of default pins defined
 * as follows:
 *
 *
 */
void jshSPISetup(
    IOEventFlags device, //!< The identity of the SPI device being initialized.
    JshSPIInfo *inf      //!< Flags for the SPI device.
) {
  ESP_LOGD(tag,">> jshSPISetup device=%s, baudRate=%d, spiMode=%d, spiMSB=%d",
      jshGetDeviceString(device),
      inf->baudRate,
      inf->spiMode,
      inf->spiMSB);
  spi_t *spi = NULL;
  uint8_t dataMode = SPI_MODE0;
  switch(inf->spiMode) {
  case 0:
    dataMode = SPI_MODE0;
    break;
  case 1:
    dataMode = SPI_MODE1;
    break;
  case 2:
    dataMode = SPI_MODE2;
    break;
  case 3:
    dataMode = SPI_MODE3;
    break;
  }
  uint8_t bitOrder;
  if (inf->spiMSB == true) {
    bitOrder = SPI_MSBFIRST;
  } else {
    bitOrder = SPI_LSBFIRST;
  }
  switch(device) {
  case EV_SPI1:
    HSPI_spi = spiStartBus(HSPI, 1000000, dataMode, bitOrder);
    spi = HSPI_spi;
    break;
  case EV_SPI2:
    VSPI_spi = spiStartBus(VSPI, 1000000, dataMode, bitOrder);
    spi = VSPI_spi;
    break;
  default:
    ESP_LOGW(tag, "Unexpected device for SPI initialization: %d", device);
    break;
  }
  if (spi != NULL) {
    spiAttachSCK(spi, -1);
    spiAttachMISO(spi, -1);
    spiAttachMOSI(spi, -1);
    spiAttachSS(spi, 0, -1);
    spiEnableSSPins(spi, 1<<0);
    spiSSEnable(spi);
  }
  ESP_LOGD(tag,"<< jshSPISetup");
}


/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(
    IOEventFlags device, //!< The identity of the SPI device through which data is being sent.
    int data             //!< The data to be sent or an indication that no data is to be sent.
) {
  ESP_LOGD(tag,">> jshSPISend device=%s, data=%x", jshGetDeviceString(device), data);
  spi_t *spi;
  switch(device) {
  case EV_SPI1:
    spi = HSPI_spi;
    break;
  case EV_SPI2:
    spi= VSPI_spi;
    break;
  default:
    return -1;
  }

  if (device != EV_SPI1 && device != EV_SPI2) {
    return -1;
  }

  //os_printf("> jshSPISend - device=%d, data=%x\n", device, data);
  int retData = (int)g_lastSPIRead;
  if (data >=0) {
    // Send 8 bits of data taken from "data" over the selected spi and store the returned
    // data for subsequent retrieval.
    spiTransferBits(spi, (uint32_t)data, &g_lastSPIRead, 8);
  } else {
    g_lastSPIRead = (uint32_t)-1;
  }
  ESP_LOGD(tag,"<< jshSPISend");
  return (int)retData;
}


/**
 * Send 16 bit data through the given SPI device.
 */
void jshSPISend16(
    IOEventFlags device, //!< Unknown
    int data             //!< Unknown
) {
  UNUSED(device);
  UNUSED(data);
  ESP_LOGD(tag,">> jshSPISend16");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshSPISend16");
}


/**
 * Set whether to send 16 bits or 8 over SPI.
 */
void jshSPISet16(
    IOEventFlags device, //!< Unknown
    bool is16            //!< Unknown
) {
  UNUSED(device);
  UNUSED(is16);
  ESP_LOGD(tag,">> jshSPISet16");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshSPISet16");
}


/**
 * Wait until SPI send is finished.
 */
void jshSPIWait(
    IOEventFlags device //!< Unknown
) {
  UNUSED(device);
  ESP_LOGD(tag,">> jshSPIWait");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshSPIWait");
}


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  UNUSED(device);
  UNUSED(isReceive);
  ESP_LOGD(tag,">> jshSPISetReceive");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshSPISetReceive");
}

//===== I2C =====

// Let's get this working with only one device
i2c_t * i2c=NULL;

/** Set-up I2C master for ESP32, default pins are SCL:21, SDA:22. Only device I2C1 is supported
 *  and only master mode. */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *info) {
  if (device != EV_I2C1) {
    jsError("Only I2C1 supported"); 
	return;
  }
  Pin scl = info->pinSCL != PIN_UNDEFINED ? info->pinSCL : 21;
  Pin sda = info->pinSDA != PIN_UNDEFINED ? info->pinSDA : 22;

  //jshPinSetState(scl, JSHPINSTATE_I2C);
  //jshPinSetState(sda, JSHPINSTATE_I2C);
   
  int num=1; // Master mode only 0 is slave mode..
 
  i2c_err_t err;
  i2c = i2cInit(num, 0, false);
  //ESP_LOGE(tag, "jshI2CSetup: Frequency: %d", info->bitrate);
  err=i2cSetFrequency(i2c, (uint32_t)info->bitrate);
  if ( err != I2C_ERROR_OK ) {
    ESP_LOGE(tag, "jshI2CSetup: i2cSetFrequency error: %d", err);
	return;
  }
  err=i2cAttachSDA(i2c, pinToESP32Pin(sda));
  if ( err != I2C_ERROR_OK ) {
    ESP_LOGE(tag, "jshI2CSetup: i2cAttachSDA error: %d", err);
	return;
  }  
  err=i2cAttachSCL(i2c, pinToESP32Pin(scl));
  if ( err != I2C_ERROR_OK ) {
    ESP_LOGE(tag, "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}

void jshI2CWrite(IOEventFlags device,
  unsigned char address,
  int nBytes,
  const unsigned char *data,
  bool sendStop) {
// i2cWrite(i2c_t * i2c, uint16_t address, bool addr_10bit, uint8_t * data, uint8_t len, bool sendStop);
  i2c_err_t err=i2cWrite(i2c,address,false,data,nBytes,sendStop);
  if ( err != I2C_ERROR_OK ) {
    ESP_LOGE(tag, "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}

void jshI2CRead(IOEventFlags device,
  unsigned char address,
  int nBytes,
  unsigned char *data,
  bool sendStop) {
  i2c_err_t err=i2cRead(i2c,address,false,data,nBytes,sendStop);
  if ( err != I2C_ERROR_OK ) {
    ESP_LOGE(tag, "jshI2CSetup: i2cAttachSCL error: %d", err);
	return;
  }
}

//===== System time stuff =====

/**
 * Given a time in milliseconds as float, get us the value in microsecond
 */
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) (ms * 1000.0);
}

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (JsVarFloat) time / 1000.0;
}


/**
 * Return the current time in microseconds.
 */
JsSysTime CALLED_FROM_INTERRUPT jshGetSystemTime() { // in us -- can be called at interrupt time
  struct timeval tm;
  gettimeofday(&tm, 0);
  return (JsSysTime)(tm.tv_sec)*1000000L + tm.tv_usec;
}

/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime newTime) {
  struct timeval tm;
  struct timezone tz;
  
  tm.tv_sec=(time_t)(newTime/1000000L);
  tm.tv_usec=0;
  tz.tz_minuteswest=0;
  tz.tz_dsttime=0;
  settimeofday(&tm, &tz);
  ESP_LOGD(tag,"<< jshSetSystemTime");
}

void jshUtilTimerDisable() {
  ESP_LOGD(tag,">> jshUtilTimerDisable");
  ESP_LOGD(tag,"<< jshUtilTimerDisable");
}

void jshUtilTimerStart(JsSysTime period) {
  UNUSED(period);
  ESP_LOGD(tag,">> jshUtilTimerStart");
  ESP_LOGD(tag,"<< jshUtilTimerStart");
}

void jshUtilTimerReschedule(JsSysTime period) {
  ESP_LOGD(tag,">> jshUtilTimerReschedule");
  jshUtilTimerDisable();
  jshUtilTimerStart(period);
  ESP_LOGD(tag,"<< jshUtilTimerReschedule");
}

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
  UNUSED(device);
  ESP_LOGD(tag,">> jshIsDeviceInitialised");
  ESP_LOGD(tag,"<< jshIsDeviceInitialised");
 return 0;
} // End of jshIsDeviceInitialised

// the esp8266 doesn't have any temperature sensor
JsVarFloat jshReadTemperature() {
  ESP_LOGD(tag,">> jshReadTemperature");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshReadTemperature");
  return NAN;
}

// the esp8266 can read the VRef but then there's no analog input, so we don't support this
JsVarFloat jshReadVRef() {
  ESP_LOGD(tag,">> jshReadVRef");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshReadVRef");
  return NAN;
}

unsigned int jshGetRandomNumber() {
  ESP_LOGD(tag,">> jshGetRandomNumber");
  ESP_LOGD(tag,"<< jshGetRandomNumber");
  return (unsigned int)rand();
}

//===== Read-write flash =====

/**
 * Determine available flash depending on EEprom size
 *
 */
uint32_t jshFlashMax() {
  ESP_LOGD(tag,">> jshFlashMax");
  return (FLASH_MAX-1);
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

  if(len == 1){ // Can't read a single byte using the API, so read 4 and select the byte requested
    uint word;
    spi_flash_read(addr & 0xfffffffc,&word,4);
	*(uint8_t *)buf = (word >> ((addr & 3) << 3 )) & 255;
  }
  else spi_flash_read(addr, buf, len);
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
  // This function is called too often during save() and load() processing to be
  // useful for logging the entry/exit.
  //ESP_LOGD(tag,">> jshFlashWrite");
  spi_flash_write(addr, buf, len);
  //ESP_LOGD(tag,"<< jshFlashWrite");
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
  ESP_LOGD(tag,">> jshFlashGetPage: addr=0x%x", addr);
  if (addr >= FLASH_MAX) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true; 
}



JsVar *jshFlashGetFree() {
  ESP_LOGD(tag,">> jshFlashGetFree");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshFlashGetFree");
  return 0;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
  ESP_LOGD(tag,">> jshFlashErasePage: addr=0x%x", addr);
  spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
  ESP_LOGD(tag,"<< jshFlashErasePage");
}

unsigned int jshSetSystemClock(JsVar *options) {
  UNUSED(options);
  ESP_LOGD(tag,">> jshSetSystemClock");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag,"<< jshSetSystemClock");
  return 0;
}

/**
 * Convert an Espruino pin id to a native ESP32 pin id.
 */
static gpio_num_t pinToESP32Pin(Pin pin) {
  if ( pin < 40 ) 
	return pin + GPIO_NUM_0;
  ESP_LOGE(tag, "pinToESP32Pin: Unknown pin: %d", pin);
  return -1;
}
