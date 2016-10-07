/**
 *
 * @defgroup fstorage_config Flash storage module configuration
 * @{
 * @ingroup fstorage
 */
/** @brief Enabling fstorage module *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FSTORAGE_ENABLED

/** @brief Configures the size of the internal queue.
 *
 * Increase this if there are many users, or if it is likely that many
 * operation will be queued at once without waiting for the previous operations
 * to complete. In general, increase the queue size if you frequently receive
 * @ref FS_ERR_QUEUE_FULL errors when calling @ref fs_store or @ref fs_erase.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FS_QUEUE_SIZE


/** @brief Number attempts to execute an operation if the SoftDevice fails.
 *
 * Increase this value if events return the @ref FS_ERR_OPERATION_TIMEOUT
 * error often. The SoftDevice may fail to schedule flash access due to high BLE activity.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FS_OP_MAX_RETRIES


/** @brief Maximum number of words to be written to flash in a single operation.
 *
 * Tweaking this value can increase the chances of the SoftDevice being
 * able to fit flash operations in between radio activity. This value is bound by the
 * maximum number of words which the SoftDevice can write to flash in a single call to
 * @ref sd_flash_write, which is 256 words for nRF51 ICs and 1024 words for nRF52 ICs.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define FS_MAX_WRITE_SIZE_WORDS



/** @} */
