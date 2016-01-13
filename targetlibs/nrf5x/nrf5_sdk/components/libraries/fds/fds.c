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

#include "fds.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "fds_config.h"
#include "fds_internal_defs.h"
#include "fstorage.h"
#include "nrf_error.h"

#if defined(FDS_CRC_ENABLED)
    #include "crc16.h"
#endif


static void fs_callback(fs_evt_t evt, uint32_t result);

// Our fstorage configuration.
FS_SECTION_VARS_ADD(fs_config_t fs_config) = {.cb = fs_callback, .num_pages = FDS_PHY_PAGES};


static uint32_t const fds_page_tag_swap[]  = {FDS_PAGE_TAG_MAGIC, FDS_PAGE_TAG_SWAP};
static uint32_t const fds_page_tag_valid[] = {FDS_PAGE_TAG_MAGIC, FDS_PAGE_TAG_VALID};

static fds_tl_t const m_fds_tl_invalid     = {.type = FDS_TYPE_ID_INVALID, .length_words = 0xFFFF};

/**@brief Internal status flags. */
static uint8_t              m_flags;

static uint8_t              m_users;
static fds_cb_t             m_cb_table[FDS_MAX_USERS];

/**@brief The last record ID. Setup page by page_scan() during pages_init(). */
static fds_record_id_t      m_last_rec_id;

/**@brief The internal queues. */
static fds_cmd_queue_t      m_cmd_queue;
static fds_chunk_queue_t    m_chunk_queue;

static fds_page_t           m_pages[FDS_MAX_PAGES];
static fds_swap_page_t      m_swap_page;

static fds_gc_data_t        m_gc;


static void app_notify(fds_evt_t const * const evt)
{
    for (uint8_t user = 0; user < FDS_MAX_USERS; user++)
    {
        if (m_cb_table[user] != NULL)
        {
            m_cb_table[user](evt);
        }
    }
}


static void flag_set(fds_flags_t flag)
{
    CRITICAL_SECTION_ENTER();
    m_flags |= flag;
    CRITICAL_SECTION_EXIT();
}


static void flag_clear(fds_flags_t flag)
{
    CRITICAL_SECTION_ENTER();
    m_flags &= ~(flag);
    CRITICAL_SECTION_EXIT();
}


static bool flag_is_set(fds_flags_t flag)
{
    bool ret;
    CRITICAL_SECTION_ENTER();
    ret = (m_flags & flag);
    CRITICAL_SECTION_EXIT();
    return ret;
}


/**@brief Function to check if a header has valid information. */
static bool header_is_valid(fds_header_t const * const p_header)
{
    return ((p_header->tl.type     != FDS_TYPE_ID_INVALID) &&
            (p_header->ic.instance != FDS_INSTANCE_ID_INVALID));
}


static bool address_within_page_bounds(uint32_t const * const p_addr)
{
    return (p_addr >= fs_config.p_start_addr) &&
           (p_addr <= fs_config.p_end_addr)   &&
           (is_word_aligned(p_addr));
}


static bool page_id_within_bounds(uint32_t page)
{
    return (page < FDS_MAX_PAGES) && (page != FDS_PAGE_ID_UNKNOWN);
}


/**@brief Internal function to identify the page type. */
static fds_page_type_t page_identify(uint32_t const * const p_page_addr)
{
    if (p_page_addr[0] != FDS_PAGE_TAG_MAGIC)
    {
        return FDS_PAGE_UNDEFINED;
    }

    switch (p_page_addr[1])
    {
        case FDS_PAGE_TAG_SWAP:
            return FDS_PAGE_SWAP;

        case FDS_PAGE_TAG_VALID:
            return FDS_PAGE_VALID;

        default:
            return FDS_PAGE_UNDEFINED;
    }
}


/**@note Only works after the module has initialized. */
static bool page_has_space(uint16_t page, uint16_t length_words)
{
    CRITICAL_SECTION_ENTER();
    length_words += m_pages[page].write_offset;
    length_words += m_pages[page].words_reserved;
    CRITICAL_SECTION_EXIT();

    return (length_words < FDS_PAGE_SIZE);
}


/**@brief This function scans a page to determine how many words have been written to it.
 *        This information is used to set the page write offset during initialization (mount).
 *        Additionally, this function will update the last known record ID as it proceeds.
 */
static void page_scan(uint32_t const * p_addr, uint16_t * words_written, bool * can_gc)
{
    bool invalid_header_found = false;

    uint32_t const * const p_end_addr = p_addr + FDS_PAGE_SIZE;

    p_addr         += FDS_PAGE_TAG_SIZE;
    *words_written  = FDS_PAGE_TAG_SIZE;

    while ((p_addr < p_end_addr) && (*p_addr != FDS_ERASED_WORD))
    {
        fds_header_t const * const p_header = (fds_header_t*)p_addr;

        if (!invalid_header_found)
        {
            if (!header_is_valid(p_header))
            {
                invalid_header_found = true;
            }
        }

        /** Note: If an header has an invalid type (0x0000) or
         *  a missing instance (0xFFFF) then we WANT to skip it. */

         // Update the last known record ID.
         if (p_header->id > m_last_rec_id)
         {
            m_last_rec_id = p_header->id;
         }

         // Jump to the next record.
         p_addr         += (FDS_HEADER_SIZE + p_header->tl.length_words);
         *words_written += (FDS_HEADER_SIZE + p_header->tl.length_words);
    }

    if (can_gc != NULL)
    {
        *can_gc = invalid_header_found;
    }
}


static bool page_is_empty(uint32_t const * const p_page_addr)
{
    for (uint32_t i = 0; i < FDS_PAGE_SIZE; i++)
    {
        if (*(p_page_addr + i) != FDS_ERASED_WORD)
        {
            return false;
        }
    }

    return true;
}


static uint32_t record_id_new()
{
    return ++m_last_rec_id;
}


static void cmd_to_evt(fds_cmd_t const * const p_cmd, fds_evt_t * const evt)
{
    evt->id = (fds_evt_id_t)p_cmd->id;

    switch (p_cmd->id)
    {
        case FDS_CMD_INIT:
            break;

        case FDS_CMD_WRITE:
        case FDS_CMD_UPDATE:
            evt->write.record_id           = p_cmd->record_header.id;
            evt->write.record_key.type     = p_cmd->record_header.tl.type;
            evt->write.record_key.instance = p_cmd->record_header.ic.instance;
            break;

        case FDS_CMD_CLEAR:
            evt->clear.record_id = p_cmd->record_to_clear;
            evt->clear.record_key.type     = p_cmd->record_header.tl.type;
            evt->clear.record_key.instance = p_cmd->record_header.ic.instance;
            break;

        case FDS_CMD_CLEAR_INST:
            evt->clear.record_key.type     = FDS_TYPE_ID_INVALID;
            evt->clear.record_key.instance = p_cmd->record_header.ic.instance;
            break;

        case FDS_CMD_GC:
            break;

        default:
            break;
    }
}

