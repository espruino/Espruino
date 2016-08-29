#ifndef ANT_SEARCH_CONFIG_H__
#define ANT_SEARCH_CONFIG_H__

/** @file
 *
 * @defgroup ant_sdk_search_config ANT search configuration
 * @{
 * @ingroup ant_sdk_utils
 * @brief ANT channel search configuration module.
 */

#include <stdint.h>
#include "ant_parameters.h"

#define ANT_SEARCH_SHARING_CYCLES_DISABLE   0x00    ///< Disable search sharing.

#define ANT_LOW_PRIORITY_SEARCH_DISABLE     0x00    ///< Disable low priority search.
#define ANT_LOW_PRIORITY_TIMEOUT_DISABLE    0xFF    ///< Disable low priority search time-out.
#define ANT_DEFAULT_LOW_PRIORITY_TIMEOUT    0x02    ///< Default low priority search time-out.

#define ANT_HIGH_PRIORITY_SEARCH_DISABLE    0x00    ///< Disable high priority search.
#define ANT_HIGH_PRIORITY_TIMEOUT_DISABLE   0xFF    ///< Disable high priority search time-out.
#define ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT   0x0A    ///< Default high priority search time-out.

/**@brief Search priority. */
typedef enum
{
    ANT_SEARCH_PRIORITY_DEFAULT = 0,                                ///< Default search priority level.
    ANT_SEARCH_PRIORITY_LOWEST  = ANT_SEARCH_PRIORITY_DEFAULT,      ///< Lowest search priority level.
    ANT_SEARCH_PRIORITY_0       = ANT_SEARCH_PRIORITY_LOWEST,
    ANT_SEARCH_PRIORITY_1       = 1,
    ANT_SEARCH_PRIORITY_2       = 2,
    ANT_SEARCH_PRIORITY_3       = 3,
    ANT_SEARCH_PRIORITY_4       = 4,
    ANT_SEARCH_PRIORITY_5       = 5,
    ANT_SEARCH_PRIORITY_6       = 6,
    ANT_SEARCH_PRIORITY_7       = 7,
    ANT_SEARCH_PRIORITY_HIGHEST = ANT_SEARCH_PRIORITY_7,            ///< Highest search priority level.
} ant_search_priority_t;

/**@brief ANT search waveform. */
typedef enum
{
    ANT_WAVEFORM_DEFAULT    = 316,  ///< Standard search waveform value.
    ANT_WAVEFORM_FAST       = 97,   ///< Accelerated search waveform value.
} ant_waveform_t;

/**@brief ANT search configuration structure. */
typedef struct
{
    uint8_t                 channel_number;         ///< Assigned channel number.
    uint8_t                 low_priority_timeout;   ///< Low priority time-out (in 2.5 second increments).
    uint8_t                 high_priority_timeout;  ///< High priority time-out (in 2.5 second increments).
    uint8_t                 search_sharing_cycles;  ///< Number of search cycles to run before alternating searches. Search sharing can be disabled by @ref ANT_SEARCH_SHARING_CYCLES_DISABLE.
    ant_search_priority_t   search_priority;        ///< Search priority.
    ant_waveform_t          waveform;               ///< Search waveform. Do not use custom values.
} ant_search_config_t;

/**@brief Initializes the default ANT search configuration structure.
 *
 * @param[in]  CHANNEL_NUMBER       Number of the channel.
 */
#define DEFAULT_ANT_SEARCH_CONFIG(CHANNEL_NUMBER)                   \
{                                                                   \
    .channel_number         = CHANNEL_NUMBER,                       \
    .low_priority_timeout   = ANT_DEFAULT_LOW_PRIORITY_TIMEOUT,     \
    .high_priority_timeout  = ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT,    \
    .search_sharing_cycles  = ANT_SEARCH_SHARING_CYCLES_DISABLE,    \
    .search_priority        = ANT_SEARCH_PRIORITY_DEFAULT,          \
    .waveform               = ANT_WAVEFORM_DEFAULT,                 \
}

/**@brief Function for configuring the ANT channel search.
 *
 * @param[in]  p_config        Pointer to the search configuration structure.
 *
 * @retval     NRF_SUCCESS     If the channel was successfully configured. Otherwise, an error code is returned.
 */
uint32_t ant_search_init(ant_search_config_t const * p_config);

#endif // ANT_SEARCH_CONFIG_H__
/** @} */
