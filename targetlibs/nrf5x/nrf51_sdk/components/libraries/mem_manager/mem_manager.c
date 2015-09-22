/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
#include "nrf51.h"
#include "sdk_config.h"
#include "sdk_common.h"
#include "mem_manager.h"
#include "app_trace.h"

/**
 * @defgroup mem_manager_log Module's Log Macros
 *
 * @details Macros used for creating module logs which can be useful in understanding handling
 *          of events or actions on API requests. These are intended for debugging purposes and
 *          can be disabled by defining the MEM_MANAGER_DISABLE_LOGS.
 *
 * @note That if ENABLE_DEBUG_LOG_SUPPORT is disabled, having MEM_MANAGER_DISABLE_LOGS has no effect.
 * @{
 */
#if (MEM_MANAGER_DISABLE_LOGS == 0)

#define MM_LOG  app_trace_log                                                                       /**< Used for logging details. */
#define MM_ERR  app_trace_log                                                                       /**< Used for logging errors in the module. */
#define MM_TRC  app_trace_log                                                                       /**< Used for getting trace of execution in the module. */
#define MM_DUMP app_trace_dump                                                                      /**< Used for dumping octet information to get details of bond information etc. */

#else //MEM_MANAGER_DISABLE_LOGS

#define MM_DUMP(...)                                                                                /**< Disables dumping of octet streams. */
#define MM_LOG(...)                                                                                 /**< Disables detailed logs. */
#define MM_ERR(...)                                                                                 /**< Disables error logs. */
#define MM_TRC(...)                                                                                 /**< Disables traces. */

#endif //MEM_MANAGER_DISABLE_LOGS
/** @} */

/**
 * @defgroup device_manager_mutex_lock_unlock Module's Mutex Lock/Unlock Macros.
 *
 * @details Macros used to lock and unlock modules. Currently the SDK does not use mutexes but
 *          framework is provided in case need arises to use an alternative architecture.
 * @{
 */
#define MM_MUTEX_LOCK()   SDK_MUTEX_LOCK(m_mm_mutex)                                                /**< Lock module using mutex. */
#define MM_MUTEX_UNLOCK() SDK_MUTEX_UNLOCK(m_mm_mutex)                                              /**< Unlock module using mutex. */
/** @} */

#if (MEM_MANAGER_DISABLE_API_PARAM_CHECK == 0)

/**@brief Macro for verifying NULL parameters are not passed to API.
 *
 * @param[in] PARAM Parameter checked for NULL.
 *
 * @retval (NRF_ERROR_NULL | MEMORY_MANAGER_ERR_BASE) when @ref PARAM is NULL.
 */
#define NULL_PARAM_CHECK(PARAM)                            \
    if ((PARAM) == NULL)                                   \
    {                                                      \
        return (NRF_ERROR_NULL | MEMORY_MANAGER_ERR_BASE); \
    }


/**@brief Macro for verifying module's initialization status.
 *
 * @retval (NRF_ERROR_INVALID_STATE | MEMORY_MANAGER_ERR_BASE) when module is not initialized.
 */
#define VERIFY_MODULE_INITIALIZED()                                     \
    do                                                                  \
    {                                                                   \
        if (!m_module_initialized)                                      \
        {                                                               \
            return (NRF_ERROR_INVALID_STATE | MEMORY_MANAGER_ERR_BASE); \
        }                                                               \
    } while (0)

#define VERIFY_REQUESTED_SIZE(SIZE)                                     \
    do                                                                  \
    {                                                                   \
        if (((SIZE) == 0) ||((SIZE) >  MAX_MEM_SIZE))                   \
        {                                                               \
            return (NRF_ERROR_INVALID_PARAM | MEMORY_MANAGER_ERR_BASE); \
        }                                                               \
    } while (0)

/**@} */
#else  //MEM_MANAGER_DISABLE_API_PARAM_CHECK

#define NULL_PARAM_CHECK(PARAM)
#define VERIFY_MODULE_INITIALIZED()
#define VERIFY_REQUESTED_SIZE(SIZE)

#endif //MEM_MANAGER_DISABLE_API_PARAM_CHECK

#define BLOCK_CAT_COUNT                3                                                            /**< Block category count is 3 (small, medium and large). Having one of the block count to zero has no impact on this count. */
#define BLOCK_CAT_SMALL                0                                                            /**< Small category identifier. */
#define BLOCK_CAT_MEDIUM               1                                                            /**< Medium category identifier. */
#define BLOCK_CAT_LARGE                2                                                            /**< Large category identifier. */


