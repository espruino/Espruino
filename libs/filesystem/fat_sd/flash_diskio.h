/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 *  Copyright (C) 2016 by Rhys Williams (wilberforce)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * For flash access as a File System
 * ----------------------------------------------------------------------------
 */

 #include "platform_config.h"
 
 // Ideally these should be defined in the BOARD.py file...
 
 // 1MB = 1024*1024 / 4096 = 256
#ifdef USE_ESP32
#define FS_SECTOR_COUNT 256
#else
// last 4 sectors used for wifi phy data
#define FS_SECTOR_COUNT 250
#endif
// Set sector size as the same a flash block size
#define FS_SECTOR_SIZE FLASH_PAGE_SIZE
// cluster = 1 sector
#define FS_BLOCK_SIZE 1
// last 1Mb block in 4MB Flash
#define FS_FLASH_BASE 0x300000;

uint8_t flashFatFsInit( uint32_t addr, uint16_t sectors );

/* 
#ifndef RELEASE
#define jsDebug(fmt,...) jsWarn(fmt,##__VA_ARGS__)
#else
#define jsDebug(fmt,...)
#endif
#endif
*/
#define jsDebug(fmt,...)
