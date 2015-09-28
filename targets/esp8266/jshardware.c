#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <gpio.h>
#include <mem.h>
#include <espmissingincludes.h>
#include <driver/uart.h>

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

// The maximum time that we can safely delay/block without risking a watch dog
// timer error or other undesirable WiFi interaction.  The time is measured in
// microseconds.
#define MAX_SLEEP_TIME_US	10000

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

IOEventFlags pinToEVEXTI(Pin pin) {
	return (IOEventFlags) 0;
}

/**
 * Initialize the ESP8266 hardware environment.
 */
void jshInit() {
	// A call to jshInitDevices is architected as something we have to do.
	jshInitDevices();
} // End of jshInit

void jshReset() {
	// TODO
} // End of jshReset

void jshKill() {
	// TODO
} // End of jshKill

/**
 * Hardware idle processing.
 */
void jshIdle() {
} // End of jshIdle

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
	const char *code = "ESP8266";
	strncpy((char *) data, code, maxChars);
	return strlen(code);
} // End of jshSerialNumber

// ----------------------------------------------------------------------------

void jshInterruptOff() {
	// TODO
} // End of jshInterruptOff

void jshInterruptOn() {
	// TODO
} // End of jshInterruptOn

/**
 * Delay (blocking) for the supplied number of microseconds.
 * Note that for the ESP8266 we must NOT CPU block for more than
 * 10 milliseconds or else we may starve the WiFi subsystem.
 */
void jshDelayMicroseconds(int microsec) {
	// Get the current time
	/*
	uint32 endTime = system_get_time() + microsec;
	while ((endTime - system_get_time()) > 10000) {
		os_delay_us(10000);
		system_soft_wdt_feed();
	}
	int lastDelta = endTime - system_get_time();
	if (lastDelta > 0) {
		os_delay_us(lastDelta);
	}
	*/

	// This is a place holder implementation.  We can and must do better
	// than this.  This fails because we will sleep too long.  We will sleep
	// for the given number of microseconds PLUS multiple calls back to the
	// WiFi environment.
	int count = microsec / MAX_SLEEP_TIME_US;
	int i;
	for (i=0; i<count; i++) {
		os_delay_us(MAX_SLEEP_TIME_US);
		// We may have a problem here.  It was my understanding that system_soft_wdt_feed() fed
		// the underlying OS but this appears not to be the case and all it does is prevent a
		// watchdog timer from firing.  What that means is that we may very well loose network
		// connectivity because we are not servicing the housekeeping.   This might be one of those
		// locations where we need to look at a callback or some kind of yield technology.
		system_soft_wdt_feed();
		microsec -= MAX_SLEEP_TIME_US;
	}
	assert(microsec < MAX_SLEEP_TIME_US);
	if (microsec > 0) {
		os_delay_us(microsec);
	}

	/*
	if (0 < microsec) {
		os_delay_us(microsec);
	}
	*/
} // End of jshDelayMicroseconds


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

#define FUNC_SPI 1
#define FUNC_GPIO 3
#define FUNC_UART 4

static uint8_t pinFunction(JshPinState state) {
	switch (state) {
	case JSHPINSTATE_GPIO_OUT:
	case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
	case JSHPINSTATE_GPIO_IN:
	case JSHPINSTATE_GPIO_IN_PULLUP:
	case JSHPINSTATE_GPIO_IN_PULLDOWN:
		return FUNC_GPIO;
	case JSHPINSTATE_USART_OUT:
	case JSHPINSTATE_USART_IN:
		return FUNC_UART;
	case JSHPINSTATE_I2C:
		return FUNC_SPI;
	case JSHPINSTATE_AF_OUT:
	case JSHPINSTATE_AF_OUT_OPENDRAIN:
	case JSHPINSTATE_DAC_OUT:
	case JSHPINSTATE_ADC_IN:
	default:
		return 0;
	}
} // End of pinFunction

/**
 * \brief Set the state of the specific pin.
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
 */
void jshPinSetState(Pin pin, //!< The pin to have its state changed.
		JshPinState state    //!< The new desired state of the pin.
	) {
	jsiConsolePrintf("ESP8266: jshPinSetState %d, %d\n", pin, state);

	assert(pin < 16);
	int periph = PERIPHS_IO_MUX + PERIPHS[pin];

	// Disable the pin's pull-up.
	PIN_PULLUP_DIS(periph);
	//PIN_PULLDWN_DIS(periph);

	uint8_t primary_func =
			pin < 6 ?
					(PERIPHS_IO_MUX_U0TXD_U == pin
							|| PERIPHS_IO_MUX_U0RXD_U == pin) ?
							FUNC_UART : FUNC_GPIO
					: 0;
	uint8_t select_func = pinFunction(state);
	PIN_FUNC_SELECT(periph, primary_func == select_func ? 0 : select_func);

	switch (state) {
	case JSHPINSTATE_GPIO_OUT:
	case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
		//case JSHPINSTATE_AF_OUT:
		//case JSHPINSTATE_AF_OUT_OPENDRAIN:
		//case JSHPINSTATE_USART_OUT:
		//case JSHPINSTATE_DAC_OUT:
		gpio_output_set(0, 1 << pin, 1 << pin, 0);
		break;

	case JSHPINSTATE_GPIO_IN_PULLUP:
		PIN_PULLUP_EN(periph);
		//case JSHPINSTATE_GPIO_IN_PULLDOWN: if (JSHPINSTATE_GPIO_IN_PULLDOWN == pin) PIN_PULLDWN_EN(periph);
	case JSHPINSTATE_GPIO_IN:
		gpio_output_set(0, 0, 0, 1 << pin);
		break;

	case JSHPINSTATE_ADC_IN:
	case JSHPINSTATE_USART_IN:
	case JSHPINSTATE_I2C:
		PIN_PULLUP_EN(periph);
		break;

	default:
		break;
	}
} // End of jshPinSetState


