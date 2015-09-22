/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "pstorage.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "app_util.h"

#define INVALID_OPCODE     0x00                            /**< Invalid op code identifier. */
#define SOC_MAX_WRITE_SIZE 1024                            /**< Maximum write size allowed for a single call to \ref sd_flash_write as specified in the SoC API. */
#define RAW_MODE_APP_ID    (PSTORAGE_NUM_OF_PAGES + 1)     /**< Application id for raw mode. */

/**
 * @defgroup api_param_check API Parameters check macros.
 *
 * @details Macros that verify parameters passed to the module in the APIs. These macros
 *          could be mapped to nothing in final versions of code to save execution and size.
 *
 * @{
 */

/**
 * @brief Check if the input pointer is NULL, if it is returns NRF_ERROR_NULL.
 */
#define NULL_PARAM_CHECK(PARAM)                                                                   \
        if ((PARAM) == NULL)                                                                      \
        {                                                                                         \
            return NRF_ERROR_NULL;                                                                \
        }

/**
 * @brief Verifies the module identifier supplied by the application is within permissible
 *        range.
 */
#define MODULE_ID_RANGE_CHECK(ID)                                                                 \
        if ((((ID)->module_id) >= PSTORAGE_NUM_OF_PAGES) ||                                       \
            (m_app_table[(ID)->module_id].cb == NULL))                                            \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**
 * @brief Verifies the block identifier supplied by the application is within the permissible
 *        range.
 */
#define BLOCK_ID_RANGE_CHECK(ID)                                                                  \
        if (((ID)->block_id) >= (m_app_table[(ID)->module_id].base_id +                           \
            (m_app_table[(ID)->module_id].block_count * MODULE_BLOCK_SIZE(ID))))                  \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**
 * @brief Verifies the block size requested by the application can be supported by the module.
 */
#define BLOCK_SIZE_CHECK(X)                                                                       \
        if (((X) > PSTORAGE_MAX_BLOCK_SIZE) || ((X) < PSTORAGE_MIN_BLOCK_SIZE))                   \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**
 * @brief Verifies block size requested by Application in registration API.
 */
#define BLOCK_COUNT_CHECK(COUNT, SIZE)                                                            \
        if (((COUNT) == 0) || ((m_next_page_addr + ((COUNT) *(SIZE)) > PSTORAGE_SWAP_ADDR)))      \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**
 * @brief Verifies size parameter provided by application in API.
 */
#define SIZE_CHECK(ID, SIZE)                                                                      \
        if(((SIZE) == 0) || ((SIZE) > MODULE_BLOCK_SIZE(ID)))                                     \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**
 * @brief Verifies offset parameter provided by application in API.
 */
#define OFFSET_CHECK(ID, OFFSET, SIZE)                                                            \
        if(((SIZE) + (OFFSET)) > MODULE_BLOCK_SIZE(ID))                                           \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

#ifdef PSTORAGE_RAW_MODE_ENABLE

/**
 * @brief Verifies the module identifier supplied by the application is registered for raw mode.
 */
#define MODULE_RAW_ID_RANGE_CHECK(ID)                                                             \
        if ((PSTORAGE_NUM_OF_PAGES+1 != ((ID)->module_id)) ||                                     \
            (m_raw_app_table.cb == NULL))                                                         \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

#endif // PSTORAGE_RAW_MODE_ENABLE

/**@} */


/**@brief    Verify module's initialization status.
 *
 * @details   Verify module's initialization status. Returns NRF_ERROR_INVALID_STATE in case a
 *            module API is called without initializing the module.
 */
#define VERIFY_MODULE_INITIALIZED()                                                               \
        do                                                                                        \
        {                                                                                         \
            if (!m_module_initialized)                                                            \
            {                                                                                     \
                 return NRF_ERROR_INVALID_STATE;                                                  \
            }                                                                                     \
        } while(0)

/**@brief Macro to fetch the block size registered for the module. */
#define MODULE_BLOCK_SIZE(ID) (m_app_table[(ID)->module_id].block_size)


