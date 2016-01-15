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

#ifndef FDS_H__
#define FDS_H__

/**
 * @defgroup flash_data_storage Flash Data Storage
 * @ingroup app_common
 * @{
 * @brief   Flash Data Storage (FDS).
 *
 * @details Flash Data Storage (FDS) is a minimalistic filesystem for the on-chip flash.
 *          It can be used to manipulate @e records, which consist of a piece of data, made up
 *          of one or more chunks, and an associated key pair which can be used to look up the
 *          record.
 */

#include <stdint.h>
#include <stdbool.h>
#include "sdk_errors.h"


/**@brief   Reserved type key. May not be used as a record key by the application. */
#define FDS_TYPE_ID_INVALID         (0x0000)

/**@brief   Reserved instance key. May not be used as a record key by the application. */
#define FDS_INSTANCE_ID_INVALID     (0xFFFF)

/**@brief   FDS return codes. */
enum
{
    FDS_SUCCESS                 = NRF_SUCCESS,
    FDS_ERR_OPERATION_TIMEOUT,  /**< Error. The operation has timed out. */
    FDS_ERR_BUSY,               /**< Error. The underlying flash subsystem was busy. */
    FDS_ERR_NOT_INITIALIZED,    /**< Error. The module is not initialized. */
    FDS_ERR_INITIALIZING,       /**< Error. The module is already initializing. */
    FDS_ERR_INVALID_KEYS,       /**< Error. Invalid key value(s). */
    FDS_ERR_UNALIGNED_ADDR,     /**< Error. Data has unaligned address. */
    FDS_ERR_INVALID_PARAM,      /**< Error. Parameter contains invalid data. */
    FDS_ERR_INVALID_DESC,       /**< Error. Invalid record descriptor. */
    FDS_ERR_NULL_PARAM,         /**< Error. NULL parameter. */
    FDS_ERR_NO_RECORDS_OPEN,    /**< Error. Attempted to close a record when there were none open. */
    FDS_ERR_NO_SPACE_IN_FLASH,  /**< Error. No space in flash memory. */
    FDS_ERR_NO_SPACE_IN_QUEUES, /**< Error. No space in the internal queues. */
    FDS_ERR_RECORD_TOO_LARGE,   /**< Error. The record exceeds the maximum allowed size. */
    FDS_ERR_NOT_FOUND,          /**< Error. Record was not found. */
    FDS_ERR_NO_PAGES,           /**< Error. No flash pages available. */
    FDS_ERR_USER_LIMIT_REACHED, /**< Error. Maximum number of users reached. */
    FDS_ERR_CRC_CHECK_FAILED,   /**< Error. A CRC check failed. */
    FDS_ERR_INTERNAL,           /**< Error. Internal error. */
};


typedef uint32_t fds_record_id_t;


/**@brief   A piece of record metadata; holds one of the record's keys (type)
 *          and the record's lenght, expressed in 4 byte words.
 */
typedef struct 
{
    uint16_t type;          /**< The record 'type' key. */
    uint16_t length_words;  /**< Length of the record data, in 4 byte words. */
} fds_tl_t;


/**@brief   A piece of record metadata; holds one of the record's keys (instance)
*           and the record's CRC16 check value.
*/
typedef struct
{
    uint16_t instance;  /**< The record 'instance' key. */

    /** CRC16 check value, calculated over the entire record as stored in flash, including its
     *  metadata, but excluding the CRC field itself. */
    uint16_t crc16;
} fds_ic_t;


/**@brief   The record metadata, as stored in flash. */
typedef struct
{
    fds_tl_t        tl;     /**< See @ref fds_tl_t. */
    fds_ic_t        ic;     /**< See @ref fds_ic_t. */
    fds_record_id_t id;     /**< The unique record ID (32 bits). */
} fds_header_t;


typedef fds_header_t fds_record_header_t;

