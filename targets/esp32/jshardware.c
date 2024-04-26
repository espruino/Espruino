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
#include <sys/time.h>

#include "jshardware.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwareTimer.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"

#ifdef BLUETOOTH
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_gatts_func.h"
#endif
#include "jshardwareESP32.h"

#include "jsutils.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jspininfo.h"

#include "jswrap_esp32_network.h"

#if ESP_IDF_VERSION_5
#include "soc/uart_reg.h"
#include "esp_mac.h"
#endif 
#include "esp_attr.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "rom/ets_sys.h"
#include "rom/uart.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"

#if ESP_IDF_VERSION_5
#include "esp_flash.h"
#include "soc/gpio_reg.h"
#else
#include "esp_spi_flash.h"
#endif

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

/// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

/// Has the watchdog been enabled?
bool wdt_enabled = false;

static uint64_t DEVICE_INITIALISED_FLAGS = 0L;

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  uint64_t mask = 1ULL << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

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
  // 16-17 are used for PSRAM (future use)
  jshPinSetStateRange(0,0,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(12,15,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(18,19,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(21,22,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(25,27,JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetStateRange(34,39,JSHPINSTATE_GPIO_IN_PULLUP);

}

/**
 * Initialize the JavaScript hardware interface.
 */
void jshInit() {
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) esp32_wifi_init();
#ifdef BLUETOOTH
  if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) gattc_init();
#endif
  jshInitDevices();
  BITFIELD_CLEAR(jshPinSoftPWM);
  if (JSHPINSTATE_I2C != 13 || JSHPINSTATE_GPIO_IN_PULLDOWN != 6 || JSHPINSTATE_MASK != 15) {
    jsError("JshPinState #defines have changed, please update pinStateToString()");
  }
  gpio_isr_register(gpio_intr_handler,NULL,0,NULL);  //changed to automatic assign of interrupt
  // Initialize something for each of the possible pins.
  jshPinDefaultPullup();
} // End of jshInit

void jshKill() {
}

/**
 * Reset the Espruino environment.
 */
void jshReset() {
  jshResetDevices();
  jshPinDefaultPullup() ;
//  UartReset();
  RMTReset();
  ADCReset();
  SPIReset();
  I2CReset();
#ifdef BLUETOOTH
  if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) gatts_reset(false);
#endif
}

/**
 * Re-init the ESP32 after a soft-reset
 */
void jshSoftInit() {
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) jswrap_esp32_wifi_soft_init();
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
  esp_efuse_mac_get_default(data);
  return 6;
}

void jshInterruptOff() {
  taskDISABLE_INTERRUPTS();
}

void jshInterruptOn()  {
  taskENABLE_INTERRUPTS();
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return false; // FIXME!
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
  ets_delay_us((uint32_t)microsec);
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
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }
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
#if ESP_IDF_VERSION_5
  esp_rom_gpio_pad_select_gpio(gpioNum);
#else
  gpio_pad_select_gpio(gpioNum);
#endif  
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

/**
 * Check if state is default - return true if default
*/
bool jshIsPinStateDefault(Pin pin, JshPinState state) {
  return state == JSHPINSTATE_GPIO_IN_PULLUP || state == JSHPINSTATE_ADC_IN;
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
#if ESP_IDF_VERSION_5
  gpio_iomux_out(gpioNum,SIG_GPIO_OUT_IDX,0);  // reset pin to be GPIO in case it was used as rmt or something else
#else  
  gpio_matrix_out(gpioNum,SIG_GPIO_OUT_IDX,0,0);  // reset pin to be GPIO in case it was used as rmt or something else
#endif  
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
  if (value<0) value=0;
  if (value>1) value=1;
  if (!isfinite(freq)) freq=0;
  if(pin == 25 || pin == 26){
  if(flags & (JSAOF_ALLOW_SOFTWARE | JSAOF_FORCE_SOFTWARE)) jsError("pin does not support software PWM");
    writeDAC(pin,(uint8_t)(value * 256));
  }
  else{
  if(flags & JSAOF_ALLOW_SOFTWARE){
    if (!jshGetPinStateIsManual(pin)){
        BITFIELD_SET(jshPinSoftPWM, pin, 0);
        jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
      }
      BITFIELD_SET(jshPinSoftPWM, pin, 1);
      if ((freq<=0)) freq=50;
      jstPinPWM(freq, value, pin);
      return 0;
    }
    else writePWM(pin,( uint16_t)(value * PWMTimerRange),(int) freq);
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
    // convert the 16 bit value to a 10 bit value.
    value=value >> (16 - PWMTimerBit);
    setPWM( (Pin)pin, (uint16_t)value);
  }
}

