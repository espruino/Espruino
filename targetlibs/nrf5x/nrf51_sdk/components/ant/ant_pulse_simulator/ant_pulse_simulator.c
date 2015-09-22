
#include "ant_pulse_simulator.h"
#include "sensorsim.h"

/**
 * @addtogroup ant_hrm_pulse_sim  Puls rate simulator for ANT hrm
 * @{
 */
 
#define ITERATION_ANT_CYCLES    HRM_MSG_PERIOD_4Hz ///< period of calculation [1/32678 s], defined in ANT device profile
#define ITERATION_PERIOD (ITERATION_ANT_CYCLES * 1024 / ANT_CLOCK_FREQUENCY) ///< integer part of calculation's period [1/1024 s]
#define ITERATION_FRACTION  (ITERATION_ANT_CYCLES * 1024 % ANT_CLOCK_FREQUENCY) ///< fractional part of calculation's period [1/32678 s]
 
static const sensorsim_cfg_t m_sensorsim_cfg = /**< Configuration for sensorsim */
{
    .min = MIN_HEART_RATE_VALUE,
    .max = MAX_HEART_RATE_VALUE,
    .incr = HEART_RATE_VALUE_INCREMENT,
    .start_at_max = false // star from minimal value
};
 
static ant_hrm_profile_t * mp_related_hrm_profile; /**< Pointer to used hrm profile data instance */
static uint32_t            m_heart_rate_value;     /**< Instantaneous heart rate value. */
static sensorsim_state_t   m_sensorsim_state;      /**< Sate of used sensorsim module. */
static bool m_auto_change;                         /**< Value of enabling @ref sensorsim module to perform heart rate changes */
static uint32_t m_time_since_last_hb;                     /**< Time since last heart beat occurred in: integer part */
static uint32_t m_fraction_since_last_hb;                 /**< Time since last heart beat occurred in: fractional part */


void puls_simulate_init( ant_hrm_profile_t * p_related_profile, uint8_t init_heart_rate, bool auto_change)
{    
    mp_related_hrm_profile = p_related_profile;
    
    m_heart_rate_value = init_heart_rate;
    
    m_auto_change = auto_change;
    
    if (m_auto_change)
    {
        sensorsim_init( &m_sensorsim_state, &m_sensorsim_cfg);
    }
    
    m_time_since_last_hb = 0;
    m_fraction_since_last_hb = 0;
}

void pulse_simulate_one_iteration(void)
{    
    if (m_auto_change)
    {
        m_heart_rate_value = sensorsim_measure( &m_sensorsim_state, &m_sensorsim_cfg);
    }
    
    // @note: Take a local copy within scope in order to assist the compiler in variable register 
    // allocation. 
    const uint32_t computed_heart_rate_value = m_heart_rate_value;
    
    

    // @note: This implementation assumes that the current instantaneous heart can vary and this
    // function is called with static frequency.
    // value and the heart rate pulse interval is derived from it. The computation is based on 60 
    // seconds in a minute and the used time base is 1/1024 seconds.
    const uint32_t current_hb_pulse_interval = (60u * 1024u) / computed_heart_rate_value;
    
    //update time from last hb detected
    m_time_since_last_hb += ITERATION_PERIOD;

    // extended celculadion by fraction make calculating accurat in long time perspective
    m_fraction_since_last_hb += ITERATION_FRACTION;
    uint32_t add_period = m_fraction_since_last_hb / ANT_CLOCK_FREQUENCY;
    
    if(add_period > 0)
    {
        m_time_since_last_hb++;
        m_fraction_since_last_hb %= ANT_CLOCK_FREQUENCY;
    }
    
    //calc number of hb as will fill
    uint32_t new_beats = m_time_since_last_hb / current_hb_pulse_interval;
    uint32_t add_event_time = new_beats * current_hb_pulse_interval;
    
    if (new_beats > 0)
    {
        mp_related_hrm_profile->page_0.computed_heart_rate = (uint8_t)computed_heart_rate_value;
        
        // Current heart beat event time is the previous event time added with the current heart rate 
        // pulse interval. 
        uint32_t current_heart_beat_event_time = mp_related_hrm_profile->page_0.beat_time + add_event_time;

        // Set current event time.
        mp_related_hrm_profile->page_0.beat_time = current_heart_beat_event_time; // <- B<4,5> <-
    
        // Set previous event time. // p4.B<2,3> <- B<4,5>
        mp_related_hrm_profile->page_4.prev_beat = mp_related_hrm_profile->page_0.beat_time - current_hb_pulse_interval;
        
        // Event count.
        mp_related_hrm_profile->page_0.beat_count += new_beats; // B<6>
        
        m_time_since_last_hb -= add_event_time;
    }
}

void hrm_tx_heart_rate_increment(void)
{
    // Increment heart rate and roll-over to minimum value if needed.
    m_heart_rate_value += HEART_RATE_VALUE_INCREMENT;
    if ((m_heart_rate_value > MAX_HEART_RATE_VALUE) || (m_heart_rate_value < MIN_HEART_RATE_VALUE))
    {
        m_heart_rate_value = MIN_HEART_RATE_VALUE; 
    }  
}


void hrm_tx_heart_rate_decrement(void)
{
    // Decrement heart rate and roll-over to maximum value if needed.
    m_heart_rate_value -= HEART_RATE_VALUE_DECREMENT;
    if ((m_heart_rate_value < MIN_HEART_RATE_VALUE) || (m_heart_rate_value > MAX_HEART_RATE_VALUE))
    {
        m_heart_rate_value = MAX_HEART_RATE_VALUE; 
    }
}

void button_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            // Increase heart rate measurement.
            hrm_tx_heart_rate_increment();
            break;
        case BSP_EVENT_KEY_1:
            // Decrease heart rate measurement.
            hrm_tx_heart_rate_decrement();
            break;
        default:
            break;
    }
}
/**
  * @}
  */
