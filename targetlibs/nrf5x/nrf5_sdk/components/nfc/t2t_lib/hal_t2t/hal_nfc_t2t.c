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

#include "hal_nfc_t2t.h"
#include <stdint.h>
#include <stdbool.h>
#include "nfc_t2t_lib.h"
#include "nfc_fixes.h"
#include "nrf.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#include "hal_nfc_t2t_logger.h"
#include "nrf_drv_clock.h"


// Pin for debug signals
#define HCLOCK_ON_PIN     11
#define HCLOCK_OFF_PIN    12
#define NFC_EVENT_PIN     24
#define DETECT_EVENT_PIN  25
#define TIMER4_EVENT_PIN  28

#ifdef HAL_NFC_DEBUG_PIN_ENABLE
    #include "nrf_gpio.h"
    
    #define HAL_NFC_DEBUG_PIN_CONFIG(pin_num) nrf_gpio_cfg_output(pin_num)
    #define HAL_NFC_DEBUG_PIN_CLEAR(pin_num)  nrf_gpio_pin_clear(pin_num)
    #define HAL_NFC_DEBUG_PIN_SET(pin_num)    nrf_gpio_pin_set(pin_num)
    
    
    #define HAL_NFC_DEBUG_PINS_INITIALIZE()         \
        HAL_NFC_DEBUG_PIN_CONFIG(HCLOCK_OFF_PIN);   \
        HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_OFF_PIN);    \
        HAL_NFC_DEBUG_PIN_CONFIG(HCLOCK_ON_PIN);    \
        HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_ON_PIN);     \
        HAL_NFC_DEBUG_PIN_CONFIG(NFC_EVENT_PIN);    \
        HAL_NFC_DEBUG_PIN_CLEAR(NFC_EVENT_PIN);     \
        HAL_NFC_DEBUG_PIN_CONFIG(DETECT_EVENT_PIN); \
        HAL_NFC_DEBUG_PIN_CLEAR(DETECT_EVENT_PIN);  \
        HAL_NFC_DEBUG_PIN_CONFIG(TIMER4_EVENT_PIN); \
        HAL_NFC_DEBUG_PIN_CLEAR(TIMER4_EVENT_PIN)
    
    
#else
    #define HAL_NFC_DEBUG_PIN_CLEAR(pin_num)
    #define HAL_NFC_DEBUG_PIN_SET(pin_num)
    #define HAL_NFC_DEBUG_PINS_INITIALIZE()
#endif // HAL_NFC_DEBUG_PIN_ENABLE


/* NFC library version history: 
 * #define NFC_LIB_VERSION          0x00 first experimental version intended for nRF52 IC rev. Engineering A (PCA10036, part of nRF52 Preview Development Kit)
 * #define NFC_LIB_VERSION          0x01 experimental version intended for nRF52 IC rev. Engineering B (PCA10040, part of nRF52 Development Kit)
 * #define NFC_LIB_VERSION          0x02 experimental version intended for fix IC-12826 and fix: not released HFCLK in SENSE mode
 */

#define NFC_LIB_VERSION             0x03u                                       /**< Internal: current NFC lib. version  */

#define T2T_INTERNAL_BYTES_NR       10u                                         /**< Number of internal bytes defined by Type 2 Tag Operation Technical Specification */
#define T2T_INTERNAL_BYTE_SN0_SHIFT 0u                                          /**< Internal Byte SN0, NRF_FICR->NFC.TAGHEADER0.MFGID which is Manufacturer ID */
#define T2T_INTERNAL_BYTE_SN1_SHIFT 8u                                          /**< Internal Byte SN1, NRF_FICR->NFC.TAGHEADER0.UID0 */
#define T2T_INTERNAL_BYTE_SN2_SHIFT 16u                                         /**< Internal Byte SN2, NRF_FICR->NFC.TAGHEADER0.UID1 */
#define T2T_INTERNAL_BYTE_SN3_SHIFT 0u                                          /**< Internal Byte SN3, NRF_FICR->NFC.TAGHEADER1.UID3 */
#define T2T_INTERNAL_BYTE_SN4_SHIFT 8u                                          /**< Internal Byte SN4, NRF_FICR->NFC.TAGHEADER1.UID4 */
#define T2T_INTERNAL_BYTE_SN5_SHIFT 16u                                         /**< Internal Byte SN5, NRF_FICR->NFC.TAGHEADER1.UID5 */
#define T2T_INTERNAL_BYTE_SN6_SHIFT 24u                                         /**< Internal Byte SN6, NRF_FICR->NFC.TAGHEADER1.UID6 */
#define CASCADE_TAG_BYTE            0x88u                                       /**< Constant defined by ISO/EIC 14443-3 */

#define NFCID1_2ND_LAST_BYTE2_SHIFT 16u                                         /**< Shift value for NFC ID byte 2 */
#define NFCID1_2ND_LAST_BYTE1_SHIFT 8u                                          /**< Shift value for NFC ID byte 1 */
#define NFCID1_2ND_LAST_BYTE0_SHIFT 0u                                          /**< Shift value for NFC ID byte 0 */
#define NFCID1_LAST_BYTE3_SHIFT     24u                                         /**< Shift value for NFC ID byte 3 */
#define NFCID1_LAST_BYTE2_SHIFT     16u                                         /**< Shift value for NFC ID byte 2 */
#define NFCID1_LAST_BYTE1_SHIFT     8u                                          /**< Shift value for NFC ID byte 1 */
#define NFCID1_LAST_BYTE0_SHIFT     0u                                          /**< Shift value for NFC ID byte 0 */

