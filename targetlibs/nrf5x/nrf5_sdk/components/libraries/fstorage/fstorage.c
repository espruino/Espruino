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

#include "fstorage.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "fstorage_config.h"
#include "nrf_error.h"
#include "nrf_soc.h"


#define FS_FLAG_INIT                (1 << 0)    /**< fstorage has been initialized. */
#define FS_FLAG_PROCESSING          (1 << 1)    /**< fstorage is executing queued flash operations. */
#define FS_FLAG_FLASH_REQ_PENDING   (1 << 2)    /**< fstorage is waiting for a flash operation initiated by another module to complete. */


/**@brief   Macro to register the section 'fs_data'.
 *
 * @details Required for compilation.
 */

/*lint -e19*/
NRF_SECTION_VARS_REGISTER_SECTION(fs_data);
/*lint +e19*/

/**@brief   Macro to declare symbols used to find the beginning and end of the section fs_data.
 *
 * @details Required for compilation.
 */
NRF_SECTION_VARS_REGISTER_SYMBOLS(fs_config_t, fs_data);


/**@defgroup    Helper macros for section variables.
 *
 * @details Macros used to manipulate registered section variables.
 */

 /**@brief  Get section variable with fstorage configuration by index. */
#define FS_SECTION_VARS_GET(i)          NRF_SECTION_VARS_GET(i, fs_config_t, fs_data)
 /**@brief  Get the number of registered section variables. */
#define FS_SECTION_VARS_COUNT           NRF_SECTION_VARS_COUNT(fs_config_t, fs_data)
 /**@brief  Get the start address of the registered section variables. */
#define FS_SECTION_VARS_START_ADDR      NRF_SECTION_VARS_START_ADDR(fs_data)
 /**@brief  Get the end address of the registered section variables. */
#define FS_SECTION_VARS_END_ADDR        NRF_SECTION_VARS_END_ADDR(fs_data)

/** @} */


#if defined(__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined(__ICCARM__)
  #pragma language=extended
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#endif

/**@brief The command queue element.
 *
 * @details Encapsulate details of a command requested to this module.
 */
typedef struct
{
    fs_config_t  const * p_config;   /**< The configuration of the user who requested the operation. */
    fs_op_code_t         op_code;    /**< Operation code. */
    union
    {
        struct
        {
            uint32_t const * p_src;          /**< Pointer to the data to be written to flash. The data must be kept in memory until the operation has finished. */
            uint32_t const * p_dest;         /**< Destination of the data in flash. */
            uint16_t         length_words;   /**< Length of the data to store, in words. */
            uint16_t         offset;         /**< Data offset, if written in chunks. */
        } store;
        struct
        {
            uint16_t page;
            uint16_t pages_erased;
            uint16_t pages_to_erase;
        } erase;
    };
} fs_cmd_t;

#if defined(__CC_ARM)
  #pragma pop
#elif defined(__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#endif

/**@brief Structure that defines the command queue
 *
 * @details This queue holds flash operations requested to the module. 
 *          The data to be written must be kept in memory until the write operation is completed,
 *          i.e., a callback indicating completion is received by the application.
 */
typedef struct
{
    uint8_t  rp;                        /**< The current element being processed. */
    uint8_t  count;                     /**< Number of elements in the queue. */
    fs_cmd_t cmd[FS_CMD_QUEUE_SIZE];    /**< Array to maintain flash access operation details. */
} fs_cmd_queue_t;


static uint8_t          m_flags;        /**< FStorage status flags. */
static uint16_t         m_retry_count;  /**< Number of times a single flash operation was retried. */
static fs_cmd_queue_t   m_cmd_queue;    /**< Flash operation request queue. */


/**@brief Function to check that the configuration is non-NULL and within
*         valid section variable memory bounds.
 *
 * @param[in]   config    Configuration to check.
 */
