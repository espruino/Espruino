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

#include "app_timer.h"
#include <stdlib.h>
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "sdk_common.h"

#define RTC1_IRQ_PRI            APP_IRQ_PRIORITY_LOW                        /**< Priority of the RTC1 interrupt (used for checking for timeouts and executing timeout handlers). */
#define SWI_IRQ_PRI             APP_IRQ_PRIORITY_LOW                        /**< Priority of the SWI  interrupt (used for updating the timer list). */

// The current design assumes that both interrupt handlers run at the same interrupt level.
// If this is to be changed, protection must be added to prevent them from interrupting each other
// (e.g. by using guard/trigger flags).
STATIC_ASSERT(RTC1_IRQ_PRI == SWI_IRQ_PRI);

#define MAX_RTC_COUNTER_VAL     0x00FFFFFF                                  /**< Maximum value of the RTC counter. */

#define APP_HIGH_USER_ID        0                                           /**< User Id for the Application High "user". */
#define APP_LOW_USER_ID         1                                           /**< User Id for the Application Low "user". */
#define THREAD_MODE_USER_ID     2                                           /**< User Id for the Thread Mode "user". */

#define RTC_COMPARE_OFFSET_MIN  3                                           /**< Minimum offset between the current RTC counter value and the Capture Compare register. Although the nRF51 Series User Specification recommends this value to be 2, we use 3 to be safer.*/

#define MAX_RTC_TASKS_DELAY     47                                          /**< Maximum delay until an RTC task is executed. */

#ifdef NRF51
#define SWI_IRQn SWI1_IRQn
#define SWI_IRQHandler SWI1_IRQHandler
#elif defined NRF52
#define SWI_IRQn SWI1_EGU1_IRQn
#define SWI_IRQHandler SWI1_EGU1_IRQHandler
#endif

/**@brief Timer node type. The nodes will be used form a linked list of running timers. */
typedef struct
{
    uint32_t                    ticks_to_expire;                            /**< Number of ticks from previous timer interrupt to timer expiry. */
    uint32_t                    ticks_at_start;                             /**< Current RTC counter value when the timer was started. */
    uint32_t                    ticks_first_interval;                       /**< Number of ticks in the first timer interval. */
    uint32_t                    ticks_periodic_interval;                    /**< Timer period (for repeating timers). */
    bool                        is_running;                                 /**< True if timer is running, False otherwise. */
    app_timer_mode_t            mode;                                       /**< Timer mode. */
    app_timer_timeout_handler_t p_timeout_handler;                          /**< Pointer to function to be executed when the timer expires. */
    void *                      p_context;                                  /**< General purpose pointer. Will be passed to the timeout handler when the timer expires. */
    void *                      next;                                       /**< Pointer to the next node. */
} timer_node_t;

STATIC_ASSERT(sizeof(timer_node_t) == APP_TIMER_NODE_SIZE);

/**@brief Set of available timer operation types. */
typedef enum
{
    TIMER_USER_OP_TYPE_NONE,                                                /**< Invalid timer operation type. */
    TIMER_USER_OP_TYPE_START,                                               /**< Timer operation type Start. */
    TIMER_USER_OP_TYPE_STOP,                                                /**< Timer operation type Stop. */
    TIMER_USER_OP_TYPE_STOP_ALL                                             /**< Timer operation type Stop All. */
} timer_user_op_type_t;

/**@brief Structure describing a timer start operation. */
typedef struct
{
    uint32_t ticks_at_start;                                                /**< Current RTC counter value when the timer was started. */
    uint32_t ticks_first_interval;                                          /**< Number of ticks in the first timer interval. */
    uint32_t ticks_periodic_interval;                                       /**< Timer period (for repeating timers). */
    void *   p_context;                                                     /**< General purpose pointer. Will be passed to the timeout handler when the timer expires. */
} timer_user_op_start_t;

/**@brief Structure describing a timer operation. */
typedef struct
{
    timer_user_op_type_t op_type;                                             /**< Id of timer on which the operation is to be performed. */
    timer_node_t *       p_node;
    union
    {
        timer_user_op_start_t start;                                        /**< Structure describing a timer start operation. */
    } params;
} timer_user_op_t;

STATIC_ASSERT(sizeof(timer_user_op_t) <= APP_TIMER_USER_OP_SIZE);
STATIC_ASSERT(sizeof(timer_user_op_t) % 4 == 0);

