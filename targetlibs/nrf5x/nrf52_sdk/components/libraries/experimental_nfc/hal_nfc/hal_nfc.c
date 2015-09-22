#include <stdint.h>
#include <stdbool.h>
#include "hal_nfc.h"
#include "nfc_lib.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "app_util_platform.h"

#include "nrf_gpio.h"

#define NFC_LIB_VERSION 0x00

/* Begin: Bugfix for IC-9563 */
#define NRF_NFCT_SHUNTREGTHRESHOLDS  (*(uint32_t volatile *)(0x40005610))
#define NRF_NFCT_MODSTEPFIXED        (*(uint32_t volatile *)(0x40005614))
#define NRF_NFCT_MODSTEPMULTIPLIER   (*(uint32_t volatile *)(0x40005618))
#define NRF_NFCT_INITIALLOADCTRLVAL  (*(uint32_t volatile *)(0x40005688))
/* End:   Bugfix for IC-9563 */

#define NFC_RX_BUFFER 16
#define READ_CMD      0x30

static hal_nfc_callback nfc_lib_callback;
static void *nfc_lib_context;
static uint8_t nfc_rx_buffer[NFC_RX_BUFFER];

/* Begin:   Bugfix for IC-6915 */
volatile uint8_t m_nfc_active = 0;
/* End:   Bugfix for IC-6915 */

static uint8_t nfcInternal[] =
        {0x5F, 0x12, 0x34, 0xF1, 0x56, 0x78, 0x9A, 0xBC, 0x08, NFC_LIB_VERSION};

/* Begin: Bugfix for IC-9563 */
/* The following three function definitions are a workaround for IC-9563: NFC in ASIC_MPW3 does not
 * give the field lost signal when field is turned off. */
typedef void (*anon_cb)(void);
static anon_cb rtc_callback;
static bool field_on = false;