/** Memory block type. */
typedef struct
{
   uint8_t   * p_block;                                                                             /**< Pointer to memory region of the block. */
   bool        is_free;                                                                             /**< Indicates whether the memory region has been assigned or is free. */
   uint8_t     block_cat;                                                                           /**< Identifies to which category the block belongs, small, large or medium. */
}mem_block_t;

/** Based on which blocks are defined, MAX_MEM_SIZE is determined.
    Also, in case none of these are defined, a compile time error is indicated. */
#if (MEMORY_MANAGER_LARGE_BLOCK_COUNT != 0)
    #define MAX_MEM_SIZE MEMORY_MANAGER_LARGE_BLOCK_SIZE
#elif (MEMORY_MANAGER_MEDIUM_BLOCK_COUNT != 0)
    #define MAX_MEM_SIZE MEMORY_MANAGER_MEDIUM_BLOCK_SIZE
#elif (MEMORY_MANAGER_SMALL_BLOCK_COUNT != 0)
    #define MAX_MEM_SIZE MEMORY_MANAGER_SMALL_BLOCK_SIZE
#else
    #err "One of MEMORY_MANAGER_SMALL_BLOCK_COUNT, MEMORY_MANAGER_MEDIUM_BLOCK_COUNT or     \
         or MEMORY_MANAGER_LARGE_BLOCK_COUNT should be defined."
#endif


/** Total count of block managed by the module. */
#define TOTAL_BLOCK_COUNT (MEMORY_MANAGER_SMALL_BLOCK_COUNT +                                       \
                           MEMORY_MANAGER_MEDIUM_BLOCK_COUNT +                                      \
                           MEMORY_MANAGER_LARGE_BLOCK_COUNT)


#define TOTAL_MEMORY_SIZE ((MEMORY_MANAGER_SMALL_BLOCK_COUNT * MEMORY_MANAGER_SMALL_BLOCK_SIZE)   + \
                           (MEMORY_MANAGER_MEDIUM_BLOCK_COUNT * MEMORY_MANAGER_MEDIUM_BLOCK_SIZE) + \
                           (MEMORY_MANAGER_LARGE_BLOCK_COUNT  * MEMORY_MANAGER_LARGE_BLOCK_SIZE))


static uint8_t m_memory[TOTAL_MEMORY_SIZE];                                                         /**< Memory managed by the module. */

static mem_block_t m_mem_pool[TOTAL_BLOCK_COUNT];                                                   /**< Pool of memory blocks managed by the module. */

static const uint32_t m_block_size[BLOCK_CAT_COUNT] =                                               /**< Lookup table used to know the max size of block */
{
    MEMORY_MANAGER_SMALL_BLOCK_SIZE,
	MEMORY_MANAGER_MEDIUM_BLOCK_SIZE,
	MEMORY_MANAGER_LARGE_BLOCK_SIZE
};

SDK_MUTEX_DEFINE(m_mm_mutex)                                                                        /**< Mutex variable. Currently unused, this declaration does not occupy any space in RAM. */
#if (MEM_MANAGER_DISABLE_API_PARAM_CHECK == 0)
static bool     m_module_initialized = false;                                                       /**< State indicating if module is initialized or not. */
#endif // MEM_MANAGER_DISABLE_API_PARAM_CHECK



/**@brief Initializes the block by setting it to be free. */
static __INLINE void block_init (mem_block_t * p_block)
{
    p_block->is_free = true;
}


