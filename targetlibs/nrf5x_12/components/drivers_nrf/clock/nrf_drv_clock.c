/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
#if CLOCK_ENABLED

#include "nrf_drv_clock.h"
#include "nrf_error.h"
#include "nordic_common.h"
#include "app_util_platform.h"
#ifdef SOFTDEVICE_PRESENT
#include "softdevice_handler.h"
#include "nrf_sdm.h"
#include "nrf_soc.h"
#endif

/* Validate configuration */
INTERRUPT_PRIORITY_VALIDATION(CLOCK_CONFIG_IRQ_PRIORITY);

/*lint -save -e652 */
#define NRF_CLOCK_LFCLK_RC    CLOCK_LFCLKSRC_SRC_RC
#define NRF_CLOCK_LFCLK_Xtal  CLOCK_LFCLKSRC_SRC_Xtal
#define NRF_CLOCK_LFCLK_Synth CLOCK_LFCLKSRC_SRC_Synth
/*lint -restore */

#if (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LFCLK_RC) && !defined(SOFTDEVICE_PRESENT)
#define CALIBRATION_SUPPORT 1
#else
#define CALIBRATION_SUPPORT 0
#endif
typedef enum
{
    CAL_STATE_IDLE,
    CAL_STATE_CT,
    CAL_STATE_HFCLK_REQ,
    CAL_STATE_CAL,
    CAL_STATE_ABORT,
} nrf_drv_clock_cal_state_t;

/**@brief CLOCK control block. */
typedef struct
{
    bool                                    module_initialized; /*< Indicate the state of module */
    volatile bool                           hfclk_on;           /*< High-frequency clock state. */
    volatile bool                           lfclk_on;           /*< Low-frequency clock state. */
    volatile uint32_t                       hfclk_requests;     /*< High-frequency clock request counter. */
    volatile nrf_drv_clock_handler_item_t * p_hf_head;
    volatile uint32_t                       lfclk_requests;     /*< Low-frequency clock request counter. */
    volatile nrf_drv_clock_handler_item_t * p_lf_head;
#if CALIBRATION_SUPPORT
    nrf_drv_clock_handler_item_t            cal_hfclk_started_handler_item;
    nrf_drv_clock_event_handler_t           cal_done_handler;
    volatile nrf_drv_clock_cal_state_t      cal_state;
#endif // CALIBRATION_SUPPORT
} nrf_drv_clock_cb_t;

static nrf_drv_clock_cb_t m_clock_cb;


/**@brief Function for starting LFCLK. This function will return immediately without waiting for start.
 */
static void lfclk_start(void)
{
    nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
    nrf_clock_int_enable(NRF_CLOCK_INT_LF_STARTED_MASK);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
}

/**@brief Function for stopping LFCLK and calibration (if it was set up).
 */
static void lfclk_stop(void)
{
#if CALIBRATION_SUPPORT
    (void)nrf_drv_clock_calibration_abort();
#endif

#ifdef SOFTDEVICE_PRESENT
    // If LFCLK is requested to stop while SD is still enabled,
    // it indicates an error in the application.
    // Enabling SD should increment the LFCLK request.
    ASSERT(!softdevice_handler_is_enabled());
#endif // SOFTDEVICE_PRESENT

    nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTOP);
    while (nrf_clock_lf_is_running())
    {}
    m_clock_cb.lfclk_on = false;
}

static void hfclk_start(void)
{
#ifdef SOFTDEVICE_PRESENT
    if (softdevice_handler_is_enabled())
    {
        (void)sd_clock_hfclk_request();
        return;
    }
#endif // SOFTDEVICE_PRESENT

    nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    nrf_clock_int_enable(NRF_CLOCK_INT_HF_STARTED_MASK);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
}

static void hfclk_stop(void)
{
#ifdef SOFTDEVICE_PRESENT
    if (softdevice_handler_is_enabled())
    {
        (void)sd_clock_hfclk_release();
        return;
    }
#endif // SOFTDEVICE_PRESENT

    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
    while (nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY))
    {}
    m_clock_cb.hfclk_on = false;
}