/** @brief States for the Update/Clear swap backup state machine. */
typedef enum
{
    STATE_INIT,                /**< State for indicating that swap can be used when using update/clear API. */
    STATE_DATA_TO_SWAP_WRITE,  /**< State for doing backup of data page into the swap page when using update/clear API. */
    STATE_DATA_ERASE,          /**< State for erasing data page when using update/clear API. */
    STATE_HEAD_RESTORE,        /**< State for restoring head (beginning) of backed up data from swap to data page when using update/clear API. */
    STATE_TAIL_RESTORE,        /**< State for restoring tail (end) of backed up data from swap to data page when using update/clear API. */
    STATE_NEW_BODY_WRITE,      /**< State for writing body (middle) data to the data page when using update/clear API. */
    STATE_SWAP_ERASE,          /**< State for erasing the swap page when using the update/clear API. */
    STATE_COMPLETE,            /**< State for indicating that update/clear sequence is completed internal in the module when using the update/clear API. */
    STATE_SWAP_DIRTY           /**< State for initializing the swap region on module initialization. */
} swap_backup_state_t;


/**
 * @brief Application registration information.
 *
 * @details Define application specific information that application needs to maintain to be able
 *          to process requests from each one of them.
 */
typedef struct
{
    pstorage_ntf_cb_t    cb;             /**< Callback registered with the module to be notified of result of flash access.  */
    pstorage_block_t     base_id;        /**< Base block id assigned to the module. */
    pstorage_size_t      block_size;     /**< Size of block for the module. */
    pstorage_size_t      block_count;    /**< Number of block requested by application. */
    pstorage_size_t      num_of_pages;   /**< Variable to remember how many pages have been allocated for this module. This information is used for clearing of block, so that application does not need to have knowledge of number of pages its using. */
} pstorage_module_table_t;


#ifdef PSTORAGE_RAW_MODE_ENABLE
/**
 * @brief Application registration information.
 *
 * @details Define application specific information that application registered for raw mode.
 */
typedef struct
{
    pstorage_ntf_cb_t      cb;             /**< Callback registered with the module to be notified of result of flash access.  */
    uint16_t               num_of_pages;   /**< Variable to remember how many pages have been allocated for this module. This information is used for clearing of block, so that application does not need to have knowledge of number of pages its using. */
} pstorage_raw_module_table_t;
#endif // PSTORAGE_RAW_MODE_ENABLE


/**
 * @brief Defines command queue element.
 *
 * @details Defines command queue element. Each element encapsulates needed information to process
 *          a flash access command.
 */
typedef struct
{
    uint8_t              op_code;       /**< Identifies flash access operation being queued. Element is free if op-code is INVALID_OPCODE. */
    pstorage_size_t      size;          /**< Identifies size in bytes requested for the operation. */
    pstorage_size_t      offset;        /**< Offset requested by the application for access operation. */
    pstorage_handle_t    storage_addr;  /**< Address/Identifier for persistent memory. */
    uint8_t *            p_data_addr;   /**< Address/Identifier for data memory. This is assumed to be resident memory. */
} cmd_queue_element_t;


/**
 * @brief Defines command queue, an element is free if op_code field is not invalid.
 *
 * @details Defines commands enqueued for flash access. At any point of time, this queue has one or
 *          more flash access operation pending if the count field is not zero. When the queue is
 *          not empty, the rp (read pointer) field points to the flash access command in progress
 *          or to requested next. The queue implements a simple first in first out algorithm.
 *          Data addresses are assumed to be resident.
 */
typedef struct
{
    uint8_t              rp;                           /**< Read pointer, pointing to flash access that is ongoing or to be requested next. */
    uint8_t              count;                        /**< Number of elements in the queue.  */
    bool                 flash_access;                 /**< Flag to ensure an flash event received is for an request issued by the module. */
    cmd_queue_element_t  cmd[PSTORAGE_CMD_QUEUE_SIZE]; /**< Array to maintain flash access operation details. */
} cmd_queue_t;


static cmd_queue_t         m_cmd_queue;                  /**< Flash operation request queue. */
static pstorage_size_t     m_next_app_instance;          /**< Points to the application module instance that can be allocated next. */
static uint32_t            m_next_page_addr;             /**< Points to the flash address that can be allocated to a module next, this is needed as blocks of a module can span across flash pages. */
static pstorage_size_t     m_round_val;                  /**< Round value for multiple round operations. For erase operations, the round value will contain current round counter which is identical to number of pages erased. For store operations, the round value contains current round of operation * SOC_MAX_WRITE_SIZE to ensure each store to the SoC Flash API is within the SoC limit. */
static bool                m_module_initialized = false; /**< Flag for checking if module has been initialized. */
static swap_backup_state_t m_swap_state;                 /**< Swap page state. */


static pstorage_module_table_t m_app_table[PSTORAGE_NUM_OF_PAGES]; /**< Registered application information table. */

