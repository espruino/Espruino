/* Copyright (c)  2013 Nordic Semiconductor. All Rights Reserved.
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
#include "nordic_common.h"
#include "nrf_error.h"
#include "ble_flash.h"
#include "app_util.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define no_SUPPORT_MODULES_LARGER_THAN_PAGE

/**
 * @defgroup api_param_check API Parameters check macros.
 *
 * @details Macros that verify parameters passed to the module in the APIs. These macros
 *          could be mapped to nothing in final versions of code to save execution and size.
 *
 * @{
 */
/**
 * @brief Checks if API parameters are null. Mostly used for pointer parameters.
 */
#define NULL_PARAM_CHECK(PARAM)                                                                   \
        if ((PARAM) == NULL)                                                                      \
        {                                                                                         \
            return NRF_ERROR_NULL;                                                                \
        }

/**@brief Verifies the module identifier supplied by the application is within permissible
 *        range.
 */
#define MODULE_ID_RANGE_CHECK(ID)                                                                 \
        if ((PSTORAGE_NUM_OF_PAGES <= ((ID)->module_id)) ||                                       \
            (m_app_table[(ID)->module_id].cb == NULL))                                            \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**@brief Verifies the block identifier supplied by the application is within the permissible
 *        range.
 */
#define BLOCK_ID_RANGE_CHECK(ID)                                                                  \
        if (                                                                                      \
            (                                                                                     \
             m_app_table[(ID)->module_id].base_id                                                 \
             +                                                                                    \
             (                                                                                    \
              m_app_table[(ID)->module_id].block_count                                            \
              *                                                                                   \
              MODULE_BLOCK_SIZE(ID)                                                               \
             )                                                                                    \
            )                                                                                     \
            <=                                                                                    \
            ((ID)->block_id)                                                                      \
           )                                                                                      \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }


/**@brief Verifies the block size requested by the application can be supported by the module. */
#define BLOCK_SIZE_CHECK(X)                                                                       \
        if (((X) > PSTORAGE_MAX_BLOCK_SIZE) || ((X) < PSTORAGE_MIN_BLOCK_SIZE))                   \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**@brief Verifies block size requested by Application in registration API */
#define BLOCK_COUNT_CHECK(COUNT, SIZE)                                                            \
        if (((COUNT) == 0) || ((m_next_page_addr + ((COUNT) *(SIZE)) > PSTORAGE_DATA_END_ADDR)))  \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**@brief Verifies size parameter provided by application in API. */
#define SIZE_CHECK(ID, SIZE)                                                                      \
        if(((SIZE) == 0) || ((SIZE) > MODULE_BLOCK_SIZE(ID)))                                     \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**@brief Verifies offset parameter provided by application in API. */
#define OFFSET_CHECK(ID, OFFSET, SIZE)                                                            \
        if(((SIZE) + (OFFSET)) > MODULE_BLOCK_SIZE(ID))                                           \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

/**@brief SET BASE by masking offset bits for a address within page */
#define PAGE_BASE_ADDR(flash_address)  ((flash_address)&(~((uint32_t)PSTORAGE_FLASH_PAGE_SIZE-1)))

/**@brief Verifies the total module size requested by the application can be supported by the module. */
#define MODULE_SIZE_CHECK(X)                                                                      \
        if ((X) > PSTORAGE_FLASH_PAGE_SIZE)                   									  \
        {                                                                                         \
            return NRF_ERROR_INVALID_PARAM;                                                       \
        }

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

/**@} */

/**
 * @brief   Application registration information.
 *
 * @details Abstracts application specific information that application needs to maintain to be able
 *          to process requests from each one of them.
 *
 */
typedef struct ps_module_table
{
    pstorage_ntf_cb_t      cb;             /**< Callback registered with the module to be notified of any error occurring in persistent memory management */
    pstorage_block_t       base_id;        /**< Base block id assigned to the module */
    uint16_t               block_size;     /**< Size of block for the module */
    uint16_t               block_count;    /**< Number of block requested by application */
    uint16_t               no_of_pages;    /**< Variable to remember how many pages have been allocated for this module. This information is used for clearing of block, so that application does not need to have knowledge of number of pages its using. */
} pstorage_module_table_t;

static pstorage_module_table_t m_app_table[PSTORAGE_NUM_OF_PAGES];        /**< Registered application information table. */

static uint32_t m_next_app_instance;                                      /**< Points to the application module instance that can be allocated next */
static uint32_t m_next_page_addr;                                         /**< Points to the flash address that can be allocated to a module next, this is needed as blocks of a module can span across flash pages. */

static bool     m_module_initialized = false;                             /**< Flag for checking if module has been initialized. */


/**
 * @brief Routine to notify application of any errors.
 *
 * @param[in] result Result of event being notified.
 */