ret_code_t nrf_drv_clock_init(void)
{
    if (m_clock_cb.module_initialized)
    {
        return MODULE_ALREADY_INITIALIZED;
    }

    m_clock_cb.p_hf_head      = NULL;
    m_clock_cb.hfclk_requests = 0;
    m_clock_cb.p_lf_head      = NULL;
    m_clock_cb.lfclk_requests = 0;
    nrf_drv_common_irq_enable(POWER_CLOCK_IRQn, CLOCK_CONFIG_IRQ_PRIORITY);
#ifdef SOFTDEVICE_PRESENT
    if (!softdevice_handler_is_enabled())
#endif
    {
        nrf_clock_lf_src_set((nrf_clock_lfclk_t)CLOCK_CONFIG_LF_SRC);
    }

#if CALIBRATION_SUPPORT
    m_clock_cb.cal_state = CAL_STATE_IDLE;
#endif

    m_clock_cb.module_initialized = true;
    return NRF_SUCCESS;
}

void nrf_drv_clock_uninit(void)
{
    ASSERT(m_clock_cb.module_initialized);
    nrf_drv_common_irq_disable(POWER_CLOCK_IRQn);
    nrf_clock_int_disable(0xFFFFFFFF);

    lfclk_stop();
    hfclk_stop();
    m_clock_cb.module_initialized = false;
}

static void item_enqueue(nrf_drv_clock_handler_item_t ** p_head,
                         nrf_drv_clock_handler_item_t * p_item)
{
    p_item->p_next = (*p_head ? *p_head : NULL);
    *p_head = p_item;
}

static nrf_drv_clock_handler_item_t * item_dequeue(nrf_drv_clock_handler_item_t ** p_head)
{
    nrf_drv_clock_handler_item_t * p_item = *p_head;
    if (p_item)
    {
        *p_head = p_item->p_next;
    }
    return p_item;
}

void nrf_drv_clock_lfclk_request(nrf_drv_clock_handler_item_t * p_handler_item)
{
    ASSERT(m_clock_cb.module_initialized);

    if (m_clock_cb.lfclk_on)
    {
        if (p_handler_item)
        {
            p_handler_item->event_handler(NRF_DRV_CLOCK_EVT_LFCLK_STARTED);
        }
        CRITICAL_REGION_ENTER();
        ++(m_clock_cb.lfclk_requests);
        CRITICAL_REGION_EXIT();
    }
    else
    {
        CRITICAL_REGION_ENTER();
        if (p_handler_item)
        {
            item_enqueue((nrf_drv_clock_handler_item_t **)&m_clock_cb.p_lf_head,
                p_handler_item);
        }
        if (m_clock_cb.lfclk_requests == 0)
        {
            lfclk_start();
        }
        ++(m_clock_cb.lfclk_requests);
        CRITICAL_REGION_EXIT();
    }

    ASSERT(m_clock_cb.lfclk_requests > 0);
}

void nrf_drv_clock_lfclk_release(void)
{
    ASSERT(m_clock_cb.module_initialized);
    ASSERT(m_clock_cb.lfclk_requests > 0);

    CRITICAL_REGION_ENTER();
    --(m_clock_cb.lfclk_requests);
    if (m_clock_cb.lfclk_requests == 0)
    {
        lfclk_stop();
    }
    CRITICAL_REGION_EXIT();
}

bool nrf_drv_clock_lfclk_is_running(void)
{
    ASSERT(m_clock_cb.module_initialized);

#ifdef SOFTDEVICE_PRESENT
    if (softdevice_handler_is_enabled())
    {
        return true;
    }
#endif // SOFTDEVICE_PRESENT

    return nrf_clock_lf_is_running();
}

void nrf_drv_clock_hfclk_request(nrf_drv_clock_handler_item_t * p_handler_item)
{
    ASSERT(m_clock_cb.module_initialized);

    if (m_clock_cb.hfclk_on)
    {
        if (p_handler_item)
        {
            p_handler_item->event_handler(NRF_DRV_CLOCK_EVT_HFCLK_STARTED);
        }
        CRITICAL_REGION_ENTER();
        ++(m_clock_cb.hfclk_requests);
        CRITICAL_REGION_EXIT();
    }
    else
    {
        CRITICAL_REGION_ENTER();
        if (p_handler_item)
        {
            item_enqueue((nrf_drv_clock_handler_item_t **)&m_clock_cb.p_hf_head,
                p_handler_item);
        }
        if (m_clock_cb.hfclk_requests == 0)
        {
            hfclk_start();
        }
        ++(m_clock_cb.hfclk_requests);
        CRITICAL_REGION_EXIT();
    }

    ASSERT(m_clock_cb.hfclk_requests > 0);
}

