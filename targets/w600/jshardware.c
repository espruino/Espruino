#include "jshardware.h"

/// jshInit is called at start-up, put hardware dependent init stuff in this function
void jshInit(){

}

/// jshReset is called from JS 'reset()' - try to put peripherals back to their power-on state
void jshReset(){

}

/** Code that is executed each time around the idle loop. Prod watchdog timers here,
 * and on platforms without GPIO interrupts you can check watched Pins for changes. */
void jshIdle(){

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
  return false;
}

/** Clean up ready to stop Espruino. Unused on embedded targets, but used on Linux,
 * where GPIO that have been exported may need unexporting, and so on. */
void jshKill(){

}

/** Get this IC's serial number. Passed max # of chars and a pointer to write to.
 * Returns # of chars of non-null-terminated string.
 *
 * This is reported back to `process.env` and is sometimes used in USB enumeration.
 * It doesn't have to be unique, but some users do use this in their code to distinguish
 * between boards.
 */
int jshGetSerialNumber(unsigned char *data, int maxChars){
  return 0;
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
  return 0;
}
/** Set the system time (in ticks since the epoch) - this should only be called rarely as it
could mess up things like jsinteractive's timers! */
void jshSetSystemTime(JsSysTime time){

}
/// Convert a time in Milliseconds since the epoch to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms){
  return 0;
}
/// Convert ticks to a time in Milliseconds since the epoch
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time){
  return 0;
}

// software IO functions...
void jshInterruptOff(){ ///< disable interrupts to allow short delays to be accurate

}

void jshInterruptOn(){  ///< re-enable interrupts

}

bool jshIsInInterrupt(){ ///< Are we currently in an interrupt?
  return false;
}

void jshDelayMicroseconds(int microsec){  ///< delay a few microseconds. Should use used sparingly and for very short periods - max 1ms

}

void jshPinSetValue(Pin pin, bool value){ ///< Set a digital output to 1 or 0. DOES NOT change pin state OR CHECK PIN VALIDITY

}

bool jshPinGetValue(Pin pin){ ///< Get the value of a digital input. DOES NOT change pin state OR CHECK PIN VALIDITY
  return false;
}

/// Set the pin state (Output, Input, etc)
void jshPinSetState(Pin pin, JshPinState state){

}

/** Get the pin state (only accurate for simple IO - won't return
 * JSHPINSTATE_USART_OUT for instance). Note that you should use
 * JSHPINSTATE_MASK as other flags may have been added
 * (like JSHPINSTATE_PIN_IS_ON if pin was set to output) */
JshPinState jshPinGetState(Pin pin){
  return 0;
}

/** Returns an analog value between 0 and 1. 0 is expected to be 0v, and
 * 1 means jshReadVRef() volts. On most devices jshReadVRef() would return
 * around 3.3, so a reading of 1 represents 3.3v. */
JsVarFloat jshPinAnalog(Pin pin){
  return 0;
}

/** Returns a quickly-read analog value in the range 0-65535.
 * This is basically `jshPinAnalog()*65535`
 * For use from an IRQ where high speed is needed */
int jshPinAnalogFast(Pin pin){
  return 0;
}

/// Output an analog value on a pin - either via DAC, hardware PWM, or software PWM
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags){ // if freq<=0, the default is used
  return 0;
}
/// Pulse a pin for a certain time, but via IRQs, not JS: `digitalWrite(pin,value);setTimeout("digitalWrite(pin,!value)", time*1000);`
void jshPinPulse(Pin pin, bool value, JsVarFloat time){

}
/// Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin){
  return false;
}
/// start watching pin - return the EXTI (IRQ number flag) associated with it
IOEventFlags jshPinWatch(Pin pin, bool shouldWatch){
  return 0;
}

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin){
  return 0;
}

/** Given a pin function, set that pin to the 16 bit value
 * (used mainly for fast DAC and PWM handling from Utility Timer) */
void jshSetOutputValue(JshPinFunction func, int value){

}

/// Enable watchdog with a timeout in seconds, it'll reset the chip if jshKickWatchDog isn't called within the timeout
void jshEnableWatchDog(JsVarFloat timeout){
  
}

// Kick the watchdog
void jshKickWatchDog(){

}

/// Check the pin associated with this EXTI - return true if the pin's input is a logic 1
bool jshGetWatchedPinState(IOEventFlags device){
  return false;
}

/// Given an event, check the EXTI flags and see if it was for the given pin
bool jshIsEventForPin(IOEvent *event, Pin pin){
  return false;
}

/** Is the given device initialised?
 * eg. has jshUSARTSetup/jshI2CSetup/jshSPISetup been called previously? */
bool jshIsDeviceInitialised(IOEventFlags device){
  return false;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf){

}

/** Kick a device into action (if required). For instance we may have data ready
 * to sent to a USART, but we need to enable the IRQ such that it can automatically
 * fetch the characters to send.
 *
 * Later down the line this could potentially even set up something like DMA.*/
void jshUSARTKick(IOEventFlags device){

}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf){

}
/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data){
  return 0;
}
/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data){

}
/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16){

}
/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive){

}
/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device){

}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf){

}

/** Write a number of btes to the I2C device. Addresses are 7 bit - that is, between 0 and 0x7F.
 *  sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop){

}
/** Read a number of bytes from the I2C device. */
void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop){

}

/** Return start address and size of the flash page the given address resides in. Returns false if
  * the page is outside of the flash address range */
bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize){
  return false;
}
/** Return a JsVar array containing objects of the form `{addr, length}` for each contiguous block of free
 * memory available. These should be one complete pages, so that erasing the page containing any address in
 * this block won't erase anything useful! */
JsVar *jshFlashGetFree(){
  return 0;
}
/// Erase the flash page containing the address
void jshFlashErasePage(uint32_t addr){

}
/** Read data from flash memory into the buffer, the flash address has no alignment restrictions
  * and the len may be (and often is) 1 byte */
void jshFlashRead(void *buf, uint32_t addr, uint32_t len){

}
/** Write data to flash memory from the buffer, the buffer address and flash address are
  * guaranteed to be 4-byte aligned, and length is a multiple of 4.  */
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len){

}

/** On most platforms, the address of something really is that address.
 * In ESP32/ESP8266 the flash memory is mapped up at a much higher address,
 * so we need to tweak any pointers that we use.
 * */
size_t jshFlashGetMemMapAddress(size_t ptr){
  return 0;
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

}
/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period){

}
/// Stop the timer
void jshUtilTimerDisable(){

}

// ---------------------------------------------- LOW LEVEL
/// the temperature from the internal temperature sensor, in degrees C
JsVarFloat jshReadTemperature(){
  return 0;
}

/// The voltage that a reading of 1 from `analogRead` actually represents, in volts
JsVarFloat jshReadVRef(){
  return 0;
}

/** Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()` */
unsigned int jshGetRandomNumber(){
  return 0;
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

}
