#include "wm_osal.h"
#include "wm_efuse.h"
#include "wm_gpio.h"
#include "wm_gpio_afsel.h"
#include "wm_uart.h"
#include "wm_internal_flash.h"
#include "wm_timer.h"

#include "jshardware.h"
#include "jsinteractive.h"
#include "jstimer.h"

#define JSH_MAX_SLEEP_MS    100
#define JSH_CLOCKS_PER_MSEC 0.5 //(500/1000)

BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);
static uint8_t  g_pinState[JSH_PIN_COUNT];
static uint64_t DEVICE_INITIALISED_FLAGS = 0L;

volatile JsSysTime  offsetSystemTimeMs=0;
volatile uint32_t   sys_section_status;
volatile uint8_t    timer_id=WM_TIMER_ID_INVALID;

void gpio_iqr_callback(void *arg){
  Pin pin=(Pin)arg;
  uint16_t ret= tls_get_gpio_irq_status(pin);
  if(ret){
    tls_clr_gpio_irq_status(pin);
    jshPushIOWatchEvent((IOEventFlags)(EV_EXTI0 + pin));
  }
}

void uart_rx(uint16_t uart_no,uint16_t len){
  IOEventFlags device=(uart_no==TLS_UART_0)?EV_SERIAL1:EV_SERIAL2;
  uint8_t data;
  jshInterruptOff();
  if(jshIsDeviceInitialised(device)){
    while(len>0){
      tls_uart_read(uart_no,&data,1);
      jshPushIOCharEvent(device,(char)data);
      len--;
    }
  }
  jshInterruptOn();
}

int16_t uart0_rx(uint16_t len){
  uart_rx(TLS_UART_0,len);
  return WM_SUCCESS;
}

int16_t uart1_rx(uint16_t len){
  uart_rx(TLS_UART_1,len);
  return WM_SUCCESS;
}

void util_timer_irq(uint8_t *arg){
  jstUtilTimerInterruptHandler();
}

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  uint64_t mask = 1ULL << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

/// jshInit is called at start-up, put hardware dependent init stuff in this function
void jshInit(){
  jshInitDevices();

  BITFIELD_CLEAR(jshPinSoftPWM);
  for (int i=0; i<JSH_PIN_COUNT; i++) {
    g_pinState[i] = 0;
  }

}

/// jshReset is called from JS 'reset()' - try to put peripherals back to their power-on state
void jshReset(){
  jshResetDevices();
}

/** Code that is executed each time around the idle loop. Prod watchdog timers here,
 * and on platforms without GPIO interrupts you can check watched Pins for changes. */
void jshIdle(){
  return;
}
/** Enter sleep mode for the given period of time. Can be woken up by interrupts.
 * If time is 0xFFFFFFFFFFFFFFFF then go to sleep without setting a timer to wake
 * up.
 *
 * This function can also check `jsiStatus & JSIS_ALLOW_DEEP_SLEEP`, and if there
 * is no pending serial data and nothing working on Timers, it will put the device
 * into deep sleep mode where the high speed oscillator turns off.
 *
 * Returns true on success
 */
bool jshSleep(JsSysTime timeUntilWake){
  JsVarFloat sleep_ms=0;
  JsVarFloat time_wake=jshGetMillisecondsFromTime(timeUntilWake);

  if(time_wake>JSH_MAX_SLEEP_MS||timeUntilWake<1){
    sleep_ms=JSH_MAX_SLEEP_MS;
  }else{
    sleep_ms=time_wake;
  }

  jsiSetSleep(JSI_SLEEP_ASLEEP);
  tls_os_time_delay((uint32_t)(sleep_ms*JSH_CLOCKS_PER_MSEC));
  jsiSetSleep(JSI_SLEEP_AWAKE);
  
  return true;
}

/** Clean up ready to stop Espruino. Unused on embedded targets, but used on Linux,
 * where GPIO that have been exported may need unexporting, and so on. */