static bool check_config(fs_config_t const * const config)
{
    if (config == NULL)
    {
        return false;
    }

    if ((FS_SECTION_VARS_START_ADDR <= (uint32_t)config) &&
        ((uint32_t)config < FS_SECTION_VARS_END_ADDR))
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**@brief Function to store data to flash.
 *
 * @param[in]   p_cmd   The queue element associated with the operation.
 *
 * @retval NRF_SUCCESS  Success. The request was sent to the SoftDevice.
 * @retval Any error returned by the SoftDevice flash API.
 */
static ret_code_t store_execute(fs_cmd_t * const p_cmd)
{
    ret_code_t ret;
    uint16_t   chunk_len;

    if (p_cmd->store.length_words - p_cmd->store.offset < FS_MAX_WRITE_SIZE_WORDS)
    {
        chunk_len = p_cmd->store.length_words - p_cmd->store.offset;
    }
    else
    {
        chunk_len = FS_MAX_WRITE_SIZE_WORDS;
    }
    
    ret =  sd_flash_write((uint32_t*)&p_cmd->store.p_dest[p_cmd->store.offset],
                          (uint32_t*)&p_cmd->store.p_src[p_cmd->store.offset],
                          chunk_len);

    p_cmd->store.offset += chunk_len;

    return ret;
}


/**@brief Function to erase a page.
 *
 * @param[in]   p_cmd   The queue element associated with the operation.
 *
 * @retval NRF_SUCCESS  Success. The request was sent to the SoftDevice.
 * @retval Any error returned by the SoftDevice flash API.
 */
static ret_code_t erase_execute(fs_cmd_t * const p_cmd)
{
    ret_code_t ret = sd_flash_page_erase(p_cmd->erase.page++);

    p_cmd->erase.pages_erased++;

    return ret;
}


/**@brief Function to notify users.
 *
 * @param[in]   result      Result of the flash operation.
 * @param[in]   p_cmd       The command associated with the callback.
 */
static void app_notify(fs_cmd_t const * const p_cmd, uint32_t result)
{
    fs_evt_t evt;

    switch (p_cmd->op_code)
    {
        case FS_OP_STORE:
            evt.id           = FS_EVT_STORE;
            evt.store.p_data = p_cmd->store.p_dest;
            break;

        case FS_OP_ERASE:
            evt.id               = FS_EVT_ERASE;
            evt.erase.first_page = p_cmd->erase.page - p_cmd->erase.pages_erased;
            evt.erase.last_page  = p_cmd->erase.page;
            break;

        default:
            break;
    }
    
    p_cmd->p_config->cb(evt, result);
}


/**@brief Function to consume queue item and notify the return value of the operation.
 *
 * @details This function will report the result and remove the command from the queue after
 *          notification.
 */
static void queue_advance()
{
    
    if (--m_cmd_queue.count == 0)
    {
        // There are no elements left. Stop processing the queue.
        m_flags &= ~FS_FLAG_PROCESSING;
    }

    if (++m_cmd_queue.rp == FS_CMD_QUEUE_SIZE)
    {
        m_cmd_queue.rp = 0;
    }
}


/**@brief Function to process the current element in the queue and return the result.
 *
 * @retval NRF_SUCCESS          Success.
 * @retval NRF_ERROR_FORBIDDEN  Error. Undefined command.
 * @retval Any error returned by the SoftDevice flash API.
 */
static ret_code_t queue_process(void)
{
    ret_code_t       ret;
    fs_cmd_t * const p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];
    
    if (m_cmd_queue.count > 0)
    {
        switch (p_cmd->op_code)
        {
            case FS_OP_STORE:
                ret = store_execute(p_cmd);
                break;

            case FS_OP_ERASE:
                ret = erase_execute(p_cmd);
                break;

            default:
                ret = NRF_ERROR_INTERNAL;
                break;
        }
    }
    else
    {
        ret = NRF_SUCCESS;
    }

    /** There is ongoing flash-operation which was not
    *  initiated by fstorage. */
    if (ret == NRF_ERROR_BUSY)
    {
        // Wait for a system callback.
        m_flags |= FS_FLAG_FLASH_REQ_PENDING;

        // Stop processing the queue.
        m_flags &= ~FS_FLAG_PROCESSING;

        ret = NRF_SUCCESS;
    }
    else if (ret != NRF_SUCCESS)
    {
        // Another error has occurred.
        app_notify(p_cmd, ret);
    }

    return ret;
}