#define NFC_RX_BUFFER_SIZE          16u                                         /**< NFC Rx data buffer size */
#define T2T_READ_CMD                0x30u                                       /**< Type 2 Tag Read command identifier */
#define NFC_SLP_REQ_CMD             0x50u                                       /**< NFC SLP_REQ command identifier */
#define NFC_CRC_SIZE                2u                                          /**< CRC size in bytes */

#define NRF_NFCT_ERRORSTATUS_ALL    (NFCT_ERRORSTATUS_NFCFIELDTOOWEAK_Msk   | \
                                     NFCT_ERRORSTATUS_NFCFIELDTOOSTRONG_Msk | \
                                     NFCT_ERRORSTATUS_FRAMEDELAYTIMEOUT_Msk)    /**< Mask for clearing all error flags in NFCT_ERRORSTATUS register */
#define NRF_NFCT_FRAMESTATUS_RX_MSK (NFCT_FRAMESTATUS_RX_OVERRUN_Msk      | \
                                     NFCT_FRAMESTATUS_RX_PARITYSTATUS_Msk | \
                                     NFCT_FRAMESTATUS_RX_CRCERROR_Msk)          /**< Mask for clearing all flags in NFCT_FRAMESTATUS_RX register */
#define NFC_FIELD_ON_MASK            NFCT_FIELDPRESENT_LOCKDETECT_Msk           ///< Mask for checking FIELDPRESENT register for state: FIELD ON.
#define NFC_FIELD_OFF_MASK           NFCT_FIELDPRESENT_FIELDPRESENT_Msk         ///< Mask for checking FIELDPRESENT register for state: FIELD OFF.

typedef enum
{
    NFC_FIELD_STATE_NONE,           ///< Initial value indicating no NFCT Field events.
    NFC_FIELD_STATE_OFF,            ///< NFCT FIELDLOST Event has been set.
    NFC_FIELD_STATE_ON,             ///< NFCT FIELDDETECTED Event has been set.
    NFC_FIELD_STATE_UNKNOWN         ///< Both NFCT Field Events have been set - ambiguous state.
}nfct_field_sense_state_t;

#ifdef HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
/* Begin: Bugfix for FTPAN-57 (IC-9563) */
#define NRF_NFCT_SHUNTREGTHRESHOLDS  (*(uint32_t volatile *)(0x40005610))
#define NRF_NFCT_MODSTEPFIXED        (*(uint32_t volatile *)(0x40005614))
#define NRF_NFCT_MODSTEPMULTIPLIER   (*(uint32_t volatile *)(0x40005618))
#define NRF_NFCT_INITIALLOADCTRLVAL  (*(uint32_t volatile *)(0x40005688))
/* End:   Bugfix for FTPAN-57 (IC-9563) */

/* Begin: Bugfix for FTPAN-24 */
#define NRF_NFCT_AUTOCOLRESSTATUS    (*(uint32_t volatile *)(0x40005408))
/* End: Bugfix for FTPAN-24 */

/* Begin: Bugfix for FTPAN-27 */
#define NRF_NFCT_TASKS_DISABLERXDATA (*(uint32_t volatile *)(0x40005020))
/* End: Bugfix for FTPAN-27 */

/* Begin: Bugfix for FTPAN-17 */
#define NFC_HAL_FIELDPRESENT_MASK      (NFCT_FIELDPRESENT_LOCKDETECT_Msk | NFCT_FIELDPRESENT_FIELDPRESENT_Msk)

#define NFC_HAL_FIELDPRESENT_IS_LOST   ((NFCT_FIELDPRESENT_FIELDPRESENT_NoField << NFCT_FIELDPRESENT_FIELDPRESENT_Pos) \
                                       |(NFCT_FIELDPRESENT_LOCKDETECT_NotLocked << NFCT_FIELDPRESENT_LOCKDETECT_Pos))
                                     
#define NFC_HAL_FIELDPRESENT_NO_FIELD  (NFCT_FIELDPRESENT_FIELDPRESENT_NoField << NFCT_FIELDPRESENT_FIELDPRESENT_Pos)
/* End: Bugfix for FTPAN-17*/

static void hal_nfc_field_check(void);
#endif // HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND

/* Static function declarations */
static inline void nrf_nfct_event_clear(volatile uint32_t * p_event);
static inline void nrf_nfct_clock_event_handler(nrf_drv_clock_evt_type_t event);
static inline void nrf_nfct_field_event_handler(volatile nfct_field_sense_state_t field_state);