/**@brief   The record descriptor structure, used to manipulate records.
 *
 * @note    This structure is meant to be opaque to the user.
 *
 * @warning Do not reuse the same descriptor for different records.
 *          If you do, be sure to set its fields to zero.
 */
typedef struct
{
    uint32_t         record_id;     /**< The unique record ID. */
    uint32_t const * p_rec;         /**< The last known location of the record in flash. */
    uint16_t         page;          /**< The virtual page ID in which the record is stored. */
    uint16_t         gc_magic;      /**< Number of times the GC algorithm has been run. */
    uint16_t         ptr_magic;     /**< Used to verify the validity of p_rec. */
} fds_record_desc_t;


/**@brief   The record key, used to look up records.
 *
 * @note    The uniqueness of either field is not enforced by FDS.
 */
typedef struct
{
    uint16_t type;
    uint16_t instance;
} fds_record_key_t;


/**@brief   Structure used for reading a record back from flash memory.
*/
typedef struct
{
    fds_record_header_t const * p_header;  /**< The record header (metadata), as stored in flash. */
    uint32_t            const * p_data;    /**< The record data, as stored in flash. */
} fds_record_t;


/**@brief   A record chunk, containing a piece of data to be stored in a record.
 *
 * @note    p_data must be aligned on a (4 bytes) word boundary. Additionally, it must be kept
 *          alive in memory until the write operation has completed.
 */
typedef struct
{
    void     const * p_data;        /**< Pointer to the data to store. Must be word aligned. */
    uint16_t         length_words;  /**< Length of data pointed to by p_data, in 4 byte words. */
} fds_record_chunk_t;


/**@brief   A token to a reserved space in flash, created by @ref fds_reserve.
 *          Use @ref fds_write_reserved to write the record in the reserved space,
 *          or @ref fds_reserve_cancel to cancel the reservation.
 */
typedef struct
{
    uint16_t page;           /**< The logical ID of the page where space was reserved. */
    uint16_t length_words;   /**< The amount of space reserved, in 4 byte words. */
} fds_write_token_t;


/**@brief   A token to keep information about the progress of @ref fds_find, @ref fds_find_by_type
 *          and @ref fds_find_by_instance operations.
 *
 * @note    The token must be zero-initialized before use.
 *
 * @note    This structure is meant to be opaque to the user.
 *
 * @warning Do not reuse the same token to search for different records.
 *          If you do, be sure to set its fields to zero.
 */
typedef struct
{
    uint32_t const * p_addr;
    uint32_t         magic;
    uint16_t         page;
} fds_find_token_t;


/**@brief   FDS events. */
typedef enum
{
    FDS_EVT_INIT,       /**< Event for @ref fds_init. */
    FDS_EVT_WRITE,      /**< Event for @ref fds_write and @ref fds_write_reserved. */
    FDS_EVT_UPDATE,     /**< Event for @ref fds_update. */
    FDS_EVT_CLEAR,      /**< Event for @ref fds_clear. */
    FDS_EVT_CLEAR_MANY, /**< Event for @ref fds_clear_by_instance. */
    FDS_EVT_GC          /**< Event for @ref fds_gc. */
} fds_evt_id_t;


#if defined(__CC_ARM)
    #pragma push
    #pragma anon_unions
#elif defined(__ICCARM__)
    #pragma language=extended
#elif defined(__GNUC__)
    /* anonymous unions are enabled by default */
#endif 

typedef struct
{
    fds_evt_id_t id;        /**< The event ID. See @ref fds_evt_id_t. */
    ret_code_t   result;    /**< The event result. */
    union
    {
        struct
        {
            // Unused.
            uint16_t pages_not_mounted;
        } mount;
        struct
        {
            fds_record_id_t  record_id;
            fds_record_key_t record_key;
            bool             record_updated;
            //fds_record_id_t  record_cleared;
        } write; /*write, write_res and update*/
        struct
        {
            fds_record_id_t  record_id;
            fds_record_key_t record_key;
        } clear;
        struct
        {
            fds_record_key_t record_key;
            uint16_t         records_cleared; // Unused.
        } clear_many;
        struct
        {
            // Unused.
            uint16_t pages_skipped;
            uint16_t space_reclaimed;
        } gc;
    };
} fds_evt_t;