#ifdef PSTORAGE_RAW_MODE_ENABLE
static pstorage_raw_module_table_t m_raw_app_table;                    /**< Registered application information table for raw mode. */
#endif // PSTORAGE_RAW_MODE_ENABLE


/**
 * @brief Routine called to actually issue the flash access request to the SoftDevice.
 *
 * @retval    NRF_SUCCESS    on success, else an error code indicating reason for failure.
 */
static uint32_t cmd_process(void);


/**
 * @brief Routine to notify application of any errors.
 *
 * @param[in] result Result of event being notified.
 */
static void app_notify(uint32_t result);


/**
 * @defgroup utility_functions Utility internal functions.
 * @{
 * @details Utility functions needed for interfacing with flash through SoC APIs.
 * SoC APIs are non blocking and provide the result of flash access through an event.
 *
 * @note Only one flash access operation is permitted at a time by SoC. Hence a queue is
 * maintained by this module.
 */


/**
 * @brief Initializes command queue element.
 *
 * @param[in] index Element index being initialized.
 */
static void cmd_queue_element_init(uint32_t index)
{
    // Internal function and checks on range of index can be avoided.
    m_cmd_queue.cmd[index].op_code                = INVALID_OPCODE;
    m_cmd_queue.cmd[index].size                   = 0;
    m_cmd_queue.cmd[index].storage_addr.module_id = PSTORAGE_NUM_OF_PAGES;
    m_cmd_queue.cmd[index].storage_addr.block_id  = 0;
    m_cmd_queue.cmd[index].p_data_addr            = NULL;
    m_cmd_queue.cmd[index].offset                 = 0;
}


/**
 * @brief Initializes command queue.
 */
static void cmd_queue_init(void)
{
    uint32_t cmd_index;

    m_round_val              = 0;
    m_swap_state             = STATE_INIT;
    m_cmd_queue.rp           = 0;
    m_cmd_queue.count        = 0;
    m_cmd_queue.flash_access = false;

    for (cmd_index = 0; cmd_index < PSTORAGE_CMD_QUEUE_SIZE; cmd_index++)
    {
        cmd_queue_element_init(cmd_index);
    }
}


/**
 * @brief Routine to enqueue a flash access operation.
 *
 * @param[in] opcode         Identifies operation requested to be enqueued.
 * @param[in] p_storage_addr Identiifes module and flash address on which operation is requested.
 * @param[in] p_data_addr    Identifies data address for flash access.
 * @param[in] size           Size in bytes of data requested for the access operation.
 * @param[in] offset         Offset within the flash memory block at which operation is requested.
 *
 * @retval    NRF_SUCCESS    on success, else an error code indicating reason for failure.
 *
 * @note All paramater check should be performed before requesting in an enqueue.
 */
static uint32_t cmd_queue_enqueue(uint8_t             opcode,
                                  pstorage_handle_t * p_storage_addr,
                                  uint8_t           * p_data_addr,
                                  pstorage_size_t     size,
                                  pstorage_size_t     offset)
{
    uint32_t retval;
    uint8_t  write_index = 0;

    if (m_cmd_queue.count != PSTORAGE_CMD_QUEUE_SIZE)
    {
        // Enqueue the command if it is queue is not full.
        write_index = m_cmd_queue.rp + m_cmd_queue.count;

        if (write_index >= PSTORAGE_CMD_QUEUE_SIZE)
        {
            write_index -= PSTORAGE_CMD_QUEUE_SIZE;
        }

        m_cmd_queue.cmd[write_index].op_code      = opcode;
        m_cmd_queue.cmd[write_index].p_data_addr  = p_data_addr;
        m_cmd_queue.cmd[write_index].storage_addr = (*p_storage_addr);
        m_cmd_queue.cmd[write_index].size         = size;
        m_cmd_queue.cmd[write_index].offset       = offset;
        retval                                    = NRF_SUCCESS;
        if (m_cmd_queue.flash_access == false)
        {
            retval = cmd_process();
            if (retval == NRF_ERROR_BUSY)
            {
                // In case of busy error code, it is possible to attempt to access flash.
                retval = NRF_SUCCESS;
            }
        }
        m_cmd_queue.count++;
    }
    else
    {
        retval = NRF_ERROR_NO_MEM;
    }

    return retval;
}


/**
 * @brief Dequeues a command element.
 *
 * @retval    NRF_SUCCESS    on success, else an error code indicating reason for failure.
 */
