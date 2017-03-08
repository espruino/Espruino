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
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwareTimer.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"

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
#include "rom/ets_sys.h"
#include "rom/uart.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"

#include "jshardwareI2c.h"
#include "jshardwareSpi.h"

#define FLASH_MAX (4*1024*1024) //4MB
#define FLASH_PAGE_SHIFT 12 // Shift is much faster than division by 4096 (size of page)
#define FLASH_PAGE ((uint32_t)1<<FLASH_PAGE_SHIFT)  //4KB

#define UNUSED(x) (void)(x)

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
void IRAM_ATTR gpio_intr_handler(void* arg){
  //GPIO intr process. Mainly copied from esp-idf
  UNUSED(arg);
  IOEventFlags exti;
  Pin gpio_num = 0;
  uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31
  uint32_t gpio_intr_status_h = READ_PERI_REG(GPIO_STATUS1_REG);//read status1 to get interrupt status for GPIO32-39
  SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);    //Clear intr for gpio0-gpio31
  SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status_h); //Clear intr for gpio32-39
  do {
    g_pinState[gpio_num] = 0;
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

void jshPinSetStateRange( Pin start, Pin end, JshPinState state ) {
    for ( Pin p=start; p<=end; p++ ) {
        jshPinSetState(p, state);
    }
}

void jshPinDefaultPullup() {
  // 6-11 are used by Flash chip
  // 32-33 are routed to rtc for xtal
  jshPinSetStateRange(0,0,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(12,19,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(21,22,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(25,27,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(34,39,JSHPINSTATE_GPIO_IN_PULLUP);
  
}

/**
 * Initialize the JavaScript hardware interface.
 */
void jshInit() {
  uint32_t freeHeapSize = esp_get_free_heap_size();
  jsWarn( "Free heap size: %d", freeHeapSize);
  esp32_wifi_init();
  //jswrap_ESP32_wifi_soft_init();
  jshInitDevices();
  if (JSHPINSTATE_I2C != 13 || JSHPINSTATE_GPIO_IN_PULLDOWN != 6 || JSHPINSTATE_MASK != 15) {
    jsError("JshPinState #defines have changed, please update pinStateToString()");
  }
  /*
  jsWarn( "JSHPINSTATE_I2C %d\n",JSHPINSTATE_I2C );
  jsWarn( "JSHPINSTATE_GPIO_IN_PULLDOWN %d\n",JSHPINSTATE_GPIO_IN_PULLDOWN );
  jsWarn( "JSHPINSTATE_MASK %d\n",JSHPINSTATE_MASK );
  */
  gpio_isr_register(gpio_intr_handler,NULL,0,NULL);  //changed to automatic assign of interrupt
  // Initialize something for each of the possible pins.
  jshPinDefaultPullup();
} // End of jshInit

/**
 * Reset the Espruino environment.
 */
void jshReset() {
    jshResetDevices();
    jshPinDefaultPullup() ;
    UartReset();
	RMTReset();
	ADCReset();
	SPIReset();
	I2CReset();
}

/**
 * Re-init the ESP32 after a soft-reset
 */
void jshSoftInit() {
  //jsWarn(">> jshSoftInit()\n");
  jswrap_ESP32_wifi_soft_init();
}

/**
 * Handle whatever needs to be done in the idle loop when there's nothing to do.
 *
 * Nothing is needed on the ESP32.
 */
void jshIdle() {

}

// ESP32 chips don't have a serial number but they do have a MAC address
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  assert(maxChars >= 6); // it's 32
  esp_wifi_get_mac(WIFI_IF_STA, data);
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
   return true;
} // End of jshSleep


/**
 * Delay (blocking) for the supplied number of microseconds.
 */
void jshDelayMicroseconds(int microsec) {
  ets_delay_us(microsec);
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
    mode = GPIO_MODE_INPUT_OUTPUT;
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
    mode = GPIO_MODE_INPUT_OUTPUT_OD;
    break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
    mode = GPIO_MODE_INPUT_OUTPUT_OD;
    pull_mode=GPIO_PULLUP_ONLY;
    break;
  default:
    jsError( "jshPinSetState: Unexpected state: %d", state);
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
  if ( jshPinGetValue(pin) & 1 ) 
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
  gpio_num_t gpioNum = pinToESP32Pin(pin);
  gpio_matrix_out(gpioNum,SIG_GPIO_OUT_IDX,0,0);  // reset pin to be GPIO in case it was used as rmt or something else
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
  return (JsVarFloat) readADC(pin) / 4096;
}


int jshPinAnalogFast(Pin pin) {
  return readADC(pin) << 4;
}


/**
 * Set the output PWM value.
 */
JshPinFunction jshPinAnalogOutput(Pin pin,
    JsVarFloat value,
    JsVarFloat freq,
    JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  UNUSED(flags);
  if(pin == 25 || pin == 26){
    value = (value * 256);
    uint8_t val8 = value;
    writeDAC(pin,val8);
  }
  else{
    value = (value * PWMTimerRange);
    uint16_t val16 = value;
    writePWM(pin,val16,(int) freq);
  }
  return 0;
}


/**
 *
 */
void jshSetOutputValue(JshPinFunction func, int value) {
  int pin;
  if (JSH_PINFUNCTION_IS_DAC(func)) {
    uint8_t val = (uint8_t)(value >> 8);
    switch (func & JSH_MASK_INFO) {
      case JSH_DAC_CH1:  writeDAC(25,val); break;
      case JSH_DAC_CH2:  writeDAC(26,val); break;
    }
  }
  else{
    pin = ((func >> JSH_SHIFT_INFO) << 4) + ((func >> JSH_SHIFT_TYPE) & 15);
    value >> (16 - PWMTimerBit);
    setPWM(pin,value);
  }
}


/**
 *
 */
void jshEnableWatchDog(JsVarFloat timeout) {
  UNUSED(timeout);
  jsError(">> jshEnableWatchDog Not implemented,using taskwatchdog from RTOS");
}


// Kick the watchdog
void jshKickWatchDog() {
  jsError(">> jshKickWatchDog Not implemented,using taskwatchdog from RTOS");
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
  int duration = (int)pulseTime * 1000; //from millisecs to microsecs
  sendPulse(pin, pulsePolarity, duration);
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
            jsError("*** jshPinWatch error");
        }
      }
      return pin;
}


/**
 *
 */
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  if (jshIsPinValid(pin)) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      JshPinFunction func = pinInfo[pin].functions[i];
      if (JSH_PINFUNCTION_IS_TIMER(func) ||
          JSH_PINFUNCTION_IS_DAC(func))
        return func;
    }
  }
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
  initSerial(device,inf);
}