/**@brief Tags a page as swap, i.e., reserved for GC. */
static ret_code_t page_tag_write_swap()
{
    return fs_store(&fs_config, m_swap_page.p_addr, fds_page_tag_swap, FDS_PAGE_TAG_SIZE);
}


/**@brief Tags a page as valid, i.e, ready for storage. */
static ret_code_t page_tag_write_valid(uint32_t const * const p_page_addr)
{
    return fs_store(&fs_config, p_page_addr, fds_page_tag_valid, FDS_PAGE_TAG_SIZE);
}


/**@brief Given a page and a record, finds the next valid record on that page. */
static bool find_next_valid(uint16_t page, uint32_t const ** p_record)
{
    uint32_t const * p_next_rec = (*p_record);

    if (p_next_rec == NULL)
    {
        // This if the first invocation on this page, start from the beginning.
        p_next_rec = m_pages[page].p_addr + FDS_PAGE_TAG_SIZE;
    }
    else
    {
        // Jump to the next record.
        p_next_rec += (FDS_HEADER_SIZE + ((fds_header_t*)(*p_record))->tl.length_words);
    }

    // Scan until we find a valid record or until the end of the page.
    while ((p_next_rec < (m_pages[page].p_addr + FDS_PAGE_SIZE)) &&
           (*p_next_rec != FDS_ERASED_WORD)) // Did we jump to an erased word?
    {
        fds_header_t const * const p_header = (fds_header_t*)p_next_rec;

        if (header_is_valid(p_header))
        {
            // Bingo!
            *p_record = p_next_rec;
            return true;
        }
        else
        {
            // The item is not valid, jump to the next.
            p_next_rec += (FDS_HEADER_SIZE + (p_header->tl.length_words));
        }
    }

    // No more valid records on this page.
    return false;
}


// README: p_desc must not be NULL.
static bool find_record(fds_record_desc_t * const p_desc)
{
    uint32_t const * p_record       = NULL;
    bool             scan_all_pages = false;

    if ((p_desc->ptr_magic == FDS_MAGIC_HWORD) &&
        (p_desc->gc_magic  == m_gc.runs))
    {
        // No need to find the record again.
        return true;
    }

    /** The pointer in the descriptor is not initialized, or GC
     *  has been run since the last time it was retrieved.
     *  We must find the record again. */

    if (!page_id_within_bounds(p_desc->page))
    {
        // If the page is out of bounds (or unknown), scan all pages.
        p_desc->page = 0;
        scan_all_pages = true;
    }

    do
    {
        while (find_next_valid(p_desc->page, &p_record))
        {
            fds_header_t const * const p_header = (fds_header_t*)p_record;

            if (p_header->id == p_desc->record_id)
            {
                // Update the pointer in the descriptor.
                p_desc->p_rec     = p_record;
                p_desc->ptr_magic = FDS_MAGIC_HWORD;
                p_desc->gc_magic  = m_gc.runs;

                return true;
            }
        }

        p_record = NULL;
    }
    while (scan_all_pages ? p_desc->page++ < FDS_MAX_PAGES : 0);

    return false;
}