/**@brief Starts processing the queue if there are no pending flash operations
 *        for which we are awaiting a callback.
 */
static ret_code_t queue_process_start(void)
{
    ret_code_t ret = NRF_SUCCESS;

    /** If the queue is not being processed and there are no events pending,
     *  then start processing. */
    if (!(m_flags & FS_FLAG_PROCESSING) &&
        !(m_flags & FS_FLAG_FLASH_REQ_PENDING))
    {
        m_flags |= FS_FLAG_PROCESSING;

        ret = queue_process();        
    }

    // If already processing the queue, return immediately.
    return ret;
}



/**@brief Flash operation success callback handler.
 *
 * @details     This function updates read/write pointers.
 *              This function resets retry count.
 */
static void on_operation_success(fs_cmd_t const * const p_cmd)
{
    m_retry_count = 0;

    switch (p_cmd->op_code)
    {
        case FS_OP_STORE:
            // If offset is equal to or larger than length, then the operation has finished.
            if (p_cmd->store.offset >= p_cmd->store.length_words)
            {
                app_notify(p_cmd, NRF_SUCCESS);
                queue_advance();
            }
            break;

        case FS_OP_ERASE:
            if (p_cmd->erase.pages_erased == p_cmd->erase.pages_to_erase)
            {
                app_notify(p_cmd, NRF_SUCCESS);
                queue_advance();
            }
            break;

        default:
            break;
    }
}


/**@brief Flash operation failure callback handler.
 *
 * @details Function to keep track of retries and notify failures.
 */
static void on_operation_failure(fs_cmd_t const * const p_cmd)
{
    if (++m_retry_count > FS_CMD_MAX_RETRIES)
    {
        app_notify(p_cmd, NRF_ERROR_TIMEOUT);
        queue_advance();
    }
}


/**@brief Function to enqueue flash access command
 *
 * @param[in]   config      Registered configuration.
 * @param[in]   op_code     Operation code.
 * @param[in]   address     Destination of the data.
 * @param[in]   p_src       Source of data or NULL if n/a.
 * @param[in]   length      Length of the data, in 4 byte words.
 *
 * @retval NRF_SUCCESS      Success. Command enqueued.
 * @retval NRF_ERROR_NO_MEM Error. Queue is full.
 * @retval Any error returned by the SoftDevice flash API.
 */
static ret_code_t cmd_enqueue(fs_cmd_t * p_cmd)
{
    uint8_t idx;

    if (m_cmd_queue.count == FS_CMD_QUEUE_SIZE - 1)
    {
        return NRF_ERROR_NO_MEM;
    }
 
    idx = (m_cmd_queue.rp + m_cmd_queue.count) % FS_CMD_QUEUE_SIZE;
    
    m_cmd_queue.count++;

    memset(&m_cmd_queue.cmd[idx], 0x00, sizeof(fs_cmd_t));
    memcpy((void*)&m_cmd_queue.cmd[idx], (const void*)p_cmd, sizeof(fs_cmd_t));

    return queue_process_start();
}


