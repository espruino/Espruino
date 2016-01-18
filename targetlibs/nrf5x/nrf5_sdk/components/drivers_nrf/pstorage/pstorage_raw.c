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

#include "pstorage.h"
#include <stdlib.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "app_util.h"

/** @file
 *
 * @defgroup persistent_storage_raw Persistent Storage Interface - Raw Mode Implementation
 * @{
 * @ingroup persistent_storage
 * @brief Persistent Storage Interface - Raw Mode Implementation.
 *
 * @details This file contains the source code for raw mode implementation of pstorage.
 * It is intended for special use cases where flash size is critical or the application must have
 * full control of the flash, such as DFU. The registration function in this implementation only
 * allocates a module id for the queue but does not locate any flash pages for the registrant.
 * This implementation provides no safety checking of addresses when clearing or storing data into
 * flash. The application is responsible for handling flash addresses and care must therefore be 
 * taken in application not to erase application area.
 * This implementation does not support the @ref pstorage_update function.
 */

#define INVALID_OPCODE              0x00    /**< Invalid op code identifier. */

#ifdef NRF51
#define SOC_MAX_WRITE_SIZE          1024    /**< Maximum write size allowed for a single call to \ref sd_flash_write as specified in the SoC API on the nRF51. */
#elif NRF52
#define SOC_MAX_WRITE_SIZE          4096    /**< Maximum write size allowed for a single call to \ref sd_flash_write as specified in the SoC API on the nRF52. */
#else
#error No target defined
#endif


/**
 * @brief Application registration information.
 *
 * @details Define application specific information that application needs to maintain to be able
 *          to process requests from each one of them.
 */
typedef struct
{
    pstorage_ntf_cb_t cb;   /**< Callback registered with the module to be notified of result of flash access.  */
} pstorage_module_table_t;


/**
 * @brief Defines command queue element.
 *
 * @details Defines command queue element. Each element encapsulates needed information to process
 *          a flash access command.
 */
typedef struct
{
    uint8_t              op_code;       /**< Identifies flash access operation being queued. Element is free is op-code is INVALID_OPCODE */
    pstorage_size_t      size;          /**< Identifies size in bytes requested for the operation. */
    pstorage_size_t      offset;        /**< Offset requested by the application for access operation. */    
    pstorage_handle_t    storage_addr;  /**< Address/Identifier for persistent memory. */
    uint8_t            * p_data_addr;   /**< Address/Identifier for data memory. This is assumed to be resident memory. */    
} cmd_queue_element_t;


/**
 * @brief Defines command queue, an element is free is op_code field is not invalid.
 *
 * @details Defines commands enqueued for flash access. At any point of time, this queue has one or
 *          more flash access operation pending if the count field is not zero. When the queue is
 *          not empty, the rp (read pointer) field points to the flash access command in progress
 *          or to requested next. The queue implements a simple first in first out algorithm.
 *          Data addresses are assumed to be resident.
 */
typedef struct
{
    uint8_t              rp;                              /**< Read pointer, pointing to flash access that is ongoing or to be requested next. */
    uint8_t              count;                           /**< Number of elements in the queue.  */
    bool                 flash_access;                    /**< Flag to ensure an flash event received is for an request issued by the module. */
    cmd_queue_element_t  cmd[PSTORAGE_CMD_QUEUE_SIZE];    /**< Array to maintain flash access operation details */
}cmd_queue_t;

static cmd_queue_t             m_cmd_queue;                             /**< Flash operation request queue. */
static pstorage_module_table_t m_app_table[PSTORAGE_NUM_OF_PAGES];      /**< Registered application information table. */
static pstorage_size_t         m_next_app_instance;                     /**< Points to the application module instance that can be allocated next */
static pstorage_size_t         m_round_val;                             /**< Round value for multiple round operations. For erase operations, the round value will contain current round counter which is identical to number of pages erased. For store operations, the round value contains current round of operation * SOC_MAX_WRITE_SIZE to ensure each store to the SoC Flash API is within the SoC limit. */