static uint32_t cmd_queue_dequeue(void)
{
    uint32_t retval;
    retval = NRF_SUCCESS;

    // If any flash operation is enqueued, schedule.
    if (m_cmd_queue.count > 0)
    {
        retval = cmd_process();
        if (retval != NRF_SUCCESS)
        {
            // Flash could be accessed by modules other than Bond Manager, hence a busy error is
            // acceptable, but any other error needs to be indicated to the bond manager.
            if (retval != NRF_ERROR_BUSY)
            {
                app_notify(retval);
            }
            else
            {
                // In case of busy next trigger will be a success or a failure event.
            }
        }
    }
    else
    {
        // No flash access request pending.
    }

    return retval;
}


/**
 * @brief Routine to notify application of any errors.
 *
 * @param[in] result Result of event being notified.
 */
static void app_notify(uint32_t result)
{
    pstorage_ntf_cb_t ntf_cb;
    uint8_t           op_code = m_cmd_queue.cmd[m_cmd_queue.rp].op_code;

#ifdef PSTORAGE_RAW_MODE_ENABLE
    if (m_cmd_queue.cmd[m_cmd_queue.rp].storage_addr.module_id == RAW_MODE_APP_ID)
    {
        ntf_cb = m_raw_app_table.cb;
    }
    else
#endif // PSTORAGE_RAW_MODE_ENABLE
    {
        ntf_cb = m_app_table[m_cmd_queue.cmd[m_cmd_queue.rp].storage_addr.module_id].cb;
    }

    // Indicate result to client.
    // For PSTORAGE_CLEAR_OP_CODE no size is returned as the size field is used only internally
    // for clients registering multiple pages.
    ntf_cb(&m_cmd_queue.cmd[m_cmd_queue.rp].storage_addr,
           op_code,
           result,
           m_cmd_queue.cmd[m_cmd_queue.rp].p_data_addr,
           m_cmd_queue.cmd[m_cmd_queue.rp].size);
}


/**
 * @brief Handles Flash Access Result Events declared in pstorage_platform.h.
 *
 * @param[in] sys_evt System event to be handled.
 */
void pstorage_sys_event_handler(uint32_t sys_evt)
{
    uint32_t retval = NRF_SUCCESS;

    // Its possible the flash access was not initiated by bond manager, hence
    // event is processed only if the event triggered was for an operation requested by the
    // bond manager.
    if (m_cmd_queue.flash_access == true)
    {
        cmd_queue_element_t * p_cmd;

        m_cmd_queue.flash_access = false;

        if (m_swap_state == STATE_SWAP_DIRTY)
        {
            if (sys_evt == NRF_EVT_FLASH_OPERATION_SUCCESS)
            {
                m_swap_state = STATE_INIT;
            }
            else
            {
                // If clearing the swap fails, set the application back to un-initialized, to give
                // the application a chance for a retry.
                m_module_initialized = false;
            }

            // Schedule any queued flash access operations.
            retval = cmd_queue_dequeue();
            if (retval != NRF_SUCCESS)
            {
                app_notify(retval);
            }
            return;
        }

        switch (sys_evt)
        {
            case NRF_EVT_FLASH_OPERATION_SUCCESS:
            {
                p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];
                m_round_val++;

                const bool store_finished =
                    ((p_cmd->op_code == PSTORAGE_STORE_OP_CODE) &&
                     ((m_round_val * SOC_MAX_WRITE_SIZE) >= p_cmd->size));

                const bool update_finished =
                    ((p_cmd->op_code == PSTORAGE_UPDATE_OP_CODE) &&
                     (m_swap_state == STATE_COMPLETE));

                const bool clear_block_finished =
                    ((p_cmd->op_code == PSTORAGE_CLEAR_OP_CODE) &&
                     (m_swap_state == STATE_COMPLETE));

                const bool clear_all_finished =
                    ((p_cmd->op_code == PSTORAGE_CLEAR_OP_CODE) &&
                     ((m_round_val * SOC_MAX_WRITE_SIZE) >= p_cmd->size) &&
                     (m_swap_state == STATE_INIT));

                if (update_finished ||
                    clear_block_finished ||
                    clear_all_finished ||
                    store_finished)
                {
                    m_swap_state = STATE_INIT;

                    app_notify(retval);

                    // Initialize/free the element as it is now processed.
                    cmd_queue_element_init(m_cmd_queue.rp);
                    m_round_val = 0;
                    m_cmd_queue.count--;
                    m_cmd_queue.rp++;

                    if (m_cmd_queue.rp >= PSTORAGE_CMD_QUEUE_SIZE)
                    {
                        m_cmd_queue.rp -= PSTORAGE_CMD_QUEUE_SIZE;
                    }
                }
                // Schedule any queued flash access operations.
                retval = cmd_queue_dequeue();

                if (retval != NRF_SUCCESS)
                {
                    app_notify(retval);
                }
            }
            break;

            case NRF_EVT_FLASH_OPERATION_ERROR:
                app_notify(NRF_ERROR_TIMEOUT);
                break;

            default:
                // No implementation needed.
                break;

        }
    }
}