static ret_code_t find_record_by_key(uint16_t          const * const p_type,
                                     uint16_t          const * const p_inst,
                                     fds_record_desc_t       * const p_desc,
                                     fds_find_token_t        * const p_token)
{
    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if (p_desc == NULL || p_token == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    // Here we distinguish between the first invocation and the and the others.
    if ((p_token->magic != FDS_MAGIC_WORD) ||
        !address_within_page_bounds(p_token->p_addr)) // Is the address is really okay?
    {
        // Initialize the token.
        p_token->magic  = FDS_MAGIC_WORD;
        p_token->page   = 0;
        p_token->p_addr = NULL;
    }

    // Begin (or resume) searching for a record.
    for (; p_token->page < FDS_MAX_PAGES; p_token->page++)
    {
        if (m_pages[p_token->page].page_type != FDS_PAGE_VALID)
        {
            // Skip this page.
            continue;
        }

        while (find_next_valid(p_token->page, &p_token->p_addr))
        {
            fds_header_t const * const p_header = (fds_header_t*)p_token->p_addr;

            // A valid record was found, check its header for a match.
            bool item_match = false;

            if (p_type != NULL)
            {
                if (p_header->tl.type == *p_type)
                {
                    item_match = true;
                }
            }

            if (p_inst != NULL)
            {
                if (p_header->ic.instance == *p_inst)
                {
                    item_match = (p_type == NULL) ? true : item_match && true;
                }
                else
                {
                    item_match = false;
                }
            }

            if (item_match)
            {
                // We found the record! Update the descriptor.
                p_desc->record_id = p_header->id;
                p_desc->page      = p_token->page;
                p_desc->p_rec     = p_token->p_addr;
                p_desc->ptr_magic = FDS_MAGIC_HWORD;
                p_desc->gc_magic  = m_gc.runs;

                return FDS_SUCCESS;
            }
        }

        /** We have scanned an entire page. Set the address in the token to NULL
         *  so that it will be set again on the next iteration. */
        p_token->p_addr = NULL;
    }

    /** If we couldn't find the record, zero the token structure
     *  so that it can be reused. */
    p_token->magic = 0x00;

    return FDS_ERR_NOT_FOUND;
}


static void gc_init()
{
    m_gc.runs++;

    // Reset the state.
    m_gc.cur_page      = 0;
    m_gc.p_record_src  = NULL;

    /** Setup which pages to GC. Delay checking for open records and the can_gc flag,
     *  as other operations might change those flags. */
    for (uint16_t i = 0; i < FDS_MAX_PAGES; i++)
    {
        m_gc.do_gc_page[i] = (m_pages[i].page_type == FDS_PAGE_VALID);
    }
}


static bool gc_get_next_page(uint16_t * const next_page)
{
    for (uint16_t i = 0; i < FDS_MAX_PAGES; i++)
    {
        if (m_gc.do_gc_page[i])
        {
            uint16_t records_open;
            bool     can_gc;

            CRITICAL_SECTION_ENTER();
            records_open = m_pages[i].records_open;
            can_gc       = m_pages[i].can_gc;
            CRITICAL_SECTION_EXIT();

            // Do not attempt to GC this page anymore.
            m_gc.do_gc_page[i] = false;

            /** Only GC pages with no open records and with some records
             *  which have been cleared. */
            if ((records_open == 0) && (can_gc == true))
            {
                *next_page = i;
                return true;
            }
        }
    }

    return false;
}


static ret_code_t gc_erase_swap()
{
    m_gc.state = GC_TAG_SWAP;

    m_swap_page.write_offset = FDS_PAGE_TAG_SIZE;

    return fs_erase(&fs_config, m_swap_page.p_addr, FDS_PHY_PAGES_IN_VPAGE);
}


static ret_code_t gc_swap()
{
    uint32_t       ret;
    uint16_t const gc = m_gc.cur_page;

    CRITICAL_SECTION_ENTER();
    uint16_t const records_open = m_pages[gc].records_open;
    CRITICAL_SECTION_EXIT();

    if (records_open == 0)
    {
        /** The page being GC will be the new swap, and the current swap
         *  will be used as a valid page. */
        uint32_t const * const p_addr = m_swap_page.p_addr;

        m_swap_page.p_addr = m_pages[gc].p_addr;
        m_pages[gc].p_addr = p_addr;

        m_pages[gc].write_offset = m_swap_page.write_offset;

        // Erase the new swap page.
        ret = fs_erase(&fs_config, m_swap_page.p_addr, FDS_PHY_PAGES_IN_VPAGE);

        m_gc.state = GC_TAG_VALID;
    }
    else
    {
        /** If there are open records, we can't GC the page anymore.
         *  Discard the swap and try to GC another page. */
        ret = gc_erase_swap();
    }

    return ret;
}


static ret_code_t gc_copy_record()
{
    ret_code_t ret;

    // Find the next valid record to copy.
    if (find_next_valid(m_gc.cur_page, &m_gc.p_record_src))
    {
        fds_header_t const * const p_header   = (fds_header_t*)m_gc.p_record_src;
        uint16_t     const         record_len = FDS_HEADER_SIZE + p_header->tl.length_words;

        uint32_t     const * const p_dest     = m_swap_page.p_addr +
                                                m_swap_page.write_offset;

        m_gc.state = GC_COPY_RECORD;

        /** Copy the record to swap; it is guaranteed to fit in the destination page,
         *  so we don't need to check its size. This will either succeed or timeout. */
        ret = fs_store(&fs_config, p_dest, (uint32_t*)m_gc.p_record_src, record_len);

        // Update the swap page write offset, even upon error.
        CRITICAL_SECTION_ENTER();
        m_swap_page.write_offset += record_len;
        CRITICAL_SECTION_EXIT();
    }
    else
    {
        /** No more records left to copy on this page.
         *  Execute the swap. */
        ret = gc_swap();
    }

    return ret;
}


static ret_code_t gc_tag_page_valid()
{
    m_gc.state = GC_TAG_SWAP;

    return page_tag_write_valid(m_pages[m_gc.cur_page].p_addr);
}


static ret_code_t gc_tag_swap()
{
    m_gc.state        = GC_NEXT_PAGE;
    m_gc.p_record_src = NULL;

    return page_tag_write_swap();
}


static ret_code_t gc_next_page()
{
    if (!gc_get_next_page(&m_gc.cur_page))
    {
        /** No pages left to GC; GC has terminated.
         *  The state will be reset by a new call to fds_gc(). */
        return COMMAND_COMPLETED;
    }

    return gc_copy_record();
}


static ret_code_t gc_execute(uint32_t last_ret)
{
    ret_code_t ret = FDS_SUCCESS;

    if (last_ret != NRF_SUCCESS)
    {
        return FDS_ERR_OPERATION_TIMEOUT;
    }

    switch (m_gc.state)
    {
        case GC_BEGIN:
            gc_init();
            // Fall through.

        case GC_NEXT_PAGE:
            ret = gc_next_page();
            break;

        case GC_COPY_RECORD:
            ret = gc_copy_record();
            break;

        case GC_ERASE_SWAP:
            ret = gc_erase_swap();
            break;

        case GC_TAG_VALID:
            ret = gc_tag_page_valid();
            break;

        case GC_TAG_SWAP:
            ret = gc_tag_swap();
            break;

        default:
            // Should not happen.
            break;
    }

    if (ret == COMMAND_EXECUTING || ret == COMMAND_COMPLETED)
    {
        return ret;
    }
    else
    {
        return FDS_ERR_BUSY;
    }
}


void chunk_queue_next(fds_record_chunk_t ** pp_chunk)
{
    if ((*pp_chunk) != &m_chunk_queue.chunk[FDS_CHUNK_QUEUE_SIZE - 1])
    {
        (*pp_chunk)++;
        return;
    }

    *pp_chunk = &m_chunk_queue.chunk[0];
}


/**@brief Advances one position in the command queue. Returns true if the queue is not empty. */
static bool cmd_queue_advance(void)
{
    // Reset the current element.
    memset(&m_cmd_queue.cmd[m_cmd_queue.rp], 0, sizeof(fds_cmd_t));

    CRITICAL_SECTION_ENTER();
    if (m_cmd_queue.count != 0)
    {
        // Advance in the queue, wrapping around if necessary.
        m_cmd_queue.rp = (m_cmd_queue.rp + 1) % FDS_CMD_QUEUE_SIZE;
        m_cmd_queue.count--;
    }
    CRITICAL_SECTION_EXIT();

    return (m_cmd_queue.count != 0);
}


/**@brief Returns the current chunk, and advances to the next in the queue. */
static bool chunk_queue_get_and_advance(fds_record_chunk_t ** pp_chunk)
{
    bool chunk_popped = false;

    CRITICAL_SECTION_ENTER();
    if (m_chunk_queue.count != 0)
    {
        // Point to the current chunk and advance the queue.
        *pp_chunk = &m_chunk_queue.chunk[m_chunk_queue.rp];

        m_chunk_queue.rp = (m_chunk_queue.rp + 1) % FDS_CHUNK_QUEUE_SIZE;
        m_chunk_queue.count--;

        chunk_popped = true;
    }
    CRITICAL_SECTION_EXIT();

    return chunk_popped;
}


static bool chunk_queue_skip(uint8_t num_op)
{
    bool chunk_skipped = false;

    CRITICAL_SECTION_ENTER();
    if (num_op <= m_chunk_queue.count)
    {
        m_chunk_queue.count -= num_op;
        chunk_skipped = true;
    }
    CRITICAL_SECTION_EXIT();

    return chunk_skipped;
}


/**@brief Reserves resources on both queues. */
static bool queue_reserve(fds_cmd_t          ** pp_cmd,
                          uint8_t               num_chunks,
                          fds_record_chunk_t ** pp_chunk)
{
    bool    ret;
    uint8_t idx;

    CRITICAL_SECTION_ENTER();

    // Ensure there is enough space in the queues.
    if ((m_cmd_queue.count   < FDS_CMD_QUEUE_SIZE - 1) &&
        (m_chunk_queue.count < FDS_CHUNK_QUEUE_SIZE - num_chunks))
    {
        // Find the write position in the commands queue.
        idx = (m_cmd_queue.count + m_cmd_queue.rp);
        idx = idx % FDS_CMD_QUEUE_SIZE;

        *pp_cmd = &m_cmd_queue.cmd[idx];

        m_cmd_queue.count++;

        /* If no chunks are associated with the command, such as is the case
         * for initialization and GC, pp_chunk can be NULL. */
        if (num_chunks != 0)
        {
            idx = (m_chunk_queue.count + m_chunk_queue.rp);
            idx = idx % FDS_CHUNK_QUEUE_SIZE;

            *pp_chunk = &m_chunk_queue.chunk[idx];

            m_chunk_queue.count += num_chunks;
        }

        ret = true;
    }
    else
    {
        ret = false;
    }

    CRITICAL_SECTION_EXIT();

    return ret;
}


static fds_init_opts_t pages_init()
{
    fds_init_opts_t  ret = NO_PAGES;

    // The index in m_pages[] where to store valid and erased pages.
    uint16_t page = 0;

    for (uint16_t i = 0; i < FDS_VIRTUAL_PAGES; i++)
    {
        uint32_t        const * const p_page_addr = fs_config.p_start_addr + (i * FDS_PAGE_SIZE);
        fds_page_type_t const         page_type   = page_identify(p_page_addr);

        switch (page_type)
        {
            case FDS_PAGE_UNDEFINED:
            {
                if (page_is_empty(p_page_addr))
                {
                    ret |= PAGE_ERASED;

                    if (m_swap_page.p_addr != NULL)
                    {
                        m_pages[page].page_type    = FDS_PAGE_ERASED;
                        m_pages[page].p_addr       = p_page_addr;
                        m_pages[page].write_offset = FDS_PAGE_TAG_SIZE;

                        // This is a potential candidate for a new swap page.
                        m_gc.cur_page = page;

                        page++;
                    }
                    else
                    {
                        m_swap_page.p_addr       = p_page_addr;
                        m_swap_page.write_offset = FDS_PAGE_TAG_SIZE;
                    }
                }
            }
            break;

            case FDS_PAGE_VALID:
            {
                m_pages[page].page_type = FDS_PAGE_VALID;
                m_pages[page].p_addr    = p_page_addr;
                page_scan(p_page_addr, &m_pages[page].write_offset, &m_pages[page].can_gc);

                ret |= PAGE_VALID;
                page++;
            }
            break;

            case FDS_PAGE_SWAP:
            {
                m_swap_page.p_addr = p_page_addr;
                /** We can only decide at a later moment if this offset is to be discarded or not.
                 *  If the swap is promoted, the offset should be kept, otherwise,
                 *  it should be set to FDS_PAGE_TAG_SIZE. */
                page_scan(p_page_addr, &m_swap_page.write_offset, NULL);

                ret |= (m_swap_page.write_offset == FDS_PAGE_TAG_SIZE) ?
                        SWAP_EMPTY : SWAP_DIRTY;
            }
            break;

            default:
                // Shouldn't happen.
                break;
        }
    }

    return ret;
}


// Note: Adds FDS_HEADER_SIZE automatically.
static ret_code_t write_space_reserve(uint16_t length_words, uint16_t * p_page)
{
    bool     space_reserved  = false;
    uint16_t total_len_words = length_words + FDS_HEADER_SIZE;

    if (total_len_words >= FDS_PAGE_SIZE - FDS_PAGE_TAG_SIZE)
    {
        return FDS_ERR_RECORD_TOO_LARGE;
    }

    for (uint16_t page = 0; page < FDS_MAX_PAGES; page++)
    {
        if ((m_pages[page].page_type == FDS_PAGE_VALID) &&
            (page_has_space(page, total_len_words)))
        {
            space_reserved = true;
            *p_page        = page

            CRITICAL_SECTION_ENTER();
            m_pages[page].words_reserved += total_len_words;
            CRITICAL_SECTION_EXIT();

            break;
        }
    }

    return space_reserved ? FDS_SUCCESS : FDS_ERR_NO_SPACE_IN_FLASH;
}


static void write_space_free(uint16_t length_words, uint16_t page)
{
    CRITICAL_SECTION_ENTER();
    m_pages[page].words_reserved -= length_words;
    CRITICAL_SECTION_EXIT();
}


static bool chunk_is_aligned(fds_record_chunk_t const * const p_chunk, uint32_t num_parts)
{
    for (uint32_t i = 0; i < num_parts; i++)
    {
        // Check word alignment.
        if ((uint32_t)p_chunk[i].p_data & 0x3)
        {
            return false;
        }
    }

    return true;
}


static ret_code_t write_tl(fds_tl_t * tl, uint32_t * const p_addr)
{
    ret_code_t ret;
    ret = fs_store(&fs_config, p_addr + FDS_OFFSET_TL,
                  (uint32_t*)tl, FDS_HEADER_SIZE_TL /*Words*/);

    return ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;
}


static ret_code_t clear_tl(fds_cmd_t * const p_cmd)
{
    ret_code_t        ret;
    fds_record_desc_t desc;

    memset(&desc, 0x00, sizeof(fds_record_desc_t));

    desc.record_id = p_cmd->record_to_clear;
    // Assume we don't have a page.
    desc.page      = FDS_PAGE_ID_UNKNOWN;

    if (find_record(&desc))
    {
        // Copy the record key, so that it may be returned by the callback.
        p_cmd->record_header.tl.type     = ((fds_header_t*)desc.p_rec)->tl.type;
        p_cmd->record_header.ic.instance = ((fds_header_t*)desc.p_rec)->ic.instance;

        ret = fs_store(&fs_config, desc.p_rec, (uint32_t*)&m_fds_tl_invalid, FDS_HEADER_SIZE_TL);
        ret = ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;

        // This page can now be garbage collected.
        m_pages[desc.page].can_gc = true;
    }
    else
    {
        // The record never existed, or it is already cleared.
        ret = FDS_ERR_NOT_FOUND;
    }

    return ret;
}


static ret_code_t clear_inst(fds_cmd_t * const p_cmd)
{
    ret_code_t        ret;
    fds_record_desc_t desc = {0};

    // This must persist across calls.
    static fds_find_token_t tok;

    ret = find_record_by_key(NULL, &p_cmd->record_header.ic.instance, &desc, &tok);
    if (ret == FDS_ERR_NOT_FOUND)
    {
        // Zero the token, so that we may reuse it.
        memset(&tok, 0, sizeof(fds_find_token_t));
    }
    else
    {
        // A record was found: invalidate it.
        ret = fs_store(&fs_config, desc.p_rec, (uint32_t*)&m_fds_tl_invalid, FDS_HEADER_SIZE_TL);
        ret = ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;
        m_pages[desc.page].can_gc = true;
    }

    return ret;
}


static ret_code_t write_id(fds_record_id_t * id, uint32_t * const p_addr)
{
    ret_code_t ret;
    ret = fs_store(&fs_config, p_addr + FDS_OFFSET_ID,
                    (uint32_t*)id, FDS_HEADER_SIZE_ID /*Words*/);

    return ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;
}


static ret_code_t write_chunks(fds_cmd_t * const p_cmd, uint32_t * const p_addr)
{
    ret_code_t           ret;
    fds_record_chunk_t * p_chunk = NULL;

    // Decrement the number of chunks left to write.
    p_cmd->num_chunks--;

    // Retrieve the chunk to be written.
    chunk_queue_get_and_advance(&p_chunk);

    ret = fs_store(&fs_config, p_addr + p_cmd->chunk_offset,
                   p_chunk->p_data, p_chunk->length_words);

      // Accumulate the offset.
    p_cmd->chunk_offset += p_chunk->length_words;

    return ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;
}


static ret_code_t write_ic(fds_ic_t * ic, uint32_t * const p_addr)
{
    ret_code_t ret;
    ret = fs_store(&fs_config, p_addr + FDS_OFFSET_IC,
                   (uint32_t*)ic, FDS_HEADER_SIZE_IC /*Words*/);

    return ret == NRF_SUCCESS ? FDS_SUCCESS : FDS_ERR_BUSY;
}


static void update_page_offsets(fds_page_t * p_page, uint16_t length_words)
{
    CRITICAL_SECTION_ENTER();
    p_page->write_offset   += (FDS_HEADER_SIZE + length_words);
    p_page->words_reserved -= (FDS_HEADER_SIZE + length_words);
    CRITICAL_SECTION_EXIT();
}


static ret_code_t init_execute(uint32_t last_ret)
{
    ret_code_t         ret   = FDS_SUCCESS;
    fds_cmd_t  * const p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];

    if (last_ret != NRF_SUCCESS)
    {
        return FDS_ERR_OPERATION_TIMEOUT;
    }

    switch (p_cmd->init_op)
    {
        case FDS_INIT_TAG_SWAP:
            /** We don't need to set the offset here, since page_init() could determine it,
             *  it has been already set there. */
            ret = page_tag_write_swap();
            p_cmd->init_op = FDS_INIT_TAG_VALID;
            break;

        case FDS_INIT_ERASE_SWAP:
            ret = fs_erase(&fs_config, m_swap_page.p_addr, FDS_PHY_PAGES_IN_VPAGE);
            // If we decided upon discarding the swap we need to reset the write_offset.
            m_swap_page.write_offset = FDS_PAGE_TAG_SIZE;
            p_cmd->init_op           = FDS_INIT_TAG_SWAP;
            break;

        case FDS_INIT_PROMOTE_SWAP:
        {
            // When promoting the swap, keep the write_offset set by pages_init().
            ret = page_tag_write_valid(m_swap_page.p_addr);

            uint16_t const         gc         = m_gc.cur_page;
            uint32_t const * const p_old_swap = m_swap_page.p_addr;

            // Execute the swap.
            m_swap_page.p_addr = m_pages[gc].p_addr;
            m_pages[gc].p_addr = p_old_swap;

            // Copy the offset from the swap to the new page.
            m_pages[gc].write_offset = m_swap_page.write_offset;
            m_swap_page.write_offset = FDS_PAGE_TAG_SIZE;

            m_pages[gc].page_type = FDS_PAGE_VALID;
            p_cmd->init_op        = FDS_INIT_TAG_SWAP;
        }
        break;

        case FDS_INIT_TAG_VALID:
        {
            bool write_reqd = false;
            for (uint16_t i = 0; i < FDS_MAX_PAGES; i++)
            {
                if (m_pages[i].page_type == FDS_PAGE_ERASED)
                {
                    ret = page_tag_write_valid(m_pages[i].p_addr);
                    m_pages[i].page_type = FDS_PAGE_VALID;
                    write_reqd           = true;
                    break;
                }
            }
            if (!write_reqd)
            {
                flag_set(FDS_FLAG_INITIALIZED);
                flag_clear(FDS_FLAG_INITIALIZING);
                return COMMAND_COMPLETED;
            }
        }
        break;
    }

    if (ret != FDS_SUCCESS)
    {
        return FDS_ERR_BUSY;
    }

    return COMMAND_EXECUTING;
}


