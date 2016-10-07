/**
 *
 * @defgroup mem_manager_config Dynamic memory allocator configuration
 * @{
 * @ingroup mem_manager
 */
/** @brief Enabling mem_manager module *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEM_MANAGER_ENABLED

/** @brief Size of each memory blocks identified as 'small' block. *
 *  Minimum value: 0
 *  Maximum value: 255
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_SMALL_BLOCK_COUNT


/** @brief  Size of each memory blocks identified as 'small' block.
 *
 *  Size of each memory blocks identified as 'small' block. Memory block are recommended to be word-sized.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_SMALL_BLOCK_SIZE


/** @brief Size of each memory blocks identified as 'medium' block. *
 *  Minimum value: 0
 *  Maximum value: 255
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_MEDIUM_BLOCK_COUNT


/** @brief  Size of each memory blocks identified as 'medium' block.
 *
 *  Size of each memory blocks identified as 'medium' block. Memory block are recommended to be word-sized.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_MEDIUM_BLOCK_SIZE


/** @brief Size of each memory blocks identified as 'large' block. *
 *  Minimum value: 0
 *  Maximum value: 255
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_LARGE_BLOCK_COUNT


/** @brief  Size of each memory blocks identified as 'large' block.
 *
 *  Size of each memory blocks identified as 'large' block. Memory block are recommended to be word-sized.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEMORY_MANAGER_LARGE_BLOCK_SIZE


/** @brief Enable debug trace in the module. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEM_MANAGER_ENABLE_LOGS


/** @brief Disable API parameter checks in the module. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define MEM_MANAGER_DISABLE_API_PARAM_CHECK



/** @} */
