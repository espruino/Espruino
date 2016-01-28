/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef FDS_CONFIG_H__
#define FDS_CONFIG_H__

 /**
 * @file fds_config.h
 *
 * @addtogroup flash_data_storage
 * @{
 * @brief   Configuration options for FDS.
 */

/**@brief   Configures the size of the internal queue. */
#define FDS_CMD_QUEUE_SIZE          (4)
 
/**@brief   Determines how many @ref fds_record_chunk_t structures can be buffered at any time. */
#define FDS_CHUNK_QUEUE_SIZE        (8)

/**@brief   Configures the maximum number of callbacks which can be registered. */
#define FDS_MAX_USERS               (3)

/**@brief   Configures the number of virtual flash pages to use.
 *
 * @details The total amount of flash memory that will be used by FDS amounts to:
 *          FDS_VIRTUAL_PAGES * FDS_VIRTUAL_PAGE_SIZE * 4 bytes.
 *          This defaults to:
 *          3 * 256  * 4 bytes = 3072  bytes on the nRF51 and
 *          3 * 1024 * 4 bytes = 12288 bytes on the nRF52.
 *
 * @note    Out of the total, one is reserved by the system for garbage collection, hence,
 *          two pages is the minimum: one page to store data and one page to be used by the system
 *          for garbage collection.
 */
#define FDS_VIRTUAL_PAGES           (3)

/**@brief   Configures the size of a virtual page of flash memory, expressed in number of words.
 *
 * @details Defaults to 1024 bytes on the nRF51 and 4096 bytes on the nRF52.
 *
 * @note    The size of a virtual page must be a multiple of the size of a physical page.
 */
#if   defined(NRF51)
    #define FDS_VIRTUAL_PAGE_SIZE   (256)
#elif defined(NRF52)
    #define FDS_VIRTUAL_PAGE_SIZE   (1024)
#endif

/** Page tags. */
#define FDS_PAGE_TAG_MAGIC          (0xDEADC0DE)
#define FDS_PAGE_TAG_SWAP           (0xF11E01FF)
#define FDS_PAGE_TAG_VALID          (0xF11E01FE)

/** @} */

#endif // FDS_CONFIG_H__