void jshKill(){
  return;
}

/** Get this IC's serial number. Passed max # of chars and a pointer to write to.
 * Returns # of chars of non-null-terminated string.
 *
 * This is reported back to `process.env` and is sometimes used in USB enumeration.
 * It doesn't have to be unique, but some users do use this in their code to distinguish
 * between boards.
 */
int jshGetSerialNumber(unsigned char *data, int maxChars){
  if(maxChars<16){
    return 0;
  }else{
    return tls_get_chipid(data);
  }
}

/** Is the USB port connected such that we could move the console over to it
 * (and that we should store characters sent to USB). On non-USB boards this just returns false. */
bool jshIsUSBSERIALConnected(){
  return false;
}

/** The system time is used all over Espruino - for:
 *  * setInterval/setTimeout
 *  * new Date()
 *  * getTime
 *  * scheduling the utility timer (digitalPulse/Waveform/etc)
 *  * timestamping watches (so measuring pulse widths)
 *
 * It is time since 1970 - in whatever units make sense for the platform. For real-time
 * platforms units should really be at the uS level. Often this timer directly counts
 * clock cycles with the SysTick timer.
 */

/// Get the system time (in ticks since the epoch)
JsSysTime jshGetSystemTime(){
  long sys_ticks=tls_os_get_time();
  long sys_ms=(long)(sys_ticks/JSH_CLOCKS_PER_MSEC);
  return (JsSysTime)(offsetSystemTimeMs+sys_ms);
}

/** Set the system time (in ticks since the epoch) - this should only be called rarely as it
could mess up things like jsinteractive's timers! */
void jshSetSystemTime(JsSysTime time_ms){
  time_ms=time_ms/1000L;
  jsiLastIdleTime = time_ms;
  offsetSystemTimeMs=0;
  offsetSystemTimeMs=time_ms-jshGetSystemTime();
}

/// Convert a time in Milliseconds since the epoch to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat time_ms){
  return (JsSysTime)(time_ms);
}
/// Convert ticks to a time in Milliseconds since the epoch
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time_ticks){
  return (JsVarFloat)(time_ticks);
}

// software IO functions...
void jshInterruptOff(){ ///< disable interrupts to allow short delays to be accurate
  sys_section_status=tls_os_set_critical();
}

void jshInterruptOn(){  ///< re-enable interrupts
  tls_os_release_critical(sys_section_status);
}

bool jshIsInInterrupt(){ ///< Are we currently in an interrupt?
  return false;
}

void jshDelayMicroseconds(int microsec){  ///< delay a few microseconds. Should use used sparingly and for very short periods - max 1ms
  if(microsec>0){
    tls_usleep((unsigned int)microsec);
  }
}



void jshPinSetValue(Pin pin, bool value){ ///< Set a digital output to 1 or 0. DOES NOT change pin state OR CHECK PIN VALIDITY
  tls_gpio_write(pin,value);
}

bool jshPinGetValue(Pin pin){ ///< Get the value of a digital input. DOES NOT change pin state OR CHECK PIN VALIDITY
  return tls_gpio_read(pin);
}

/// Set the pin state (Output, Input, etc)
void jshPinSetState(Pin pin, JshPinState state){
  switch(state){
    case JSHPINSTATE_GPIO_OUT:{
      tls_gpio_cfg(pin,WM_GPIO_DIR_OUTPUT,WM_GPIO_ATTR_FLOATING);
      break;
    }
    case JSHPINSTATE_GPIO_IN:{
      tls_gpio_cfg(pin,WM_GPIO_DIR_INPUT,WM_GPIO_ATTR_FLOATING);
      break;
    }
    case JSHPINSTATE_GPIO_IN_PULLUP:{
      tls_gpio_cfg(pin,WM_GPIO_DIR_INPUT,WM_GPIO_ATTR_PULLHIGH);
      break;
    }
    case JSHPINSTATE_GPIO_IN_PULLDOWN:{
      tls_gpio_cfg(pin,WM_GPIO_DIR_INPUT,WM_GPIO_ATTR_PULLLOW);
      break;
    }
    default:{
      jsError("Pin state not supported");
      return;
    }
  }

  if(BITFIELD_GET(jshPinSoftPWM,pin)){
    BITFIELD_SET(jshPinSoftPWM,pin,0);
    jstPinPWM(0,0,pin);
  }

  g_pinState[pin]=(uint8_t)state;

}

