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

// SAMD Includes
#include "../../targetlibs/samd/include/due_sam3x.init.h"

#define SYSCLK_FREQ 84000000 // Using standard HFXO freq
#define USE_RTC

//---------------------- RTC/clock ----------------------------/
#define RTC_INITIALISE_TICKS 4 // SysTicks before we initialise the RTC - we need to wait until the LSE starts up properly
#define JSSYSTIME_SECOND_SHIFT 20
#define JSSYSTIME_SECOND  (1<<JSSYSTIME_SECOND_SHIFT) // Random value we chose - the accuracy we're allowing (1 microsecond)
#define JSSYSTIME_MSECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000
#define JSSYSTIME_USECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000000

void jshInit() {
	// Blink a LED to say that we're there!
	/* The general init (clock, libc, watchdog ...) */
  	init_controller();

 	/* Board pin 13 == PB27 */
	PIO_Configure(PIOB, PIO_OUTPUT_1, PIO_PB27, PIO_DEFAULT);

	/* Main loop */
	while(1) {
		Sleep(250);
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