#if defined(__CC_ARM)
    #pragma pop
#elif defined(__ICCARM__)
    /* leave anonymous unions enabled */
#elif defined(__GNUC__)
    /* anonymous unions are enabled by default */
#endif


/**@brief   Flash data storage callback function.
 *
 * @param   p_evt   The event associated with the callback.
 */
typedef void (*fds_cb_t)(fds_evt_t const * const p_evt);


/**@brief   Function to register a callback for events.
 *
 * @details The maximum amount of callback which can be registered can be configured by
 *          changing the FDS_MAX_USERS macro in fds_config.h.
 * 
 * @param[in]   cb The callback function.
 *
 * @retval  FDS_SUCCESS                 Success.
 * @retval  FDS_ERR_USER_LIMIT_REACHED  Error. Maximum number of registered callbacks reached.
 */
ret_code_t fds_register(fds_cb_t cb);


/**@brief   Function to initialize the module.
 *
 * @details This function initializes the module and installs the filesystem, if it is not
 *          installed yet.
 *
 * @note    This function is asynchronous. Completion is reported with a callback through the
 *          registered event handler. To be able to receive such callback, be sure to call
 *          @ref fds_register before calling @ref fds_init.
 *
 * @retval  FDS_SUCCESS                 Success. The command was accepted.
 * @retval  FDS_ERR_INITIALIZING        Error. The module is already initializing.
 * @retval  FDS_ERR_OPERATION_TIMEOUT   Error. The operation timed out.
 * @retval  FDS_ERR_BUSY                Error. The underlying flash subsystem was busy.
 * @retval  FDS_ERR_NO_SPACE_IN_FLASH   Error. No space available in flash memory for installation.
 */
ret_code_t fds_init(void);


/**@brief   Function to queue writing a record in flash memory.
 *
 * @details If the module is idle, i.e., not processing other commands, then the command
 *          will be processed immediately.
 *
 *          A record 'type' key must be different from FDS_TYPE_ID_INVALID; the 'instance' key
 *          must be different from FDS_INSTANCE_IC_INVALID.
 *          A record data consists of multiple chunks and is supplied to the function as an
 *          array of fds_record_chunk_t structures. The data must be aligned on a 4 byte boundary,
 *          and because it is not buffered internally, it must be kept in memory by the
 *          application until the callback for the command has been received. The lenght of the
 *          data may not exceed FDS_VIRTUAL_PAGE_SIZE words minus 14 bytes.
 *
 * @note This function is asynchronous.
 *       Completion is reported with a callback through the registered event handler.
 *
 * @param[out]  p_desc      The record descriptor. It may be NULL.
 * @param[in]   key         The record key pair.
 * @param[in]   num_chunks  The number of elements in the chunks array.
 * @param[in]   chunks      An array of record chunks making up the record data.
 *
 * @retval  FDS_SUCCESS                  Success. The command was accepted.
 * @retval  FDS_ERR_NOT_INITIALIZED      Error. The module is not initialized.
 * @retval  FDS_ERR_INVALID_KEYS         Error. One or both keys are invalid.
 * @retval  FDS_ERR_UNALIGNED_ADDR       Error. The record data is not aligned on a 4 byte boundary.
 * @retval  FDS_ERR_RECORD_TOO_LARGE     Error. The record data exceeds the maximum lenght.
 * @retval  FDS_ERR_NO_SPACE_IN_QUEUES   Error. Insufficient internal resources to queue the command.
 * @retval  FDS_ERR_NO_SPACE_IN_FLASH    Error. Insufficient space in flash memory to store the record.
 * @retval  FDS_ERR_BUSY                 Error. The underlying flash subsystem was busy.
 */