/* Static data */
static hal_nfc_callback             m_nfc_lib_callback = (hal_nfc_callback) NULL;                 /**< Callback to nfc_lib layer */
static void *                       m_nfc_lib_context;                                            /**< Callback execution context */
static volatile uint8_t             m_nfc_rx_buffer[NFC_RX_BUFFER_SIZE]   = {0};                  /**< Buffer for NFC Rx data */
static volatile bool                m_slp_req_received                    = false;                /**< Flag indicating that SLP_REQ Command was received */
#ifndef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
static volatile uint32_t            m_nfc_fieldpresent_mask               = NFC_FIELD_OFF_MASK;   /**< Mask used for NFC Field polling in NFCT_FIELDPRESENT register */
#endif
static volatile bool                m_field_on                            = false;                /**< Flag indicating that NFC Tag field is present */
static nrf_drv_clock_handler_item_t m_clock_handler_item;                                         /**< Clock event handler item structure */

#ifdef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
static inline void hal_nfc_re_setup(void);
static void hal_nfc_field_check(void);

#define NFC_HAL_FIELDPRESENT_MASK      (NFCT_FIELDPRESENT_LOCKDETECT_Msk | NFCT_FIELDPRESENT_FIELDPRESENT_Msk)

#define NFC_HAL_FIELDPRESENT_IS_LOST   ((NFCT_FIELDPRESENT_FIELDPRESENT_NoField << NFCT_FIELDPRESENT_FIELDPRESENT_Pos) \
                                       |(NFCT_FIELDPRESENT_LOCKDETECT_NotLocked << NFCT_FIELDPRESENT_LOCKDETECT_Pos))

#define NRF_NFCT_POWER  (*(uint32_t volatile *)(0x40005FFC))
#endif