/** Get the pin state (only accurate for simple IO - won't return
 * JSHPINSTATE_USART_OUT for instance). Note that you should use
 * JSHPINSTATE_MASK as other flags may have been added
 * (like JSHPINSTATE_PIN_IS_ON if pin was set to output) */
JshPinState jshPinGetState(Pin pin){
  uint8_t rc=g_pinState[pin];
  
  if(tls_gpio_read(pin)){
    rc|=JSHPINSTATE_PIN_IS_ON;
  }

  return rc;
}

/** Returns an analog value between 0 and 1. 0 is expected to be 0v, and
 * 1 means jshReadVRef() volts. On most devices jshReadVRef() would return
 * around 3.3, so a reading of 1 represents 3.3v. */
JsVarFloat jshPinAnalog(Pin pin){
  return NAN;
}

/** Returns a quickly-read analog value in the range 0-65535.
 * This is basically `jshPinAnalog()*65535`
 * For use from an IRQ where high speed is needed */
int jshPinAnalogFast(Pin pin){
  return 0;
}

/// Output an analog value on a pin - either via DAC, hardware PWM, or software PWM
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags){ // if freq<=0, the default is used
  if (value<0) value=0;
  if (value>1) value=1;
  if (!isfinite(freq)) freq=0;
  
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
    return JSH_NOTHING;
  }

  return JSH_NOTHING;
}

/// Pulse a pin for a certain time, but via IRQs, not JS: `digitalWrite(pin,value);setTimeout("digitalWrite(pin,!value)", time*1000);`
void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime){
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
}

/// Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin){
  return true;
}

/// start watching pin - return the EXTI (IRQ number flag) associated with it
IOEventFlags jshPinWatch(Pin pin, bool shouldWatch){
  if(shouldWatch){
    tls_gpio_isr_register(pin,gpio_iqr_callback,(void *)pin);
    tls_gpio_irq_enable(pin,WM_GPIO_IRQ_TRIG_DOUBLE_EDGE);
  }else{
    tls_gpio_irq_disable(pin);
  }
  return (IOEventFlags)(EV_EXTI0 + pin);
}

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin){
  return JSH_NOTHING;
}

/** Given a pin function, set that pin to the 16 bit value
 * (used mainly for fast DAC and PWM handling from Utility Timer) */
void jshSetOutputValue(JshPinFunction func, int value){
  return;
}

/// Enable watchdog with a timeout in seconds, it'll reset the chip if jshKickWatchDog isn't called within the timeout
void jshEnableWatchDog(JsVarFloat timeout){
  return ;
}

// Kick the watchdog
void jshKickWatchDog(){
  return;
}

/// Check the pin associated with this EXTI - return true if the pin's input is a logic 1
bool jshGetWatchedPinState(IOEventFlags device){
  return jshPinGetValue((Pin)(device-EV_EXTI0));
}

/// Given an event, check the EXTI flags and see if it was for the given pin
bool jshIsEventForPin(IOEvent *event, Pin pin){
  return IOEVENTFLAGS_GETTYPE(event->flags) == (IOEventFlags)(EV_EXTI0+pin);
}

/** Is the given device initialised?
 * eg. has jshUSARTSetup/jshI2CSetup/jshSPISetup been called previously? */