bool jshIsUSBSERIALConnected() {
  return false; // "On non-USB boards this just returns false"
}

/**
 * Kick a device into action (if required).
 *
 */
void jshUSARTKick(
    IOEventFlags device //!< The device to be kicked.
) {
  int c = jshGetCharToTransmit(device);
  while(c >= 0) {
    if(device == EV_SERIAL1) uart_tx_one_char((uint8_t)c); 
    else writeSerial(device,(uint8_t)c);
    c = jshGetCharToTransmit(device);
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
}

void jshUtilTimerDisable() {
  disableTimer(0);
}

void jshUtilTimerStart(JsSysTime period) {
  startTimer(0,(uint64_t) period);
}

void jshUtilTimerReschedule(JsSysTime period) {
  rescheduleTimer(0,(uint64_t) period);
}

//===== Miscellaneous =====

static uint64_t DEVICE_INITIALISED_FLAGS = 0L;
bool jshIsDeviceInitialised(IOEventFlags device) {
  uint64_t mask = 1ULL << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;

//  UNUSED(device);
//  jsError(">> jshIsDeviceInitialised not implemented");
// return 0;
} // End of jshIsDeviceInitialised

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  uint64_t mask = 1ULL << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

// the esp32 temperature sensor - undocumented library function call. Unsure of values returned.
JsVarFloat jshReadTemperature() {
  extern uint8_t temprature_sens_read();
  return temprature_sens_read();
}

// the esp8266 can read the VRef but then there's no analog input, so we don't support this
JsVarFloat jshReadVRef() {
  jsError(">> jshReadVRef Not implemented");
  return NAN;
}

unsigned int jshGetRandomNumber() {
  return (unsigned int)rand();
}

//===== Read-write flash =====

/**
 * Determine available flash depending on EEprom size
 *
 */
uint32_t jshFlashMax() {
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
  spi_flash_write(addr, buf, len);
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
  if (addr >= FLASH_MAX) return false;
  *startAddr = addr & ~(FLASH_PAGE-1);
  *pageSize = FLASH_PAGE;
  return true; 
}

void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  // Space should be reserved here in the parition table - assume 4Mb EEPROM
  // Set just after programme save area 
  addFlashArea(jsFreeFlash, 0x100000 + FLASH_PAGE * 16, 0x300000-FLASH_PAGE * 16-1);
  
  return jsFreeFlash;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
  spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
}

unsigned int jshSetSystemClock(JsVar *options) {
  UNUSED(options);
  jsError(">> jshSetSystemClock Not implemented");
  return 0;
}

/**
 * Convert an Espruino pin id to a native ESP32 pin id.
 */
gpio_num_t pinToESP32Pin(Pin pin) {
  if ( pin < 40 ) 
    return pin + GPIO_NUM_0;
  jsError( "pinToESP32Pin: Unknown pin: %d", pin);
  return -1;
}