/**@brief Function to execute write and update commands.
 *
 */
static ret_code_t store_execute(uint32_t last_ret, fds_cmd_t * const p_cmd)
{
    ret_code_t   ret;
    fds_page_t * p_page;
    uint32_t   * p_write_addr;

    p_page = &m_pages[p_cmd->page];

    if (last_ret != NRF_SUCCESS)
    {
        // The previous operation has timed out, update the page data.
        if ((p_cmd->id == FDS_CMD_WRITE) ||
            (p_cmd->id == FDS_CMD_UPDATE))
        {
            update_page_offsets(p_page, p_cmd->record_header.tl.length_words);
        }

        return FDS_ERR_OPERATION_TIMEOUT;
    }

    // Compute the address where to write data.
    p_write_addr = (uint32_t*)(p_page->p_addr + p_page->write_offset);

    // Execute the current operation, and set the operation to be executed next.
    switch (p_cmd->op_code)
    {
        case FDS_OP_WRITE_TL:
            ret = write_tl(&p_cmd->record_header.tl, p_write_addr);
            p_cmd->op_code = FDS_OP_WRITE_ID;
            break;

        case FDS_OP_CLEAR_TL:
            ret = clear_tl(p_cmd);
            p_cmd->op_code = FDS_OP_DONE;
            break;

        case FDS_OP_CLEAR_INSTANCE:
            ret = clear_inst(p_cmd);
            if (ret == FDS_ERR_NOT_FOUND)
            {
                /** No more records could be found, or fstorage was busy.
                 *  We won't receive another callback, so return now. */
                ret = COMMAND_COMPLETED;
            }
            break;

        case FDS_OP_WRITE_ID:
            ret = write_id(&p_cmd->record_header.id, p_write_addr);
            p_cmd->op_code = FDS_OP_WRITE_CHUNK;
            break;

        case FDS_OP_WRITE_CHUNK:
            // Advances the chunk queue and updates p_cmd.
            ret = write_chunks(p_cmd, p_write_addr);
            if (p_cmd->num_chunks == 0)
            {
                /** All the record chunks have been written; write the last word of
                 *  header (IC) as a mean to 'validate' the record. */
                p_cmd->op_code = FDS_OP_WRITE_IC;
            }
            break;

        case FDS_OP_WRITE_IC:
            ret = write_ic(&p_cmd->record_header.ic, p_write_addr);
            /** If it's a normal write, then this is the final operation.
             *  If it's an update, prepare to clear the old record. */
            p_cmd->op_code = p_cmd->id == FDS_CMD_UPDATE ? FDS_OP_CLEAR_TL : FDS_OP_DONE;
            break;

        case FDS_OP_DONE:
            ret = COMMAND_COMPLETED;

#if defined(FDS_CRC_ENABLED)
            if ((p_cmd->id == FDS_CMD_WRITE) ||
                (p_cmd->id == FDS_CMD_UPDATE))
            {
                if (flag_is_set(FDS_FLAG_VERIFY_CRC))
                {
                    uint16_t crc;

                    crc = crc16_compute((uint8_t*)p_write_addr, 6, NULL);
                    crc = crc16_compute((uint8_t*)p_write_addr + 8,
                        (FDS_HEADER_SIZE_ID + p_cmd->record_header.tl.length_words) * sizeof(uint32_t),
                        &crc);

                    if (crc != p_cmd->record_header.ic.crc16)
                    {
                        ret = FDS_ERR_CRC_CHECK_FAILED;
                    }
                }

            }
#endif
            break;

        default:
            ret = FDS_ERR_INTERNAL;
            break;
    }

    /** A command has either completed or failed.
     *  It can have failed because fstorage had no space in the queue for our request,
     *  or because we tried to clear a record which did not exist. */
    if (ret != COMMAND_EXECUTING)
    {
        // We won't receive another callback, so update the page offset and return.
        if ((p_cmd->id == FDS_CMD_WRITE) ||
            (p_cmd->id == FDS_CMD_UPDATE))
        {
            update_page_offsets(p_page, p_cmd->record_header.tl.length_words);
        }
    }

    return ret;
}