/** @brief Function for handling flash accesses when using swap.
 *
 * __________________________________________________________
 * |                       Page                             |
 * |________________________________________________________|
 * | head | affected body (to be updated or cleared) | tail |
 * |______|__________________________________________|______|
 *
 * @param[in] p_cmd          Queue element being processed.
 * @param[in] page_number    The affected page number.
 * @param[in] head_word_size Size of the head in number of words.
 * @param[in] tail_word_size Size of the tail in number of words.
 *
 * @retval    NRF_SUCCESS    on success, else an error code indicating reason for failure.
 */
static uint32_t swap_state_process(cmd_queue_element_t * p_cmd,
                                   uint32_t              page_number,
                                   uint32_t              head_word_size,
                                   uint32_t              tail_word_size)
{
    uint32_t retval = NRF_ERROR_INTERNAL;

    // Adjust entry point to state machine if needed. When we update has no head or tail its
    // no need for using the swap.
    if (m_swap_state == STATE_INIT)
    {
        if ((head_word_size == 0) && (tail_word_size == 0))
        {
            // Only skip swap usage if the new data fills a whole flash page.
            m_swap_state = STATE_DATA_ERASE;
        }
        else
        {
            // Else start backing up application data to swap.
            m_swap_state = STATE_DATA_TO_SWAP_WRITE;
        }
    }

    switch (m_swap_state)
    {
        case STATE_DATA_TO_SWAP_WRITE:
            // Backup previous content into swap page.
            retval = sd_flash_write((uint32_t *)(PSTORAGE_SWAP_ADDR),
                                    (uint32_t *)(page_number * PSTORAGE_FLASH_PAGE_SIZE),
                                    PSTORAGE_FLASH_PAGE_SIZE / sizeof(uint32_t));
            if (retval == NRF_SUCCESS)
            {
                m_swap_state = STATE_DATA_ERASE;
            }
            break;

        case STATE_DATA_ERASE:
            // Clear the application data page.
            retval = sd_flash_page_erase(page_number);
            if (retval == NRF_SUCCESS)
            {
                if (head_word_size == 0)
                {
                    if (tail_word_size == 0)
                    {
                        if (p_cmd->op_code == PSTORAGE_CLEAR_OP_CODE)
                        {
                            m_swap_state = STATE_COMPLETE;
                        }
                        else
                        {
                            m_swap_state = STATE_NEW_BODY_WRITE;
                        }
                    }
                    else
                    {
                        m_swap_state = STATE_TAIL_RESTORE;
                    }
                }
                else
                {
                    m_swap_state = STATE_HEAD_RESTORE;
                }
            }
            break;

        case STATE_HEAD_RESTORE:
            // Restore head from swap to application data page.
            retval = sd_flash_write((uint32_t *)(page_number * PSTORAGE_FLASH_PAGE_SIZE),
                                    (uint32_t *)PSTORAGE_SWAP_ADDR,
                                    head_word_size);
            if (retval == NRF_SUCCESS)
            {
                if (tail_word_size == 0)
                {
                    if (p_cmd->op_code == PSTORAGE_CLEAR_OP_CODE)
                    {
                        m_swap_state = STATE_SWAP_ERASE;
                    }
                    else
                    {
                        m_swap_state = STATE_NEW_BODY_WRITE;
                    }
                }
                else
                {
                    m_swap_state = STATE_TAIL_RESTORE;
                }
            }
            break;

        case STATE_TAIL_RESTORE:
            // Restore tail from swap to application data page.
            retval = sd_flash_write((uint32_t *)((page_number * PSTORAGE_FLASH_PAGE_SIZE) +
                                                 (head_word_size * sizeof(uint32_t)) +
                                                 p_cmd->size),
                                    (uint32_t *)(PSTORAGE_SWAP_ADDR +
                                                 (head_word_size * sizeof(uint32_t)) +
                                                 p_cmd->size),
                                    tail_word_size);
            if (retval == NRF_SUCCESS)
            {
                if (p_cmd->op_code == PSTORAGE_CLEAR_OP_CODE)
                {
                    m_swap_state = STATE_SWAP_ERASE;
                }
                else
                {
                    m_swap_state = STATE_NEW_BODY_WRITE;
                }
            }
            break;

        case STATE_NEW_BODY_WRITE:
            // Write new data (body) to application data page.
            retval = sd_flash_write((uint32_t *)((page_number * PSTORAGE_FLASH_PAGE_SIZE) +
                                                 (head_word_size * sizeof(uint32_t))),
                                    (uint32_t *)p_cmd->p_data_addr,
                                    p_cmd->size / sizeof(uint32_t));
            if (retval == NRF_SUCCESS)
            {
                if ((head_word_size == 0) && (tail_word_size == 0))
                {
                    m_swap_state = STATE_COMPLETE;
                }
                else
                {
                    m_swap_state = STATE_SWAP_ERASE;
                }
            }
            break;

        case STATE_SWAP_ERASE:
            // Clear the swap page for subsequent use.
            retval = sd_flash_page_erase(PSTORAGE_SWAP_ADDR / PSTORAGE_FLASH_PAGE_SIZE);
            if (retval == NRF_SUCCESS)
            {
                m_swap_state = STATE_COMPLETE;
            }
            break;

        default:
            break;
    }

    return retval;
}


