#ifndef FS_CONFIG_H__
#define FS_CONFIG_H__

#include <stdint.h>
#include "nrf.h"

/**
 * @defgroup fstorage_config FStorage configuration
 * @ingroup fstorage
 * @{
 * @brief FStorage configuration options.
 */


/**@brief   Configures the size of the command queue. */
#define FS_CMD_QUEUE_SIZE   (4)

/**@brief   Configures how many times should a flash operation be retried. */
#define FS_CMD_MAX_RETRIES  (3)

#define FS_ERASED_WORD      (0xFFFFFFFF)

/**@brief   Size of a flash page in bytes. */
#if   defined (NRF51)
    #define FS_PAGE_SIZE    (1024)
#elif defined (NRF52)
    #define FS_PAGE_SIZE    (4096)
#else
    #error "Device family must be defined. See nrf.h."
#endif

/*@brief    Size of a flash page in words.
*/
#define FS_PAGE_SIZE_WORDS  (FS_PAGE_SIZE / 4)

/**@brief Function to obtain the address of the last page.
 *
 * @details If there is a bootloader present the bootloader address read from UICR
 *          will act as the page beyond the end of the available flash storage.
 */
static __INLINE uint32_t fs_flash_page_end_addr()
{
    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
    return  ((bootloader_addr != FS_ERASED_WORD) ?
             bootloader_addr : NRF_FICR->CODESIZE * FS_PAGE_SIZE);
}

/**@brief   Macro to obtain the address of the last page.
 *
 * @details If there is a bootloader present the bootloader address read from UICR
 *          will act as the page beyond the end of the available flash storage.
 */
#define FS_PAGE_END_ADDR  fs_flash_page_end_addr()

/**@brief   Configures the maximum number of words to be written to flash in a single operation.
 *
 * @note    This value is bound by the maximum number of words which can be written to flash
 *          in a single operation by the SoftDevice.
 */
#if   defined (NRF51)
    #define FS_MAX_WRITE_SIZE_WORDS	    (256)
#elif defined (NRF52)
    #define FS_MAX_WRITE_SIZE_WORDS     (1024)
#else
    #error "Device family must be defined. See nrf.h"
#endif

/** @} */

#endif // FS_CONFIG_H__