/**@brief Structure describing a timer user.
 *
 * @details For each user of the timer module, there will be a timer operations queue. This queue
 *          will hold timer operations issued by this user until the timer interrupt handler
 *          processes these operations. For the current implementation, there will be one user for
 *          each interrupt level available to the application (APP_HIGH, APP_LOW and THREAD_MODE),
 *          but the module can easily be modified to e.g. have one queue per process when using an
 *          RTOS. The purpose of the queues is to be able to have a completely lockless timer
 *          implementation.
 */
typedef struct
{
    uint8_t           first;                                                    /**< Index of first entry to have been inserted in the queue (i.e. the next entry to be executed). */
    uint8_t           last;                                                     /**< Index of last entry to have been inserted in the queue. */
    uint8_t           user_op_queue_size;                                       /**< Queue size. */
    timer_user_op_t * p_user_op_queue;                                          /**< Queue buffer. */
} timer_user_t;

STATIC_ASSERT(sizeof(timer_user_t) == APP_TIMER_USER_SIZE);
STATIC_ASSERT(sizeof(timer_user_t) % 4 == 0);

/**@brief User id type.
 *
 * @details In the current implementation, this will automatically be generated from the current
 *          interrupt level.
 */
typedef uint32_t timer_user_id_t;

#define CONTEXT_QUEUE_SIZE_MAX      (2)                 

static uint8_t                       m_user_array_size;                         /**< Size of timer user array. */
static timer_user_t *                mp_users = NULL;                           /**< Array of timer users. */
static timer_node_t *                mp_timer_id_head;                          /**< First timer in list of running timers. */
static uint32_t                      m_ticks_latest;                            /**< Last known RTC counter value. */
static uint32_t                      m_ticks_elapsed[CONTEXT_QUEUE_SIZE_MAX];   /**< Timer internal elapsed ticks queue. */
static uint8_t                       m_ticks_elapsed_q_read_ind;                /**< Timer internal elapsed ticks queue read index. */
static uint8_t                       m_ticks_elapsed_q_write_ind;               /**< Timer internal elapsed ticks queue write index. */
static app_timer_evt_schedule_func_t m_evt_schedule_func;                       /**< Pointer to function for propagating timeout events to the scheduler. */
static bool                          m_rtc1_running;                            /**< Boolean indicating if RTC1 is running. */
static bool                          m_rtc1_reset;                              /**< Boolean indicating if RTC1 counter has been reset due to last timer removed from timer list during the timer list handling. */
 

#define MODULE_INITIALIZED (mp_users != NULL)
#include "sdk_macros.h"


/**@brief Function for initializing the RTC1 counter.
 *
 * @param[in] prescaler   Value of the RTC1 PRESCALER register. Set to 0 for no prescaling.
 */
static void rtc1_init(uint32_t prescaler)
{
    NRF_RTC1->PRESCALER = prescaler;
    NVIC_SetPriority(RTC1_IRQn, RTC1_IRQ_PRI);
}


/**@brief Function for starting the RTC1 timer.
 */
static void rtc1_start(void)
{
    NRF_RTC1->EVTENSET = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Msk;

    NVIC_ClearPendingIRQ(RTC1_IRQn);
    NVIC_EnableIRQ(RTC1_IRQn);

    NRF_RTC1->TASKS_START = 1;
    nrf_delay_us(MAX_RTC_TASKS_DELAY);

    m_rtc1_running = true;
}


/**@brief Function for stopping the RTC1 timer.
 */
static void rtc1_stop(void)
{
    NVIC_DisableIRQ(RTC1_IRQn);

    NRF_RTC1->EVTENCLR = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENCLR = RTC_INTENSET_COMPARE0_Msk;

    NRF_RTC1->TASKS_STOP = 1;
    nrf_delay_us(MAX_RTC_TASKS_DELAY);

    NRF_RTC1->TASKS_CLEAR = 1;
    m_ticks_latest        = 0;
    nrf_delay_us(MAX_RTC_TASKS_DELAY);

    m_rtc1_running = false;
}


/**@brief Function for returning the current value of the RTC1 counter.
 *
 * @return     Current value of the RTC1 counter.
 */
static __INLINE uint32_t rtc1_counter_get(void)
{
    return NRF_RTC1->COUNTER;
}


/**@brief Function for computing the difference between two RTC1 counter values.
 *
 * @return     Number of ticks elapsed from ticks_old to ticks_now.
 */
static __INLINE uint32_t ticks_diff_get(uint32_t ticks_now, uint32_t ticks_old)
{
    return ((ticks_now - ticks_old) & MAX_RTC_COUNTER_VAL);
}