/**
 * @brief Routine called to actually issue the flash access request to the SoftDevice.
 *
 * @retval    NRF_SUCCESS    on success, else an error code indicating reason for failure.
 */
static uint32_t cmd_process(void)
{
    uint32_t              retval;
    uint32_t              storage_addr;
    cmd_queue_element_t * p_cmd;

    retval = NRF_ERROR_FORBIDDEN;

    p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];

    storage_addr = p_cmd->storage_addr.block_id;

    switch (p_cmd->op_code)
    {
        case PSTORAGE_STORE_OP_CODE:
        {
            uint32_t  size;
            uint32_t  offset;
            uint8_t * p_data_addr = p_cmd->p_data_addr;

            offset        = (m_round_val * SOC_MAX_WRITE_SIZE);
            size          = p_cmd->size - offset;
            p_data_addr  += offset;
            storage_addr += (p_cmd->offset + offset);

            if (size < SOC_MAX_WRITE_SIZE)
            {
                retval = sd_flash_write(((uint32_t *)storage_addr),
                                        (uint32_t *)p_data_addr,
                                        size / sizeof(uint32_t));
            }
            else
            {
                retval = sd_flash_write(((uint32_t *)storage_addr),
                                        (uint32_t *)p_data_addr,
                                        SOC_MAX_WRITE_SIZE / sizeof(uint32_t));
            }
        }
        break;

        case PSTORAGE_CLEAR_OP_CODE:
        {
            // Calculate page number before clearing.
            uint32_t page_number;

            pstorage_size_t block_size =
                m_app_table[p_cmd->storage_addr.module_id].block_size;

            pstorage_size_t block_count =
                m_app_table[p_cmd->storage_addr.module_id].block_count;

            pstorage_block_t base_address =
                m_app_table[p_cmd->storage_addr.module_id].base_id;

            // If the whole module should be cleared.
            if (((base_address == storage_addr) && (block_size * block_count == p_cmd->size)) ||
                (p_cmd->storage_addr.module_id == RAW_MODE_APP_ID))
            {
                page_number = ((storage_addr / PSTORAGE_FLASH_PAGE_SIZE) + m_round_val);

                retval = sd_flash_page_erase(page_number);
            }
            // If one block is to be erased.
            else
            {
                page_number = (storage_addr / PSTORAGE_FLASH_PAGE_SIZE);

                uint32_t head_word_size = (
                    storage_addr -
                    (page_number * PSTORAGE_FLASH_PAGE_SIZE)
                    ) / sizeof(uint32_t);

                uint32_t tail_word_size = (
                    ((page_number + 1) * PSTORAGE_FLASH_PAGE_SIZE) -
                    (storage_addr + p_cmd->size)
                    ) / sizeof(uint32_t);

                retval = swap_state_process(p_cmd,
                                            page_number,
                                            head_word_size,
                                            tail_word_size);
            }
        }
        break;

        case PSTORAGE_UPDATE_OP_CODE:
        {
            uint32_t page_number = (storage_addr / PSTORAGE_FLASH_PAGE_SIZE);

            uint32_t head_word_size = (
                storage_addr + p_cmd->offset -
                (page_number * PSTORAGE_FLASH_PAGE_SIZE)
                ) / sizeof(uint32_t);

            uint32_t tail_word_size = (
                ((page_number + 1) * PSTORAGE_FLASH_PAGE_SIZE) -
                (storage_addr + p_cmd->offset + p_cmd->size)
                ) / sizeof(uint32_t);

            retval = swap_state_process(p_cmd, page_number, head_word_size, tail_word_size);
        }
        break;

        default:
            // Should never reach here.
            break;
    }

    if (retval == NRF_SUCCESS)
    {
        m_cmd_queue.flash_access = true;
    }

    return retval;
}
/** @} */