static void start_rtc_with_callback(anon_cb cb)
{
    rtc_callback = cb;
    NRF_TIMER4->MODE      = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
    NRF_TIMER4->BITMODE   = TIMER_BITMODE_BITMODE_08Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER4->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;
    NRF_TIMER4->CC[0] = 100 << TIMER_CC_CC_Pos;
    NRF_TIMER4->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
    NRF_TIMER4->INTENSET = TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos;
    NVIC_ClearPendingIRQ(TIMER4_IRQn);
    NVIC_SetPriority(TIMER4_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(TIMER4_IRQn);
    // debug pins setup
    nrf_gpio_cfg_output(12);
    nrf_gpio_cfg_output(11);
    nrf_gpio_pin_clear(12);
    nrf_gpio_pin_clear(11);
}

void RTC2_IRQHandler(void)
{
    rtc_callback();
    NRF_RTC2->EVENTS_TICK = 0;
}

void TIMER4_IRQHandler(void)
{
    nrf_gpio_pin_set(11);
    rtc_callback();
    NRF_TIMER4->EVENTS_COMPARE[0] = 0;
}

static void check_fieldlost(void)
{
    static int field_lost_cnt = 0;


    if (field_on && (NRF_NFCT->FIELDPRESENT == 0))
    {
        ++field_lost_cnt;
        if (field_lost_cnt > 7)
        {
            NRF_NFCT->EVENTS_FIELDLOST = 1;
        }
        return;
    }
    field_lost_cnt = 0;
}
/* End: Bugfix for IC-9563 */

hal_nfc_retval hal_nfc_setup(hal_nfc_callback callback, void *cbContext)
{
    nfc_lib_callback = callback;
    nfc_lib_context  = cbContext;

    NRF_NFCT->SHORTS =
            (NFCT_SHORTS_FIELDDETECTED_ACTIVATE_Enabled << NFCT_SHORTS_FIELDDETECTED_ACTIVATE_Pos) |
            (NFCT_SHORTS_FIELDLOST_SENSE_Enabled        << NFCT_SHORTS_FIELDLOST_SENSE_Pos);

    NRF_NFCT->INTENSET = (NFCT_INTENSET_FIELDLOST_Enabled  << NFCT_INTENSET_FIELDLOST_Pos);
    NRF_NFCT->INTENSET = (NFCT_INTENSET_RXFRAMEEND_Enabled << NFCT_INTENSET_RXFRAMEEND_Pos);
    NRF_NFCT->INTENSET = (NFCT_INTENSET_ERROR_Enabled      << NFCT_INTENSET_ERROR_Pos);
    NRF_NFCT->INTENSET = (NFCT_INTENSET_RXERROR_Enabled    << NFCT_INTENSET_RXERROR_Pos);
    NRF_NFCT->INTENSET = (NFCT_INTENSET_SELECTED_Enabled   << NFCT_INTENSET_SELECTED_Pos);

    NRF_NFCT->INTENSET = (NFCT_INTENSET_FIELDDETECTED_Enabled   << NFCT_INTENSET_FIELDDETECTED_Pos);

    /* Begin:   Bugfix for IC-6915 */
    NRF_NFCT->INTENSET = (NFCT_INTENSET_RXFRAMESTART_Enabled   << NFCT_INTENSET_RXFRAMESTART_Pos);
    NRF_NFCT->INTENSET = (NFCT_INTENSET_TXFRAMESTART_Enabled   << NFCT_INTENSET_TXFRAMESTART_Pos);
    /* End:   Bugfix for IC-6915 */

    /* TODO: NFCID should come from FICR and be used to create nfcInternal[] content and to set
     * NRF_NFCT->NFCID1_2ND_LAST and NRF_NFCT->NFCID1_LAST. */
    NRF_NFCT->NFCID1_2ND_LAST = 0x005F1234;
    NRF_NFCT->NFCID1_LAST     = 0x56789ABC;
    (void) nfcSetInternal((char *) nfcInternal, sizeof(nfcInternal));

    /* Begin: Bugfix for IC-9929 */
    /* Workaround for wrong SENSRES values */
    NRF_NFCT->SENSRES =
            (NFCT_SENSRES_NFCIDSIZE_NFCID1Double << NFCT_SENSRES_NFCIDSIZE_Pos) |
            (NFCT_SENSRES_BITFRAMESDD_SDD00100 << NFCT_SENSRES_BITFRAMESDD_Pos);
    /* End:   Bugfix for IC-9929 */

    /* Begin: Bugfix for IC-9563 */
    /* Values taken from IC-9563 */
    NRF_NFCT_SHUNTREGTHRESHOLDS = 0x00000005;
    NRF_NFCT_MODSTEPFIXED       = 0x0000003F;
    NRF_NFCT_MODSTEPMULTIPLIER  = 0x00000001;
    NRF_NFCT_INITIALLOADCTRLVAL = 0x00000001;
    /* End:   Bugfix for IC-9563 */

    /* Begin: Bugfix for IC-9563 */
    /* Activating workaround. */
    start_rtc_with_callback(check_fieldlost);
    /* End:   Bugfix for IC-9563 */

    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_set_parameter(hal_nfc_param_id id, void *data, size_t dataLength)
{
    (void)id;
    (void)data;
    (void)dataLength;

    return HAL_NFC_RETVAL_OK;
} 

hal_nfc_retval hal_nfc_get_parameter(hal_nfc_param_id id, void *data, size_t *maxDataLength)
{
    (void)id;
    (void)data;
    (void)maxDataLength;

    return HAL_NFC_RETVAL_OK;
}


hal_nfc_retval hal_nfc_start(void)
{
    NRF_NFCT->MAXLEN    = NFC_RX_BUFFER;
    NRF_NFCT->PACKETPTR = (uint32_t)nfc_rx_buffer;

    /* TODO: MPW3 Bug: Write '1' in FP1 */
    NRF_NFCT->ERRORSTATUS = 0xFFFFFFFF;
    NVIC_ClearPendingIRQ(NFCT_IRQn);
    NVIC_SetPriority(NFCT_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(NFCT_IRQn);
    NRF_NFCT->TASKS_SENSE = 1;

    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_send(const char *data, size_t dataLength)
{
    NRF_NFCT->PACKETPTR     = (uint32_t)data;
    NRF_NFCT->TXD.AMOUNT    = dataLength<<3;
    NRF_NFCT->INTENSET      = (NFCT_INTENSET_TXFRAMEEND_Enabled << NFCT_INTENSET_TXFRAMEEND_Pos);
    NRF_NFCT->TASKS_STARTTX = 1;

    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_stop(void)
{
    NRF_NFCT->TASKS_DISABLE;

    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_done(void)
{
    return HAL_NFC_RETVAL_OK;
}

void NFCT_IRQHandler(void)
{
    volatile uint32_t dummy;

    if (NRF_NFCT->EVENTS_FIELDLOST)
    {
        nfc_lib_callback(nfc_lib_context, HAL_NFC_EVENT_FIELD_OFF, 0, 0);
        NRF_NFCT->TASKS_SENSE      = 1;
        NRF_NFCT->EVENTS_FIELDLOST = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_FIELDLOST;
        (void)dummy;

        /* Begin: Bugfix for IC-9563 */
        field_on = false;
        NRF_TIMER4->TASKS_SHUTDOWN = 1;
        /* End:   Bugfix for IC-9563 */
    }

    if (NRF_NFCT->EVENTS_TXFRAMEEND)
    {
        nfc_lib_callback(nfc_lib_context, HAL_NFC_EVENT_DATA_TRANSMITTED, 0, 0);

        NRF_NFCT->PACKETPTR          = (uint32_t)nfc_rx_buffer;
        NRF_NFCT->MAXLEN             = NFC_RX_BUFFER;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;
        NRF_NFCT->INTENCLR           = (NFCT_INTENSET_TXFRAMEEND_Enabled <<
                                            NFCT_INTENSET_TXFRAMEEND_Pos);
        NRF_NFCT->EVENTS_TXFRAMEEND  = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_TXFRAMEEND;
        (void)dummy;

        /* Begin:   Bugfix for IC-6915 */
        m_nfc_active = 0;
        /* End:   Bugfix for IC-6915 */
    }

    if (NRF_NFCT->EVENTS_RXFRAMEEND)
    {
        if (READ_CMD == nfc_rx_buffer[0])
        {
            nfc_lib_callback(nfc_lib_context, HAL_NFC_EVENT_DATA_RECEIVED, (void*)nfc_rx_buffer, 2);
        }
        else
        {
            NRF_NFCT->TASKS_ENABLERXDATA = 1;
        }

        NRF_NFCT->EVENTS_RXFRAMEEND = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_RXFRAMEEND;
        (void)dummy;

        /* Begin:   Bugfix for IC-6915 */
        m_nfc_active = 0;
        /* End:   Bugfix for IC-6915 */
    }

    /* Begin:   Bugfix for IC-6915 */
    if (NRF_NFCT->EVENTS_RXFRAMESTART)
    {
        NRF_NFCT->EVENTS_RXFRAMESTART = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_RXFRAMESTART;
        (void)dummy;

        if (m_nfc_active == 0)
        {
            m_nfc_active = 1;
        }
    }
    if (NRF_NFCT->EVENTS_TXFRAMESTART)
    {
        NRF_NFCT->EVENTS_TXFRAMESTART = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_TXFRAMESTART;
        (void)dummy;

        if (m_nfc_active == 0)
        {
            m_nfc_active = 1;
        }
    }
    /* End:   Bugfix for IC-6915 */

    if (NRF_NFCT->EVENTS_ERROR)
    {
        NRF_NFCT->PACKETPTR          = (uint32_t)nfc_rx_buffer;
        NRF_NFCT->MAXLEN             = NFC_RX_BUFFER;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;
        NRF_NFCT->ERRORSTATUS        = 0xFFFFFFFF;
        NRF_NFCT->EVENTS_ERROR       = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_ERROR;
        (void)dummy;
    }

    if (NRF_NFCT->EVENTS_RXERROR)
    {
        NRF_NFCT->FRAMESTATUS.RX     = 1;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;
        NRF_NFCT->EVENTS_RXERROR     = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_RXERROR;
        (void)dummy;
    }

    if (NRF_NFCT->EVENTS_SELECTED)
    {
        NRF_NFCT->PACKETPTR          = (uint32_t)nfc_rx_buffer;
        NRF_NFCT->MAXLEN             = NFC_RX_BUFFER;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;
        nfc_lib_callback(nfc_lib_context, HAL_NFC_EVENT_FIELD_ON, 0, 0);
        NRF_NFCT->EVENTS_SELECTED    = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_SELECTED;
        (void)dummy;
        /* Begin: Bugfix for IC-9563 */
        field_on = true;
        /* End:   Bugfix for IC-9563 */
    }

    // Additional handler of FIELDDETECTED event
    if (NRF_NFCT->EVENTS_FIELDDETECTED)
    {
        /* Begin:   Bugfix for IC-9563 */
        NRF_TIMER4->TASKS_START = 1;
        /* End:   Bugfix for IC-9563 */

        NRF_NFCT->EVENTS_FIELDDETECTED = 0;
        /* Perform read to be ensure clearing is effective */
        dummy = NRF_NFCT->EVENTS_SELECTED;
        (void)dummy;

#if defined(SOFTDEVICE_PRESENT)
        (void) sd_clock_hfclk_request();
        uint32_t hfclk_running;
        do
        {
            (void) sd_clock_hfclk_is_running(&hfclk_running);
        }
        while(!hfclk_running);
#else
        NRF_CLOCK->TASKS_HFCLKSTART = 1;
        while(!NRF_CLOCK->EVENTS_HFCLKSTARTED);
#endif /* defined(SOFTDEVICE_PRESENT) */

    }
}