/**
 * @brief Function for processing of commands and issuing flash access request to the SoftDevice.
 *
 * @return The return value received from SoftDevice.
 */
static uint32_t cmd_process(void);


/**
 * @brief Function for notifying application of any errors.
 *
 * @param[in] result Result of event being notified.
 * @param[in] p_elem Pointer to the element for which a notification should be given.
 */
static void app_notify(uint32_t result, cmd_queue_element_t * p_elem);


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
 * @brief Function for initializing a command queue element.
 *
 * @param[in] index Index identifying element to be initialized.
 */
static void cmd_queue_element_init(uint32_t index)
{
    // Internal function and checks on range of index can be avoided
    m_cmd_queue.cmd[index].op_code                = INVALID_OPCODE;
    m_cmd_queue.cmd[index].size                   = 0;
    m_cmd_queue.cmd[index].storage_addr.module_id = PSTORAGE_NUM_OF_PAGES;
    m_cmd_queue.cmd[index].storage_addr.block_id  = 0;
    m_cmd_queue.cmd[index].p_data_addr            = NULL;
    m_cmd_queue.cmd[index].offset                 = 0;
}


/**
 * @brief Function for initializing the command queue.
 */
static void cmd_queue_init(void)
{
    uint32_t cmd_index;

    m_round_val              = 0;
    m_cmd_queue.rp           = 0;
    m_cmd_queue.count        = 0;
    m_cmd_queue.flash_access = false;

    for(cmd_index = 0; cmd_index < PSTORAGE_CMD_QUEUE_SIZE; cmd_index++)
    {
        cmd_queue_element_init(cmd_index);
    }
}


/**
 * @brief Function for enqueueing a flash access operation.
 *
 * @param[in] opcode         Operation code for the command to queue.
 * @param[in] p_storage_addr Pointer to the destination address.
 * @param[in] p_data_addr    Pointer to the source address containing the data.
 * @param[in] size           Size of data clear or write.
 * @param[in] offset         Offset to the address identified by the source data address.
 *
 * @retval NRF_SUCCESS      If the enqueueing succeeded.
 * @retval NRF_ERROR_NO_MEM In case the queue is full.
 * @return Any error returned by the SoftDevice flash API.
 */