bool jshIsDeviceInitialised(IOEventFlags device){
  uint64_t mask = 1ULL << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf){
  if(DEVICE_IS_USART(device)){
    jshSetDeviceInitialised(device,true);

    tls_uart_options_t opt;
    
    switch(inf->bytesize){
      case 5:{
        opt.charlength=TLS_UART_CHSIZE_5BIT;
        break;
      }
      case 6:{
        opt.charlength=TLS_UART_CHSIZE_6BIT;
        break;
      }
      case 7:{
        opt.charlength=TLS_UART_CHSIZE_7BIT;
        break;
      }
      default:{
        opt.charlength=TLS_UART_CHSIZE_8BIT;
        break;
      }
    }

    switch(inf->stopbits){
      case 2:{
        opt.stopbits=TLS_UART_TWO_STOPBITS;
        break;
      }
      default:{
        opt.stopbits=TLS_UART_ONE_STOPBITS;
        break;
      }
    }
    
    opt.paritytype=(TLS_UART_PMODE_T)inf->parity;
    opt.baudrate=inf->baudRate;
    opt.flow_ctrl=TLS_UART_FLOW_CTRL_NONE;

    if(device==EV_SERIAL1){
      wm_uart0_rx_config(inf->pinRX);
      wm_uart0_tx_config(inf->pinTX);
      tls_uart_port_init(TLS_UART_0,&opt,0);
      tls_uart_rx_callback_register(TLS_UART_0,uart0_rx);
    }else if(device==EV_SERIAL2){
      wm_uart1_rx_config(inf->pinRX);
      wm_uart1_tx_config(inf->pinTX);
      tls_uart_port_init(TLS_UART_1,&opt,0);
      tls_uart_rx_callback_register(TLS_UART_1,uart1_rx);
    }

  }
}

/** Kick a device into action (if required). For instance we may have data ready
 * to sent to a USART, but we need to enable the IRQ such that it can automatically
 * fetch the characters to send.
 *
 * Later down the line this could potentially even set up something like DMA.*/
void jshUSARTKick(IOEventFlags device){
  int c=jshGetCharToTransmit(device);
  char data;
  while(c>=0){
    data=(char)c;
    tls_uart_write(device==EV_SERIAL1?TLS_UART_0:TLS_UART_1,&data,1);
    c=jshGetCharToTransmit(device);
  }
}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf){
  jsWarn("jshSPISetup not impl\n");
}
/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data){
  jsWarn("jshSPISend not impl\n");
  return 0;
}
/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data){
  jsWarn("jshSPISend16 not impl\n");
}
/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16){
  jsWarn("jshSPISet16 not impl\n");
}
/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive){
  jsWarn("jshSPISetReceive not impl\n");
}
/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device){
  jsWarn("jshSPIWait not impl\n");
}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf){
  jsWarn("jshI2CSetup not impl\n");
}

/** Write a number of btes to the I2C device. Addresses are 7 bit - that is, between 0 and 0x7F.
 *  sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop){
  jsWarn("jshI2CWrite not impl\n");
}
/** Read a number of bytes from the I2C device. */
void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop){
  jsWarn("jshI2CRead not impl\n");
}

/** Return start address and size of the flash page the given address resides in. Returns false if
  * the page is outside of the flash address range */
bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize){
  if(addr<FLASH_SAVED_CODE_START||addr>FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH){
    return false;
  }

  *startAddr=addr&~(INSIDE_FLS_SECTOR_SIZE-1);
  *pageSize=INSIDE_FLS_SECTOR_SIZE;
  return true;
}
/** Return a JsVar array containing objects of the form `{addr, length}` for each contiguous block of free
 * memory available. These should be one complete pages, so that erasing the page containing any address in
 * this block won't erase anything useful! */
