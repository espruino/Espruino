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

#ifndef FDS_INTERNAL_DEFS_H__
#define FDS_INTERNAL_DEFS_H__

#include <stdint.h>
#include <stdbool.h>
#include "fds_config.h"

#if defined (FDS_CRITICAL_SECTION_ENABLED)
    #include "nrf_soc.h"
#endif

#define FDS_PAGE_TAG_SIZE       (2) /**< Page tag size, in 4 byte words. */

#define COMMAND_EXECUTING       (NRF_SUCCESS)
#define COMMAND_COMPLETED       (0x1234)

#define FDS_MAGIC_HWORD         (0xF11E)
#define FDS_MAGIC_WORD          (0x15ABE11A)
#define FDS_ERASED_WORD         (0xFFFFFFFF)

#define FDS_PAGE_ID_UNKNOWN     (0xFFFF)

#define FDS_OFFSET_TL           (0) /**< Offset of TL from the record base address, in 4 byte words. */
#define FDS_OFFSET_IC           (1) /**< Offset of IC from the record base address, in 4 byte words. */
#define FDS_OFFSET_ID           (2) /**< Offset of ID from the record base address, in 4 byte words. */
#define FDS_OFFSET_DATA         (3) /**< Offset of the data (chunks) from the record base address, in 4 byte words. */

#define FDS_HEADER_SIZE_TL      (1) /**< Size of the TL part of the header, in 4 byte words. */
#define FDS_HEADER_SIZE_ID      (1) /**< Size of the IC part of the header, in 4 byte words. */
#define FDS_HEADER_SIZE_IC      (1) /**< Size of the IC part of the header, in 4 byte words. */
#define FDS_HEADER_SIZE         (3) /**< Size of the whole header, in 4 byte words. */


/**@brief   The size of a physical page, in 4 byte words. */
#if   defined(NRF51)
    #define FDS_PHY_PAGE_SIZE   (256)
#elif defined(NRF52)
    #define FDS_PHY_PAGE_SIZE   (1024)
#endif

/**@brief   The number of physical pages to be used. */
#define FDS_PHY_PAGES               ((FDS_VIRTUAL_PAGES * FDS_VIRTUAL_PAGE_SIZE) / FDS_PHY_PAGE_SIZE)

/**@brief   Defines the size of a virtual page, in number of physical pages. */
#define FDS_PHY_PAGES_IN_VPAGE      (FDS_VIRTUAL_PAGE_SIZE / FDS_PHY_PAGE_SIZE)

/**@brief   The number of pages available to store data; which is the total minus the swap. */
#define FDS_MAX_PAGES               (FDS_VIRTUAL_PAGES - 1)

 /**@brief  Just a shorter name for the size, in words, of a virtual page. */
#define FDS_PAGE_SIZE               (FDS_VIRTUAL_PAGE_SIZE)


#if (FDS_VIRTUAL_PAGE_SIZE % FDS_PHY_PAGE_SIZE != 0)
    #error "FDS_VIRTUAL_PAGE_SIZE must be a multiple of the size of a physical page."
#endif

#if (FDS_VIRTUAL_PAGES < 2)
    #error "FDS requires at least two virtual pages."
#endif

/**@brief   Macros to enable and disable application interrupts. */
#if defined (FDS_CRITICAL_SECTION_ENABLED)
    static uint8_t m_nested_critical;

    #define CRITICAL_SECTION_ENTER()    sd_nvic_critical_region_enter(&m_nested_critical)
    #define CRITICAL_SECTION_EXIT()     sd_nvic_critical_region_exit ( m_nested_critical)
#else
    #define CRITICAL_SECTION_ENTER()
    #define CRITICAL_SECTION_EXIT()
#endif


/**@brief   FDS internal status flags. */
typedef enum
{
    FDS_FLAG_INITIALIZING       = (1 << 0),  /**< The module is initializing. */
    FDS_FLAG_INITIALIZED        = (1 << 1),  /**< The module is initialized. */
    FDS_FLAG_PROCESSING         = (1 << 2),  /**< The queue is being processed. */
    FDS_FLAG_VERIFY_CRC         = (1 << 3),  /**< CRC verification upon writes is enabled. */
} fds_flags_t;


/**@brief   Page types. */
typedef enum
{
    FDS_PAGE_UNDEFINED, /**< Undefined page type. */
    FDS_PAGE_ERASED,    /**< Page is erased. */
    FDS_PAGE_VALID,     /**< Page is ready for storage. */
    FDS_PAGE_SWAP,      /**< Page is reserved for garbage collection. */
} fds_page_type_t;