void nrf_drv_clock_hfclk_release(void)
{
    ASSERT(m_clock_cb.module_initialized);
    ASSERT(m_clock_cb.hfclk_requests > 0);

    CRITICAL_REGION_ENTER();
    --(m_clock_cb.hfclk_requests);
    if (m_clock_cb.hfclk_requests == 0)
    {
        hfclk_stop();
    }
    CRITICAL_REGION_EXIT();
}

bool nrf_drv_clock_hfclk_is_running(void)
{
    ASSERT(m_clock_cb.module_initialized);

#ifdef SOFTDEVICE_PRESENT
    if (softdevice_handler_is_enabled())
    {
        uint32_t is_running;
        UNUSED_VARIABLE(sd_clock_hfclk_is_running(&is_running));
        return (is_running ? true : false);
    }
#endif // SOFTDEVICE_PRESENT

    return nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY);
}

#if CALIBRATION_SUPPORT
static void clock_calibration_hf_started(nrf_drv_clock_evt_type_t event)
{
    if (m_clock_cb.cal_state == CAL_STATE_ABORT)
    {
        nrf_drv_clock_hfclk_release();
        m_clock_cb.cal_state = CAL_STATE_IDLE;
        if (m_clock_cb.cal_done_handler)
        {
            m_clock_cb.cal_done_handler(NRF_DRV_CLOCK_EVT_CAL_ABORTED);
        }
    }
    else
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_DONE);
        nrf_clock_int_enable(NRF_CLOCK_INT_DONE_MASK);
        m_clock_cb.cal_state = CAL_STATE_CAL;
        nrf_clock_task_trigger(NRF_CLOCK_TASK_CAL);
    }
}
#endif // CALIBRATION_SUPPORT

ret_code_t nrf_drv_clock_calibration_start(uint8_t interval, nrf_drv_clock_event_handler_t handler)
{
#if CALIBRATION_SUPPORT
    ASSERT(m_clock_cb.cal_state == CAL_STATE_IDLE);
    ret_code_t ret = NRF_SUCCESS;
    if (m_clock_cb.lfclk_on == false)
    {
        ret = NRF_ERROR_INVALID_STATE;
    }
    else if (m_clock_cb.cal_state == CAL_STATE_IDLE)
    {
        m_clock_cb.cal_done_handler = handler;
        m_clock_cb.cal_hfclk_started_handler_item.event_handler = clock_calibration_hf_started;
        if (interval == 0)
        {
            m_clock_cb.cal_state = CAL_STATE_HFCLK_REQ;
            nrf_drv_clock_hfclk_request(&m_clock_cb.cal_hfclk_started_handler_item);
        }
        else
        {
            m_clock_cb.cal_state = CAL_STATE_CT;
            nrf_clock_cal_timer_timeout_set(interval);
            nrf_clock_event_clear(NRF_CLOCK_EVENT_CTTO);
            nrf_clock_int_enable(NRF_CLOCK_INT_CTTO_MASK);
            nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTART);
        }
    }
    else
    {
        ret = NRF_ERROR_BUSY;
    }
    return ret;
#else
    return NRF_ERROR_FORBIDDEN;
#endif // CALIBRATION_SUPPORT
}

ret_code_t nrf_drv_clock_calibration_abort(void)
{
#if CALIBRATION_SUPPORT
    CRITICAL_REGION_ENTER();
    switch (m_clock_cb.cal_state)
    {
    case CAL_STATE_CT:
        nrf_clock_int_disable(NRF_CLOCK_INT_CTTO_MASK);
        nrf_clock_task_trigger(NRF_CLOCK_TASK_CTSTOP);
        m_clock_cb.cal_state = CAL_STATE_IDLE;
        if (m_clock_cb.cal_done_handler)
        {
            m_clock_cb.cal_done_handler(NRF_DRV_CLOCK_EVT_CAL_ABORTED);
        }
        break;
    case CAL_STATE_HFCLK_REQ:
        /* fall through. */
    case CAL_STATE_CAL:
        m_clock_cb.cal_state = CAL_STATE_ABORT;
        break;
    default:
        break;
    }
    CRITICAL_REGION_EXIT();
    return NRF_SUCCESS;
#else
    return NRF_ERROR_FORBIDDEN;
#endif // CALIBRATION_SUPPORT
}