static ret_code_t cmd_queue_process(uint32_t result, bool is_cb)
{
    ret_code_t         ret;
    fds_cmd_t  * const p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];

    switch (p_cmd->id)
    {
        case FDS_CMD_INIT:
            ret = init_execute(result);
            break;

        case FDS_CMD_WRITE:
        case FDS_CMD_UPDATE:
        case FDS_CMD_CLEAR:
        case FDS_CMD_CLEAR_INST:
            ret = store_execute(result, p_cmd);
            break;

        case FDS_CMD_GC:
            ret = gc_execute(result);
            break;

        default:
            ret = NRF_ERROR_INTERNAL;
            break;
    }

    // If the command is executing, then return and wait for a new callback.
    if (ret == COMMAND_EXECUTING /*=FDS_SUCCESS*/)
    {
        return FDS_SUCCESS;
    }

    /** Otherwise, the command has either completed or failed.
     *  In case this is a callback, notify the user. */
    if (is_cb || ret == COMMAND_COMPLETED)
    {
        fds_evt_t evt;

        if (ret == COMMAND_COMPLETED)
        {
            evt.result = FDS_SUCCESS;
        }
        else
        {
            // Either FDS_ERR_BUSY, FDS_ERR_OPERATION_TIMEOUT or FDS_ERR_NOT_FOUND.
            evt.result = ret;

            // If this command had any chunks in the queue, skip them.
            chunk_queue_skip(p_cmd->num_chunks);
        }

        if (p_cmd->id == FDS_CMD_UPDATE)
        {
            evt.write.record_updated = (p_cmd->op_code == FDS_OP_DONE) ? true : false;
        }

        cmd_to_evt(p_cmd, &evt);
        app_notify(&evt);
    }

    // Advance the command queue, and if there is still something in the queue, process it.
    if (cmd_queue_advance())
    {
        cmd_queue_process(NRF_SUCCESS, false);
    }
    else
    {
        /** No more elements in the queue. Clear the FDS_FLAG_PROCESSING flag,
         *  so that new commands can start processing the queue. */
        flag_clear(FDS_FLAG_PROCESSING);
    }

    /** For callbacks, this return value is unused. Otherwise, this is the value
     *  that the user is returned synchronously by fds_* calls which put an element in the queue. */
    return ret;
}