uint32_t nrf51_sdk_mem_init(void)
{
    MM_LOG("[MM]: >> nrf51_sdk_mem_init.\r\n");

    SDK_MUTEX_INIT(m_mm_mutex);

    MM_MUTEX_LOCK();

    uint32_t index = 0;
    uint32_t pool_block_size;
    uint8_t  * p_memory = m_memory;
    uint32_t block_count = 0;

#if (MEMORY_MANAGER_SMALL_BLOCK_COUNT != 0)

    pool_block_size = MEMORY_MANAGER_SMALL_BLOCK_SIZE;
    block_count    += MEMORY_MANAGER_SMALL_BLOCK_COUNT;
	
    for(; index < block_count; index++)
    {
        block_init(&m_mem_pool[index]);
        m_mem_pool[index].p_block    = p_memory;
        p_memory                    += pool_block_size;
		m_mem_pool[index].block_cat  = BLOCK_CAT_SMALL;        
    }
#endif // MEMORY_MANAGER_SMALL_BLOCK_COUNT

#if (MEMORY_MANAGER_MEDIUM_BLOCK_COUNT != 0)

    pool_block_size = MEMORY_MANAGER_MEDIUM_BLOCK_SIZE;
    block_count    += MEMORY_MANAGER_MEDIUM_BLOCK_COUNT;
	
    for(; index < block_count; index++)
    {
        block_init(&m_mem_pool[index]);
        m_mem_pool[index].p_block   = p_memory;
        p_memory                   += pool_block_size;
		m_mem_pool[index].block_cat = BLOCK_CAT_MEDIUM;
    }
#endif // MEMORY_MANAGER_MEDIUM_BLOCK_COUNT

#if (MEMORY_MANAGER_LARGE_BLOCK_COUNT != 0)

    pool_block_size = MEMORY_MANAGER_LARGE_BLOCK_SIZE;
    block_count    += MEMORY_MANAGER_LARGE_BLOCK_COUNT;
	
    for(; index <block_count; index++)
    {
        block_init(&m_mem_pool[index]);
        m_mem_pool[index].p_block   = p_memory;
        p_memory                   += pool_block_size;
		m_mem_pool[index].block_cat = BLOCK_CAT_LARGE;
    }
#endif // MEMORY_MANAGER_LARGE_BLOCK_COUNT

#if (MEM_MANAGER_DISABLE_API_PARAM_CHECK == 0)
    m_module_initialized = true;
#endif // MEM_MANAGER_DISABLE_API_PARAM_CHECK

    MM_MUTEX_UNLOCK();

    MM_LOG("[MM]: << nrf51_sdk_mem_init.\r\n");

    return NRF_SUCCESS;
}


uint32_t nrf51_sdk_mem_alloc(uint8_t ** pp_buffer, uint32_t * p_size)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(pp_buffer);
    NULL_PARAM_CHECK(p_size);
	
	const uint32_t requested_size = (*p_size);	
    VERIFY_REQUESTED_SIZE(requested_size);

    MM_LOG("[MM]: >> nrf51_sdk_mem_alloc, size 0x%04lX.\r\n", requested_size);

    MM_MUTEX_LOCK();

    uint32_t err_code = (NRF_ERROR_NO_MEM | MEMORY_MANAGER_ERR_BASE);
    uint32_t index;

    // Check which block size is best suited for requested memory size.
    if (requested_size <= MEMORY_MANAGER_SMALL_BLOCK_SIZE)
    {
        index = 0;
    }
    else if(requested_size <= MEMORY_MANAGER_MEDIUM_BLOCK_SIZE)
    {
       index = MEMORY_MANAGER_SMALL_BLOCK_COUNT;
    }
    else
    {
       index = (MEMORY_MANAGER_SMALL_BLOCK_COUNT + MEMORY_MANAGER_MEDIUM_BLOCK_COUNT);
    }

    MM_LOG("[MM]: Start index for the pool = 0x%08lX, total block count 0x%08X\r\n",
           index, TOTAL_BLOCK_COUNT);

    for (; index < TOTAL_BLOCK_COUNT; index++)
    {
        if (m_mem_pool[index].is_free == true)
        {
            MM_LOG("[MM]: Assigning block 0x%08lX\r\n", index);
            // Found a free block of memory, assign.
            m_mem_pool[index].is_free = false;
            (*pp_buffer)              = m_mem_pool[index].p_block;
            err_code                  = NRF_SUCCESS;
			(*p_size)                 = m_block_size[m_mem_pool[index].block_cat];
            break;
        }
    }

    MM_MUTEX_UNLOCK();

    MM_LOG("[MM]: << nrf51_sdk_mem_alloc %p, result 0x%08lX.\r\n", (*pp_buffer), err_code);

    return err_code;
}


uint32_t nrf51_sdk_mem_free(uint8_t * p_buffer)
{
    VERIFY_MODULE_INITIALIZED();
    NULL_PARAM_CHECK(p_buffer);

    MM_LOG("[MM]: >> nrf51_sdk_mem_free %p.\r\n", p_buffer);

    MM_MUTEX_LOCK();
    uint32_t err_code = (NRF_ERROR_INVALID_ADDR | MEMORY_MANAGER_ERR_BASE);
    uint32_t index;

    for (index = 0; index < TOTAL_BLOCK_COUNT; index++)
    {
        if (m_mem_pool[index].p_block == p_buffer)
        {
            // Found a free block of memory, assign.
            block_init(&m_mem_pool[index]);
            err_code = NRF_SUCCESS;
            break;
        }
    }

    MM_MUTEX_UNLOCK();

    MM_LOG("[MM]: << nrf51_sdk_mem_free, result 0x%08lX.\r\n", err_code);
    return err_code;
}