ret_code_t fds_write(fds_record_desc_t * const p_desc,
                     fds_record_key_t          key,
                     uint8_t                   num_chunks,
                     fds_record_chunk_t        chunks[]);


/**@brief   Function to reserve space for a record in flash memory.
 *
 * @details This function can be used to reserve space for a record in flash memory.
 *          To write a record into the reserved space, use @ref fds_write_reserved.
 *          It is possible to cancel a reservation by using @ref fds_reserve_cancel.
 *
 * @param[out]  p_tok           A token which can be used to write a record in the reserved space
 *                              using @ref fds_write_reserved.
 * @param[in]   length_words    The lenght of the record data, in 4 byte words.
 *
 * @retval  FDS_SUCCESS                 Success. Flash space successfully reserved.
 * @retval  FDS_ERR_NULL_PARAM          Error. p_tok is NULL.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_RECORD_TOO_LARGE    Error. The record data exceeds the maximum lenght.
 * @retval  FDS_ERR_NO_SPACE_IN_FLASH   Error. Insufficient space in flash memory.
 */
ret_code_t fds_reserve(fds_write_token_t * const p_tok, uint16_t length_words);


/**@brief   Function to cancel a space reservation.
 *
 * @param[in]   p_tok   The token produced by @ref fds_reserve, identifying the reservation to cancel.
 *
 * @retval  FDS_SUCCESS             Success. Reservation canceled.
 * @retval  FDS_ERR_NOT_INITIALIZED Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM      Error. p_tok is NULL.
 * @retval  FDS_ERR_INVALID_PARAM   Error. p_tok contains invalid data.
 */
ret_code_t fds_reserve_cancel(fds_write_token_t * const p_tok);


/**@brief   Function to queue writing a record for which the space in flash memory has been
 *          previously reserved using @ref fds_reserve.
 *
 * @details This function behaves similarly to @ref fds_write, with the exception that it never
 *          fails with FDS_ERR_NO_SPACE_IN_FLASH. If the module is idle, i.e., not processing
 *          other commands, then the command will be processed immediately.
 *
 *          A record 'type' key must be different from FDS_TYPE_ID_INVALID; the 'instance' key
 *          must be different from FDS_INSTANCE_IC_INVALID.
 *          A record data consists of multiple chunks and is supplied to the function as an
 *          array of fds_record_chunk_t structures. The data must be aligned on a 4 byte boundary,
 *          and because it is not buffered internally, it must be kept in memory by the
 *          application until the callback for the command has been received. The lenght of the
 *          data may not exceed FDS_VIRTUAL_PAGE_SIZE words minus 14 bytes.
 *
 * @note    This function is asynchronous.
 *          Completion is reported with a callback through the registered event handler.
 *
 * @param[in]   p_tok       The token return by @ref fds_reserve.
 * @param[out]  p_desc      The record descriptor. It may be NULL.
 * @param[in]   key         The record key pair.
 * @param[in]   num_chunks  The number of elements in the chunks array.
 * @param[in]   chunks      An array of chunks making up the record data.
 *
 * @retval  FDS_SUCCESS                 Success. The command was accepted.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. p_tok is NULL.
 * @retval  FDS_ERR_INVALID_KEYS        Error. One or both keys are invalid.
 * @retval  FDS_ERR_UNALIGNED_ADDR      Error. The record data is not aligned on a 4 byte boundary.
 * @retval  FDS_ERR_RECORD_TOO_LARGE    Error. The record data exceeds the maximum lenght.
 * @retval  FDS_ERR_NO_SPACE_IN_QUEUES  Error. Insufficient internal resources to queue the command.
 * @retval  FDS_ERR_BUSY                Error. The underlying flash subsystem was busy.
 */