static __INLINE void app_notify(pstorage_handle_t    * p_handle,
                                uint8_t              * p_addr,
                                uint8_t                op_code,
                                pstorage_size_t        size,
                                uint32_t               result)
{
    pstorage_ntf_cb_t  ntf_cb;

    ntf_cb = m_app_table[p_handle->module_id].cb;

    // Indicate result to client.
    // For PSTORAGE_CLEAR_OP_CODE no size is returned as the size field is used only internally
    // for clients registering multiple pages.
    ntf_cb(p_handle,
           op_code,
           result,
           p_addr,
           size);
}


uint32_t pstorage_init(void)
{
    m_next_app_instance  = 0;
    m_next_page_addr     = PSTORAGE_DATA_START_ADDR;
    m_module_initialized = true;
    return NRF_SUCCESS;
}


uint32_t pstorage_register(pstorage_module_param_t * p_module_param,
                           pstorage_handle_t       * p_block_id)
{


    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_module_param);
    NULL_PARAM_CHECK(p_block_id);
    NULL_PARAM_CHECK(p_module_param->cb);
    BLOCK_SIZE_CHECK(p_module_param->block_size);
    BLOCK_COUNT_CHECK(p_module_param->block_count, p_module_param->block_size);

#ifndef SUPPORT_MODULES_LARGER_THAN_PAGE
    MODULE_SIZE_CHECK(((uint16_t)(p_module_param->block_size)*(p_module_param->block_count)));
#endif


    if (m_next_app_instance == PSTORAGE_NUM_OF_PAGES)
    {
        return NRF_ERROR_NO_MEM;
    }

    p_block_id->module_id                        = m_next_app_instance;
    p_block_id->block_id                         = m_next_page_addr;

    m_app_table[m_next_app_instance].base_id     = p_block_id->block_id;
    m_app_table[m_next_app_instance].cb          = p_module_param->cb;
    m_app_table[m_next_app_instance].block_size  = p_module_param->block_size;
    m_app_table[m_next_app_instance].block_count = p_module_param->block_count;

    // Calculate number of flash pages allocated for the device.


#ifdef SUPPORT_MODULES_LARGER_THAN_PAGE
    uint16_t page_count = 0;
    uint32_t total_size;
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

    } while (total_size >= PSTORAGE_FLASH_PAGE_SIZE);
    m_app_table[m_next_app_instance].no_of_pages = page_count;
#else
    m_app_table[m_next_app_instance].no_of_pages = 1;
    m_next_page_addr += PSTORAGE_FLASH_PAGE_SIZE;
#endif
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
    OFFSET_CHECK(p_dest, offset, size);

    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    uint32_t storage_addr = p_dest->block_id + offset;

    uint32_t retval = ble_flash_block_write((uint32_t *)storage_addr,
                                            (uint32_t *)p_src,
                                            (size /sizeof(uint32_t)));

    app_notify(p_dest, p_src, PSTORAGE_STORE_OP_CODE, size, retval);
    
    return retval;
}

/** @brief Function for handling flash updates using swap page
 *
 * __________________________________________________
 * |              Page1                            |
 * |_______________________________________________|
 * | head |    body (to be updated )        | tail |
 * |______|_________________________________|______|
 *
 * ________________________________________________________________________________________
 * |              Page1                            |             Page2                    |
 * |_______________________________________________|______________________________________|
 * | head           |          body (to be updated )         | tail                       |
 * |________________|________________________________________|____________________________|
 *
 *
 * ToDo:  pointers not uint32_t for addresses
 *
 */

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

    uint32_t *p_swap_addr = (uint32_t *)PSTORAGE_SWAP_ADDR;
    uint32_t *p_page_addr1 =  (uint32_t *) PAGE_BASE_ADDR(p_dest->block_id + offset);
#ifdef SUPPORT_MODULES_LARGER_THAN_PAGE
    uint32_t *p_page_addr2 =  (uint32_t *) PAGE_BASE_ADDR(p_dest->block_id + offset + size-1);
