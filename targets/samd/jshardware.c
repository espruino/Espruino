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
#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.
#include "jsflags.h"

#define SYSCLK_FREQ 84000000 // Using standard HFXO freq
#define USE_RTC

//---------------------- RTC/clock ----------------------------/
#define RTC_INITIALISE_TICKS 4 // SysTicks before we initialise the RTC - we need to wait until the LSE starts up properly
#define JSSYSTIME_SECOND_SHIFT 20
#define JSSYSTIME_SECOND  (1<<JSSYSTIME_SECOND_SHIFT) // Random value we chose - the accuracy we're allowing (1 microsecond)
#define JSSYSTIME_MSECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000
#define JSSYSTIME_USECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000000

/********************************************************************************
 * Device interrupt vector. 
 ********************************************************************************/
static void __phantom_handler(void) { while(1); }

void NMI_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void HardFault_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void MemManage_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void BusFault_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("__phantom_handler")));
void DebugMon_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void SVC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PendSV_Handler    (void) __attribute__ ((weak, alias("__phantom_handler")));
void SysTick_Handler(void) { TimeTick_Increment(); }
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

// Uart Receive does only loopback for now
void UART_Handler() {
	uint32_t status = UART->UART_SR;
	if((status & UART_SR_RXRDY) == UART_SR_RXRDY) {
		while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
		//UART->UART_THR = UART->UART_RHR;
		uint8_t test = UART->UART_RHR;
		UART->UART_THR = test;
	}
}


void jshInit() {
	// Send "Hello World" over TX1 to say that we're there!
	
	/* The general init (clock, libc, watchdog ...) */
	init_controller();

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
	UART->UART_BRGR = (SYSCLK_FREQ / 9600) >> 4;

	// Configure interrupts
	UART->UART_IDR = 0xFFFFFFFF;
	UART->UART_IER = UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME;

	// Enable UART interrupt in NVIC
	NVIC_EnableIRQ(UART_IRQn);

	// Enable receiver and transmitter
	UART->UART_CR = UART_CR_RXEN | UART_CR_TXEN;

        /* Board pin 13 == PB27 */
        PIO_Configure(PIOB, PIO_OUTPUT_1, PIO_PB27, PIO_DEFAULT);

	/* Main loop */
	while(1) {
               Sleep(5000);

				// Now send Hello World, one char at a time, whenever the UART Transmitter is ready!
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'H';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'e';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'l';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'l';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'o';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = ' ';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'W';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'o';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'r';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'l';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = 'd';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = '!';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = '!';
				
                while((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY);
				UART->UART_THR = '!';

		/* And blink the LED */
		if(PIOB->PIO_ODSR & PIO_PB27) {
			/* Set clear register */
                       	PIOB->PIO_CODR = PIO_PB27;
               	} else {
                	/* Set set register */
                       	PIOB->PIO_SODR = PIO_PB27;
               	}
 	}
 	
	return 0;
}