/**@brief Function for setting the RTC1 Capture Compare register 0, and enabling the corresponding
 *        event.
 *
 * @param[in] value   New value of Capture Compare register 0.
 */
static __INLINE void rtc1_compare0_set(uint32_t value)
{
    NRF_RTC1->CC[0] = value;
}


/**@brief Function for inserting a timer in the timer list.
 *
 * @param[in]  timer_id   Id of timer to insert.
 */
static void timer_list_insert(timer_node_t * p_timer)
{
    if (mp_timer_id_head == NULL)
    {
        mp_timer_id_head = p_timer;
    }
    else
    {
        if (p_timer->ticks_to_expire <= mp_timer_id_head->ticks_to_expire)
        {
            mp_timer_id_head->ticks_to_expire -= p_timer->ticks_to_expire;
            
            p_timer->next   = mp_timer_id_head;
            mp_timer_id_head = p_timer;
        }
        else
        {
            timer_node_t * p_previous;
            timer_node_t * p_current;
            uint32_t       ticks_to_expire;

            ticks_to_expire   = p_timer->ticks_to_expire;
            p_previous        = mp_timer_id_head;
            p_current         = mp_timer_id_head;
            
            while ((p_current != NULL) && (ticks_to_expire > p_current->ticks_to_expire))
            {
                ticks_to_expire   -= p_current->ticks_to_expire;
                p_previous         = p_current;
                p_current          = p_current->next;
            }

            if (p_current != NULL)
            {
                p_current->ticks_to_expire -= ticks_to_expire;
            }

            p_timer->ticks_to_expire = ticks_to_expire;
            p_timer->next            = p_current;
            p_previous->next         = p_timer;
        }
    }
}


/**@brief Function for removing a timer from the timer queue.
 *
 * @param[in]  timer_id   Id of timer to remove.
 */
static void timer_list_remove(timer_node_t * p_timer)
{
    timer_node_t * p_previous;
    timer_node_t * p_current;
    uint32_t       timeout;

    // Find the timer's position in timer list.
    p_previous = mp_timer_id_head;
    p_current  = p_previous;
    
    while (p_current != NULL)
    {
        if (p_current == p_timer)
        {
            break;
        }
        p_previous = p_current;
        p_current  = p_current->next;
    }

    // Timer not in active list.
    if (p_current == NULL)
    {
        return;
    }

    // Timer is the first in the list
    if (p_previous == p_current)
    {
        mp_timer_id_head = mp_timer_id_head->next;

        // No more timers in the list. Reset RTC1 in case Start timer operations are present in the queue.
        if (mp_timer_id_head == NULL)
        {
            NRF_RTC1->TASKS_CLEAR = 1;
            m_ticks_latest        = 0;
            m_rtc1_reset          = true;
        }
    }

    // Remaining timeout between next timeout.
    timeout = p_current->ticks_to_expire;

    // Link previous timer with next of this timer, i.e. removing the timer from list.
    p_previous->next = p_current->next;

    // If this is not the last timer, increment the next timer by this timer timeout.
    p_current = p_previous->next;
    if (p_current != NULL)
    {
        p_current->ticks_to_expire += timeout;
    }
}


/**@brief Function for scheduling a check for timeouts by generating a RTC1 interrupt.
 */
static void timer_timeouts_check_sched(void)
{
    NVIC_SetPendingIRQ(RTC1_IRQn);
}


/**@brief Function for scheduling a timer list update by generating a SWI interrupt.
 */
static void timer_list_handler_sched(void)
{
    NVIC_SetPendingIRQ(SWI_IRQn);
}


/**@brief Function for executing an application timeout handler, either by calling it directly, or
 *        by passing an event to the @ref app_scheduler.
 *
 * @param[in]  p_timer   Pointer to expired timer.
 */
static void timeout_handler_exec(timer_node_t * p_timer)
{
    if (m_evt_schedule_func != NULL)
    {
        uint32_t err_code = m_evt_schedule_func(p_timer->p_timeout_handler, p_timer->p_context);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        p_timer->p_timeout_handler(p_timer->p_context);
    }
}


/**@brief Function for checking for expired timers.
 */