#endif
    // block may not be aligned with page, so head must include all trailing blocks
    uint16_t head_word_count =  (uint32_t)((uint32_t *)p_dest->block_id - p_page_addr1) + (offset)/sizeof(uint32_t);
    uint16_t body_word_count =  size/sizeof(uint32_t);
    uint16_t tail_word_count;
    uint32_t retval;

    // Verify word alignment.
    if ((!is_word_aligned(p_src)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    // erase swap page
    (void) ble_flash_page_erase(PSTORAGE_SWAP_ADDR / BLE_FLASH_PAGE_SIZE);
#ifdef SUPPORT_MODULES_LARGER_THAN_PAGE
    if (p_page_addr1 == p_page_addr2)
    {
#endif
    	// tail is in the same page as head
        tail_word_count = BLE_FLASH_PAGE_SIZE/sizeof(uint32_t) - head_word_count - body_word_count;
    	// copy of the head
        if (head_word_count)
        {
        	retval = ble_flash_block_write(p_swap_addr, p_page_addr1, head_word_count );
        	if (retval != NRF_SUCCESS)
        	{
        		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
        		return retval;
        	}
        }
        // copy of the body
        retval = ble_flash_block_write(p_swap_addr+head_word_count, (uint32_t *)p_src, body_word_count);
        if (retval != NRF_SUCCESS)
        {
          app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
          return retval;
        }
        // copy of the tail
        if (tail_word_count)
        {
        	retval = ble_flash_block_write(p_swap_addr+head_word_count+body_word_count, p_page_addr1+head_word_count+body_word_count, tail_word_count );
        	if (retval != NRF_SUCCESS)
        	{
        		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
        		return retval;
        	}
        }
        // erase active page
        (void) ble_flash_page_erase((uint32_t)p_page_addr1 / BLE_FLASH_PAGE_SIZE);
        // restore updated page
        retval = ble_flash_block_write(p_page_addr1, p_swap_addr, BLE_FLASH_PAGE_SIZE/sizeof(uint32_t));
#ifdef SUPPORT_MODULES_LARGER_THAN_PAGE
    }
    else
    {
    	// tail is in NOT in the same page as head - need to swap twice
    	uint16_t body1_word_count =  BLE_FLASH_PAGE_SIZE/sizeof(uint32_t) - head_word_count;
    	uint16_t body2_word_count =  body_word_count - body1_word_count;
        tail_word_count = BLE_FLASH_PAGE_SIZE/sizeof(uint32_t) - body2_word_count;
        // copy head
        retval = ble_flash_block_write(p_swap_addr, p_page_addr1, head_word_count );
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
        // copy body1
        retval = ble_flash_block_write(p_swap_addr, (uint32_t *)p_src, body1_word_count );
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
        // erase active page
        (void) ble_flash_page_erase((uint32_t)p_page_addr1 / BLE_FLASH_PAGE_SIZE);
        // restore updated page1
        retval = ble_flash_block_write(p_page_addr1, p_swap_addr, BLE_FLASH_PAGE_SIZE/sizeof(uint32_t));
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
        // erase swap page
        (void) ble_flash_page_erase(PSTORAGE_SWAP_ADDR / BLE_FLASH_PAGE_SIZE);
        // copy body2
        retval = ble_flash_block_write(p_swap_addr, (uint32_t *)p_src + body1_word_count, body2_word_count );
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
        // copy tail
        retval = ble_flash_block_write(p_swap_addr, p_page_addr2+body2_word_count, tail_word_count );
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
        // erase active page
        (void) ble_flash_page_erase((uint32_t)p_page_addr2 / BLE_FLASH_PAGE_SIZE);
        // restore updated page2
        retval = ble_flash_block_write(p_page_addr2, p_swap_addr, BLE_FLASH_PAGE_SIZE/sizeof(uint32_t));
    	if (retval != NRF_SUCCESS)
    	{
    		app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    		return retval;
    	}
    }
#endif
    app_notify(p_dest, p_src, PSTORAGE_UPDATE_OP_CODE, size, retval);
    return retval;

}

uint32_t pstorage_load(uint8_t *           p_dest,
                       pstorage_handle_t * p_src,
                       pstorage_size_t     size,
                       pstorage_size_t     offset)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_src);
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK (p_src);
    BLOCK_ID_RANGE_CHECK(p_src);
    SIZE_CHECK(p_src,size);
    OFFSET_CHECK(p_src,offset,size);

    // Verify word alignment.
    if ((!is_word_aligned(p_dest)) || (!is_word_aligned((void *)(uint32_t)offset)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    memcpy(p_dest, (((uint8_t *)p_src->block_id) + offset), size);

    app_notify(p_src, p_dest, PSTORAGE_LOAD_OP_CODE, size, NRF_SUCCESS);

    return NRF_SUCCESS;
}


uint32_t pstorage_clear(pstorage_handle_t * p_dest, pstorage_size_t size)
{
    uint32_t page_addr;
    uint32_t retval;
    uint16_t page_count;

    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_dest);
    MODULE_ID_RANGE_CHECK(p_dest);

    page_addr = p_dest->block_id / BLE_FLASH_PAGE_SIZE;

    retval = NRF_SUCCESS;

    for (page_count = 0; page_count < m_app_table[p_dest->module_id].no_of_pages; page_count++)
    {
        retval = ble_flash_page_erase(page_addr);
        page_addr++;
        if (retval != NRF_SUCCESS)
        {
            break;
        }
    }
    app_notify(p_dest, NULL, PSTORAGE_CLEAR_OP_CODE, size, retval);
    return retval;
}

void pstorage_sys_event_handler(uint32_t sys_evt)
{

}

uint32_t pstorage_access_status_get(uint32_t * p_count)
{
    if (p_count)
    {
        *p_count = 0;
    }
    return NRF_SUCCESS;
}