ret_code_t fds_write_reserved(fds_write_token_t  const * const p_tok,
                              fds_record_desc_t        * const p_desc,
                              fds_record_key_t                 key,
                              uint8_t                          num_chunks,
                              fds_record_chunk_t               chunks[]);


/**@brief   Function to queue clearing a record.
 *
 * @details Clearing a record has the effect of preventing the system from retrieving its
 *          descriptor using the @ref fds_find, @ref fds_find_by_type and @ref fds_find_by_instance
 *          functions. Additionally, @ref fds_open calls shall fail when supplied a descritpor for
 *          a record which has been cleared.
 *
 * @note    Clearing a record does not free the space it occupies in flash memory.
 *          The reclaim flash space used by cleared records, use @ref fds_gc.
 *
 * @note    This function is asynchronous, therefore, completion is reported with a callback
 *          through the registered event handler.
 *
 * @param[in]   p_desc  The descriptor of the record to be cleared.
 *
 * @retval  FDS_SUCCESS                 Success. The command was accepted.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. p_desc is NULL.
 * @retval  FDS_ERR_NO_SPACE_IN_QUEUES  Error. Insufficient internal resources to queue the command.
 */
ret_code_t fds_clear(fds_record_desc_t * const p_desc);


/**@brief   Function to queue clearing all records with a given instance.
 *
 * @details Clearing a record has the effect of preventing the system from retrieving its
 *          descriptor using the @ref fds_find, @ref fds_find_by_type and @ref fds_find_by_instance
 *          functions. Additionally, @ref fds_open calls shall fail when supplied a descritpor for
 *          a record which has been cleared.
 *
 * @note    Clearing a record does not free the space it occupies in flash memory.
 *          The reclaim flash space used by cleared records, use @ref fds_gc.
 *
 * @note    This function is asynchronous.
 *          Completion is reported with a callback through the registered event handler.
 *
 * @param[in]   instance    The 'instance' key of the records to be clearead.
 *
 * @retval  FDS_SUCCESS                 Success. The command was queued.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. p_desc is NULL.
 * @retval  FDS_ERR_NO_SPACE_IN_QUEUES  Error. Insufficient internal resources to queue the command.
 */
ret_code_t fds_clear_by_instance(uint16_t instance);


/**@brief   Function to queque updating a record.
 *
 * @details Updating a record writes a new record with the given keys and data in flash, and then
 *          clears the old record. If the module is idle, i.e., not processing other commands,
 *          then the command will be processed immediately.
 *
 *          A record 'type' key must be different from FDS_TYPE_ID_INVALID; the 'instance' key
 *          must be different from FDS_INSTANCE_IC_INVALID.
 *          A record data consists of multiple chunks and is supplied to the function as an
 *          array of fds_record_chunk_t structures. The data must be aligned on a 4 byte boundary,
 *          and because it is not buffered internally, it must be kept in memory by the
 *          application until the callback for the command has been received. The lenght of the
 *          data may not exceed FDS_VIRTUAL_PAGE_SIZE words minus 14 bytes.
 *
 * @note    This function is asynchronous, therefore, completion is reported with a callback
 *          through the registered event handler.
 * 
 * @param[in, out]  p_desc  The descriptor of the record to update. When the function has returned
 *                          with FDS_SUCCESS, this parameter will contain a descriptor for the new
 *                          record.
 * @param[in]   key         The record new key pair.
 * @param[in]   num_chunks  The number of elements in the chunks array.
 * @param[in]   chunks      An array of chunks making up the record new data.
 *
 * @retval  FDS_SUCCESS                 Success. The command was queued.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_INVALID_KEYS        Error. One or both keys are invalid.
 * @retval  FDS_ERR_UNALIGNED_ADDR      Error. The record data is not aligned on a 4 byte boundary.
 * @retval  FDS_ERR_RECORD_TOO_LARGE    Error. The record data exceeds the maximum lenght.
 * @retval  FDS_ERR_NO_SPACE_IN_QUEUES  Error. Insufficient internal resources to queue the command.
 * @retval  FDS_ERR_NO_SPACE_IN_FLASH   Error. Insufficient space in flash memory to store the record.
 * @retval  FDS_ERR_BUSY                Error. The underlying flash subsystem was busy.
 */