static void timer_timeouts_check(void)
{
    // Handle expired of timer 
    if (mp_timer_id_head != NULL)
    {
        timer_node_t *  p_timer;
        timer_node_t *  p_previous_timer;
        uint32_t        ticks_elapsed;
        uint32_t        ticks_expired;

        // Initialize actual elapsed ticks being consumed to 0.
        ticks_expired = 0;

        // ticks_elapsed is collected here, job will use it.
        ticks_elapsed = ticks_diff_get(rtc1_counter_get(), m_ticks_latest);

        // Auto variable containing the head of timers expiring.
        p_timer = mp_timer_id_head;

        // Expire all timers within ticks_elapsed and collect ticks_expired.
        while (p_timer != NULL)
        {
            // Do nothing if timer did not expire.
            if (ticks_elapsed < p_timer->ticks_to_expire)
            {
                break;
            }

            // Decrement ticks_elapsed and collect expired ticks.
            ticks_elapsed -= p_timer->ticks_to_expire;
            ticks_expired += p_timer->ticks_to_expire;

            // Move to next timer.
            p_previous_timer = p_timer;
            p_timer = p_timer->next;

            // Execute Task.
            timeout_handler_exec(p_previous_timer);
        }

        // Prepare to queue the ticks expired in the m_ticks_elapsed queue.
        if (m_ticks_elapsed_q_read_ind == m_ticks_elapsed_q_write_ind)
        {
            // The read index of the queue is equal to the write index. This means the new
            // value of ticks_expired should be stored at a new location in the m_ticks_elapsed
            // queue (which is implemented as a double buffer).

            // Check if there will be a queue overflow.
            if (++m_ticks_elapsed_q_write_ind == CONTEXT_QUEUE_SIZE_MAX)
            {
                // There will be a queue overflow. Hence the write index should point to the start
                // of the queue.
                m_ticks_elapsed_q_write_ind = 0;
            }
        }

        // Queue the ticks expired.
        m_ticks_elapsed[m_ticks_elapsed_q_write_ind] = ticks_expired;

        timer_list_handler_sched();
    }
}


/**@brief Function for acquiring the number of ticks elapsed.
 *
 * @param[out] p_ticks_elapsed   Number of ticks elapsed.
 *
 * @return     TRUE if elapsed ticks was read from queue, FALSE otherwise.
 */
static bool elapsed_ticks_acquire(uint32_t * p_ticks_elapsed)
{
    // Pick the elapsed value from queue.
    if (m_ticks_elapsed_q_read_ind != m_ticks_elapsed_q_write_ind)
    {
        // Dequeue elapsed value.
        m_ticks_elapsed_q_read_ind++;
        if (m_ticks_elapsed_q_read_ind == CONTEXT_QUEUE_SIZE_MAX)
        {
            m_ticks_elapsed_q_read_ind = 0;
        }

        *p_ticks_elapsed = m_ticks_elapsed[m_ticks_elapsed_q_read_ind];

        m_ticks_latest += *p_ticks_elapsed;
        m_ticks_latest &= MAX_RTC_COUNTER_VAL;

        return true;
    }
    else
    {
        // No elapsed value in queue.
        *p_ticks_elapsed = 0;
        return false;
    }
}


/**@brief Function for handling the timer list deletions.
 *
 * @return     TRUE if Capture Compare register must be updated, FALSE otherwise.
 */
static bool list_deletions_handler(void)
{
    timer_node_t * p_timer_old_head;
    uint8_t        user_id;

    // Remember the old head, so as to decide if new compare needs to be set.
    p_timer_old_head = mp_timer_id_head;

    user_id = m_user_array_size;
    while (user_id--)
    {
        timer_user_t * p_user         = &mp_users[user_id];
        uint8_t        user_ops_first = p_user->first;
        
        while (user_ops_first != p_user->last)
        {
            timer_node_t *    p_timer;
            timer_user_op_t * p_user_op = &p_user->p_user_op_queue[user_ops_first];

            // Traverse to next operation in queue.
            user_ops_first++;
            if (user_ops_first == p_user->user_op_queue_size)
            {
                user_ops_first = 0;
            }

            switch (p_user_op->op_type)
            {
                case TIMER_USER_OP_TYPE_STOP:
                    // Delete node if timer is running.
                    p_timer = p_user_op->p_node;
                    if (p_timer->is_running)
                    {
                        timer_list_remove(p_user_op->p_node);
                        p_timer->is_running = false;
                    }
                    break;
                    
                case TIMER_USER_OP_TYPE_STOP_ALL:
                    // Delete list of running timers, and mark all timers as not running.
                    while (mp_timer_id_head != NULL)
                    {
                        timer_node_t * p_head = mp_timer_id_head;

                        p_head->is_running = false;
                        mp_timer_id_head    = p_head->next;
                    }
                    break;
                    
                default:
                    // No implementation needed.
                    break;
            }
        }
    }

    // Detect change in head of the list.
    return (mp_timer_id_head != p_timer_old_head);
}