#if defined(HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND) || defined(HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND)
static void field_timer_with_callback_config()
{
    NRF_TIMER4->MODE      = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
    NRF_TIMER4->BITMODE   = TIMER_BITMODE_BITMODE_08Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER4->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;
    NRF_TIMER4->CC[0]     = 100 << TIMER_CC_CC_Pos;
    NRF_TIMER4->SHORTS    = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
    NRF_TIMER4->INTENSET  = TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos;

    NVIC_ClearPendingIRQ(TIMER4_IRQn);
    NVIC_SetPriority(TIMER4_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(TIMER4_IRQn);
}

void TIMER4_IRQHandler(void)
{
    HAL_NFC_DEBUG_PIN_SET(TIMER4_EVENT_PIN);
    hal_nfc_field_check();
    NRF_TIMER4->EVENTS_COMPARE[0] = 0;
    HAL_NFC_DEBUG_PIN_CLEAR(TIMER4_EVENT_PIN);
}
#endif

#ifdef HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
/* Begin: Bugfix for FTPAN-45 (IC-6915) */
volatile uint8_t m_nfc_active = 0;
/* End: Bugfix for FTPAN-45  (IC-6915) */

/* Begin: Bugfix for FTPAN-17 (IC-9563) */
/* The following three function definitions are a workaround for IC-9563: NFC in nRF52 IC rev. Engineering A does not
 * give the field lost signal when field is turned off. */
 
static   bool field_on              = false;
volatile bool hal_nfc_fielddetected = false;
volatile bool hal_nfc_fieldlost     = false;

static void hal_nfc_field_check(void)
{
    static uint32_t   field_state_cnt = 0;
    volatile uint32_t dummy;    
    uint32_t          nfc_fieldpresen_masked;

    /* Begin: Bugfix for FTPAN-24 */
    NRF_NFCT_AUTOCOLRESSTATUS = 0; /* dummy write - no effect. */
    NRF_NFCT_AUTOCOLRESSTATUS = 0; /* dummy write - no effect. */
    // Don't worry about interrupted case - NRF_NFCT->FIELDPRESENT is read each 100 us, so the workaround should succeed most of the times.
    /* End: Bugfix for FTPAN-24 */

    nfc_fieldpresen_masked = NRF_NFCT->FIELDPRESENT & NFC_HAL_FIELDPRESENT_MASK;

    if (field_on)
    {
        if (nfc_fieldpresen_masked == NFC_HAL_FIELDPRESENT_IS_LOST)
        {
            ++field_state_cnt;
            if (field_state_cnt > 7)
            {
                field_state_cnt   = 0;
                hal_nfc_fieldlost = true;
                dummy             = hal_nfc_fieldlost;
                field_on          = false;
                NVIC_SetPendingIRQ(NFCT_IRQn);
                UNUSED_VARIABLE(dummy);
            }
            
            return;
        }
    }
    else
    {     
        nfc_fieldpresen_masked &= NFCT_FIELDPRESENT_FIELDPRESENT_Msk;
        if (nfc_fieldpresen_masked !=  NFC_HAL_FIELDPRESENT_NO_FIELD)
        {
            ++field_state_cnt;
            if (field_state_cnt > 7)
            {
                field_state_cnt       = 0;
                hal_nfc_fielddetected = true;
                dummy                 = hal_nfc_fielddetected;
                field_on              = true;
                NVIC_SetPendingIRQ(NFCT_IRQn);
                UNUSED_VARIABLE(dummy);
            }
            
            return;
        }
    }

    field_state_cnt = 0;
}
/* End: Bugfix for FTPAN-17, FTPAN-27 */
#endif // HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND

/**
 * @brief Common part of setup used for NFCT initialization and reinitialization.
 */
#ifdef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
static void hal_nfc_common_hw_setup(uint8_t * const nfc_internal)
#else
static inline void hal_nfc_common_hw_setup(uint8_t * const nfc_internal)
#endif
{
    uint32_t nfc_tag_header0 = NRF_FICR->NFC.TAGHEADER0;
    uint32_t nfc_tag_header1 = NRF_FICR->NFC.TAGHEADER1;
    
/* Begin: Bugfix for FTPAN-17 */
/* fixed by avoiding usage of FIELDLOST and FIELDETECTED events */
#ifndef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
    #ifdef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
        NRF_NFCT->INTENSET = (NFCT_INTENSET_FIELDDETECTED_Enabled << NFCT_INTENSET_FIELDDETECTED_Pos);
    #else
        NRF_NFCT->INTENSET = (NFCT_INTENSET_FIELDDETECTED_Enabled << NFCT_INTENSET_FIELDDETECTED_Pos) |
                             (NFCT_INTENSET_FIELDLOST_Enabled     << NFCT_INTENSET_FIELDLOST_Pos);
    #endif
#endif // HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
/* End: Bugfix for FTPAN-17 */

    NRF_NFCT->INTENSET = (NFCT_INTENSET_ERROR_Enabled    << NFCT_INTENSET_ERROR_Pos) |
                         (NFCT_INTENSET_SELECTED_Enabled << NFCT_INTENSET_SELECTED_Pos);

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
    /* Begin:   Bugfix for FTPAN-45 (IC-6915) */
    NRF_NFCT->INTENSET = (NFCT_INTENSET_RXFRAMESTART_Enabled << NFCT_INTENSET_RXFRAMESTART_Pos) |
                         (NFCT_INTENSET_TXFRAMESTART_Enabled << NFCT_INTENSET_TXFRAMESTART_Pos);
    /* End:   Bugfix for FTPAN-45 (IC-6915) */
#endif // HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND

    /* According to ISO/IEC 14443-3 */
    nfc_internal[0] = (uint8_t) (LSB_32(nfc_tag_header0 >> T2T_INTERNAL_BYTE_SN0_SHIFT));      //SN0
    nfc_internal[1] = (uint8_t) (LSB_32(nfc_tag_header0 >> T2T_INTERNAL_BYTE_SN1_SHIFT));      //SN1
    nfc_internal[2] = (uint8_t) (LSB_32(nfc_tag_header0 >> T2T_INTERNAL_BYTE_SN2_SHIFT));      //SN2
    nfc_internal[3] = (uint8_t) ((CASCADE_TAG_BYTE) ^ nfc_internal[0] ^ 
                                  nfc_internal[1]   ^ nfc_internal[2]);                        //BCC0 = CASCADE_TAG_BYTE ^ SN0 ^ SN1 ^ SN2
    nfc_internal[4] = (uint8_t) (LSB_32(nfc_tag_header1 >> T2T_INTERNAL_BYTE_SN3_SHIFT));      //SN3
    nfc_internal[5] = (uint8_t) (LSB_32(nfc_tag_header1 >> T2T_INTERNAL_BYTE_SN4_SHIFT));      //SN4
    nfc_internal[6] = (uint8_t) (LSB_32(nfc_tag_header1 >> T2T_INTERNAL_BYTE_SN5_SHIFT));      //SN5
    nfc_internal[7] = (uint8_t) (LSB_32(nfc_tag_header1 >> T2T_INTERNAL_BYTE_SN6_SHIFT));      //SN6
    nfc_internal[8] = (uint8_t) (nfc_internal[4] ^ nfc_internal[5] ^
                                 nfc_internal[6] ^ nfc_internal[7]);                           //BCC1 = SN3 ^ SN4 ^ SN5 ^ SN6
    nfc_internal[9] = (uint8_t) (NFC_LIB_VERSION);                                             //For internal use


    /* MSB of NFCID1_2ND_LAST register is not used - always 0 */
    NRF_NFCT->NFCID1_2ND_LAST = ((uint32_t) nfc_internal[0] << NFCID1_2ND_LAST_BYTE2_SHIFT) |
                                ((uint32_t) nfc_internal[1] << NFCID1_2ND_LAST_BYTE1_SHIFT) |
                                ((uint32_t) nfc_internal[2] << NFCID1_2ND_LAST_BYTE0_SHIFT);

    NRF_NFCT->NFCID1_LAST = ((uint32_t) nfc_internal[4] << NFCID1_LAST_BYTE3_SHIFT) |
                            ((uint32_t) nfc_internal[5] << NFCID1_LAST_BYTE2_SHIFT) |
                            ((uint32_t) nfc_internal[6] << NFCID1_LAST_BYTE1_SHIFT) |
                            ((uint32_t) nfc_internal[7] << NFCID1_LAST_BYTE0_SHIFT);

    /* Begin: Bugfix for FTPAN-25 (IC-9929) */
    /* Workaround for wrong SENSRES values require using SDD00001, but here SDD00100 is used
       because it's required to operate with Windows Phone */
    NRF_NFCT->SENSRES =
            (NFCT_SENSRES_NFCIDSIZE_NFCID1Double << NFCT_SENSRES_NFCIDSIZE_Pos) |
            (NFCT_SENSRES_BITFRAMESDD_SDD00100   << NFCT_SENSRES_BITFRAMESDD_Pos);
    /* End:   Bugfix for FTPAN-25 (IC-9929)*/
}


hal_nfc_retval hal_nfc_setup(hal_nfc_callback callback, void *cbContext)
{
    uint8_t  nfc_internal[T2T_INTERNAL_BYTES_NR];
    
    m_nfc_lib_callback = callback;
    m_nfc_lib_context  = cbContext;
    
    hal_nfc_common_hw_setup(nfc_internal);

    (void) nfcSetInternal((char *) nfc_internal, sizeof(nfc_internal));
    
    /* Initialize SDK Clock module for handling high precission clock requests */
    m_clock_handler_item.event_handler = nrf_nfct_clock_event_handler;
    m_clock_handler_item.p_next        = NULL;

    ret_code_t err_code = nrf_drv_clock_init();
    
#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND    
    /* Begin: Bugfix for FTPAN-57 (IC-9563) */
    /* Values taken from IC-9563 */
    NRF_NFCT_SHUNTREGTHRESHOLDS = 0x00000005;
    NRF_NFCT_MODSTEPFIXED       = 0x0000003F;
    NRF_NFCT_MODSTEPMULTIPLIER  = 0x00000001;
    NRF_NFCT_INITIALLOADCTRLVAL = 0x00000001;
    /* End: Bugfix for FTPAN-57  (IC-9563) */

    /* Begin: Bugfix for FTPAN-17 (IC-9563) */
    /* Activating workaround. */
    field_timer_with_callback_config();
    NRF_TIMER4->TASKS_START = 1;
    /* End:   Bugfix for FTPAN-17 (IC-9563) */
#endif // HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND

#ifdef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
    field_timer_with_callback_config();
#endif

    LOG_HAL_NFC("[NFC_HAL]: initialize\r\n");
    
    HAL_NFC_DEBUG_PINS_INITIALIZE();

    if ((err_code == NRF_SUCCESS) || (err_code == MODULE_ALREADY_INITIALIZED))
    {
        return HAL_NFC_RETVAL_OK;
    }
    else
    {
        return HAL_NFC_RETVAL_ERROR;
    }
}

/**@brief Function for clearing an event flag in NRF_NFCT registers.
 *
 * @param[in]  p_event  Pointer to event register.
 *
 */
static inline void nrf_nfct_event_clear(volatile uint32_t * p_event)
{
    *p_event = 0;

    /* Perform read to ensure clearing is effective */
    volatile uint32_t dummy = *p_event;
    (void)dummy;
}

/**@brief Function for handling events from Clock Module.
 *
 * @param[in]  event  Clock event.
 *
 */
static inline void nrf_nfct_clock_event_handler(nrf_drv_clock_evt_type_t event)
{
    switch(event)
    {
        case NRF_DRV_CLOCK_EVT_HFCLK_STARTED:
            /* Activate NFCT only when HFXO is running */
            HAL_NFC_DEBUG_PIN_SET(HCLOCK_ON_PIN);  //DEBUG!
            NRF_NFCT->TASKS_ACTIVATE = 1;
            HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_ON_PIN);  //DEBUG!
            break;

        default:
            /* No implementation required */
            break;
    }
}

