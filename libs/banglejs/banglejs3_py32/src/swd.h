/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Small SWD driver for Bangle.js 3
 * ----------------------------------------------------------------------------
 */

// grab SWD control
void swdInit();
// issue a reset command
void swdReset();
// release SWD
void swdKill();


/// Read from a specific memory address
uint32_t swdReadMem(uint32_t addr);
/// Write to a specific memory address
void swdWriteMem(uint32_t addr, uint32_t value);

void swdHalt();
void swdResume();
/// ARM Core System Reset
void swdSoftReset();

// Erase entire PY32 flash memory
void swdPY32FlashErase();
// Initialise PY32 flash write registers
void swdPY32FlashWriteInit();
 // write to flash - len in bytes. Must start at 256b boundary, but no size limit
void swdPY32FlashWrite(uint32_t addr, uint32_t *buf, int len);