/**@brief Function for updating the timer list for expired timers.
 *
 * @param[in]  ticks_elapsed         Number of elapsed ticks.
 * @param[in]  ticks_previous        Previous known value of the RTC counter.
 * @param[out] p_restart_list_head   List of repeating timers to be restarted.
 */
static void expired_timers_handler(uint32_t         ticks_elapsed,
                                   uint32_t         ticks_previous,
                                   timer_node_t **  p_restart_list_head)
{
    uint32_t ticks_expired = 0;

    while (mp_timer_id_head != NULL)
    {
        timer_node_t * p_timer;
        timer_node_t * p_timer_expired;

        // Auto variable for current timer node.
        p_timer = mp_timer_id_head;

        // Do nothing if timer did not expire 
        if (ticks_elapsed < p_timer->ticks_to_expire)
        {
            p_timer->ticks_to_expire -= ticks_elapsed;
            break;
        }

        // Decrement ticks_elapsed and collect expired ticks.
        ticks_elapsed -= p_timer->ticks_to_expire;
        ticks_expired += p_timer->ticks_to_expire;

        // Timer expired, set ticks_to_expire zero.
        p_timer->ticks_to_expire = 0;
        p_timer->is_running      = false;

        // Remove the expired timer from head.
        p_timer_expired  = mp_timer_id_head;
        mp_timer_id_head = p_timer->next;

        // Timer will be restarted if periodic.
        if (p_timer->ticks_periodic_interval != 0)
        {
            p_timer->ticks_at_start       = (ticks_previous + ticks_expired) & MAX_RTC_COUNTER_VAL;
            p_timer->ticks_first_interval = p_timer->ticks_periodic_interval;
            p_timer->next                 = *p_restart_list_head;
            *p_restart_list_head          = p_timer_expired;
        }
    }
}


/**@brief Function for handling timer list insertions.
 *
 * @param[in]  p_restart_list_head   List of repeating timers to be restarted.
 *
 * @return     TRUE if Capture Compare register must be updated, FALSE otherwise.
 */
static bool list_insertions_handler(timer_node_t * p_restart_list_head)
{
    timer_node_t * p_timer_id_old_head;
    uint8_t        user_id;

    // Remember the old head, so as to decide if new compare needs to be set.
    p_timer_id_old_head = mp_timer_id_head;

    user_id = m_user_array_size;
    while (user_id--)
    {
        timer_user_t * p_user = &mp_users[user_id];

        // Handle insertions of timers.
        while ((p_restart_list_head != NULL) || (p_user->first != p_user->last))
        {
            timer_node_t * p_timer;

            if (p_restart_list_head != NULL)
            {
                p_timer           = p_restart_list_head;
                p_restart_list_head = p_timer->next;
            }
            else
            {
                timer_user_op_t * p_user_op = &p_user->p_user_op_queue[p_user->first];

                p_user->first++;
                if (p_user->first == p_user->user_op_queue_size)
                {
                    p_user->first = 0;
                }

                p_timer = p_user_op->p_node;

                if ((p_user_op->op_type != TIMER_USER_OP_TYPE_START) || p_timer->is_running)
                {
                    continue;
                }

                p_timer->ticks_at_start          = p_user_op->params.start.ticks_at_start;
                p_timer->ticks_first_interval    = p_user_op->params.start.ticks_first_interval;
                p_timer->ticks_periodic_interval = p_user_op->params.start.ticks_periodic_interval;
                p_timer->p_context               = p_user_op->params.start.p_context;

                if (m_rtc1_reset)
                {
                    p_timer->ticks_at_start = 0;
                }
            }

            // Prepare the node to be inserted.
            if (
                 ((p_timer->ticks_at_start - m_ticks_latest) & MAX_RTC_COUNTER_VAL)
                 <
                 (MAX_RTC_COUNTER_VAL / 2)
                )
            {
                p_timer->ticks_to_expire = ticks_diff_get(p_timer->ticks_at_start, m_ticks_latest) + 
                                           p_timer->ticks_first_interval;
            }
            else
            {
                uint32_t delta_current_start;

                delta_current_start = ticks_diff_get(m_ticks_latest, p_timer->ticks_at_start);
                if (p_timer->ticks_first_interval > delta_current_start)
                {
                    p_timer->ticks_to_expire = p_timer->ticks_first_interval - delta_current_start;
                }
                else
                {
                    p_timer->ticks_to_expire = 0;
                }
            }

            p_timer->ticks_at_start       = 0;
            p_timer->ticks_first_interval = 0;
            p_timer->is_running           = true;
            p_timer->next                 = NULL;

            // Insert into list 
            timer_list_insert(p_timer);
        }
    }
    
    return (mp_timer_id_head != p_timer_id_old_head);
}