#ifndef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
/**@brief Function for evaluating and handling NFC field events.
 *
 * @param[in]  field_state  Current field state.
 *
 */
static inline void nrf_nfct_field_event_handler(volatile nfct_field_sense_state_t field_state)
{
    if (field_state == NFC_FIELD_STATE_UNKNOWN)
    {
        /* Probe NFC field */
        uint32_t field_present = NRF_NFCT->FIELDPRESENT;

        if (field_present & m_nfc_fieldpresent_mask)
        {
            field_state = NFC_FIELD_STATE_ON;
        }
        else
        {
            field_state = NFC_FIELD_STATE_OFF;
        }
    }

    /* Field event service */
    switch(field_state)
    {
        case NFC_FIELD_STATE_ON:
            if (!m_field_on)
            {
                HAL_NFC_DEBUG_PIN_SET(HCLOCK_ON_PIN);  //DEBUG!
                nrf_drv_clock_hfclk_request(&m_clock_handler_item);
                HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_ON_PIN);  //DEBUG!
            }
            m_field_on = true;
            break;

        case NFC_FIELD_STATE_OFF:
            HAL_NFC_DEBUG_PIN_SET(HCLOCK_OFF_PIN);  //DEBUG!
            
            NRF_NFCT->TASKS_SENSE = 1;
        
        
            nrf_drv_clock_hfclk_release();

            //NRF_NFCT->TASKS_SENSE = 1;
            m_field_on = false;

            NRF_NFCT->INTENCLR = 
                (NFCT_INTENCLR_RXFRAMEEND_Clear << NFCT_INTENCLR_RXFRAMEEND_Pos) |
                (NFCT_INTENCLR_RXERROR_Clear    << NFCT_INTENCLR_RXERROR_Pos);
                
            /* Change mask to FIELD_OFF state - trigger FIELD_ON even if HW has not locked to the field */
            m_nfc_fieldpresent_mask = NFC_FIELD_OFF_MASK;

            if ((m_nfc_lib_callback != NULL) )
            {
                m_nfc_lib_callback(m_nfc_lib_context, HAL_NFC_EVENT_FIELD_OFF, 0, 0);
            }
            
            HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_OFF_PIN);  //DEBUG!
            break;

        default:
            /* No implementation required */
            break;
    }
}
#endif