void jshEnableWatchDog(JsVarFloat timeout) {
  wdt_enabled = true;
  esp_task_wdt_init((int)(timeout+0.5)
#if !ESP_IDF_VERSION_5  
   , true
#endif   
  ); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

// Kick the watchdog
void jshKickWatchDog() {
  if (wdt_enabled)
    esp_task_wdt_reset();
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
 * Determine whether the pin can be watchable.
 * \return Returns true if the pin is watchable.
 */
bool jshCanWatch(
    Pin pin //!< The pin that we are asking whether or not we can watch it.
  ) {
  return pin == 0 || ( pin >= 12 && pin <= 19 ) || pin == 21 ||  pin == 22 || ( pin >= 25 && pin <= 27 ) || ( pin >= 34 && pin <= 39 );
}


/**
 * Do what ever is necessary to watch a pin.
 * \return The event flag for this pin.
 */
IOEventFlags jshPinWatch(
    Pin pin,          //!< The pin to be watched.
    bool shouldWatch, //!< True for watching and false for unwatching.
    JshPinWatchFlags flags
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

  if (inf->errorHandling) {
    jsExceptionHere(JSET_ERROR, "ESP32 Espruino builds can't handle framing/parity errors (errors:true)");
    return;
  }

  initSerial(device,inf);
}

bool jshIsUSBSERIALConnected() {
  return false; // "On non-USB boards this just returns false"
}

/**
 * Kick a device into action (if required).
 *
 */
void jshUSARTKick(IOEventFlags device) {
  int c = jshGetCharToTransmit(device);
  while(c >= 0) {
  switch(device){
#ifdef BLUETOOTH
    case EV_BLUETOOTH:
      gatts_sendNUSNotification(c);
      break;
#endif
    case EV_SERIAL1:
      uart_tx_one_char((uint8_t)c);
      break;
    default:
      writeSerial(device,(uint8_t)c);
      break;
    //if(device == EV_SERIAL1) uart_tx_one_char((uint8_t)c);
    //else writeSerial(device,(uint8_t)c);
  }
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
static portMUX_TYPE JSmicrosMux = portMUX_INITIALIZER_UNLOCKED;
JsSysTime CALLED_FROM_INTERRUPT jshGetSystemTime() { // in us -- can be called at interrupt time
  struct timeval tm;
  portENTER_CRITICAL_ISR(&JSmicrosMux);
  gettimeofday(&tm, 0);
  portEXIT_CRITICAL_ISR(&JSmicrosMux);
  return (JsSysTime)(tm.tv_sec)*1000000L + tm.tv_usec;
}

/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime newTime) {
  struct timeval tm;
  struct timezone tz;

  tm.tv_sec=(time_t)(newTime/1000000L);
  tm.tv_usec=(suseconds_t) (newTime - tm.tv_sec * 1000000L);
  tz.tz_minuteswest=0;
  tz.tz_dsttime=0;
  settimeofday(&tm, &tz); 
}

void jshUtilTimerDisable() {
  disableTimer(0);
}

void jshUtilTimerStart(JsSysTime period) {
  if(period <= 30){period = 30;}
  startTimer(0,(uint64_t) period);
}

void jshUtilTimerReschedule(JsSysTime period) {
  if(period <= 30){period = 30;}
  rescheduleTimer(0,(uint64_t) period);
}

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
  uint64_t mask = 1ULL << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;

//  UNUSED(device);
//  jsError(">> jshIsDeviceInitialised not implemented");
// return 0;
} // End of jshIsDeviceInitialised


// the esp32 temperature sensor - undocumented library function call. Unsure of values returned.
JsVarFloat jshReadTemperature() {
  jsError(">> jshReadTemperature Not implemented");
  return NAN;
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
#if ESP_IDF_VERSION_5    
    esp_flash_read(NULL, addr & 0xfffffffc,&word,4);
#else
    spi_flash_read(addr & 0xfffffffc,&word,4);
#endif    
    *(uint8_t *)buf = (word >> ((addr & 3) << 3 )) & 255;
  } else {
#if ESP_IDF_VERSION_5
    esp_flash_read(NULL, addr, buf, len);
#else
    spi_flash_read(addr, buf, len);
#endif
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
#if ESP_IDF_VERSION_5    
  esp_flash_write(NULL, addr, buf, len);
#else
  spi_flash_write(addr, buf, len);
#endif  
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
  // Space reserved here in the parition table -  using sub type 0x40
  // This should be read from the partition table
  addFlashArea(jsFreeFlash, 0xE000, 0x2000);
  addFlashArea(jsFreeFlash, 0x310000, 0x10000);
  addFlashArea(jsFreeFlash, 0x360000, 0xA0000);
  return jsFreeFlash;
}


/**
 * Erase the flash page containing the address.
 */
void jshFlashErasePage(
    uint32_t addr //!<
  ) {
#if ESP_IDF_VERSION_5      
  esp_flash_erase_region(NULL, addr >> FLASH_PAGE_SHIFT, FLASH_PAGE);
#else  
  spi_flash_erase_sector(addr >> FLASH_PAGE_SHIFT);
#endif  
}

size_t jshFlashGetMemMapAddress(size_t ptr) {
   // if ptr is high already, assume we know what we're accessing
  if (ptr > 0x10000000) return ptr;
  // romdata_jscode is memory mapped address from the js_code partition in rom - targets/esp32/main.c
  extern char* romdata_jscode;
  if (romdata_jscode==0) {
    jsError("Couldn't find js_code partition - update with partition_espruino.bin\n");
    return 0;
  }
  // Flash memory access is offset to 0, so remove starting location as already accounted for
  return (size_t)&romdata_jscode[ptr - FLASH_SAVED_CODE_START ];
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

/// Perform a proper hard-reboot of the device
void jshReboot() {
  esp_restart(); // Call the ESP-IDF to restart the ESP32.
}

/* Adds the estimated power usage of the microcontroller in uA to the 'devices' object. The CPU should be called 'CPU' */
void jsvGetProcessorPowerUsage(JsVar *devices) {
  jsvObjectSetChildAndUnLock(devices, "CPU", jsvNewFromInteger(20000));
  // standard power usage of ESP32S3 without Wifi
}