/**@brief Function for updating the Capture Compare register.
 */
static void compare_reg_update(timer_node_t * p_timer_id_head_old)
{
    // Setup the timeout for timers on the head of the list 
    if (mp_timer_id_head != NULL)
    {
        uint32_t ticks_to_expire = mp_timer_id_head->ticks_to_expire;
        uint32_t pre_counter_val = rtc1_counter_get();
        uint32_t cc              = m_ticks_latest;
        uint32_t ticks_elapsed   = ticks_diff_get(pre_counter_val, cc) + RTC_COMPARE_OFFSET_MIN;

        if (!m_rtc1_running)
        {
            // No timers were already running, start RTC
            rtc1_start();
        }

        cc += (ticks_elapsed < ticks_to_expire) ? ticks_to_expire : ticks_elapsed;
        cc &= MAX_RTC_COUNTER_VAL;
        
        rtc1_compare0_set(cc);

        uint32_t post_counter_val = rtc1_counter_get();

        if (
            (ticks_diff_get(post_counter_val, pre_counter_val) + RTC_COMPARE_OFFSET_MIN)
            >
            ticks_diff_get(cc, pre_counter_val)
           )
        {
            // When this happens the COMPARE event may not be triggered by the RTC.
            // The nRF51 Series User Specification states that if the COUNTER value is N
            // (i.e post_counter_val = N), writing N or N+1 to a CC register may not trigger a
            // COMPARE event. Hence the RTC interrupt is forcefully pended by calling the following
            // function.
            timer_timeouts_check_sched();
        }
    }
    else
    {
        // No timers are running, stop RTC
        rtc1_stop();
    }
}


/**@brief Function for handling changes to the timer list.
 */
static void timer_list_handler(void)
{
    timer_node_t * p_restart_list_head = NULL;

    uint32_t       ticks_elapsed;
    uint32_t       ticks_previous;
    bool           ticks_have_elapsed;
    bool           compare_update;
    timer_node_t * p_timer_id_head_old;
    
    // Back up the previous known tick and previous list head
    ticks_previous    = m_ticks_latest;
    p_timer_id_head_old = mp_timer_id_head;
    
    // Get number of elapsed ticks
    ticks_have_elapsed = elapsed_ticks_acquire(&ticks_elapsed);

    // Handle list deletions
    compare_update = list_deletions_handler();
    
    // Handle expired timers
    if (ticks_have_elapsed)
    {
        expired_timers_handler(ticks_elapsed, ticks_previous, &p_restart_list_head);
        compare_update = true;
    }
    
    // Handle list insertions
    if (list_insertions_handler(p_restart_list_head))
    {
        compare_update = true;
    }

    // Update compare register if necessary
    if (compare_update)
    {
        compare_reg_update(p_timer_id_head_old);
    }
    m_rtc1_reset = false;
}


/**@brief Function for enqueueing a new operations queue entry.
 *
 * @param[in]  p_user     User that the entry is to be enqueued for.
 * @param[in]  last_index Index of the next last index to be enqueued.
 */
static void user_op_enque(timer_user_t * p_user, uint8_t last_index)
{
    p_user->last = last_index;
}


/**@brief Function for allocating a new operations queue entry.
 *
 * @param[in]  p_user       User that the entry is to be allocated for.
 * @param[out] p_last_index Index of the next last index to be enqueued.
 *
 * @return     Pointer to allocated queue entry, or NULL if queue is full.
 */
static timer_user_op_t * user_op_alloc(timer_user_t * p_user, uint8_t * p_last_index)
{        
    uint8_t           last;
    timer_user_op_t * p_user_op;
    
    last = p_user->last + 1;
    if (last == p_user->user_op_queue_size)
    {
        // Overflow case.
        last = 0;
    }
    if (last == p_user->first)
    {
        // Queue is full.
        return NULL;
    }
    
    *p_last_index = last;    
    p_user_op     = &p_user->p_user_op_queue[p_user->last];
        
    return p_user_op;
}