/* This function is used by nfc_lib for unit testing only */
hal_nfc_retval hal_nfc_set_parameter(hal_nfc_param_id id, void *data, size_t dataLength)
{
    (void)id;
    (void)data;
    (void)dataLength;

    return HAL_NFC_RETVAL_OK;
} 

/* This function is used by nfc_lib for unit testing only */
hal_nfc_retval hal_nfc_get_parameter(hal_nfc_param_id id, void *data, size_t *maxDataLength)
{
    (void)id;
    (void)data;
    (void)maxDataLength;

    return HAL_NFC_RETVAL_OK;
}


hal_nfc_retval hal_nfc_start(void)
{
    NRF_NFCT->ERRORSTATUS = NRF_NFCT_ERRORSTATUS_ALL;

    NVIC_ClearPendingIRQ(NFCT_IRQn);
    NVIC_SetPriority(NFCT_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(NFCT_IRQn);

    NRF_NFCT->TASKS_SENSE = 1;

    LOG_HAL_NFC("[NFC_HAL]: start\r\n");
    
    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_send(const char *data, size_t dataLength)
{
    if (dataLength == 0)
    {
        return HAL_NFC_RETVAL_INVALID_SIZE;
    }

    /* Ignore previous TX END events, SW takes care only for data frames which tranmission is triggered in this function */
    nrf_nfct_event_clear(&NRF_NFCT->EVENTS_TXFRAMEEND);

    NRF_NFCT->PACKETPTR     = (uint32_t) data;
    NRF_NFCT->TXD.AMOUNT    = (dataLength << NFCT_TXD_AMOUNT_TXDATABYTES_Pos) &
                               NFCT_TXD_AMOUNT_TXDATABYTES_Msk;
    NRF_NFCT->INTENSET      = (NFCT_INTENSET_TXFRAMEEND_Enabled << NFCT_INTENSET_TXFRAMEEND_Pos);
    NRF_NFCT->TASKS_STARTTX = 1;

    LOG_HAL_NFC("[NFC_HAL]: send\r\n");
    
    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_stop(void)
{
    NRF_NFCT->TASKS_DISABLE = 1;

    LOG_HAL_NFC("[NFC_HAL]: stop\r\n");
    
    return HAL_NFC_RETVAL_OK;
}

hal_nfc_retval hal_nfc_done(void)
{
    m_nfc_lib_callback = (hal_nfc_callback) NULL;

    return HAL_NFC_RETVAL_OK;
}

void NFCT_IRQHandler(void)
{
    nfct_field_sense_state_t current_field = NFC_FIELD_STATE_NONE;

    HAL_NFC_DEBUG_PIN_SET(NFC_EVENT_PIN);  //DEBUG!
    
#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
        /* Begin: Bugfix for FTPAN-27 */
    if (hal_nfc_fielddetected)
    {
        hal_nfc_fielddetected        = false;
        NRF_NFCT_TASKS_DISABLERXDATA = 1;
        /* End: Bugfix for FTPAN-27 */
#else
    if (NRF_NFCT->EVENTS_FIELDDETECTED && (NRF_NFCT->INTEN & NFCT_INTEN_FIELDDETECTED_Msk)) 
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_FIELDDETECTED);
#endif
        HAL_NFC_DEBUG_PIN_SET(DETECT_EVENT_PIN);  //DEBUG!
        current_field = NFC_FIELD_STATE_ON;

        LOG_HAL_NFC("[NFC_HAL]: fielddetected\r\n");
        HAL_NFC_DEBUG_PIN_CLEAR(DETECT_EVENT_PIN);  //DEBUG!
    }

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
    /* Begin: Bugfix for FTPAN-27 */
    if (hal_nfc_fieldlost)
    {
        hal_nfc_fieldlost = false;
        current_field     =
           (current_field == NFC_FIELD_STATE_NONE) ? NFC_FIELD_STATE_OFF : NFC_FIELD_STATE_UNKNOWN;

        NRF_NFCT->TASKS_SENSE = 1;

        LOG_HAL_NFC("[NFC_HAL]: fieldlost\r\n");
    }
    /* End: Bugfix for FTPAN-27 */
#elif !defined( HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND )
    if (NRF_NFCT->EVENTS_FIELDLOST && (NRF_NFCT->INTEN & NFCT_INTEN_FIELDLOST_Msk))
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_FIELDLOST);
        current_field =
           (current_field == NFC_FIELD_STATE_NONE) ? NFC_FIELD_STATE_OFF : NFC_FIELD_STATE_UNKNOWN;

        LOG_HAL_NFC("[NFC_HAL]: fieldlost\r\n");
    }
#endif

    /* Perform actions if any FIELD event is active */
    if (current_field != NFC_FIELD_STATE_NONE)
    {
       nrf_nfct_field_event_handler(current_field);
    }

    if (NRF_NFCT->EVENTS_RXFRAMEEND && (NRF_NFCT->INTEN & NFCT_INTEN_RXFRAMEEND_Msk))
    {
        /* Take into account only number of whole bytes */
        uint32_t rx_data_size = ((NRF_NFCT->RXD.AMOUNT & NFCT_RXD_AMOUNT_RXDATABYTES_Msk) >>
                                 NFCT_RXD_AMOUNT_RXDATABYTES_Pos) - NFC_CRC_SIZE;
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_RXFRAMEEND);

        /* Look for Tag 2 Type READ Command */
        if (m_nfc_rx_buffer[0] == T2T_READ_CMD)
        {
            if(m_nfc_lib_callback != NULL)
            {
                /* This callback should trigger transmission of READ Response */
                m_nfc_lib_callback(m_nfc_lib_context,
                                   HAL_NFC_EVENT_DATA_RECEIVED,
                                   (void*) m_nfc_rx_buffer,
                                   rx_data_size);
            }
        }
        else
        {
            /* Indicate that SLP_REQ was received - this will cause FRAMEDELAYTIMEOUT error */
            if(m_nfc_rx_buffer[0] == NFC_SLP_REQ_CMD)
            {
                m_slp_req_received = true;
            }
            /* Not a READ Command, so wait for next frame reception */
            NRF_NFCT->TASKS_ENABLERXDATA = 1;          
        }

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
        /* Begin:   Bugfix for FTPAN-45 (IC-6915) */
        m_nfc_active = 0;
        /* End:   Bugfix for FTPAN-45 (IC-69150) */
#endif
        LOG_HAL_NFC("[NFC_HAL]: rx_fend\r\n");
    }

    if (NRF_NFCT->EVENTS_TXFRAMEEND && (NRF_NFCT->INTEN & NFCT_INTEN_TXFRAMEEND_Msk))
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_TXFRAMEEND);

        /* Disable TX END event to ignore frame transmission other than READ response */
        NRF_NFCT->INTENCLR = (NFCT_INTENCLR_TXFRAMEEND_Clear << NFCT_INTENCLR_TXFRAMEEND_Pos);

        /* Set up for reception */
        NRF_NFCT->PACKETPTR          = (uint32_t) m_nfc_rx_buffer;
        NRF_NFCT->MAXLEN             = NFC_RX_BUFFER_SIZE;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;

        if (m_nfc_lib_callback != NULL)
        {
            m_nfc_lib_callback(m_nfc_lib_context, HAL_NFC_EVENT_DATA_TRANSMITTED, 0, 0);
        }

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
        /* Begin:   Bugfix for FTPAN-45 (IC-6915) */
        m_nfc_active = 0;
        /* End:   Bugfix for FTPAN-45 (IC-6915) */