typedef enum
{
    FDS_CMD_INIT,       /**< Module initialization commnad. Used in @ref fds_init */
    FDS_CMD_WRITE,      /**< Write command. Used in @ref fds_write and @ref fds_write_reserved. */
    FDS_CMD_UPDATE,     /**< Update command. Used in @ref fds_update. */
    FDS_CMD_CLEAR,      /**< Clear record command. Used in @ref fds_clear and @ref fds_update. */
    FDS_CMD_CLEAR_INST, /**< Clear instance command. Used in @ref fds_clear_by_instance. */
    FDS_CMD_GC          /**< Garbage collection. Used in @ref fds_gc. */
} fds_cmd_id_t;


typedef enum
{
    FDS_OP_WRITE_TL,        /**< Write the type and length. */
    FDS_OP_WRITE_ID,        /**< Write the record ID. */
    FDS_OP_WRITE_CHUNK,     /**< Write the record data.  */
    FDS_OP_WRITE_IC,        /**< Write the instance and CRC. */
    FDS_OP_CLEAR_TL,
    FDS_OP_CLEAR_INSTANCE,
    FDS_OP_DONE,
} fds_opcode_t;


typedef struct
{
    fds_page_type_t         page_type;       /**< The page type. */
    uint32_t        const * p_addr;          /**< The address of the page. */
    uint16_t                write_offset;    /**< The page write offset, in 4 bytes words. */
    uint16_t                words_reserved;  /**< The amount of words reserved by fds_write_reserve(). */
    uint16_t                records_open;    /**< The number of records opened using fds_open(). */
    bool                    can_gc;          /**< Indicates that there are some records which have been cleared. */
} fds_page_t;


typedef struct
{
    uint32_t const * p_addr;
    uint16_t         write_offset;
} fds_swap_page_t;


typedef enum
{
    FDS_INIT_ERASE_SWAP,
    FDS_INIT_TAG_SWAP,
    FDS_INIT_PROMOTE_SWAP,
    FDS_INIT_TAG_VALID,
} fds_init_op_t;


typedef struct
{
    fds_record_header_t record_header;
    fds_cmd_id_t        id;                 /**< The ID of the command. */
    fds_opcode_t        op_code;
    uint16_t            page;               /**< The page where we reserved the flash space for this command. */
    uint16_t            chunk_offset;       /**< Offset used for writing the record chunks(s), in 4 byte words. */
    uint8_t             num_chunks;         /**< Number of chunks to be written. */
    fds_init_op_t       init_op;
    fds_record_id_t     record_to_clear;
} fds_cmd_t;


typedef struct
{
    uint8_t     rp;                         /**< The index of the command being executed. */
    uint8_t     count;                      /**< Number of elements in the queue. */
    fds_cmd_t   cmd[FDS_CMD_QUEUE_SIZE];    /**< Array to maintain flash access operation details. */
} fds_cmd_queue_t;


typedef struct
{
    uint8_t             rp;
    uint8_t             count;
    fds_record_chunk_t  chunk[FDS_CHUNK_QUEUE_SIZE];
} fds_chunk_queue_t;


typedef enum
{
    GC_BEGIN,
    GC_NEXT_PAGE,
    GC_COPY_RECORD,
    GC_ERASE_SWAP,
    GC_TAG_VALID,
    GC_TAG_SWAP
} fds_gc_state_t;


enum
{
    PAGE_ERASED = 0x1,
    PAGE_VALID  = 0x2,
    SWAP_EMPTY  = 0x4,
    SWAP_DIRTY  = 0x8,
};


typedef enum
{
    NO_PAGES,
    FRESH_INSTALL     = (PAGE_ERASED),
    TAG_SWAP          = (PAGE_ERASED | PAGE_VALID),
    /** Tag all erased pages as valid. */
    TAG_VALID         = (PAGE_ERASED | SWAP_EMPTY),
    TAG_VALID_INST    = (PAGE_ERASED | PAGE_VALID | SWAP_EMPTY),
    /** Tag the swap as valid; one erased page as swap, and any remaining pages as valid. */
    PROMOTE_SWAP      = (PAGE_ERASED | SWAP_DIRTY),
    PROMOTE_SWAP_INST = (PAGE_ERASED | PAGE_VALID | SWAP_DIRTY),
    /** Discard the swap. */
    DISCARD_SWAP      = (PAGE_VALID  | SWAP_DIRTY),
    /** Do nothing. */
    ALREADY_INSTALLED = (PAGE_VALID  | SWAP_EMPTY),

} fds_init_opts_t;


typedef struct
{
    fds_gc_state_t   state;
    uint16_t         cur_page;
    uint32_t const * p_record_src;
    uint16_t         runs;
    bool             do_gc_page[FDS_MAX_PAGES];
} fds_gc_data_t;


#endif // FDS_INTERNAL_DEFS_H__
