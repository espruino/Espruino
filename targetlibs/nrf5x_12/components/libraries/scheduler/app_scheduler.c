/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
#include "sdk_config.h"
#if APP_SCHEDULER_ENABLED
#include "app_scheduler.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nrf_soc.h"
#include "nrf_assert.h"
#include "app_util.h"
#include "app_util_platform.h"

/**@brief Structure for holding a scheduled event header. */
typedef struct
{
    app_sched_event_handler_t handler;          /**< Pointer to event handler to receive the event. */
    uint16_t                  event_data_size;  /**< Size of event data. */
} event_header_t;

STATIC_ASSERT(sizeof(event_header_t) <= APP_SCHED_EVENT_HEADER_SIZE);

static event_header_t * m_queue_event_headers;  /**< Array for holding the queue event headers. */
static uint8_t        * m_queue_event_data;     /**< Array for holding the queue event data. */
static volatile uint8_t m_queue_start_index;    /**< Index of queue entry at the start of the queue. */
static volatile uint8_t m_queue_end_index;      /**< Index of queue entry at the end of the queue. */
static uint16_t         m_queue_event_size;     /**< Maximum event size in queue. */
static uint16_t         m_queue_size;           /**< Number of queue entries. */

#if APP_SCHEDULER_WITH_PROFILER
static uint16_t m_max_queue_utilization;    /**< Maximum observed queue utilization. */
#endif

#if APP_SCHEDULER_WITH_PAUSE
static uint32_t m_scheduler_paused_counter = 0; /**< Counter storing the difference between pausing
                                                     and resuming the scheduler. */
#endif

/**@brief Function for incrementing a queue index, and handle wrap-around.
 *
 * @param[in]   index   Old index.
 *
 * @return      New (incremented) index.
 */
static __INLINE uint8_t next_index(uint8_t index)
{
    return (index < m_queue_size) ? (index + 1) : 0;
}


static __INLINE uint8_t app_sched_queue_full()
{
  uint8_t tmp = m_queue_start_index;
  return next_index(m_queue_end_index) == tmp;
}

/**@brief Macro for checking if a queue is full. */
#define APP_SCHED_QUEUE_FULL() app_sched_queue_full()


static __INLINE uint8_t app_sched_queue_empty()
{
  uint8_t tmp = m_queue_start_index;
  return m_queue_end_index == tmp;
}

/**@brief Macro for checking if a queue is empty. */
#define APP_SCHED_QUEUE_EMPTY() app_sched_queue_empty()


uint32_t app_sched_init(uint16_t event_size, uint16_t queue_size, void * p_event_buffer)
{
    uint16_t data_start_index = (queue_size + 1) * sizeof(event_header_t);

    // Check that buffer is correctly aligned
    if (!is_word_aligned(p_event_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Initialize event scheduler
    m_queue_event_headers = p_event_buffer;
    m_queue_event_data    = &((uint8_t *)p_event_buffer)[data_start_index];
    m_queue_end_index     = 0;
    m_queue_start_index   = 0;
    m_queue_event_size    = event_size;
    m_queue_size          = queue_size;

#if APP_SCHEDULER_WITH_PROFILER
    m_max_queue_utilization = 0;
#endif

    return NRF_SUCCESS;
}


uint16_t app_sched_queue_space_get()
{
    uint16_t start = m_queue_start_index;
    uint16_t end   = m_queue_end_index;
    uint16_t free_space = m_queue_size - ((end >= start) ?
                           (end - start) : (m_queue_size + 1 - start + end));
    return free_space;
}


#if APP_SCHEDULER_WITH_PROFILER
static void queue_utilization_check(void)
{
    uint16_t start = m_queue_start_index;
    uint16_t end   = m_queue_end_index;
    uint16_t queue_utilization = (end >= start) ? (end - start) :
        (m_queue_size + 1 - start + end);

    if (queue_utilization > m_max_queue_utilization)
    {
        m_max_queue_utilization = queue_utilization;
    }
}

uint16_t app_sched_queue_utilization_get(void)
{
    return m_max_queue_utilization;
}
#endif // APP_SCHEDULER_WITH_PROFILER


uint32_t app_sched_event_put(void                    * p_event_data,
                             uint16_t                  event_data_size,
                             app_sched_event_handler_t handler)
{
    uint32_t err_code;

    if (event_data_size <= m_queue_event_size)
    {
        uint16_t event_index = 0xFFFF;

        CRITICAL_REGION_ENTER();

        if (!APP_SCHED_QUEUE_FULL())
        {
            event_index       = m_queue_end_index;
            m_queue_end_index = next_index(m_queue_end_index);

        #if APP_SCHEDULER_WITH_PROFILER
            // This function call must be protected with critical region because
            // it modifies 'm_max_queue_utilization'.
            queue_utilization_check();
        #endif
        }

        CRITICAL_REGION_EXIT();

        if (event_index != 0xFFFF)
        {
            // NOTE: This can be done outside the critical region since the event consumer will
            //       always be called from the main loop, and will thus never interrupt this code.
            m_queue_event_headers[event_index].handler = handler;
            if ((p_event_data != NULL) && (event_data_size > 0))
            {
                memcpy(&m_queue_event_data[event_index * m_queue_event_size],
                       p_event_data,
                       event_data_size);
                m_queue_event_headers[event_index].event_data_size = event_data_size;
            }
            else
            {
                m_queue_event_headers[event_index].event_data_size = 0;
            }

            err_code = NRF_SUCCESS;
        }
        else
        {
            err_code = NRF_ERROR_NO_MEM;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_LENGTH;
    }

    return err_code;
}


#if APP_SCHEDULER_WITH_PAUSE
void app_sched_pause(void)
{
    CRITICAL_REGION_ENTER();

    if (m_scheduler_paused_counter < UINT32_MAX)
    {
        m_scheduler_paused_counter++;
    }
    CRITICAL_REGION_EXIT();
}

void app_sched_resume(void)
{
    CRITICAL_REGION_ENTER();

    if (m_scheduler_paused_counter > 0)
    {
        m_scheduler_paused_counter--;
    }
    CRITICAL_REGION_EXIT();
}
#endif //APP_SCHEDULER_WITH_PAUSE


/**@brief Function for checking if scheduler is paused which means that should break processing
 *        events.
 *
 * @return    Boolean value - true if scheduler is paused, false otherwise.
 */
static __INLINE bool is_app_sched_paused(void)
{
#if APP_SCHEDULER_WITH_PAUSE
    return (m_scheduler_paused_counter > 0);
#else
    return false;
#endif
}


void app_sched_execute(void)
{
    while (!is_app_sched_paused() && !APP_SCHED_QUEUE_EMPTY())
    {
        // Since this function is only called from the main loop, there is no
        // need for a critical region here, however a special care must be taken
        // regarding update of the queue start index (see the end of the loop).
        uint16_t event_index = m_queue_start_index;

        void * p_event_data;
        uint16_t event_data_size;
        app_sched_event_handler_t event_handler;

        p_event_data = &m_queue_event_data[event_index * m_queue_event_size];
        event_data_size = m_queue_event_headers[event_index].event_data_size;
        event_handler   = m_queue_event_headers[event_index].handler;

        event_handler(p_event_data, event_data_size);

        // Event processed, now it is safe to move the queue start index,
        // so the queue entry occupied by this event can be used to store
        // a next one.
        m_queue_start_index = next_index(m_queue_start_index);
    }
}
#endif //APP_SCHEDULER_ENABLED