static uint32_t cmd_queue_enqueue(uint8_t             opcode,
                                  pstorage_handle_t * p_storage_addr,
                                  uint8_t           * p_data_addr,
                                  pstorage_size_t     size,
                                  pstorage_size_t     offset)
{
    uint32_t retval;

    if (m_cmd_queue.count != PSTORAGE_CMD_QUEUE_SIZE)
    {
        uint8_t write_index = m_cmd_queue.rp + m_cmd_queue.count;

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
 * @brief Function for dequeueing a command element.
 *
 * @retval NRF_SUCCESS If the dequeueing succeeded and next command was processed.
 * @return Any error returned by the SoftDevice flash API.
 */
static uint32_t cmd_queue_dequeue(void)
{
    uint32_t retval = NRF_SUCCESS;

    // If any flash operation is enqueued, schedule
    if ((m_cmd_queue.count > 0) && (m_cmd_queue.flash_access == false))
    {
        retval = cmd_process();
        if (retval != NRF_SUCCESS)
        {
            // Flash could be accessed by other modules, hence a busy error is
            // acceptable, but any other error needs to be indicated.
            if (retval == NRF_ERROR_BUSY)
            {
                // In case of busy error code, it is possible to attempt to access flash.
                retval = NRF_SUCCESS;
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
 * @brief Function for notifying application of any errors.
 *
 * @param[in] result Result of event being notified.
 * @param[in] p_elem Pointer to the element for which a notification should be given.
 */
static void app_notify(uint32_t result, cmd_queue_element_t * p_elem)
{
    pstorage_ntf_cb_t ntf_cb;
    uint8_t           op_code = p_elem->op_code;
    
    ntf_cb = m_app_table[p_elem->storage_addr.module_id].cb;

    // Indicate result to client.
    ntf_cb(&p_elem->storage_addr,
           op_code,
           result,
           p_elem->p_data_addr,
           p_elem->size);
}


/**
 * @brief Function for handling of system events from SoftDevice.
 *
 * @param[in] sys_evt System event received.
 */
void pstorage_sys_event_handler(uint32_t sys_evt)
{
    uint32_t retval = NRF_SUCCESS;

    // The event shall only be processed if requested by this module.
    if (m_cmd_queue.flash_access == true)
    {
        cmd_queue_element_t * p_cmd;
        m_cmd_queue.flash_access = false;
        switch (sys_evt)
        {
            case NRF_EVT_FLASH_OPERATION_SUCCESS:
            {
                p_cmd = &m_cmd_queue.cmd[m_cmd_queue.rp];
                m_round_val++;
            
                bool command_finished = ((m_round_val * SOC_MAX_WRITE_SIZE) >= p_cmd->size);

                if (command_finished)
                {
                    uint8_t queue_rp = m_cmd_queue.rp;
                    
                    m_round_val = 0;
                    m_cmd_queue.count--;
                    m_cmd_queue.rp++;

                    if (m_cmd_queue.rp >= PSTORAGE_CMD_QUEUE_SIZE)
                    {
                        m_cmd_queue.rp -= PSTORAGE_CMD_QUEUE_SIZE;
                    }

                    app_notify(retval, &m_cmd_queue.cmd[queue_rp]);

                    // Initialize/free the element as it is now processed.
                    cmd_queue_element_init(queue_rp);
                }
                // Schedule any queued flash access operations.
                retval = cmd_queue_dequeue();
                if (retval != NRF_SUCCESS)
                {
                    app_notify(retval, &m_cmd_queue.cmd[m_cmd_queue.rp]);
                }
            }
            break;
                
            case NRF_EVT_FLASH_OPERATION_ERROR:
                app_notify(NRF_ERROR_TIMEOUT, &m_cmd_queue.cmd[m_cmd_queue.rp]);
                break;
            
            default:
                // No implementation needed.
                break;
        }
    }
}


/**
 * @brief Function for processing of commands and issuing flash access request to the SoftDevice.
 *
 * @return The return value received from SoftDevice.
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
            uint32_t page_number;

            page_number =  ((storage_addr / PSTORAGE_FLASH_PAGE_SIZE) +
                            m_round_val);

            retval = sd_flash_page_erase(page_number);
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
    cmd_queue_init();
    
    m_next_app_instance = 0;
    m_round_val         = 0;

    for(unsigned int index = 0; index < PSTORAGE_NUM_OF_PAGES; index++)
    {
        m_app_table[index].cb          = NULL;
    }

    return NRF_SUCCESS;
}


uint32_t pstorage_register(pstorage_module_param_t * p_module_param,
                           pstorage_handle_t       * p_block_id)
{
    if (m_next_app_instance == PSTORAGE_NUM_OF_PAGES)
    {
        return NRF_ERROR_NO_MEM;
    }

    p_block_id->module_id                 = m_next_app_instance;
    m_app_table[m_next_app_instance++].cb = p_module_param->cb;

    return NRF_SUCCESS;
}


uint32_t pstorage_block_identifier_get(pstorage_handle_t * p_base_id,
                                       pstorage_size_t     block_num,
                                       pstorage_handle_t * p_block_id)
{
    return NRF_ERROR_NOT_SUPPORTED;
}


uint32_t pstorage_store(pstorage_handle_t * p_dest,
                        uint8_t           * p_src,
                        pstorage_size_t     size,
                        pstorage_size_t     offset)
{
    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned(p_src+offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    return cmd_queue_enqueue(PSTORAGE_STORE_OP_CODE, p_dest, p_src, size, offset);
}


uint32_t pstorage_clear(pstorage_handle_t * p_dest, pstorage_size_t size)
{
    return cmd_queue_enqueue(PSTORAGE_CLEAR_OP_CODE, p_dest, NULL , size, 0);
}


/**
 * @}
 */
