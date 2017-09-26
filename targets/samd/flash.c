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

// Standard includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// My Includes
#include "targetlibs/samd/include/due_sam3x.init.h"
#include "jshardware.h"
#include "flash.h"
#include "../targetlibs/samd/sam/libsam/include/efc.h"

/**
 * \brief Compute the lock range associated with the given address range.
 *
 * \param ul_start Start address of lock range.
 * \param ul_end End address of lock range.
 * \param pul_actual_start Actual start address of lock range.
 * \param pul_actual_end Actual end address of lock range.
 */
static void compute_lock_range(uint32_t ul_start, uint32_t ul_end, uint32_t *pul_actual_start, uint32_t *pul_actual_end) {
	uint32_t ul_actual_start, ul_actual_end;

	ul_actual_start = ul_start - (ul_start % IFLASH_LOCK_REGION_SIZE);
	ul_actual_end = ul_end - (ul_end % IFLASH_LOCK_REGION_SIZE) + IFLASH_LOCK_REGION_SIZE - 1;

	if (pul_actual_start) {
		*pul_actual_start = ul_actual_start;
	}

	if (pul_actual_end) {
		*pul_actual_end = ul_actual_end;
	}
}

/**
 * \brief Compute the address of a flash by the given page and offset.
 *
 * \param p_efc Pointer to an EFC instance.
 * \param us_page Page number.
 * \param us_offset Byte offset inside page.
 * \param pul_addr Computed address (optional).
 */
static void compute_address(Efc *p_efc, uint16_t us_page, uint16_t us_offset, uint32_t *pul_addr)
{
	uint32_t ul_addr;

	/* Compute address */
	ul_addr = (p_efc == EFC0) ?
			IFLASH0_ADDR + us_page * IFLASH_PAGE_SIZE + us_offset :
			IFLASH1_ADDR + us_page * IFLASH_PAGE_SIZE + us_offset;

	/* Store result */
	if (pul_addr != NULL) {
		*pul_addr = ul_addr;
	}
}

/**
 * \brief Translate the given flash address to page and offset values.
 * \note pus_page and pus_offset must not be null in order to store the
 * corresponding values.
 *
 * \param pp_efc Pointer to an EFC pointer.
 * \param ul_addr Address to translate.
 * \param pus_page The first page accessed.
 * \param pus_offset Byte offset in the first page.
 */
static void translate_address(Efc **pp_efc, uint32_t ul_addr, uint16_t *pus_page, uint16_t *pus_offset) {
	Efc *p_efc;
	uint16_t us_page;
	uint16_t us_offset;

	if (ul_addr >= IFLASH1_ADDR) {
		p_efc = EFC1;
		us_page = (ul_addr - IFLASH1_ADDR) / IFLASH1_PAGE_SIZE;
		us_offset = (ul_addr - IFLASH1_ADDR) % IFLASH1_PAGE_SIZE;
	} else {
		p_efc = EFC0;
		us_page = (ul_addr - IFLASH0_ADDR) / IFLASH0_PAGE_SIZE;
		us_offset = (ul_addr - IFLASH0_ADDR) % IFLASH0_PAGE_SIZE;
	}

	/* Store values */
	if (pp_efc) {
		*pp_efc = p_efc;
	}

	if (pus_page) {
		*pus_page = us_page;
	}

	if (pus_offset) {
		*pus_offset = us_offset;
	}
}

/**
 * \brief Lock all the regions in the given address range. The actual lock
 * range is reported through two output parameters.
 *
 * \param ul_start Start address of lock range.
 * \param ul_end End address of lock range.
 * \param pul_actual_start Start address of the actual lock range (optional).
 * \param pul_actual_end End address of the actual lock range (optional).
 *
 * \return 0 if successful, otherwise returns an error code.
 */
uint32_t flash_lock(uint32_t ul_start, uint32_t ul_end, uint32_t *pul_actual_start, uint32_t *pul_actual_end) {
	Efc *p_efc;
	uint32_t ul_actual_start, ul_actual_end;
	uint16_t us_start_page, us_end_page;
	uint32_t ul_error;
	uint16_t us_num_pages_in_region = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;

	/* Compute actual lock range and store it */
	compute_lock_range(ul_start, ul_end, &ul_actual_start, &ul_actual_end);

	if (pul_actual_start != NULL) {
		*pul_actual_start = ul_actual_start;
	}

	if (pul_actual_end != NULL) {
		*pul_actual_end = ul_actual_end;
	}

	/* Compute page numbers */
	translate_address(&p_efc, ul_actual_start, &us_start_page, 0);
	translate_address(0, ul_actual_end, &us_end_page, 0);

	/* Lock all pages */
	while (us_start_page < us_end_page) {
		ul_error = efc_perform_command(p_efc, EFC_FCMD_SLB, us_start_page);

		if (ul_error) {
			return ul_error;
		}
		us_start_page += us_num_pages_in_region;
	}

	return FLASH_RC_OK;
}

/**
 * \brief Unlock all the regions in the given address range. The actual unlock
 * range is reported through two output parameters.
 *
 * \param ul_start Start address of unlock range.
 * \param ul_end End address of unlock range.
 * \param pul_actual_start Start address of the actual unlock range (optional).
 * \param pul_actual_end End address of the actual unlock range (optional).
 *
 * \return 0 if successful, otherwise returns an error code.
 */
