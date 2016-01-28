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


#ifndef FS_H__
#define FS_H__

 /** @file
 *
 * @defgroup fstorage FStorage
 * @{
 * @ingroup app_common
 * @brief Module which provides low level functionality to store data to flash.
 *
 */


#include <stdint.h>
#include "section_vars.h"
#include "fstorage_config.h"
#include "sdk_errors.h"


typedef enum
{
    FS_OP_NONE,
    FS_OP_STORE,
    FS_OP_ERASE
} fs_op_code_t;


/**@brief   FStorage event IDs. */
typedef enum
{
    FS_EVT_STORE,
    FS_EVT_ERASE
} fs_evt_id_t;


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
    fs_evt_id_t id;     /**< The event ID. See @ref fs_evt_id_t. */
    union
    {
        struct
        {
            uint32_t const * p_data;
            uint16_t         length_words;
        } store;
        struct
        {
            uint16_t first_page;
            uint16_t last_page;
        } erase;
    };
} fs_evt_t;

#if defined(__CC_ARM)
  #pragma pop
#elif defined(__ICCARM__)
  /* leave anonymous unions enabled */
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#endif


/**@brief   Callback for flash operations.
 *
 * @param[in]   evt     The event associated with the callback.
 * @param[in]   result  Result of the operation.
 */
typedef void (*fs_cb_t)(fs_evt_t evt, uint32_t result);


/**@brief   FStorage configuration.
 *
 * @details start_addr and end_address fields are updated according to ordering rules and the
 *          number of pages requested by FStorage users.
 */
typedef struct
{
    fs_cb_t  const   cb;            /**< Callback to run when the flash operation has completed. */
    uint32_t       * p_start_addr;  /**< Pointer to the start address of the allocated flash storage. Set by running @ref fs_init. */
    uint32_t       * p_end_addr;    /**< Pointer to the end address of the allcoated flash storage. Set by running @ref fs_init. */
    uint8_t  const   num_pages;     /**< The number of pages to reserve for flash storage. */
    uint8_t  const   page_order;    /**< The order used to allocate pages. */
} fs_config_t;


/**@brief Macro for registering of flash storage configuration variable.
 *
 * @details This macro is expected to be invoked in the code unit that that requiress
 *          flash storage. This macro places the registered configuration variable in a section
 *          named "fs_data" that the FStorage module uses during initialization and regular operation.
 */
#define FS_SECTION_VARS_ADD(type_def) NRF_SECTION_VARS_ADD(fs_data, type_def)


/**@brief Function to initialize FStorage.
 *
 * @details This function allocates flash data pages according to the number requested in the
 *          config variable. The data used to initialize the FStorage is section placed
 *          variables in the data section "fs_data".
 */
ret_code_t fs_init(void);


/**@brief   Function to store data in flash.
 *
 * @warning The data to be written to flash has to be kept in memory until the operation has
 *          terminated, i.e., a callback is received.
 *
 * @param[in]   p_config        FStorage configuration of the user that requests the operation.
 * @param[in]   p_dest          The address in flash memory where to store the data.
 * @param[in]   p_src           Pointer to the data to store.
 * @param[in]   length_words    Length of the data to store, in 4 byte words.
 *
 * @retval NRF_SUCCESS                  Success. Command queued.
 * @retval NRF_ERROR_INVALID_STATE      Error. The module is not initialized.
 * @retval NRF_ERROR_INVALID_ADDR       Error. Data is unaligned or invalid configuration.
 * @retval Any error returned by the SoftDevice flash API.
 */
ret_code_t fs_store(fs_config_t const * const p_config,
                    uint32_t    const * const p_dest,
                    uint32_t    const * const p_src,
                    uint16_t                  length_words);


/**@brief   Function to erase flash pages.
 *
 * @details This function erases num_pages, starting from the page at p_page_addr.
 *          p_page_addr must be aligned on a page boundary.
 *
 * @param[in]   p_config        FStorage configuration of the user that requests the operation.
 * @param[in]   p_page_addr     Address of page to erase.
 * @param[in]   num_pages       Number of pages to erase.
 *
 * @retval NRF_SUCCESS                  Success. Command queued.
 * @retval NRF_ERROR_INVALID_STATE      Error. The module is not initialized.
 * @retval NRF_ERROR_INVALID_ADDR       Error. Pointer is unaligned or invalid configuration.
 * @retval Any error returned by the SoftDevice flash API.
 */
ret_code_t fs_erase(fs_config_t const * const p_config,
                    uint32_t    const * const p_page_addr,
                    uint16_t                  num_pages);


/**@brief Function to handle system events from the SoftDevice.
 *
 * @param[in]   sys_evt     System event from the SoftDevice
 */
void fs_sys_event_handler(uint32_t sys_evt);

/** @} */

#endif // FS_H__
