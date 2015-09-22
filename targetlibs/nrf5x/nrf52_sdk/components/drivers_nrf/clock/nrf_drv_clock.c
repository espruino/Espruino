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

#include "nrf_drv_clock.h"
#include "nrf_error.h"
#include "nordic_common.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdm.h"
#include "nrf_soc.h"
#else
#include "app_util_platform.h"
#endif // SOFTDEVICE_PRESENT

/*lint -save -e652 */
#define NRF_CLOCK_LF_SRC_RC    CLOCK_LFCLKSRC_SRC_RC
#define NRF_CLOCK_LF_SRC_Xtal  CLOCK_LFCLKSRC_SRC_Xtal
#define NRF_CLOCK_LF_SRC_Synth CLOCK_LFCLKSRC_SRC_Synth
/*lint -restore */

#define INT_MAX 0xFFFFFFFF

/**@brief CLOCK control block. */
typedef struct
{
    bool           module_initialized;      /*< Indicate the state of module */
    uint32_t       hfclk_requests;          /*< High frequency clock state. */
#ifndef SOFTDEVICE_PRESENT
    uint32_t       lfclk_requests;          /*< Low frequency clock state. */
#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
    volatile bool  calibration_in_progress; /*< Indicate the state of calibration */
#endif //CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
#endif //SOFTDEVICE_PRESENT
}nrf_drv_clock_cb_t;

static nrf_drv_clock_cb_t m_clock_cb;

#if (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC) && (!defined SOFTDEVICE_PRESENT)

static const nrf_drv_clock_config_t m_default_config = NRF_DRV_CLOCK_DEAFULT_CONFIG;

static uint32_t m_in_critical_region = 0;

#define CLOCK_CRITICAL_REGION_ENTER()                 \
    do {                                              \
        nrf_drv_common_irq_disable(POWER_CLOCK_IRQn); \
        __DSB();                                      \
        __ISB();                                      \
        m_in_critical_region++;                       \
    } while (0)

#define CLOCK_CRITICAL_REGION_EXIT()                  \
    do {                                              \
        m_in_critical_region--;                       \
        if (m_in_critical_region == 0)                \
        {                                             \
            NVIC_EnableIRQ(POWER_CLOCK_IRQn);         \
        }                                             \
    } while (0)

#else

#define CLOCK_CRITICAL_REGION_ENTER()                 \
    do {                                              \
    } while (0)

#define CLOCK_CRITICAL_REGION_EXIT()                  \
    do {                                              \
    } while (0)

#endif


#ifndef SOFTDEVICE_PRESENT
/**@brief Function for starting hfclk. This function will return immediately without waiting for start.
 */
static void hfclk_start(void)
{
    nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
}


/**@brief Function for stopping hfclk.
 */
static void hfclk_stop(void)
{
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
    while (nrf_clock_hf_src_get() == NRF_CLOCK_HF_SRC_HIGH_ACCURACY)
    {
        
    }
}


/**@brief Function for starting lfclk. This function will return immediately without waiting for start.
 * It will also set up calibration if it is needed.
 */
static void lfclk_start(void)
{
    nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);

#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
    nrf_clock_int_enable(NRF_CLOCK_INT_DONE_MASK 
                         | NRF_CLOCK_INT_CTTO_MASK);
    nrf_clock_event_clear(NRF_CLOCK_EVENT_CTTO);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTART);
#endif
}


/**@brief Function for stopping lfclk and calibration(if it was set up).
 */
static void lfclk_stop(void)
{
#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
    nrf_clock_int_disable(NRF_CLOCK_INT_DONE_MASK 
                          | NRF_CLOCK_INT_CTTO_MASK 
                          | NRF_CLOCK_INT_HF_STARTED_MASK);
    m_clock_cb.calibration_in_progress = false;
    nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTOP);
#endif // CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC

    nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTOP);
    while (nrf_clock_lf_is_running())
    {

    }
}


/**@brief Function for controling lfclk. This function decides whether lfclk should be started or stopped.
 */
static void lfclk_control(void)
{
    CLOCK_CRITICAL_REGION_ENTER();

    if (m_clock_cb.lfclk_requests > 0)
    {
        if ((nrf_clock_lf_is_running() == false)
            && (nrf_clock_lf_start_task_status_get() == NRF_CLOCK_START_TASK_NOT_TRIGGERED))
        {
            lfclk_start();
        }
    }
    else
    {
        if (nrf_clock_lf_is_running()
            || (nrf_clock_lf_start_task_status_get() == NRF_CLOCK_START_TASK_TRIGGERED))
        {
            lfclk_stop();
        }
    }
    CLOCK_CRITICAL_REGION_EXIT();
}