/**@brief Function for scheduling a Timer Start operation.
 *
 * @param[in]  user_id           Id of user calling this function.
 * @param[in]  timer_id          Id of timer to start.
 * @param[in]  timeout_initial   Time (in ticks) to first timer expiry.
 * @param[in]  timeout_periodic  Time (in ticks) between periodic expiries.
 * @param[in]  p_context         General purpose pointer. Will be passed to the timeout handler when
 *                               the timer expires.
 * @return     NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t timer_start_op_schedule(timer_user_id_t user_id,
                                        timer_node_t * p_node,
                                        uint32_t        timeout_initial,
                                        uint32_t        timeout_periodic,
                                        void *          p_context)
{
    uint8_t last_index;
    
    timer_user_op_t * p_user_op = user_op_alloc(&mp_users[user_id], &last_index);
    if (p_user_op == NULL)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    p_user_op->op_type                              = TIMER_USER_OP_TYPE_START;
    p_user_op->p_node                               = p_node;
    p_user_op->params.start.ticks_at_start          = rtc1_counter_get();
    p_user_op->params.start.ticks_first_interval    = timeout_initial;
    p_user_op->params.start.ticks_periodic_interval = timeout_periodic;
    p_user_op->params.start.p_context               = p_context;
    
    user_op_enque(&mp_users[user_id], last_index);    

    timer_list_handler_sched();

    return NRF_SUCCESS;
}


/**@brief Function for scheduling a Timer Stop operation.
 *
 * @param[in]  user_id    Id of user calling this function.
 * @param[in]  timer_id   Id of timer to stop.
 *
 * @return NRF_SUCCESS on successful scheduling a timer stop operation. NRF_ERROR_NO_MEM when there
 *         is no memory left to schedule the timer stop operation.
 */
static uint32_t timer_stop_op_schedule(timer_user_id_t user_id, timer_node_t * p_node)
{
    uint8_t last_index;
    
    timer_user_op_t * p_user_op = user_op_alloc(&mp_users[user_id], &last_index);
    if (p_user_op == NULL)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    p_user_op->op_type  = TIMER_USER_OP_TYPE_STOP;
    p_user_op->p_node = p_node;
    
    user_op_enque(&mp_users[user_id], last_index);        

    timer_list_handler_sched();

    return NRF_SUCCESS;
}


/**@brief Function for scheduling a Timer Stop All operation.
 *
 * @param[in]  user_id    Id of user calling this function.
 */
static uint32_t timer_stop_all_op_schedule(timer_user_id_t user_id)
{
    uint8_t last_index;
    
    timer_user_op_t * p_user_op = user_op_alloc(&mp_users[user_id], &last_index);
    if (p_user_op == NULL)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    p_user_op->op_type  = TIMER_USER_OP_TYPE_STOP_ALL;
    p_user_op->p_node = NULL;
    
    user_op_enque(&mp_users[user_id], last_index);        

    timer_list_handler_sched();

    return NRF_SUCCESS;
}


/**@brief Function for handling the RTC1 interrupt.
 *
 * @details Checks for timeouts, and executes timeout handlers for expired timers.
 */
void RTC1_IRQHandler(void)
{
    // Clear all events (also unexpected ones)
    NRF_RTC1->EVENTS_COMPARE[0] = 0;
    NRF_RTC1->EVENTS_COMPARE[1] = 0;
    NRF_RTC1->EVENTS_COMPARE[2] = 0;
    NRF_RTC1->EVENTS_COMPARE[3] = 0;
    NRF_RTC1->EVENTS_TICK       = 0;
    NRF_RTC1->EVENTS_OVRFLW     = 0;

    // Check for expired timers
    timer_timeouts_check();
}


/**@brief Function for handling the SWI interrupt.
 *
 * @details Performs all updates to the timer list.
 */
void SWI_IRQHandler(void)
{
    timer_list_handler();
}


