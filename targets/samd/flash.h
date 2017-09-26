/**
 * \file
 *
 * \brief Embedded Flash service for SAM.
 *
 * Copyright (c) 2011-2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#define IFLASH0_SIZE             (0x40000u)
#define IFLASH0_PAGE_SIZE        (256u)
#define IFLASH0_LOCK_REGION_SIZE (16384u)
#define IFLASH0_NB_OF_PAGES      (1024u)
#define IFLASH1_SIZE             (0x40000u)
#define IFLASH1_PAGE_SIZE        (256u)
#define IFLASH1_LOCK_REGION_SIZE (16384u)
#define IFLASH1_NB_OF_PAGES      (1024u)
#define IFLASH0_ADDR   		 (0x00080000u)
#define IFLASH1_ADDR   		 (IFLASH0_ADDR+IFLASH0_SIZE)

#define IFLASH_PAGE_SIZE     	 IFLASH0_PAGE_SIZE
#define IFLASH_LOCK_REGION_SIZE  IFLASH0_LOCK_REGION_SIZE

/* Flash page buffer for alignment */
static uint32_t gs_ul_page_buffer[IFLASH_PAGE_SIZE / sizeof(uint32_t)];

typedef enum flash_rc {
	FLASH_RC_OK = 0,        //!< Operation OK
	FLASH_RC_YES = 0,       //!< Yes
	FLASH_RC_NO = 1,        //!< No
	FLASH_RC_ERROR = 0x10,  //!< General error
	FLASH_RC_INVALID,       //!< Invalid argument input
	FLASH_RC_NOT_SUPPORT = 0xFFFFFFFF    //!< Operation is not supported
} flash_rc_t;

uint32_t flash_write(uint32_t ul_address, const void *p_buffer, uint32_t ul_size, uint32_t ul_erase_flag);
uint32_t flash_lock(uint32_t ul_start, uint32_t ul_end, uint32_t *pul_actual_start, uint32_t *pul_actual_end);
uint32_t flash_unlock(uint32_t ul_start, uint32_t ul_end, uint32_t *pul_actual_start, uint32_t *pul_actual_end);
uint32_t flash_erase_all(uint32_t ul_address);