/**@brief Function for controling hfclk.
 *
 * @param[in]  interrupt_enable      Determines if this function will enable interrupt
 *                                   during hflck start.
 *
 * @retval     true                  hfclk was started
 * @retval     false                 hfclk was not started
 */
static bool hfclk_control(bool interrupt_enable)
{
    bool result = false;

    CLOCK_CRITICAL_REGION_ENTER();

    if ((m_clock_cb.hfclk_requests > 0)
#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_Synth
        || (m_clock_cb.lfclk_requests > 0)
#elif CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
        || (m_clock_cb.calibration_in_progress == true)
#endif
        )
    {
        if ((nrf_clock_hf_src_get() != NRF_CLOCK_HF_SRC_HIGH_ACCURACY)
            && (nrf_clock_hf_start_task_status_get() == NRF_CLOCK_START_TASK_NOT_TRIGGERED))
        {
            result = true;

            if (interrupt_enable)
            {
                nrf_clock_int_enable(NRF_CLOCK_INT_HF_STARTED_MASK);
            }
            hfclk_start();
        }
    }
    else
    {
        if ((nrf_clock_hf_src_get() == NRF_CLOCK_HF_SRC_HIGH_ACCURACY)
            || (nrf_clock_hf_start_task_status_get() == NRF_CLOCK_START_TASK_TRIGGERED))
        {
            hfclk_stop();
        }
    }
    CLOCK_CRITICAL_REGION_EXIT();

    return result;
}
#endif // SOFTDEVICE_PRESENT


ret_code_t nrf_drv_clock_init(nrf_drv_clock_config_t const * p_config)
{
    uint32_t result = NRF_SUCCESS;

    if (m_clock_cb.module_initialized == false)
    {
        m_clock_cb.hfclk_requests     = 0;
#ifndef SOFTDEVICE_PRESENT
        m_clock_cb.lfclk_requests     = 0;

        nrf_clock_xtalfreq_set(CLOCK_CONFIG_XTAL_FREQ);
        nrf_clock_lf_src_set((nrf_clock_lf_src_t)CLOCK_CONFIG_LF_SRC);

#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC

        m_clock_cb.calibration_in_progress = false;

        nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
        nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
        nrf_clock_event_clear(NRF_CLOCK_EVENT_DONE);
        nrf_clock_event_clear(NRF_CLOCK_EVENT_CTTO);

        if (p_config == NULL)
        {
            p_config = &m_default_config;
        }

        nrf_clock_cal_timer_timeout_set(p_config->cal_interval);
        nrf_drv_common_irq_enable(POWER_CLOCK_IRQn, p_config->interrupt_priority);

#endif // CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
#else // SOFTDEVICE_PRESENT
        uint8_t is_enabled;
        result = sd_softdevice_is_enabled(&is_enabled);
        if((result == NRF_SUCCESS) && !is_enabled)
        {
            result = NRF_ERROR_SOFTDEVICE_NOT_ENABLED;
        }
#endif // SOFTDEVICE_PRESENT
    }
    else
    {
        result = MODULE_ALREADY_INITIALIZED;
    }

    if (result == NRF_SUCCESS)
    {
        m_clock_cb.module_initialized = true;
    }

    return result;
}


void nrf_drv_clock_uninit(void)
{
    ASSERT(m_clock_cb.module_initialized);
#ifndef SOFTDEVICE_PRESENT

#if CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC
    nrf_drv_common_irq_disable(POWER_CLOCK_IRQn);
#endif // CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC

    lfclk_stop();
    hfclk_stop();
#else
    UNUSED_VARIABLE(sd_clock_hfclk_release());
#endif // SOFTDEVICE_PRESENT

    m_clock_cb.module_initialized = false;
}


void nrf_drv_clock_lfclk_request(void)
{
    ASSERT(m_clock_cb.module_initialized);
#ifndef SOFTDEVICE_PRESENT
    ASSERT(m_clock_cb.lfclk_requests != INT_MAX);
    m_clock_cb.lfclk_requests++;
    UNUSED_VARIABLE(hfclk_control(false));
    lfclk_control();
#endif // SOFTDEVICE_PRESENT
}


void nrf_drv_clock_lfclk_release(void)
{
    ASSERT(m_clock_cb.module_initialized);
#ifndef SOFTDEVICE_PRESENT
    ASSERT(m_clock_cb.lfclk_requests > 0);
    m_clock_cb.lfclk_requests--;
    UNUSED_VARIABLE(hfclk_control(false));
    lfclk_control();
#endif // SOFTDEVICE_PRESENT
}