uint32_t app_timer_init(uint32_t                      prescaler,
                        uint8_t                       op_queues_size,
                        void *                        p_buffer,
                        app_timer_evt_schedule_func_t evt_schedule_func)
{
    int i;

    // Check that buffer is correctly aligned
    if (!is_word_aligned(p_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    // Check for NULL buffer
    if (p_buffer == NULL)
    {
        mp_users = NULL;
        return NRF_ERROR_INVALID_PARAM;
    }
    
    // Stop RTC to prevent any running timers from expiring (in case of reinitialization)
    rtc1_stop();
    
    m_evt_schedule_func = evt_schedule_func;
    
    // Initialize users array
    m_user_array_size = APP_TIMER_INT_LEVELS;
    mp_users          = p_buffer;
    
    // Skip user array
    p_buffer = &((uint8_t *)p_buffer)[APP_TIMER_INT_LEVELS * sizeof(timer_user_t)];

    // Initialize operation queues
    for (i = 0; i < APP_TIMER_INT_LEVELS; i++)
    {
        timer_user_t * p_user = &mp_users[i];
        
        p_user->first              = 0;
        p_user->last               = 0;
        p_user->user_op_queue_size = op_queues_size;
        p_user->p_user_op_queue    = p_buffer;
    
        // Skip operation queue
        p_buffer = &((uint8_t *)p_buffer)[op_queues_size * sizeof(timer_user_op_t)];
    }

    mp_timer_id_head             = NULL;
    m_ticks_elapsed_q_read_ind  = 0;
    m_ticks_elapsed_q_write_ind = 0;

    NVIC_ClearPendingIRQ(SWI_IRQn);
    NVIC_SetPriority(SWI_IRQn, SWI_IRQ_PRI);
    NVIC_EnableIRQ(SWI_IRQn);

    rtc1_init(prescaler);

    m_ticks_latest = rtc1_counter_get();
    
    return NRF_SUCCESS;
}


uint32_t app_timer_create(app_timer_id_t const *      p_timer_id,
                          app_timer_mode_t            mode,
                          app_timer_timeout_handler_t timeout_handler)
{
    // Check state and parameters
    VERIFY_MODULE_INITIALIZED();

    if (timeout_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_timer_id == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (((timer_node_t*)*p_timer_id)->is_running)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    timer_node_t * p_node     = (timer_node_t *)*p_timer_id;
    p_node->is_running        = false;
    p_node->mode              = mode;
    p_node->p_timeout_handler = timeout_handler;
    return NRF_SUCCESS;
}


/**@brief Function for creating a timer user id from the current interrupt level.
 *
 * @return     Timer user id.
*/
static timer_user_id_t user_id_get(void)
{
    timer_user_id_t ret;

    STATIC_ASSERT(APP_TIMER_INT_LEVELS == 3);
    
    switch (current_int_priority_get())
    {
        case APP_IRQ_PRIORITY_HIGH:
            ret = APP_HIGH_USER_ID;
            break;
            
        case APP_IRQ_PRIORITY_LOW:
            ret = APP_LOW_USER_ID;
            break;
            
        default:
            ret = THREAD_MODE_USER_ID;
            break;
    }
    
    return ret;
}


uint32_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ticks, void * p_context)
{
    uint32_t timeout_periodic;
    timer_node_t * p_node = (timer_node_t*)timer_id;
    
    // Check state and parameters
    VERIFY_MODULE_INITIALIZED();

    if (timer_id == 0)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (timeout_ticks < APP_TIMER_MIN_TIMEOUT_TICKS)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_node->p_timeout_handler == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    // Schedule timer start operation
    timeout_periodic = (p_node->mode == APP_TIMER_MODE_REPEATED) ? timeout_ticks : 0;

    return timer_start_op_schedule(user_id_get(),
                                   p_node,
                                   timeout_ticks,
                                   timeout_periodic,
                                   p_context);
}


uint32_t app_timer_stop(app_timer_id_t timer_id)
{
    timer_node_t * p_node = (timer_node_t*)timer_id;
    // Check state and parameters
    VERIFY_MODULE_INITIALIZED();

    if ((timer_id == NULL) || (p_node->p_timeout_handler == NULL))
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    // Schedule timer stop operation
    return timer_stop_op_schedule(user_id_get(), p_node);
}


uint32_t app_timer_stop_all(void)
{
    // Check state
    VERIFY_MODULE_INITIALIZED();

    return timer_stop_all_op_schedule(user_id_get());
}


uint32_t app_timer_cnt_get(uint32_t * p_ticks)
{
    *p_ticks = rtc1_counter_get();
    return NRF_SUCCESS;
}


uint32_t app_timer_cnt_diff_compute(uint32_t   ticks_to,
                                    uint32_t   ticks_from,
                                    uint32_t * p_ticks_diff)
{
    *p_ticks_diff = ticks_diff_get(ticks_to, ticks_from);
    return NRF_SUCCESS;
}

