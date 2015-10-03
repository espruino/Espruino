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

IOEventFlags pinToEVEXTI(Pin pin) {
	return (IOEventFlags) 0;
}

// forward declaration
static void systemTimeInit(void);

/**
 * Initialize the ESP8266 hardware environment.
 *
 * TODO: we should move stuff from user_main.c here
 */
void jshInit() {
	// A call to jshInitDevices is architected as something we have to do.
	systemTimeInit();
	jshInitDevices();
} // End of jshInit

/**
 * \brief Reset the hardware to a power-on state
 */
void jshReset() {
	system_restart();
} // End of jshReset

/**
 * \brief Handle whatever needs to be done in the idle loop when there's nothing to do
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
	ets_intr_lock();
} // End of jshInterruptOff

void jshInterruptOn() {
	ets_intr_unlock();
} // End of jshInterruptOn

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
	int time = (int) timeUntilWake;
//	os_printf("jshSleep %d\n", time);
	// **** TODO: fix this, this is garbage, we need to tell the idle loop to suspend
	jshDelayMicroseconds(time);
	return true;
} // End of jshSleep

/**
 * Delay (blocking) for the supplied number of microseconds.
 * Note that for the ESP8266 we must NOT CPU block for more than
 * 10 milliseconds or else we may starve the WiFi subsystem.
 */
void jshDelayMicroseconds(int microsec) {
	// Keep things simple and make the user responsible if they sleep for too long...
	if (microsec > 0) os_delay_us(microsec);
#if 0
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
#endif
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

//===== GPIO and PIN stuff =====

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

//===== USART and Serial =====

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
} // End of jshUSARTSetup

bool jshIsUSBSERIALConnected() {
	jsiConsolePrintf("ESP8266: jshIsUSBSERIALConnected\n");
	return true;
} // End of jshIsUSBSERIALConnected

/**
 * Kick a device into action (if required). For instance we may need
 * to set up interrupts.  In this ESP8266 implementation, we transmit all the
 * data that can be found associated with the device.
 */
void jshUSARTKick(IOEventFlags device) {
	esp8266_uartTransmitAll(device);
} // End of jshUSARTKick

//===== SPI =====

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

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
} // End of jshSPISetReceive

//===== I2C =====

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
} // End of jshI2CSetup

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes,
		const unsigned char *data, bool sendStop) {
} // End of jshI2CWrite

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes,
		unsigned char *data, bool sendStop) {
} // End of jshI2CRead

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
	os_printf("RTC write: %lu %lu 0x%08x\n", (uint32_t)(rtcTimeStamp.timeStamp/1000000),
		rtcTimeStamp.hwTimeStamp, rtcTimeStamp.cksum);
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
	os_printf("RTC sys=%lu rtc=%lu\n", sysTime, rtc);

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
	os_printf("RTC read: %lu %lu 0x%08x (0x%08x)\n", (uint32_t)(rtcTimeStamp.timeStamp/1000000),
		rtcTimeStamp.hwTimeStamp, rtcTimeStamp.cksum, cksum);
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

void jshUtilTimerDisable() {
	//os_printf("ESP8266: jshUtilTimerDisable\n");
} // End of jshUtilTimerDisable


void jshUtilTimerReschedule(JsSysTime period) {
	//os_printf("ESP8266: jshUtilTimerReschedule %d\n", period);
} // End of jshUtilTimerReschedule


void jshUtilTimerStart(JsSysTime period) {
	//os_printf("ESP8266: jshUtilTimerStart %d\n", period);
} // End of jshUtilTimerStart

//===== Miscellaneous =====

bool jshIsDeviceInitialised(IOEventFlags device) {
	jsiConsolePrintf("ESP8266: jshIsDeviceInitialised %d\n", device);
	return true;
} // End of jshIsDeviceInitialised

// the esp8266 doesn't have any temperature sensor
JsVarFloat jshReadTemperature() {
	return NAN;
} // End of jshReadTemperature

// the esp8266 can read the VRef but then there's no analog input, so we don't support this
JsVarFloat jshReadVRef() {
	return NAN;
} // End of jshReadVRef

unsigned int jshGetRandomNumber() {
	return rand();
} // End of jshGetRandomNumber

//===== Read-write flash =====

/**
 * \brief Read data from flash memory into the buffer.
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
	//		buf, len, addr, FLASH_MAX);

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
} // End of jshFlashRead


/**
 * \brief Write data to flash memory from the buffer.
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
} // End of jshFlashWrite


/**
 * \brief Return start address and size of the flash page the given address resides in.
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
} // End of jshFlashGetPage


/**
 * \brief Erase the flash page containing the address.
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
} // End of jshFlashErasePage


/**
 * Callback for end of runtime.  This should never be called and has been
 * added to satisfy the linker.
 */
void _exit(int status) {
} // End of _exit
// End of file