bool nrf_drv_clock_lfclk_is_running(void)
{
    ASSERT(m_clock_cb.module_initialized);
    bool result;
#ifndef SOFTDEVICE_PRESENT
    result = nrf_clock_lf_is_running();
#else
    result = true;
#endif
    return result;
}


void nrf_drv_clock_hfclk_request(void)
{
    ASSERT(m_clock_cb.module_initialized);
    ASSERT(m_clock_cb.hfclk_requests != INT_MAX);
    m_clock_cb.hfclk_requests++;
#ifndef SOFTDEVICE_PRESENT
    UNUSED_VARIABLE(hfclk_control(false));
#else
    if(m_clock_cb.hfclk_requests == 1)
    {
        UNUSED_VARIABLE(sd_clock_hfclk_request());
    }
#endif // SOFTDEVICE_PRESENT
}


void nrf_drv_clock_hfclk_release(void)
{
    ASSERT(m_clock_cb.module_initialized);
    ASSERT(m_clock_cb.hfclk_requests > 0);
    m_clock_cb.hfclk_requests--;
#ifndef SOFTDEVICE_PRESENT
    UNUSED_VARIABLE(hfclk_control(false));
#else
    if(m_clock_cb.hfclk_requests == 0)
    {
        UNUSED_VARIABLE(sd_clock_hfclk_release());
    }
#endif // SOFTDEVICE_PRESENT
}


bool nrf_drv_clock_hfclk_is_running(void)
{
    bool result;
    ASSERT(m_clock_cb.module_initialized);
#ifndef SOFTDEVICE_PRESENT
    result = nrf_clock_hf_is_running();
#else
    uint32_t is_running;
    UNUSED_VARIABLE(sd_clock_hfclk_is_running(&is_running));
    result = is_running ? true : false;
#endif
    return result;
}


ret_code_t nrf_drv_clock_calibration_force(void)
{
#if ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))
    uint32_t result;
    ASSERT(m_clock_cb.module_initialized);
    if (m_clock_cb.lfclk_requests > 0)
    {
        result = NRF_SUCCESS;
        CLOCK_CRITICAL_REGION_ENTER();
        nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTOP);

        if (m_clock_cb.calibration_in_progress == false)
        {
            m_clock_cb.calibration_in_progress = true;

            if (hfclk_control(true) == false)
            {
                nrf_clock_task_trigger(NRF_CLOCK_TASK_CAL);
            }
        }
        CLOCK_CRITICAL_REGION_EXIT();
    }
    else
    {
        result = NRF_ERROR_INVALID_STATE;
    }
    return result;
#else // ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))
    return NRF_ERROR_FORBIDDEN;
#endif
}


ret_code_t nrf_drv_clock_is_calibrating(bool * p_is_calibrating)
{
#if ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))
    ASSERT(m_clock_cb.module_initialized);
    *p_is_calibrating = m_clock_cb.calibration_in_progress;
    return NRF_SUCCESS;
#else // ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))
    return NRF_ERROR_FORBIDDEN;
#endif
}


#if ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))
void POWER_CLOCK_IRQHandler(void)
{
    if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED) &&
        nrf_clock_int_enable_check(NRF_CLOCK_INT_HF_STARTED_MASK))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
        nrf_clock_int_disable(NRF_CLOCK_INT_HF_STARTED_MASK);

        if (m_clock_cb.calibration_in_progress == true)
        {
            nrf_clock_task_trigger(NRF_CLOCK_TASK_CAL);
        }
    }
    if (nrf_clock_event_check(NRF_CLOCK_EVENT_CTTO) && 
        nrf_clock_int_enable_check(NRF_CLOCK_INT_CTTO_MASK))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_CTTO);
        m_clock_cb.calibration_in_progress = true;

        if(hfclk_control(true) == false)
        {
            nrf_clock_task_trigger(NRF_CLOCK_TASK_CAL);
        }
    }

    if (nrf_clock_event_check(NRF_CLOCK_EVENT_DONE) &&
        nrf_clock_int_enable_check(NRF_CLOCK_INT_DONE_MASK))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_DONE);
        m_clock_cb.calibration_in_progress = false;
        UNUSED_VARIABLE(hfclk_control(true));
        nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTART);
    }
}
#endif // ((!defined SOFTDEVICE_PRESENT) && (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LF_SRC_RC))

#undef NRF_CLOCK_LF_SRC_RC
#undef NRF_CLOCK_LF_SRC_Xtal
#undef NRF_CLOCK_LF_SRC_Synth