uint32_t pstorage_init(void)
{
    uint32_t retval;

    cmd_queue_init();

    m_next_app_instance = 0;
    m_next_page_addr    = PSTORAGE_DATA_START_ADDR;
    m_round_val         = 0;

    for (uint32_t index = 0; index < PSTORAGE_NUM_OF_PAGES; index++)
    {
        m_app_table[index].cb           = NULL;
        m_app_table[index].block_size   = 0;
        m_app_table[index].num_of_pages = 0;
        m_app_table[index].block_count  = 0;
    }

#ifdef PSTORAGE_RAW_MODE_ENABLE
    m_raw_app_table.cb           = NULL;
    m_raw_app_table.num_of_pages = 0;
    m_module_initialized         = true;
    m_swap_state                 = STATE_INIT;

    retval = NRF_SUCCESS;
#else
    m_swap_state = STATE_SWAP_DIRTY;

    // Erase swap region in case it is dirty.
    retval = sd_flash_page_erase(PSTORAGE_SWAP_ADDR / PSTORAGE_FLASH_PAGE_SIZE);
    if (retval == NRF_SUCCESS)
    {
        m_cmd_queue.flash_access = true;
        m_module_initialized     = true;
    }
#endif //PSTORAGE_RAW_MODE_ENABLE

    return retval;
}


uint32_t pstorage_register(pstorage_module_param_t * p_module_param,
                           pstorage_handle_t       * p_block_id)
{
    uint16_t page_count;
    uint32_t total_size;

    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_module_param);
    NULL_PARAM_CHECK(p_block_id);
    NULL_PARAM_CHECK(p_module_param->cb);
    BLOCK_SIZE_CHECK(p_module_param->block_size);
    BLOCK_COUNT_CHECK(p_module_param->block_count, p_module_param->block_size);

    // Block size should be a multiple of word size.
    if (!((p_module_param->block_size % sizeof(uint32_t)) == 0))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (m_next_app_instance == PSTORAGE_NUM_OF_PAGES)
    {
        return NRF_ERROR_NO_MEM;
    }

    p_block_id->module_id = m_next_app_instance;
    p_block_id->block_id  = m_next_page_addr;

    m_app_table[m_next_app_instance].base_id     = p_block_id->block_id;
    m_app_table[m_next_app_instance].cb          = p_module_param->cb;
    m_app_table[m_next_app_instance].block_size  = p_module_param->block_size;
    m_app_table[m_next_app_instance].block_count = p_module_param->block_count;

    // Calculate number of flash pages allocated for the device.
    page_count = 0;
    total_size = p_module_param->block_size * p_module_param->block_count;
    do
    {
        page_count++;
        if (total_size > PSTORAGE_FLASH_PAGE_SIZE)
        {
            total_size -= PSTORAGE_FLASH_PAGE_SIZE;
        }
        else
        {
            total_size = 0;
        }
        m_next_page_addr += PSTORAGE_FLASH_PAGE_SIZE;
    }
    while (total_size >= PSTORAGE_FLASH_PAGE_SIZE);

    m_app_table[m_next_app_instance].num_of_pages = page_count;
    m_next_app_instance++;

    return NRF_SUCCESS;
}


uint32_t pstorage_block_identifier_get(pstorage_handle_t * p_base_id,
                                       pstorage_size_t     block_num,
                                       pstorage_handle_t * p_block_id)
{
    pstorage_handle_t temp_id;

    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_base_id);
    NULL_PARAM_CHECK(p_block_id);
    MODULE_ID_RANGE_CHECK(p_base_id);

    temp_id           = (*p_base_id);
    temp_id.block_id += (block_num * MODULE_BLOCK_SIZE(p_base_id));

    BLOCK_ID_RANGE_CHECK(&temp_id);

    (*p_block_id) = temp_id;

    return NRF_SUCCESS;
}