JsVar *jshFlashGetFree(){
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return jsFreeFlash;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger(FLASH_SAVED_CODE_START));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger(FLASH_SAVED_CODE_LENGTH));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
  return jsFreeFlash;
}
/// Erase the flash page containing the address
void jshFlashErasePage(uint32_t addr){
  tls_fls_erase(addr/INSIDE_FLS_SECTOR_SIZE);
}
/** Read data from flash memory into the buffer, the flash address has no alignment restrictions
  * and the len may be (and often is) 1 byte */
void jshFlashRead(void *buf, uint32_t addr, uint32_t len){
  tls_fls_read(addr-INSIDE_FLS_BASE_ADDR,(uint8_t *)buf,len);
}
/** Write data to flash memory from the buffer, the buffer address and flash address are
  * guaranteed to be 4-byte aligned, and length is a multiple of 4.  */
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len){
  tls_fls_write(addr-INSIDE_FLS_BASE_ADDR,(uint8_t *)buf,len);
}

/** On most platforms, the address of something really is that address.
 * In ESP32/ESP8266 the flash memory is mapped up at a much higher address,
 * so we need to tweak any pointers that we use.
 * */
size_t jshFlashGetMemMapAddress(size_t ptr){
  return ptr;
}


/** Utility timer handling functions
 *  ------------------------------------------
 * The utility timer is intended to generate an interrupt and then call jstUtilTimerInterruptHandler
 * as interrupt handler so Espruino can process tasks that are queued up on the timer. Typical
 * functions used in the interrupt handler include reading/write GPIO pins, reading analog and
 * writing analog. See jstimer.c for the implementation.
 *
 * These are exposed through functions like `jsDigitalPulse`, `analogWrite(..., {soft:true})`
 * and the `Waveform` class.
 */

/// Start the timer and get it to interrupt once after 'period' (i.e. it should not auto-reload)
void jshUtilTimerStart(JsSysTime period){
  if(timer_id!=WM_TIMER_ID_INVALID){
    tls_timer_stop(timer_id);
    timer_id=WM_TIMER_ID_INVALID;
  }

  struct tls_timer_cfg timer_cfg;
  timer_cfg.unit = TLS_TIMER_UNIT_MS;
  timer_cfg.timeout = jshGetMillisecondsFromTime(period);
  timer_cfg.is_repeat = 0;
  timer_cfg.callback = (tls_timer_irq_callback)util_timer_irq;
  timer_cfg.arg = NULL;
  timer_id = tls_timer_create(&timer_cfg);
  tls_timer_start(timer_id);
}
/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period){
  jshUtilTimerDisable();
  jshUtilTimerStart(period);
}
/// Stop the timer
void jshUtilTimerDisable(){
  if(timer_id!=WM_TIMER_ID_INVALID){
    tls_timer_stop(timer_id);
    timer_id=WM_TIMER_ID_INVALID;
  }
}

// ---------------------------------------------- LOW LEVEL
/// the temperature from the internal temperature sensor, in degrees C
JsVarFloat jshReadTemperature(){
  return NAN;
}

/// The voltage that a reading of 1 from `analogRead` actually represents, in volts
JsVarFloat jshReadVRef(){
  return NAN;
}

/** Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()` */
unsigned int jshGetRandomNumber(){
  return rand();
}

/** Change the processor clock info. What's in options is platform
 * specific - you should update the docs for jswrap_espruino_setClock
 * to match what gets implemented here. The return value is the clock
 * speed in Hz though. */
unsigned int jshSetSystemClock(JsVar *options){
  return 0;
}

/// Perform a proper hard-reboot of the device
void jshReboot(){
  NVIC_SystemReset();
}

/// Console setup
void jshConsoleSetup(){
  JshUSARTInfo inf;
  jshUSARTInitInfo(&inf);
  inf.pinRX=DEFAULT_CONSOLE_RX_PIN;
  inf.pinTX=DEFAULT_CONSOLE_TX_PIN;
  inf.baudRate=DEFAULT_CONSOLE_BAUDRATE;
  jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf);
}