/**
 * \brief Return the current state of the selected pin.
 * \return The current state of the selected pin.
 */
JshPinState jshPinGetState(Pin pin) {
	jsiConsolePrintf("ESP8266: jshPinGetState %d\n", pin);
	return JSHPINSTATE_UNDEFINED;
} // End of jshPinGetState


/**
 * \brief Set the value of the corresponding pin.
 */
void jshPinSetValue(Pin pin, //!< The pin to have its value changed.
		bool value           //!< The new value of the pin.
	) {
	jsiConsolePrintf("ESP8266: jshPinSetValue %d, %d\n", pin, value);
	GPIO_OUTPUT_SET(pin, value);
} // End of jshPinSetValue


/**
 * \brief Get the value of the corresponding pin.
 * \return The current value of the pin.
 */
bool jshPinGetValue(Pin pin //!< The pin to have its value read.
	) {
	jsiConsolePrintf("ESP8266: jshPinGetValue %d, %d\n", pin, GPIO_INPUT_GET(pin));
	return GPIO_INPUT_GET(pin);
} // End of jshPinGetValue


bool jshIsDeviceInitialised(IOEventFlags device) {
	jsiConsolePrintf("ESP8266: jshIsDeviceInitialised %d\n", device);
	return true;
} // End of jshIsDeviceInitialised

bool jshIsUSBSERIALConnected() {
	jsiConsolePrintf("ESP8266: jshIsUSBSERIALConnected\n");
	return true;
} // End of jshIsUSBSERIALConnected

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
//	jsiConsolePrintf("jshGetTimeFromMilliseconds %d, %f\n", (JsSysTime)(ms * 1000.0), ms);
	return (JsSysTime) (ms * 1000.0 + 0.5);
} // End of jshGetTimeFromMilliseconds

/**
 * Given a time in microseconds, get us the value in milliseconds (float)
 */
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
//	jsiConsolePrintf("jshGetMillisecondsFromTime %d, %f\n", time, (JsVarFloat)time / 1000.0);
	return (JsVarFloat) time / 1000.0;
} // End of jshGetMillisecondsFromTime

/**
 * Return the current time in microseconds.
 */
JsSysTime jshGetSystemTime() { // in us
	return system_get_time();
} // End of jshGetSystemTime


/**
 * Set the current time in microseconds.
 */
void jshSetSystemTime(JsSysTime time) {
	os_printf("ESP8266: jshSetSystemTime: %d\n", (int)time);
} // End of jshSetSystemTime

// ----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
	jsiConsolePrintf("ESP8266: jshPinAnalog: %d\n", pin);
	return (JsVarFloat) system_adc_read();
} // End of jshPinAnalog


int jshPinAnalogFast(Pin pin) {
	jsiConsolePrintf("ESP8266: jshPinAnalogFast: %d\n", pin);
	return NAN;
} // End of jshPinAnalogFast


JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, the default is used
	jsiConsolePrintf("ESP8266: jshPinAnalogOutput: %d, %d, %d\n", pin, (int)value, (int)freq);
//pwm_set(pin, value < 0.0f ? 0 : 255.0f < value ? 255 : (uint8_t)value);
	return 0;
} // End of jshPinAnalogOutput


void jshSetOutputValue(JshPinFunction func, int value) {
	jsiConsolePrintf("ESP8266: jshSetOutputValue %d %d\n", func, value);
} // End of jshSetOutputValue


void jshEnableWatchDog(JsVarFloat timeout) {
	jsiConsolePrintf("ESP8266: jshEnableWatchDog %0.3f\n", timeout);
} // End of jshEnableWatchDog


bool jshGetWatchedPinState(IOEventFlags device) {
	jsiConsolePrintf("ESP8266: jshGetWatchedPinState %d", device);
	return false;
} // End of jshGetWatchedPinState


/**
 * \brief Set the value of the pin to be the value supplied and then wait for
 * a given period and set the pin value again to be the opposite.
 */
void jshPinPulse(Pin pin, //!< The pin to be pulsed.
		bool value,       //!< The value to be pulsed into the pin.
		JsVarFloat time   //!< The period in milliseconds to hold the pin.
	) {
	if (jshIsPinValid(pin)) {
		jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
		jshPinSetValue(pin, value);
		jshDelayMicroseconds(jshGetTimeFromMilliseconds(time));
		jshPinSetValue(pin, !value);
	} else
		jsError("Invalid pin!");
} // End of jshPinPulse