#endif
        LOG_HAL_NFC("[NFC_HAL]: tx_fend\r\n");
    }

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
    /* Begin:   Bugfix for FTPAN-45 (IC-6915) */
    if (NRF_NFCT->EVENTS_RXFRAMESTART)
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_RXFRAMESTART);

        if (m_nfc_active == 0)
        {
            m_nfc_active = 1;
        }
        LOG_HAL_NFC("[NFC_HAL]: rx_fstart\r\n");
    }
    if (NRF_NFCT->EVENTS_TXFRAMESTART)
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_TXFRAMESTART);

        if (m_nfc_active == 0)
        {
            m_nfc_active = 1;
        }
        LOG_HAL_NFC("[NFC_HAL]: tx_fstart\r\n");
    }
    /* End:   Bugfix for FTPAN-45 (IC-6915) */
#endif

    if (NRF_NFCT->EVENTS_SELECTED && (NRF_NFCT->INTEN & NFCT_INTEN_SELECTED_Msk))
    {
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_SELECTED);
        /* Clear also RX END and RXERROR events because SW does not take care of commands which were received before selecting the tag */
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_RXFRAMEEND);
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_RXERROR);

        /* Set up registers for EasyDMA and start receiving packets */
        NRF_NFCT->PACKETPTR          = (uint32_t) m_nfc_rx_buffer;
        NRF_NFCT->MAXLEN             = NFC_RX_BUFFER_SIZE;
        NRF_NFCT->TASKS_ENABLERXDATA = 1;

        NRF_NFCT->INTENSET = (NFCT_INTENSET_RXFRAMEEND_Enabled << NFCT_INTENSET_RXFRAMEEND_Pos) |
                             (NFCT_INTENSET_RXERROR_Enabled    << NFCT_INTENSET_RXERROR_Pos);

        /* At this point any previous error status can be ignored */
        NRF_NFCT->FRAMESTATUS.RX = NRF_NFCT_FRAMESTATUS_RX_MSK;
        NRF_NFCT->ERRORSTATUS    = NRF_NFCT_ERRORSTATUS_ALL;

#ifndef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND
        /* Change mask to FIELD_ON state - trigger FIELD_ON only if HW has locked to the field */
        m_nfc_fieldpresent_mask = NFC_FIELD_ON_MASK;