static ret_code_t cmd_queue_process_start(void)
{
    if (!flag_is_set(FDS_FLAG_PROCESSING))
    {
        flag_set(FDS_FLAG_PROCESSING);

        return cmd_queue_process(NRF_SUCCESS, false);
    }
    else
    {
        return FDS_SUCCESS;
    }
}


static void fs_callback(fs_evt_t evt, uint32_t result)
{
    cmd_queue_process(result, true);
}


ret_code_t fds_init()
{
    if (flag_is_set(FDS_FLAG_INITIALIZED))
    {
        // No initialization is necessary. Notify immediately.
        fds_evt_t evt;

        evt.id     = FDS_EVT_INIT;
        evt.result = FDS_SUCCESS;

        app_notify(&evt);

        return FDS_SUCCESS;
    }

    if (flag_is_set(FDS_FLAG_INITIALIZING))
    {
        return FDS_ERR_INITIALIZING;
    }

    (void)fs_init();

    flag_set(FDS_FLAG_INITIALIZING);

    // Initialize the page addresses.
    fds_init_opts_t init_opts = pages_init();

    if (init_opts == NO_PAGES)
    {
        return FDS_ERR_NO_PAGES;
    }

    if (init_opts == ALREADY_INSTALLED)
    {
        /** No flash operation is necessary for initialization.
         *  Notify the application immediately. */
        flag_set(FDS_FLAG_INITIALIZED);
        flag_clear(FDS_FLAG_INITIALIZING);

        fds_evt_t evt;

        evt.id     = FDS_EVT_INIT;
        evt.result = FDS_SUCCESS;

        app_notify(&evt);

        return FDS_SUCCESS;
    }

    CRITICAL_SECTION_ENTER();

    fds_cmd_t * p_cmd = NULL;

    // This cannot fail at this stage.
    queue_reserve(&p_cmd, 0, NULL);

    p_cmd->id = FDS_CMD_INIT;

    switch (init_opts)
    {
        case FRESH_INSTALL:
        case TAG_SWAP:
            p_cmd->init_op = FDS_INIT_TAG_SWAP;
            break;

        case PROMOTE_SWAP:
        case PROMOTE_SWAP_INST:
            p_cmd->init_op = FDS_INIT_PROMOTE_SWAP;
            break;

        case DISCARD_SWAP:
            p_cmd->init_op = FDS_INIT_ERASE_SWAP;
            break;

        case TAG_VALID:
        case TAG_VALID_INST:
            p_cmd->init_op = FDS_INIT_TAG_VALID;
            break;

        default:
            break;
    }

    CRITICAL_SECTION_EXIT();

    return cmd_queue_process_start();
}