bool jshCanWatch(Pin pin) {
	return false;
} // End of jshCanWatch


IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
	if (jshIsPinValid(pin)) {
	} else
		jsError("Invalid pin!");
	return EV_NONE;
} // End of jshPinWatch


JshPinFunction jshGetCurrentPinFunction(Pin pin) {
	//os_printf("jshGetCurrentPinFunction %d\n", pin);
	return JSH_NOTHING;
} // End of jshGetCurrentPinFunction


bool jshIsEventForPin(IOEvent *event, Pin pin) {
	return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
} // End of jshIsEventForPin


void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
} // End of jshUSARTSetup


/**
 * Kick a device into action (if required). For instance we may need
 * to set up interrupts.  In this ESP8266 implementation, we transmit all the
 * data that can be found associated with the device.
 */
void jshUSARTKick(IOEventFlags device) {
	esp8266_uartTransmitAll(device);
} // End of jshUSARTKick

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
} // End of jshSPISetup

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
	return NAN;
} // End of jshSPISend

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
	jshSPISend(device, data >> 8);
	jshSPISend(device, data & 255);
} // End of jshSPISend16

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
} // End of jshSPISet16

/** Wait until SPI send is finished, */
void jshSPIWait(IOEventFlags device) {
} // End of jshSPIWait

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
} // End of jshI2CSetup

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes,
		const unsigned char *data, bool sendStop) {
} // End of jshI2CWrite

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes,
		unsigned char *data, bool sendStop) {
} // End of jshI2CRead

/**
 * \brief Save what is in RAM to flash.
 * See also `jshLoadFromFlash`.
 */
void jshSaveToFlash() {
	os_printf("ESP8266: jshSaveToFlash\n");
} // End of jshSaveToFlash

void jshLoadFromFlash() {
	os_printf("ESP8266: jshLoadFromFlash\n");
} // End of jshLoadFromFlash


bool jshFlashContainsCode() {
	os_printf("ESP8266: jshFlashContainsCode\n");
	return false;
} // End of jshFlashContainsCode


/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
	int time = (int) timeUntilWake;
//	os_printf("jshSleep %d\n", time);
	jshDelayMicroseconds(time);
	return true;
} // End of jshSleep


void jshUtilTimerDisable() {
	os_printf("ESP8266: jshUtilTimerDisable\n");
} // End of jshUtilTimerDisable


void jshUtilTimerReschedule(JsSysTime period) {
	os_printf("ESP8266: jshUtilTimerReschedule %d\n", (int)period);
} // End of jshUtilTimerReschedule


void jshUtilTimerStart(JsSysTime period) {
	os_printf("ESP8266: jshUtilTimerStart %d\n",(int) period);
} // End of jshUtilTimerStart


JsVarFloat jshReadTemperature() {
	return NAN;
} // End of jshReadTemperature


JsVarFloat jshReadVRef() {
	return NAN;
} // End of jshReadVRef


unsigned int jshGetRandomNumber() {
	return rand();
} // End of jshGetRandomNumber


/**
 * \brief Read data from flash memory into the buffer.
 *
 */
void jshFlashRead(
		void *buf,     //!< buffer to read into
		uint32_t addr, //!< Flash address to read from
		uint32_t len   //!< Length of data to read
	) {
	os_printf("ESP8266: jshFlashRead: buf=0x%x for len=%d from flash addr=0x%x\n", (int)buf, (int)len, (int)addr);
} // End of jshFlashRead


/**
 * \brief Write data to flash memory from the buffer.
 */
void jshFlashWrite(
		void *buf,     //!< Buffer to write from
		uint32_t addr, //!< Flash address to write into
		uint32_t len   //!< Length of data to write
	) {
	os_printf("ESP8266: jshFlashWrite: buf=0x%x for len=%d into flash addr=0x%x\n", (int)buf, (int)len, (int)addr);
} // End of jshFlashWrite


/**
 * \brief Return start address and size of the flash page the given address resides in. Returns false if no page.
 */
bool jshFlashGetPage(
		uint32_t addr,       //!<
		uint32_t *startAddr, //!<
		uint32_t *pageSize   //!<
	) {
	os_printf("ESP8266: jshFlashGetPage: addr=0x%x, startAddr=0x%x, pageSize=%d\n", (int)addr, (int)startAddr, (int)pageSize);
	return false;
} // End of jshFlashGetPage


/**
 * \brief Erase the flash page containing the address.
 */
void jshFlashErasePage(
		uint32_t addr //!<
	) {
	os_printf("ESP8266: jshFlashErasePage: addr=0x%x\n", (int)addr);
} // End of jshFlashErasePage


/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
} // End of jshSPISetReceive


/**
 * Callback for end of runtime.  This should never be called and has been
 * added to satisfy the linker.
 */
void _exit(int status) {
} // End of _exit
// End of file
