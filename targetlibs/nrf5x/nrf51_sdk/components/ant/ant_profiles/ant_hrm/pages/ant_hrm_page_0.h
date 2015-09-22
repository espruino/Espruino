#ifndef ANT_HRM_PAGE_0_H__
#define ANT_HRM_PAGE_0_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_hrm_page0 HRM Profile page 0
 * @{
 * @ingroup ant_sdk_profiles_hrm_pages
 */

#include <stdint.h>

/**@brief Data structure for HRM data page 0.
 *
 * This structure is used as a common page. 
 */
typedef struct
{
    uint8_t beat_count;             ///< Beat count.
    uint8_t computed_heart_rate;    ///< Computed heart rate.
    uint16_t beat_time;             ///< Beat time.
} ant_hrm_page0_data_t;

/**@brief Initialize page 0.
 */
#define DEFAULT_ANT_HRM_PAGE0()     \
    {                               \
        .beat_count          = 0,   \
        .computed_heart_rate = 0,   \
        .beat_time           = 0,   \
    }

/**@brief Function for encoding page 0.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_hrm_page_0_encode(uint8_t * p_page_buffer, volatile ant_hrm_page0_data_t const * p_page_data);

/**@brief Function for decoding page 0.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_hrm_page_0_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page0_data_t * p_page_data);

#endif // ANT_HRM_PAGE_0_H__
/** @} */