uint32_t pstorage_store(pstorage_handle_t * p_dest,
                        uint8_t           * p_src,
                        pstorage_size_t     size,
                        pstorage_size_t     offset)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_src);
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK(p_dest);
    BLOCK_ID_RANGE_CHECK(p_dest);
    SIZE_CHECK(p_dest, size);
    OFFSET_CHECK(p_dest, offset,size);

    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if ((!is_word_aligned((uint32_t *)p_dest->block_id)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    return cmd_queue_enqueue(PSTORAGE_STORE_OP_CODE, p_dest, p_src, size, offset);
}


uint32_t pstorage_update(pstorage_handle_t * p_dest,
                         uint8_t           * p_src,
                         pstorage_size_t     size,
                         pstorage_size_t     offset)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_src);
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK(p_dest);
    BLOCK_ID_RANGE_CHECK(p_dest);
    SIZE_CHECK(p_dest, size);
    OFFSET_CHECK(p_dest, offset, size);

    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if ((!is_word_aligned((uint32_t *)p_dest->block_id)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    return cmd_queue_enqueue(PSTORAGE_UPDATE_OP_CODE, p_dest, p_src, size, offset);
}


uint32_t pstorage_load(uint8_t           * p_dest,
                       pstorage_handle_t * p_src,
                       pstorage_size_t     size,
                       pstorage_size_t     offset)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_src);
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK(p_src);
    BLOCK_ID_RANGE_CHECK(p_src);
    SIZE_CHECK(p_src, size);
    OFFSET_CHECK(p_src, offset, size);

    // Verify word alignment.
    if ((!is_word_aligned(p_dest)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if ((!is_word_aligned((uint32_t *)p_src->block_id)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    memcpy(p_dest, (((uint8_t *)p_src->block_id) + offset), size);

    m_app_table[p_src->module_id].cb(p_src, PSTORAGE_LOAD_OP_CODE, NRF_SUCCESS, p_dest, size);

    return NRF_SUCCESS;
}


uint32_t pstorage_clear(pstorage_handle_t * p_dest, pstorage_size_t size)
{
    uint32_t retval;

    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK(p_dest);
    BLOCK_ID_RANGE_CHECK(p_dest);

    if ((!is_word_aligned((uint32_t *)p_dest->block_id)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    if (
        !(
            ((p_dest->block_id - m_app_table[p_dest->module_id].base_id) %
             m_app_table[p_dest->module_id].block_size) == 0
            )
        )
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    retval = cmd_queue_enqueue(PSTORAGE_CLEAR_OP_CODE, p_dest, NULL, size, 0);

    return retval;
}


uint32_t pstorage_access_status_get(uint32_t * p_count)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_count);

    (*p_count) = m_cmd_queue.count;

    return NRF_SUCCESS;
}

#ifdef PSTORAGE_RAW_MODE_ENABLE


uint32_t pstorage_raw_register(pstorage_module_param_t * p_module_param,
                               pstorage_handle_t       * p_block_id)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_module_param);
    NULL_PARAM_CHECK(p_block_id);
    NULL_PARAM_CHECK(p_module_param->cb);

    if (m_raw_app_table.cb != NULL)
    {
        return NRF_ERROR_NO_MEM;
    }

    p_block_id->module_id = RAW_MODE_APP_ID;
    m_raw_app_table.cb    = p_module_param->cb;

    return NRF_SUCCESS;
}


uint32_t pstorage_raw_store(pstorage_handle_t * p_dest,
                            uint8_t           * p_src,
                            pstorage_size_t     size,
                            pstorage_size_t     offset)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_src);
    NULL_PARAM_CHECK(p_dest);
    MODULE_RAW_ID_RANGE_CHECK(p_dest);

    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    return cmd_queue_enqueue(PSTORAGE_STORE_OP_CODE, p_dest, p_src, size, offset);
}


uint32_t pstorage_raw_clear(pstorage_handle_t * p_dest, pstorage_size_t size)
{
    uint32_t retval;

    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_dest);
    MODULE_RAW_ID_RANGE_CHECK(p_dest);

    retval = cmd_queue_enqueue(PSTORAGE_CLEAR_OP_CODE, p_dest, NULL, size, 0);

    return retval;
}

#endif // PSTORAGE_RAW_MODE_ENABLE