ret_code_t fds_update(fds_record_desc_t  * const p_desc,
                      fds_record_key_t           key,
                      uint8_t                    num_chunks,
                      fds_record_chunk_t         chunks[]);


/**@brief   Function to search for records with a given key pair.
 *
 * @details Because a key pair is not unique, to search for the next record with the given keys call
 *          the function again and supply the same fds_find_token_t structure to resume searching
 *          from the last record found.
 *
 * @param[in]   type        The record 'type' key.
 * @param[in]   instance    The record 'instance' key.
 * @param[out]  p_desc      The descriptor of the record found.
 * @param[out]  p_token     A token containing information about the progress of the operation.
 *
 * @retval  FDS_SUCCESS                 Success. Record found.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. Either p_desc or p_token are NULL.
 * @retval  FDS_ERR_NOT_FOUND           Error. No record with the given key pair was found.
 */
ret_code_t fds_find(uint16_t                  type, 
                    uint16_t                  instance, 
                    fds_record_desc_t * const p_desc,
                    fds_find_token_t  * const p_token);


/**@brief   Function to search for records with a given 'type' key.
 *
 * @details Because keys are not unique, to search for the next record with the given key call
 *          the function again and supply the same fds_find_token_t structure to resume searching
 *          from the last record found.
 *
 * @param[in]   type        The record 'type' key.
 * @param[out]  p_desc      The descriptor of the record found.
 * @param[out]  p_token     A token containing information about the progress of the operation.
 *
 * @retval  FDS_SUCCESS                 Success. Record found.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. Either p_desc or p_token are NULL.
 * @retval  FDS_ERR_NOT_FOUND           Error. No record with the given key was found.
 */
 ret_code_t fds_find_by_type(uint16_t                  type,
                             fds_record_desc_t * const p_desc,
                             fds_find_token_t  * const p_token);


/**@brief   Function to search for records with a given 'instance' key.
 *
 * @details Because keys are not unique, to search for the next record with the given key call
 *          the function again and supply the same fds_find_token_t structure to resume searching
 *          from the last record found.
 *
 * @param[in]   instance    The record 'instance' key.
 * @param[out]  p_desc      The descriptor of the record found.
 * @param[out]  p_token     A token containing information about the progress of the operation.
 *
 * @retval  FDS_SUCCESS                 Success. Record found.
 * @retval  FDS_ERR_NOT_INITIALIZED     Error. The module is not initialized.
 * @retval  FDS_ERR_NULL_PARAM          Error. Either p_desc or p_token are NULL.
 * @retval  FDS_ERR_NOT_FOUND           Error. No record with the given key was found.
 */
ret_code_t fds_find_by_instance(uint16_t                  instance,
                                fds_record_desc_t * const p_desc,
                                fds_find_token_t  * const p_token);


/**@brief   Function to open a record for reading.
 *
 * @details Function to read a record which has been written to flash. This function initializes
 *          a fds_record_t structure which can be used to access the record data as well as
 *          its associated metadata. The pointers provided in the fds_record_t structure are
 *          pointers to flash memory. Opening a record with @ref fds_open prevents the garbage
 *          collection to run on the flash page in which record is stored, therefore the contents
 *          of the memory pointed by the fds_record_t fields are guaranteed to remain unmodified,
 *          as long as the record is kept open.
 *
 * @note    When done reading a record, close it using @ref fds_close so that garbage collection
 *          can reclaim space on the page where the record is stored, if necessary.
 *
 * @param[in]   p_desc      The descriptor of the record to open.
 * @param[out]  p_record    The record, as stored in flash.
 *
 * @retval  FDS_SUCCESS             Success. The record was opened.
 * @retval  FDS_ERR_NOT_FOUND       Error. The record was not found. It may have been cleared, or it
 *                                  may have not been written yet.
 * @retval  FDS_ERR_INVALID_PARAM   Error. The descriptor contains invalid data.
 * @retval  FDS_ERR_NULL_PARAM      Error. Either p_desc or p_record are NULL.
 */