ret_code_t fs_init(void)
{
    uint16_t   lowest_index = 0;
    uint16_t   lowest_order = 0xFFFF;
    uint32_t * current_end  = (uint32_t*)FS_PAGE_END_ADDR;
    uint32_t   num_left     = FS_SECTION_VARS_COUNT;

    /** Assign pages to registered users, beginning with the ones with the lowest
     *  order, which will be assigned pages with the lowest memory address. */
    do
    {
        fs_config_t * p_config;
        for (uint16_t i = 0; i < FS_SECTION_VARS_COUNT; i++)
        {
            p_config = FS_SECTION_VARS_GET(i);

            // Skip the ones which have the end-address already set.
            if (p_config->p_end_addr != NULL)
                continue;

            if (p_config->page_order < lowest_order)
            {
                lowest_order = p_config->page_order;
                lowest_index = i;
            }
        }

        p_config = FS_SECTION_VARS_GET(lowest_index);

        p_config->p_end_addr   = current_end;
        p_config->p_start_addr = p_config->p_end_addr - (p_config->num_pages * FS_PAGE_SIZE_WORDS);

        current_end  = p_config->p_start_addr;
        lowest_order = 0xFFFF;

    } while ( --num_left > 0 );

    m_flags |= FS_FLAG_INIT;

    return NRF_SUCCESS;
}


ret_code_t fs_store(fs_config_t const * const p_config,
                    uint32_t    const * const p_dest,
                    uint32_t    const * const p_src,
                    uint16_t                  length_words)
{
    fs_cmd_t cmd;

    if ((m_flags & FS_FLAG_INIT) == 0)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (!check_config(p_config))
    {
        return NRF_ERROR_FORBIDDEN;
    }

    if (((uint32_t)p_src  & 0x3) ||
        ((uint32_t)p_dest & 0x3))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if ((p_dest < p_config->p_start_addr) || ((p_dest + length_words) > p_config->p_end_addr))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    cmd.p_config           = p_config;
    cmd.op_code            = FS_OP_STORE;
    cmd.store.p_src        = p_src;
    cmd.store.p_dest       = p_dest;
    cmd.store.length_words = length_words;
    cmd.store.offset       = 0;

    return cmd_enqueue(&cmd);
}


ret_code_t fs_erase(fs_config_t const * const p_config,
                    uint32_t    const * const p_page_addr,
                    uint16_t                  num_pages)
{

    fs_cmd_t cmd;

    if ((m_flags & FS_FLAG_INIT) == 0)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (!check_config(p_config))
    {
        return NRF_ERROR_FORBIDDEN;
    }

    if (((uint32_t)p_page_addr % FS_PAGE_SIZE) != 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if ((p_page_addr < p_config->p_start_addr) ||
        (p_page_addr + (FS_PAGE_SIZE_WORDS * num_pages) > p_config->p_end_addr))
    {
        return NRF_ERROR_INVALID_DATA;
    }

    cmd.p_config             = p_config;
    cmd.op_code              = FS_OP_ERASE;
    cmd.erase.page           = ((uint32_t)p_page_addr / FS_PAGE_SIZE);
    cmd.erase.pages_erased   = 0;
    cmd.erase.pages_to_erase = num_pages;

    return cmd_enqueue(&cmd);
}


/**@brief Function to handle system events from the SoftDevice.
 *
 * @details     This function should be dispatched system events if any of the modules used by
 *              the application rely on FStorage. Examples include @ref Peer Manager and
 *              @ref Flash Data Storage.
 *
 * @param[in]   sys_evt     System Event received.
 */
void fs_sys_event_handler(uint32_t sys_evt)
{
    fs_cmd_t const * const p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];
    
    if (m_flags & FS_FLAG_PROCESSING)
    {
        /** A flash operation was initiated by this module.
         *  Handle its result. */
        switch (sys_evt)
        {
            case NRF_EVT_FLASH_OPERATION_SUCCESS:
                on_operation_success(p_cmd);
                break;

            case NRF_EVT_FLASH_OPERATION_ERROR:
                on_operation_failure(p_cmd);
                break;
        }
    }
    else if ((m_flags & FS_FLAG_FLASH_REQ_PENDING))
    {
        /** A flash operation was initiated outside this module.
         *  We have now receveid a callback which indicates it has
         *  finished. Clear the FS_FLAG_FLASH_REQ_PENDING flag. */
         m_flags &= ~FS_FLAG_FLASH_REQ_PENDING;
    }

    // Resume processing the queue, if necessary.
    queue_process();
}