ret_code_t nrf_drv_clock_is_calibrating(bool * p_is_calibrating)
{
#if CALIBRATION_SUPPORT
    ASSERT(m_clock_cb.module_initialized);
    *p_is_calibrating = (m_clock_cb.cal_state != CAL_STATE_IDLE);
    return NRF_SUCCESS;
#else
    return NRF_ERROR_FORBIDDEN;
#endif // CALIBRATION_SUPPORT
}

__STATIC_INLINE void clock_clk_started_notify(nrf_drv_clock_evt_type_t evt_type)
{
    nrf_drv_clock_handler_item_t **p_head;
    if (evt_type == NRF_DRV_CLOCK_EVT_HFCLK_STARTED)
    {
        p_head = (nrf_drv_clock_handler_item_t **)&m_clock_cb.p_hf_head;
    }
    else
    {
        p_head = (nrf_drv_clock_handler_item_t **)&m_clock_cb.p_lf_head;
    }

    while (1)
    {
        nrf_drv_clock_handler_item_t * p_item = item_dequeue(p_head);
        if (!p_item)
        {
            break;
        }

        p_item->event_handler(evt_type);
    }
}

void POWER_CLOCK_IRQHandler(void)
{
    if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
        nrf_clock_int_disable(NRF_CLOCK_INT_HF_STARTED_MASK);
        m_clock_cb.hfclk_on = true;
        clock_clk_started_notify(NRF_DRV_CLOCK_EVT_HFCLK_STARTED);
    }
    if (nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
        nrf_clock_int_disable(NRF_CLOCK_INT_LF_STARTED_MASK);
        m_clock_cb.lfclk_on = true;
        clock_clk_started_notify(NRF_DRV_CLOCK_EVT_LFCLK_STARTED);
    }
#if CALIBRATION_SUPPORT
    if (nrf_clock_event_check(NRF_CLOCK_EVENT_CTTO))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_CTTO);
        nrf_clock_int_disable(NRF_CLOCK_INT_CTTO_MASK);
        nrf_drv_clock_hfclk_request(&m_clock_cb.cal_hfclk_started_handler_item);
    }

    if (nrf_clock_event_check(NRF_CLOCK_EVENT_DONE))
    {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_DONE);
        nrf_clock_int_disable(NRF_CLOCK_INT_DONE_MASK);
        nrf_drv_clock_hfclk_release();
        bool aborted = (m_clock_cb.cal_state == CAL_STATE_ABORT);
        m_clock_cb.cal_state = CAL_STATE_IDLE;
        if (m_clock_cb.cal_done_handler)
        {
            m_clock_cb.cal_done_handler(aborted ?
                NRF_DRV_CLOCK_EVT_CAL_ABORTED : NRF_DRV_CLOCK_EVT_CAL_DONE);
        }
    }
#endif // CALIBRATION_SUPPORT
}

#ifdef SOFTDEVICE_PRESENT

void nrf_drv_clock_on_soc_event(uint32_t evt_id)
{
    if (evt_id == NRF_EVT_HFCLKSTARTED)
    {
        clock_clk_started_notify(NRF_DRV_CLOCK_EVT_HFCLK_STARTED);
    }
}

void nrf_drv_clock_on_sd_enable(void)
{
    CRITICAL_REGION_ENTER();
    /* Make sure that nrf_drv_clock module is initialized */
    if (!m_clock_cb.module_initialized)
    {
        (void)nrf_drv_clock_init();
    }
    /* SD is one of the LFCLK requesters, but it will enable it by itself. */
    ++(m_clock_cb.lfclk_requests);
    m_clock_cb.lfclk_on = true;
    CRITICAL_REGION_EXIT();
}

void nrf_drv_clock_on_sd_disable(void)
{
    /* Reinit interrupts */
    ASSERT(m_clock_cb.module_initialized);
    nrf_drv_common_irq_enable(POWER_CLOCK_IRQn, CLOCK_CONFIG_IRQ_PRIORITY);

    /* SD leaves LFCLK enabled - disable it if it is no longer required. */
    nrf_drv_clock_lfclk_release();
}

#endif // SOFTDEVICE_PRESENT

#undef NRF_CLOCK_LFCLK_RC
#undef NRF_CLOCK_LFCLK_Xtal
#undef NRF_CLOCK_LFCLK_Synth

#endif //CLOCK_ENABLED
