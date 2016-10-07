/**
 *
 * @defgroup fds_config Flash data storage module configuration
 * @{
 * @ingroup fds
 */
/** @brief Enabling FDS module. *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_ENABLED

/** @brief Size of the internal queue. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_OP_QUEUE_SIZE


/** @brief Determines how many @ref fds_record_chunk_t structures can be buffered at any time. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_CHUNK_QUEUE_SIZE


/** @brief Maximum number of callbacks that can be registered. *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_MAX_USERS


/** @brief Number of virtual flash pages to use.
 *
 * One of the virtual pages is reserved by the system for garbage collection.
 * Therefore, the minimum is two virtual pages: one page to store data and
 * one page to be used by the system for garbage collection. The total amount
 * of flash memory that is used by FDS amounts to @ref FDS_VIRTUAL_PAGES
 * @ref FDS_VIRTUAL_PAGE_SIZE * 4 bytes.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_VIRTUAL_PAGES


/** @brief The size of a virtual page of flash memory, expressed in number of 4-byte words.
 *
 * By default, a virtual page is the same size as a physical page.
 * The size of a virtual page must be a multiple of the size of a physical page.
 *
 *  Following options are avaiable:
 * - 256 - 256 (nRF51 family only)
 * - 256 - 256 (Software Component only)
 * - 512 - 512 (nRF51 family only)
 * - 1024 - 1024
 * - 2048 - 2048 (nRF52 family only)
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FDS_VIRTUAL_PAGE_SIZE



/** @} */