ret_code_t fds_open(fds_record_desc_t * const p_desc,
                    fds_record_t      * const p_record);


/**@brief   Function to close a record after its contents have been read.
 *
 * @details Closing a record allows garbage collection to be run on the page in which the
 *          record being closed is stored (if no other records remain open on that page).
 *
 * @note    Closing a record, does @e not invalidate its descriptor, which can be safely supplied to
 *          all functions which accept a descriptor as a parameter.
 *
 * @param[in]   p_desc The descriptor of the record to close.
 *
 * @retval  FDS_SUCCESS             Success. The record was closed.
 * @retval  FDS_ERR_INVALID_PARAM   Error. The descriptor contains invalid data.
 * @retval  FDS_ERR_NULL_PARAM      Error. p_desc is NULL.
 */
ret_code_t fds_close(fds_record_desc_t const * const p_desc);


/**@brief   Function to queue garbage collection.
 *
 * @details Garbage collection reclaims the flash space occupied by records which have been cleared
 *          using @ref fds_clear.
 *
 * @note    This function is asynchronous.
 *          Completion is reported with a callback through the registered event handler.
 */
ret_code_t fds_gc(void);


/**@brief   Function to obtain a descriptor from a record ID.
 *
 * @details This function can be used to reconstruct a descriptor from a record ID, such as the
 *          one passed to the callback function.
 *
 * @warning This function does not check if a record with the given record ID exists or not. If a
 *          non-existing record ID is supplied, the resulting descriptor will cause other functions
 *          to fail when used as parameter.
 *
 * @param[out]  p_desc       The descriptor of the record with given record ID.
 * @param[in]   record_id    The record ID for which to provide a descriptor.
 *
 * @retval FDS_SUCCESS          Success.
 * @retval FDS_ERR_NULL_PARAM   Error. p_desc is NULL.
 */
ret_code_t fds_descriptor_from_rec_id(fds_record_desc_t * const p_desc,
                                      fds_record_id_t           record_id);


/**@brief Function to obtain a record ID from a record descriptor.
 *
 * @details This function can be used to extract a record ID from a descriptor. It may be used
 *          in the callback function to determine which record the callback is associated to, if
 *          you have its descriptor.
 *
 * @warning This function does not check the record descriptor sanity. If the descriptor is
 *          uninitialized, or has been tampered with, the resulting record ID may be invalid.
 *
 * @param[in]   p_desc          The descriptor from which to extract the record ID.
 * @param[out]  p_record_id     The record ID contained in the given descriptor.
 *
 * @retval FDS_SUCCESS          Success.
 * @retval FDS_ERR_NULL_PARAM   Error. Either p_desc or p_record_id are NULL.
 */
ret_code_t fds_record_id_from_desc(fds_record_desc_t const * const p_desc,
                                   fds_record_id_t         * const p_record_id);


#if defined(FDS_CRC_ENABLED)

/**@brief   Function to enable/disable verification upon write operations (@ref fds_write,
 *          @ref fds_write_reserved and @ref fds_update).
 *
 * @details This function can be used to verify that data which was queued for writing
 *          did not change before the write actually happened.
 *
 * @param[in]   enabled Enables or disables CRC verification.
 *
 * @retval  FDS_SUCCESS     Success.
 */
ret_code_t fds_verify_crc_on_writes(bool enabled);

#endif

/** @} */
                                         
#endif // FDS_H__
