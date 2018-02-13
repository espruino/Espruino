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
// Standard includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

//Espruino includes
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.
#include "jsflags.h"

// My Includes
#include "jshardware.h"
#include "flash.h"
#include "../targetlibs/samd/sam/libsam/include/efc.h"

#define UART1BAUDRATE 9600

// Systemtick counter
uint32_t uppercounter = 0;

void serdebugstring(char* debugstring) {
        int i = 0;
        while(1) {
                if (debugstring[i] == 0) {
                        break;
                }
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
                UART->UART_THR = debugstring[i];
                i++;
        }
}

void serdebugint(int debugval) {
        while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
        UART->UART_THR = (char) (debugval >> 24);
        while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
        UART->UART_THR = (char) (debugval >> 16);
        while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
        UART->UART_THR = (char) (debugval >> 8);
        while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
        UART->UART_THR = (char) (debugval);
}

/********************************************************************************
 * Device interrupt vector. 
 ********************************************************************************/

static void __phantom_handler(void) { while(1); }

void NMI_Handler        (void) { serdebugstring("nmi"); } 
void HardFault_Handler  (void) { serdebugstring("hf"); } 
void MemManage_Handler  (void) { serdebugstring("mf"); } 
void BusFault_Handler   (void) { serdebugstring("bf"); }
void UsageFault_Handler (void) { serdebugstring("uf"); }
void DebugMon_Handler   (void) __attribute__ ((weak, alias("__phantom_handler")));
void SVC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void PendSV_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void SysTick_Handler    (void) { TimeTick_Increment(); if (GetTickCount() == 0) { uppercounter++; } }
void SUPC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RSTC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void WDT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void PMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
//void UART_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SMC_INSTANCE_
void SMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_SDRAMC_INSTANCE_
void SDRAMC_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PIOA_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PIOB_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_PIOC_INSTANCE_
void PIOC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOD_INSTANCE_
void PIOD_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOE_INSTANCE_
void PIOE_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOF_INSTANCE_
void PIOF_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void USART0_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART1_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART2_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_USART3_INSTANCE_
void USART3_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void HSMCI_Handler      (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void SPI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SPI1_INSTANCE_
void SPI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void SSC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC0_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC1_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC2_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC3_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC4_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC5_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_TC2_INSTANCE_
void TC6_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC7_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC8_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PWM_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void ADC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void DACC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void DMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UOTGHS_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void TRNG_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_EMAC_INSTANCE_
void EMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void CAN0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void CAN1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));

void jshInterruptOff() {
	__disable_irq();
}

void jshInterruptOn() {
	__enable_irq();
}

// Uart Receive, bumps the char through to espruino interpreter
void UART_Handler() {
	uint32_t status = UART->UART_SR;
	if((status & UART_SR_RXRDY) == UART_SR_RXRDY) {
		while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
		uint8_t mychar = UART->UART_RHR;
		jshPushIOCharEvent(EV_SERIAL1, mychar);
	}
}

void jshInit() {
	/* The general init (clock, libc, watchdog ...) */
	/*
	 * SAM System init: Initializes the PLL / clock.
	 * Defined in CMSIS/ATMEL/sam3xa/source/system_sam3xa.c
	 */
	SystemInit();

	/*
	 * Config systick interrupt timing, core clock is in microseconds --> 10 Microsecond Accuracy
	 * Defined in CMSIS/CMSIS/include/core_cm3.h
	 */
	if (SysTick_Config(SystemCoreClock / 100000)) while (1);

	/*
	 * No watchdog now
	 *
	 */
	WDT_Disable(WDT);

	/*
	 * GCC libc init, also done in Reset_Handler()
	*/
	__libc_init_array();

	// Enable Clock for GPIO
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);

	/* Init the UART for REPL */
	// Set I/O Pins for UART to Output
	PIO_Configure(PIOA, PIO_PERIPH_A,PIO_PA8A_URXD|PIO_PA9A_UTXD, PIO_DEFAULT);

	// Enable Pullup on Rx and Tx Pin
	PIOA->PIO_PUER = PIO_PA8A_URXD | PIO_PA9A_UTXD;

	// Enable Clock for UART
	pmc_enable_periph_clk(ID_UART);

	// Disable PDC Channel
	UART->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	// Reset and disable receiver and transmitter
	UART->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;

	// Configure mode
	UART->UART_MR = UART_MR_PAR_NO | UART_MR_CHMODE_NORMAL;

	// Configure baudrate (asynchronous, no oversampling)
	UART->UART_BRGR = (SystemCoreClock / UART1BAUDRATE) >> 4;

	// Configure interrupts
	UART->UART_IDR = 0xFFFFFFFF;
	UART->UART_IER = UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME;

	// Enable UART interrupt in NVIC
	NVIC_EnableIRQ(UART_IRQn);

	// Enable receiver and transmitter
	UART->UART_CR = UART_CR_RXEN | UART_CR_TXEN;

	// Activate RTC
	RTC->RTC_MR = 0;
	RTC->RTC_CR = 0;

//	serdebugstring("init");
}

void jshIdle() {
        /* While we're idle, we check for UART Transmit */
        int check_char = jshGetCharToTransmit(EV_SERIAL1);
        if (check_char >= 0) {
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
                UART->UART_THR = check_char;
        }
}