uint32_t flash_unlock(uint32_t ul_start, uint32_t ul_end, uint32_t *pul_actual_start, uint32_t *pul_actual_end) {
	Efc *p_efc;
	uint32_t ul_actual_start, ul_actual_end;
	uint16_t us_start_page, us_end_page;
	uint32_t ul_error;
	uint16_t us_num_pages_in_region = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;

	/* Compute actual unlock range and store it */
	compute_lock_range(ul_start, ul_end, &ul_actual_start, &ul_actual_end);
	if (pul_actual_start != NULL) {
		*pul_actual_start = ul_actual_start;
	}
	if (pul_actual_end != NULL) {
		*pul_actual_end = ul_actual_end;
	}

	/* Compute page numbers */
	translate_address(&p_efc, ul_actual_start, &us_start_page, 0);
	translate_address(0, ul_actual_end, &us_end_page, 0);

	/* Unlock all pages */
	while (us_start_page < us_end_page) {
		ul_error = efc_perform_command(p_efc, EFC_FCMD_CLB,
				us_start_page);
		if (ul_error) {
			return ul_error;
		}
		us_start_page += us_num_pages_in_region;
	}

	return FLASH_RC_OK;
}

/**
 * \brief Write a data buffer on flash.
 *
 * \note This function works in polling mode, and thus only returns when the
 * data has been effectively written.
 * \note For dual bank flash, this function doesn't support cross write from
 * bank 0 to bank 1. In this case, flash_write must be called twice (ie for
 * each bank).
 *
 * \param ul_address Write address.
 * \param p_buffer Data buffer.
 * \param ul_size Size of data buffer in bytes.
 * \param ul_erase_flag Flag to set if erase first.
 *
 * \return 0 if successful, otherwise returns an error code.
 */
uint32_t flash_write(uint32_t ul_address, const void *p_buffer, uint32_t ul_size, uint32_t ul_erase_flag) {
	Efc *p_efc;
	uint32_t ul_fws_temp;
	uint16_t us_page;
	uint16_t us_offset;
	uint32_t writeSize;
	uint32_t ul_page_addr;
	uint16_t us_padding;
	uint32_t ul_error;
	uint32_t ul_idx;
	uint32_t *p_aligned_dest;
	uint8_t *puc_page_buffer = (uint8_t *) gs_ul_page_buffer;

	translate_address(&p_efc, ul_address, &us_page, &us_offset);

	/* According to the errata, set the wait state value to 6. */
	ul_fws_temp = efc_get_wait_state(p_efc);
	efc_set_wait_state(p_efc, 6);

	/* Write all pages */
	while (ul_size > 0) {
		/* Copy data in temporary buffer to avoid alignment problems. */
		writeSize = Min((uint32_t) IFLASH_PAGE_SIZE - us_offset, ul_size);
		compute_address(p_efc, us_page, 0, &ul_page_addr);
		us_padding = IFLASH_PAGE_SIZE - us_offset - writeSize;

		/* Pre-buffer data */
		memcpy(puc_page_buffer, (void *)ul_page_addr, us_offset);

		/* Buffer data */
		memcpy(puc_page_buffer + us_offset, p_buffer, writeSize);

		/* Post-buffer data */
		memcpy(puc_page_buffer + us_offset + writeSize,
				(void *)(ul_page_addr + us_offset + writeSize), us_padding);

		/* Write page.
		 * Writing 8-bit and 16-bit data is not allowed and may lead to
		 * unpredictable data corruption.
		 */
		p_aligned_dest = (uint32_t *) ul_page_addr;
		for (ul_idx = 0; ul_idx < (IFLASH_PAGE_SIZE / sizeof(uint32_t)); ++ul_idx) {
			*p_aligned_dest++ = gs_ul_page_buffer[ul_idx];
		}

		if (ul_erase_flag) {
			ul_error = efc_perform_command(p_efc, EFC_FCMD_EWP, us_page);
		} else {
			ul_error = efc_perform_command(p_efc, EFC_FCMD_WP, us_page);
		}

		if (ul_error) {
			return ul_error;
		}

		/* Progression */
		p_buffer = (void *)((uint32_t) p_buffer + writeSize);
		ul_size -= writeSize;
		us_page++;
		us_offset = 0;
	}

	/* According to the errata, restore the wait state value. */
	efc_set_wait_state(p_efc, ul_fws_temp);

	return FLASH_RC_OK;
}

/**
 * \brief Erase the entire flash.
 *
 * \note Only the flash bank including ul_address will be erased. If there are
 * two flash banks, we need to call this function twice with each bank start
 * address.
 *
 * \param ul_address  Flash bank start address.
 *
 * \return 0 if successful; otherwise returns an error code.
 */
uint32_t flash_erase_all(uint32_t ul_address)
{
	Efc *p_efc;

	translate_address(&p_efc, ul_address, NULL, NULL);

	if (EFC_RC_OK != efc_perform_command(p_efc, EFC_FCMD_EA, 0)) {
		return FLASH_RC_ERROR;
	}

	return FLASH_RC_OK;
}