ret_code_t fds_open(fds_record_desc_t * const p_desc,
                    fds_record_t      * const p_record)
{
    if (p_desc == NULL || p_record == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    if (!page_id_within_bounds(p_desc->page))
    {
        // Should not happen.
        return FDS_ERR_INVALID_PARAM;
    }

    // Seek the record if necessary.
    if (find_record(p_desc))
    {
        if (header_is_valid((fds_header_t*)p_desc->p_rec))
        {
            CRITICAL_SECTION_ENTER();
            m_pages[p_desc->page].records_open++;
            CRITICAL_SECTION_EXIT();

#if defined(FDS_CRC_ENABLED)
            uint16_t crc;
            crc = crc16_compute((uint8_t*)&p_record, 6, NULL);
            crc = crc16_compute((uint8_t*)&p_record->p_data,
                                p_record->p_header->tl.length_words * sizeof(uint32_t),
                                &crc);

            if (crc != p_record->p_header->ic.crc16)
            {
                return NRF_ERROR_INVALID_DATA;
            }
#endif
            p_record->p_header = ((fds_header_t*)p_desc->p_rec);
            p_record->p_data   = (p_desc->p_rec + FDS_HEADER_SIZE);

            return FDS_SUCCESS;
        }
    }

    /** The record could not be found.
     *  It either never existed or it has been cleared. */
    return FDS_ERR_NOT_FOUND;
}


ret_code_t fds_close(fds_record_desc_t const * const p_desc)
{
    if (p_desc == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    if (!page_id_within_bounds(p_desc->page))
    {
        return FDS_ERR_INVALID_PARAM;
    }

    CRITICAL_SECTION_ENTER();
    m_pages[p_desc->page].records_open--;
    CRITICAL_SECTION_EXIT();

    return FDS_SUCCESS;
}


static ret_code_t write_enqueue(fds_record_desc_t        * const p_desc,
                                fds_record_key_t                 key,
                                uint8_t                          num_chunks,
                                fds_record_chunk_t               chunks[],
                                fds_write_token_t  const * const p_tok,
                                bool                             do_update)
{
    ret_code_t           ret;
    fds_cmd_t          * p_cmd;
    fds_record_chunk_t * p_chunk;
    uint16_t             page;
    uint16_t             length_words = 0;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if ((key.type     == FDS_TYPE_ID_INVALID) ||
        (key.instance == FDS_INSTANCE_ID_INVALID))
    {
        return FDS_ERR_INVALID_KEYS;
    }

    if (!chunk_is_aligned(chunks, num_chunks))
    {
        return FDS_ERR_UNALIGNED_ADDR;
    }

    // No space was previously reserved for this operation.
    if (p_tok == NULL)
    {
        // Compute the total length of the record.
        for (uint8_t i = 0; i < num_chunks; i++)
        {
            length_words += chunks[i].length_words;
        }

        // Find a page where to write data.
        ret = write_space_reserve(length_words, &page);

        if (ret != NRF_SUCCESS)
        {
            /** No flash space available (FDS_ERR_NO_SPACE_IN_FLASH) or
             *  record too large (FDS_ERR_RECORD_TOO_LARGE). */
            return ret;
        }
    }
    else
    {
        page         = p_tok->page;
        length_words = p_tok->length_words;
    }

    CRITICAL_SECTION_ENTER();

    // Reserve space in both queues, and obtain pointers to the first elements reserved.
    if (queue_reserve(&p_cmd, num_chunks, &p_chunk))
    {
        // Initialize the command.
        p_cmd->id           = do_update ? FDS_CMD_UPDATE : FDS_CMD_WRITE;
        p_cmd->page         = page;
        p_cmd->op_code      = FDS_OP_WRITE_TL;
        p_cmd->num_chunks   = num_chunks;
        p_cmd->chunk_offset = FDS_OFFSET_DATA;

        // Fill in the header information.
        p_cmd->record_header.id              = record_id_new();
        p_cmd->record_header.tl.type         = key.type;
        p_cmd->record_header.tl.length_words = length_words;
        p_cmd->record_header.ic.instance     = key.instance;

        if (do_update)
        {
            p_cmd->record_to_clear = p_desc->record_id;
        }

#if defined(FDS_CRC_ENABLED)
        uint16_t  crc;
        crc = crc16_compute((uint8_t*)&p_cmd->record_header, 6, NULL);
        crc = crc16_compute((uint8_t*)&p_cmd->record_header.id, 4, &crc);
#else
        uint16_t  crc = 0;
#endif
        // Buffer the record chunks in the queue.
        for (uint8_t i = 0; i < num_chunks; i++)
        {
            p_chunk->p_data       = chunks[i].p_data;
            p_chunk->length_words = chunks[i].length_words;

#if defined(FDS_CRC_ENABLED)
            crc = crc16_compute((uint8_t*)p_chunk->p_data, p_chunk->length_words * 4, &crc);
#endif
            chunk_queue_next(&p_chunk);
        }

        p_cmd->record_header.ic.crc16 = crc;

        // Initialize the record descriptor, if provided.
        if (p_desc != NULL)
        {
            p_desc->page      = page;
            // Don't invoke record_id_new() again !
            p_desc->record_id = p_cmd->record_header.id;
        }

        ret = FDS_SUCCESS;
    }
    else
    {
        // No space was availble in the queues. Free the reserved flash space.
        write_space_free(length_words, page);

        ret = FDS_ERR_NO_SPACE_IN_QUEUES;
    }

    CRITICAL_SECTION_EXIT();

    if (ret != FDS_SUCCESS)
    {
        // Could not reserve queue resources.
        return ret;
    }

    return cmd_queue_process_start();
}


ret_code_t fds_reserve(fds_write_token_t * const p_tok, uint16_t length_words)
{
    ret_code_t ret;
    uint16_t   page;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if (p_tok == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    ret = write_space_reserve(length_words, &page);

    if (ret == FDS_SUCCESS)
    {
        p_tok->page         = page;
        p_tok->length_words = length_words;
    }

    return ret;
}


ret_code_t fds_reserve_cancel(fds_write_token_t * const p_tok)
{
    fds_page_t * p_page;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if (p_tok == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    if (!page_id_within_bounds(p_tok->page))
    {
        // Could not find the virtual page. This shouldn't happen.
        return FDS_ERR_INVALID_PARAM;
    }

    p_page = &m_pages[p_tok->page];

    if ((p_page->words_reserved - p_tok->length_words) < 0)
    {
        /** We are trying to cancel a reservation for more words than how many are
         *  currently reserved on the page. This shouldn't happen. */
        return FDS_ERR_INVALID_PARAM;
    }

    CRITICAL_SECTION_ENTER();
    // Free reserved space.
    p_page->words_reserved -= (FDS_HEADER_SIZE + p_tok->length_words);
    CRITICAL_SECTION_EXIT();

    // Clean the token.
    p_tok->page         = 0;
    p_tok->length_words = 0;

    return FDS_SUCCESS;
}


ret_code_t fds_write(fds_record_desc_t  * const p_desc,
                     fds_record_key_t           key,
                     uint8_t                    num_chunks,
                     fds_record_chunk_t         chunks[])
{
    return write_enqueue(p_desc, key, num_chunks, chunks, NULL, false /*not an update*/);
}


ret_code_t fds_write_reserved(fds_write_token_t const * const p_tok,
                              fds_record_desc_t       * const p_desc,
                              fds_record_key_t                key,
                              uint8_t                         num_chunks,
                              fds_record_chunk_t              chunks[])
{
    return write_enqueue(p_desc, key, num_chunks, chunks, p_tok, false /*not an update*/);
}


ret_code_t fds_update(fds_record_desc_t  * const p_desc,
                      fds_record_key_t           key,
                      uint8_t                    num_chunks,
                      fds_record_chunk_t         chunks[])
{
    return write_enqueue(p_desc, key, num_chunks, chunks, NULL, true /*update*/);
}


ret_code_t fds_clear(fds_record_desc_t * const p_desc)
{
    fds_cmd_t * p_cmd;
    bool        queue_reserved;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if (p_desc == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    CRITICAL_SECTION_ENTER();
    queue_reserved = queue_reserve(&p_cmd, 0, NULL);

    if (queue_reserved)
    {
        // Initialize the command.
        p_cmd->id              = FDS_CMD_CLEAR;
        p_cmd->op_code         = FDS_OP_CLEAR_TL;
        p_cmd->record_to_clear = p_desc->record_id;
    }
    CRITICAL_SECTION_EXIT();

    return queue_reserved ? cmd_queue_process_start() : FDS_ERR_NO_SPACE_IN_QUEUES;
}


ret_code_t fds_clear_by_instance(uint16_t instance)
{
    fds_cmd_t * p_cmd;
    bool        queue_reserved;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    if (instance == FDS_INSTANCE_ID_INVALID)
    {
        return FDS_ERR_INVALID_KEYS;
    }

    CRITICAL_SECTION_ENTER();
    queue_reserved = queue_reserve(&p_cmd, 0, NULL);

    if (queue_reserved)
    {
        p_cmd->id      = FDS_CMD_CLEAR_INST;
        p_cmd->op_code = FDS_OP_CLEAR_INSTANCE;

        p_cmd->record_header.ic.instance = instance;
    }
    CRITICAL_SECTION_EXIT();

    return queue_reserved ? cmd_queue_process_start() : FDS_ERR_NO_SPACE_IN_QUEUES;
}


ret_code_t fds_gc()
{
    fds_cmd_t * p_cmd;
    bool        queue_reserved;

    if (!flag_is_set(FDS_FLAG_INITIALIZED))
    {
        return FDS_ERR_NOT_INITIALIZED;
    }

    CRITICAL_SECTION_ENTER();
    queue_reserved = queue_reserve(&p_cmd, 0, NULL);

    if (queue_reserved)
    {
        p_cmd->id = FDS_CMD_GC;
        // Set GC parameters.
        m_gc.state = GC_BEGIN;
    }
    CRITICAL_SECTION_EXIT();

    return queue_reserved ? cmd_queue_process_start() : FDS_ERR_NO_SPACE_IN_QUEUES;
}


ret_code_t fds_find(uint16_t             type,
                    uint16_t         instance,
                    fds_record_desc_t * const p_desc,
                    fds_find_token_t  * const p_token)
{
    return find_record_by_key(&type, &instance, p_desc, p_token);
}


ret_code_t fds_find_by_type(uint16_t             type,
                            fds_record_desc_t * const p_desc,
                            fds_find_token_t  * const p_token)
{
    return find_record_by_key(&type, NULL, p_desc, p_token);
}


ret_code_t fds_find_by_instance(uint16_t         instance,
                                fds_record_desc_t * const p_desc,
                                fds_find_token_t  * const p_token)
{
    return find_record_by_key(NULL, &instance, p_desc, p_token);
}


ret_code_t fds_register(fds_cb_t cb)
{
    if (m_users == FDS_MAX_USERS)
    {
        return FDS_ERR_USER_LIMIT_REACHED;
    }

    m_cb_table[m_users] = cb;
    m_users++;

    return FDS_SUCCESS;
}


ret_code_t fds_descriptor_from_rec_id(fds_record_desc_t * const p_desc,
                                      fds_record_id_t           record_id)
{
    if (p_desc == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    p_desc->page      = FDS_PAGE_ID_UNKNOWN;
    p_desc->record_id = record_id;

    return FDS_SUCCESS;
}


ret_code_t fds_record_id_from_desc(fds_record_desc_t const * const p_desc,
                                   fds_record_id_t         * const p_record_id)
{
    if (p_desc == NULL || p_record_id == NULL)
    {
        return FDS_ERR_NULL_PARAM;
    }

    *p_record_id = p_desc->record_id;

    return FDS_SUCCESS;
}


#if defined(FDS_CRC_ENABLED)

ret_code_t fds_verify_crc_on_writes(bool enable)
{
    if (enable)
    {
        flag_set(FDS_FLAG_VERIFY_CRC);
    }
    else
    {
        flag_clear(FDS_FLAG_VERIFY_CRC);
    }

    return FDS_SUCCESS;
}

#endif