void jshUSARTKick(IOEventFlags device) {
        /* Check for UART Transmit */
        int check_char = jshGetCharToTransmit(device);
        if (check_char >= 0) {
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
                UART->UART_THR = check_char;
        }

}

void jshPinSetValue(Pin pin, bool value) {
	// We don't want UART0 here, because thats our Console
	if (pin == 0 || pin == 1) { return; };

	if (value) {
		PIO_Set(mypins[pin][0], mypins[pin][1]);
	} else {
		PIO_Clear(mypins[pin][0], mypins[pin][1]);
	};
}

JsVarFloat jshPinAnalog(Pin pin) {
        // We don't want UART0 here, because thats our Console
        if (pin == 0 || pin == 1) { return; };

	return 0;
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
	// GetTickCount() is in 10 Microseconds
	return ((JsVarFloat)time/100);
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
	return (JsSysTime)(ms*100);
}

void jshFlashErasePage(uint32_t addr) {
//	jsiConsolePrintf("\ntcjshFlashErasePage addr 0x%x", addr);
	// We're erasing the whole flash bank here, because only on bank1 we have the espruino code, bank2 is completly for javascript code
	flash_erase_all(addr);
}

bool jshFlashGetPage(uint32_t addr, uint32_t * startAddr, uint32_t * pageSize) {
//	jsiConsolePrintf("\ntcjshFlashGetPage addr 0x%x startAddr 0x%x pageSize 0x%x", addr, *startAddr, *pageSize);
	return false;
}

void jshFlashWrite(void * buf, uint32_t addr, uint32_t len) {
//	jsiConsolePrintf("\ntcjshFlashWrite addr 0x%x len 0x%x", addr, len);
	flash_unlock((uint32_t)addr, (uint32_t)addr + len - 1, 0, 0);
	flash_write((uint32_t)addr, buf, len, 1);
	flash_lock((uint32_t)addr, (uint32_t)addr + len - 1, 0, 0);
}

void jshFlashRead(void * buf, uint32_t addr, uint32_t len) {
//	jsiConsolePrintf("\ntcjshFlashRead addr 0x%x len 0x%x", addr, len);
	memcpy(buf, (void*)addr, len);
}

// Just pass data through, since we can access flash at the same address we wrote it
size_t jshFlashGetMemMapAddress(size_t ptr) { return ptr; }

JsSysTime jshGetSystemTime() {
	return (JsSysTime) ((unsigned long long)uppercounter * 0xFFFFFFFF) + GetTickCount();
}

bool jshIsInInterrupt() {
	return false;
}

void jshPinSetState(Pin pin, JshPinState state) {
        // We don't want UART0 here, because thats our Console
        if (pin == 0 || pin == 1) { return; };

	if (state == JSHPINSTATE_GPIO_IN) {
		PIO_Configure(mypins[pin][0], PIO_INPUT, mypins[pin][1], PIO_DEFAULT);
	}

	if (state == JSHPINSTATE_GPIO_OUT) {
		PIO_Configure(mypins[pin][0], PIO_OUTPUT_1, mypins[pin][1], PIO_DEFAULT);
	};
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

void jshSPISet16(IOEventFlags device, bool is16) {
}

void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

void jshSPIWait(IOEventFlags device) {
}

bool jshIsDeviceInitialised(IOEventFlags device) {
	return false;
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

bool jshPinGetValue(Pin pin) {
	PIO_Get(mypins[pin][0], PIO_INPUT, mypins[pin][1]);
}

void jshDelayMicroseconds(int microsec) {
}

void jshEnableWatchDog(JsVarFloat timeout) {
}

void jshKickWatchDog() {
}

int jshGetSerialNumber(unsigned char *data, int maxChars) {
	return 666;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
        // We don't want UART0 here, because thats our Console
        if (pin == 0 || pin == 1) { return; };

	return JSH_NOTHING;
}

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {
        // We don't want UART0 here, because thats our Console
        if (pin == 0 || pin == 1) { return; };

}

JshPinState jshPinGetState(Pin pin) {
	return 0;
}

bool jshCanWatch(Pin pin) {
	return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
        // We don't want UART0 here, because thats our Console
        if (pin == 0 || pin == 1) { return; };

	return EV_NONE;
}

void jshSetSystemTime(JsSysTime newTime) {
}

unsigned int jshSetSystemClock(JsVar *options) {
	return 0;
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
}

JsVar *jshFlashGetFree() {
//	jsiConsolePrintf("\ntcjshGetFree");
	return 0;
}

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
	return JSH_NOTHING;
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
}

void jshUtilTimerStart(JsSysTime period) {
}

int jshSPISend(IOEventFlags device, int data) {
	return -1;
}

void jshSPISend16(IOEventFlags device, int data) {
}

JsVarFloat jshReadTemperature() {
	return 0;
}

JsVarFloat jshReadVRef() {
	return 0;
}

bool jshSleep(JsSysTime timeUntilWake) {
	return false;
}

void jshReset() {
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
	return false;
}

void jshUtilTimerDisable() {
}

unsigned int jshGetRandomNumber() {
	return 1234;
}
