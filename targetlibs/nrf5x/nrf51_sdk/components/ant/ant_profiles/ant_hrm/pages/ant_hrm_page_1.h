#ifndef ANT_HRM_PAGE_1_H__
#define ANT_HRM_PAGE_1_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_hrm_page1 HRM Profile page 1
 * @{
 * @ingroup ant_sdk_profiles_hrm_pages
 */

#include <stdint.h>

/**@brief Data structure for HRM data page 1.
 *
 * This structure implements only page 1 specific data. 
 */
typedef struct
{
    uint32_t operating_time;        ///< Operating time.
} ant_hrm_page1_data_t;

/**@brief Initialize page 1.
 */
#define DEFAULT_ANT_HRM_PAGE1()     \
    {                               \
        .operating_time      = 0,   \
    }

/**@brief Function for encoding page 1.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_hrm_page_1_encode(uint8_t * p_page_buffer, volatile ant_hrm_page1_data_t const * p_page_data);

/**@brief Function for decoding page 1.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_hrm_page_1_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page1_data_t * p_page_data);

#endif // ANT_HRM_PAGE_1_H__
/** @} */