#endif
        
        if (m_nfc_lib_callback != NULL)
        {
            m_nfc_lib_callback(m_nfc_lib_context, HAL_NFC_EVENT_FIELD_ON, 0, 0);
        }

        LOG_HAL_NFC("[NFC_HAL]: selected\r\n");
    }

    if (NRF_NFCT->EVENTS_RXERROR && (NRF_NFCT->INTEN & NFCT_INTEN_RXERROR_Msk))
    {
        uint32_t rx_status = NRF_NFCT->FRAMESTATUS.RX;
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_RXERROR);

        LOG_HAL_NFC("[NFC_HAL]: rxerror (0x%x)\r\n", (unsigned int) rx_status);
        (void) rx_status;

        /* Clear rx frame status */
        NRF_NFCT->FRAMESTATUS.RX = NRF_NFCT_FRAMESTATUS_RX_MSK;
    }

    if (NRF_NFCT->EVENTS_ERROR && (NRF_NFCT->INTEN & NFCT_INTEN_ERROR_Msk))
    {
        uint32_t err_status = NRF_NFCT->ERRORSTATUS;
        nrf_nfct_event_clear(&NRF_NFCT->EVENTS_ERROR);

        /* Clear FRAMEDELAYTIMEOUT error (expected HW behaviour) when SLP_REQ command was received */
        if ((err_status & NFCT_ERRORSTATUS_FRAMEDELAYTIMEOUT_Msk) && m_slp_req_received)
        {
            NRF_NFCT->ERRORSTATUS = NFCT_ERRORSTATUS_FRAMEDELAYTIMEOUT_Msk;
            m_slp_req_received    = false;

            LOG_HAL_NFC("[NFC_HAL]: error (SLP_REQ)\r\n");
        }
        /* Report any other error */
        err_status &= ~NFCT_ERRORSTATUS_FRAMEDELAYTIMEOUT_Msk;
        if (err_status)
        {
            LOG_HAL_NFC("[NFC_HAL]: error (0x%x)\r\n", (unsigned int) err_status);
        }

        /* Clear error status */
        NRF_NFCT->ERRORSTATUS = NRF_NFCT_ERRORSTATUS_ALL;
    }
    
    HAL_NFC_DEBUG_PIN_CLEAR(NFC_EVENT_PIN);  //DEBUG!
}


#ifdef HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND

#ifdef  HAL_NFC_ENGINEERING_A_FTPAN_WORKAROUND
    #error Wrong workaround combination
#endif

static uint32_t   field_state_cnt = 0;
/**
 * @brief Function for evaluating and handling NFC fieldlost event.
 */
static void hal_nfc_field_check(void)
{
    uint32_t nfc_fieldpresen_masked;

    nfc_fieldpresen_masked = NRF_NFCT->FIELDPRESENT & NFC_HAL_FIELDPRESENT_MASK;

    if (nfc_fieldpresen_masked == NFC_HAL_FIELDPRESENT_IS_LOST)
    {
        ++field_state_cnt;
        if (field_state_cnt > 7)
        {
            HAL_NFC_DEBUG_PIN_SET(HCLOCK_OFF_PIN);  //DEBUG!

            NRF_TIMER4->TASKS_SHUTDOWN = 1;
        
            nrf_drv_clock_hfclk_release();
            
            /* Begin:   Bugfix for FTPAN-XX (IC-XXXX) NFCT won't release HFCLK */
            // reset the NFC for release HFCLK
            __DMB();
            NRF_NFCT_POWER = 0;
            __DMB();
            NRF_NFCT_POWER = 1;
            /* END:   Bugfix for FTPAN-XX (IC-XXXX) NFCT won't release HFCLK */

            if ((m_nfc_lib_callback != NULL) )
            {
                m_nfc_lib_callback(m_nfc_lib_context, HAL_NFC_EVENT_FIELD_OFF, 0, 0);
            }
            m_field_on = false;
            
            /* Begin:   Bugfix for FTPAN-XX (IC-XXXX) NFCT won't release HFCLK */
            // resume the NFCT to initialized state
            hal_nfc_re_setup();
            /* END:   Bugfix for FTPAN-XX (IC-XXXX) NFCT won't release HFCLK */
            
            HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_OFF_PIN);  //DEBUG!
        }
        
        return;
    }

    field_state_cnt = 0;
}

/**
 * @brief Function for enablinge hight precision clock and start eveluating fieldlost event.
 */
static inline void nrf_nfct_field_event_handler(volatile nfct_field_sense_state_t field_state)
{
    if (!m_field_on)
    {
        HAL_NFC_DEBUG_PIN_SET(HCLOCK_ON_PIN);  //DEBUG!
        nrf_drv_clock_hfclk_request(&m_clock_handler_item);
        
        NRF_TIMER4->TASKS_CLEAR = 1;
        NRF_TIMER4->TASKS_START = 1;
        field_state_cnt = 0;
        
        HAL_NFC_DEBUG_PIN_CLEAR(HCLOCK_ON_PIN);  //DEBUG!
    }
    m_field_on = true;
}

/**
 * @brief Function for resume the NFCT to initialized state after software's reset.
 */
static inline void hal_nfc_re_setup(void)
{
    uint8_t  nfc_internal[T2T_INTERNAL_BYTES_NR];
    
    hal_nfc_common_hw_setup(nfc_internal);
    
    LOG_HAL_NFC("[NFC_HAL]: reinitialize\r\n");
}
#endif // HAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND


