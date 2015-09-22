/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
#include "scanner_beacon.h"
#include <stdio.h>
#include <string.h>

#include "nrf_soc.h"
#include "app_error.h"
#include "app_util.h"


#define SCAN_INTERVAL_US  10000
#define TIMESLOT_LEN_US  (SCAN_INTERVAL_US * 3 + 1000)
#define MAX_ADV_PACK_LEN (BLE_GAP_ADV_MAX_SIZE + BLE_GAP_ADDR_LEN + 2)
#define ADV_TYPE_LEN      2
#define BEACON_DATA_LEN   21

static struct
{
    ble_uuid128_t                 uuid;
    bool                          keep_running;
    bool                          is_running;
    nrf_radio_request_t           timeslot_request;
    uint8_t                       scn_pdu[40];
    ble_scan_beacon_evt_handler_t evt_handler;
    ble_srv_error_handler_t       error_handler;                      /**< Function to be called in case of an error. */
} m_beacon_scanner;


enum mode_t
{
  SCN_INIT,
  SCN_1,
  SCN_2,
  SCN_3,
  SCN_DONE
};


static void decode_ad_type_flags(uint8_t *buffer, uint8_t len, adv_packet_t *adv_packet)
{
    int i = 1;
    adv_packet->gap_ad_flags = buffer[i];
}


static void decode_ad_manuf_spec_data(uint8_t *buffer, uint8_t len, adv_packet_t *adv_packet)
{
    int i = 1;
    adv_packet->adv_data.manuf_id = uint16_decode(&buffer[i]);
    i += sizeof(uint16_t);
    adv_packet->adv_data.beacon_dev_type = buffer[i++];
    uint8_t data_len = buffer[i++];
    if (data_len == BEACON_DATA_LEN)
    {
        memcpy(&adv_packet->adv_data.uuid, &buffer[i], sizeof(ble_uuid128_t));
        i += sizeof(ble_uuid128_t);
        adv_packet->adv_data.major = uint16_decode(&buffer[i]);
        i += sizeof(uint16_t);
        adv_packet->adv_data.minor = uint16_decode(&buffer[i]);
        i += sizeof(uint16_t);
        adv_packet->adv_data.rssi = uint16_decode(&buffer[i]);
    }
    else
    {
        memset(&adv_packet->adv_data.uuid, 0, sizeof(ble_uuid128_t));
        adv_packet->adv_data.major = 0;
        adv_packet->adv_data.minor = 0;
        adv_packet->adv_data.rssi = 0;
    }
}


static uint32_t decode_advertising(uint8_t *buffer, adv_packet_t *adv_packet)
{
    uint8_t i = 0;
    
    adv_packet->adv_type = buffer[i++];
    uint8_t total_len = buffer[i++];
    if (total_len > MAX_ADV_PACK_LEN)
    {
        return NRF_ERROR_NOT_FOUND;
    }
    i++;             /*skip next byte*/
    memcpy(adv_packet->addr.addr, &(buffer[i]), BLE_GAP_ADDR_LEN);
    i += BLE_GAP_ADDR_LEN;
    
    do
    {
        uint8_t field_len = buffer[i++];
        if(buffer[i] == BLE_GAP_AD_TYPE_FLAGS)
        {
            if (field_len != ADV_TYPE_LEN)
            {
                return NRF_ERROR_NOT_FOUND;
            }
            decode_ad_type_flags(&buffer[i], field_len, adv_packet);
            i += field_len;
        }
        else if (buffer[i] == BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA)
        {
            decode_ad_manuf_spec_data(&buffer[i], field_len, adv_packet);
            i += field_len;
        }
    }while( i < total_len);

    return NRF_SUCCESS;    
}


static nrf_radio_request_t * m_request_earliest(enum NRF_RADIO_PRIORITY priority)
{
    m_beacon_scanner.timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_beacon_scanner.timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_DEFAULT;
    m_beacon_scanner.timeslot_request.params.earliest.priority    = priority;
    m_beacon_scanner.timeslot_request.params.earliest.length_us   = TIMESLOT_LEN_US;
    m_beacon_scanner.timeslot_request.params.earliest.timeout_us  = 1000000;
    return &m_beacon_scanner.timeslot_request;
}

static void m_set_adv_ch(uint32_t channel)
{
    if (channel == 37)
    {
        NRF_RADIO->FREQUENCY    =  2;
        NRF_RADIO->DATAWHITEIV  = 37;
    }
    if (channel == 38)
    {
        NRF_RADIO->FREQUENCY    = 26;
        NRF_RADIO->DATAWHITEIV  = 38;
    }
    if (channel == 39)
    {
        NRF_RADIO->FREQUENCY    = 80;
        NRF_RADIO->DATAWHITEIV  = 39;
    }
}

static void m_configure_radio()
{
    NRF_RADIO->POWER        = 1;
    NRF_RADIO->PCNF0        =   (((1UL) << RADIO_PCNF0_S0LEN_Pos                               ) & RADIO_PCNF0_S0LEN_Msk)
                              | (((2UL) << RADIO_PCNF0_S1LEN_Pos                               ) & RADIO_PCNF0_S1LEN_Msk)
                              | (((6UL) << RADIO_PCNF0_LFLEN_Pos                               ) & RADIO_PCNF0_LFLEN_Msk);
    NRF_RADIO->PCNF1        =   (((RADIO_PCNF1_ENDIAN_Little)     << RADIO_PCNF1_ENDIAN_Pos    ) & RADIO_PCNF1_ENDIAN_Msk)
                              | (((3UL)                           << RADIO_PCNF1_BALEN_Pos     ) & RADIO_PCNF1_BALEN_Msk)
                              | (((0UL)                           << RADIO_PCNF1_STATLEN_Pos   ) & RADIO_PCNF1_STATLEN_Msk)
                              | ((((uint32_t) 37)                 << RADIO_PCNF1_MAXLEN_Pos    ) & RADIO_PCNF1_MAXLEN_Msk)
                              | ((RADIO_PCNF1_WHITEEN_Enabled     << RADIO_PCNF1_WHITEEN_Pos   ) & RADIO_PCNF1_WHITEEN_Msk);
    NRF_RADIO->CRCCNF       =   (((RADIO_CRCCNF_SKIPADDR_Skip)    << RADIO_CRCCNF_SKIPADDR_Pos ) & RADIO_CRCCNF_SKIPADDR_Msk)
                              | (((RADIO_CRCCNF_LEN_Three)        << RADIO_CRCCNF_LEN_Pos      ) & RADIO_CRCCNF_LEN_Msk);
    NRF_RADIO->CRCPOLY      = 0x0000065b;
    NRF_RADIO->RXADDRESSES  = ((RADIO_RXADDRESSES_ADDR0_Enabled)  << RADIO_RXADDRESSES_ADDR0_Pos);
    NRF_RADIO->SHORTS       = ((1 << RADIO_SHORTS_READY_START_Pos) | (1 << RADIO_SHORTS_END_DISABLE_Pos));
    NRF_RADIO->MODE         = ((RADIO_MODE_MODE_Ble_1Mbit)        << RADIO_MODE_MODE_Pos       ) & RADIO_MODE_MODE_Msk;
    NRF_RADIO->TIFS         = 150;
    NRF_RADIO->INTENSET     = (1 << RADIO_INTENSET_DISABLED_Pos) | (1 << RADIO_INTENSET_ADDRESS_Pos);
    NRF_RADIO->PREFIX0      = 0x0000008e; //access_addr[3]
    NRF_RADIO->BASE0        = 0x89bed600; //access_addr[0:3]
    NRF_RADIO->CRCINIT      = 0x00555555;
    NRF_RADIO->PACKETPTR    = (uint32_t) &m_beacon_scanner.scn_pdu[0];
    
    NVIC_EnableIRQ(RADIO_IRQn);
}

static void m_handle_start(void)
{
    // Configure TX_EN on TIMER EVENT_0
    NRF_PPI->CH[8].TEP    = (uint32_t)(&NRF_RADIO->TASKS_DISABLE);
    NRF_PPI->CH[8].EEP    = (uint32_t)(&NRF_TIMER0->EVENTS_COMPARE[0]);
    NRF_PPI->CHENSET      = (1 << 8);
    
    // Configure and initiate radio
    m_configure_radio();
    NRF_RADIO->TASKS_DISABLE = 1;
    NRF_TIMER0->CC[0] = 0; // TODO: Necessary?

    // Set up rescheduling
    NRF_TIMER0->INTENSET = (1UL << TIMER_INTENSET_COMPARE1_Pos);
    NRF_TIMER0->CC[1]    = TIMESLOT_LEN_US - 800;
    NVIC_EnableIRQ(TIMER0_IRQn);
}

static void m_handle_radio_disabled(enum mode_t mode)
{
    NRF_TIMER0->CC[0]       += SCAN_INTERVAL_US;
    
    switch (mode)
    {
        case SCN_1:
            m_set_adv_ch(37);
            NRF_RADIO->TASKS_RXEN = 1;
            break;
        case SCN_2:
            m_set_adv_ch(38);
            NRF_RADIO->TASKS_RXEN = 1;
            break;
        case SCN_3:
            m_set_adv_ch(39);
            NRF_RADIO->TASKS_RXEN = 1;
            break;
        default:
            break;
    }
}

static nrf_radio_signal_callback_return_param_t * m_timeslot_callback(uint8_t signal_type)
{
  static nrf_radio_signal_callback_return_param_t signal_callback_return_param;
  static enum mode_t mode;

  signal_callback_return_param.params.request.p_next  = NULL;
  signal_callback_return_param.callback_action        = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;

  switch (signal_type)
  {
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
        m_handle_start();

        mode = SCN_INIT;
        mode++;
        break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
        if (NRF_RADIO->EVENTS_ADDRESS == 1)
        {
            NRF_RADIO->EVENTS_ADDRESS = 0;

            ble_scan_beacon_evt_t evt;
            uint32_t err_code = decode_advertising(&m_beacon_scanner.scn_pdu[0], &(evt.rcv_adv_packet));
            if (err_code == NRF_SUCCESS)
            {
                if (m_beacon_scanner.evt_handler != NULL)
                {
                    evt.evt_type = BLE_SCAN_BEACON_ADVERTISER_FOUND;
                    m_beacon_scanner.evt_handler(&evt);
                }
            }
        }

        if (NRF_RADIO->EVENTS_DISABLED == 1)
        {
            NRF_RADIO->EVENTS_DISABLED = 0;

            if (mode == SCN_DONE)
            {
                mode = SCN_INIT;
                mode++;
            }

            m_handle_radio_disabled(mode);

            mode++;
        }
        break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
        if (NRF_TIMER0->EVENTS_COMPARE[1] == 1)
        {
            NRF_TIMER0->EVENTS_COMPARE[1] = 0;

            signal_callback_return_param.params.extend.length_us = TIMESLOT_LEN_US;
            signal_callback_return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND;
        }

        break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED:
        NRF_PPI->CHENCLR  = (1UL << 8);
        if (m_beacon_scanner.keep_running)
        {
            signal_callback_return_param.params.request.p_next   = m_request_earliest(NRF_RADIO_PRIORITY_NORMAL);
            signal_callback_return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
        }
        else
        {
            signal_callback_return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_END;
        }
        break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
        NRF_TIMER0->CC[1]    += TIMESLOT_LEN_US;
        break;
    default:
        if (m_beacon_scanner.error_handler != NULL)
        {
            m_beacon_scanner.error_handler(NRF_ERROR_INVALID_STATE);
        }
      break;
  }

  return ( &signal_callback_return_param );
}

void app_beacon_scanner_sd_evt_signal_handler(uint32_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case NRF_EVT_RADIO_SESSION_IDLE:
            if (m_beacon_scanner.is_running)
            {
                m_beacon_scanner.is_running = false;
                err_code = sd_radio_session_close();
                if ((err_code != NRF_SUCCESS) && (m_beacon_scanner.error_handler != NULL))
                {
                    m_beacon_scanner.error_handler(err_code);
                }
            }
            break;
        case NRF_EVT_RADIO_SESSION_CLOSED:
            break;
        case NRF_EVT_RADIO_BLOCKED:
        case NRF_EVT_RADIO_CANCELED: // Fall through
            if (m_beacon_scanner.keep_running)
            {
                err_code = sd_radio_request(m_request_earliest(NRF_RADIO_PRIORITY_NORMAL));
                if ((err_code != NRF_SUCCESS) && (m_beacon_scanner.error_handler != NULL))
                {
                    m_beacon_scanner.error_handler(err_code);
                }
            }
            break;
        default:
            break;
    }
}

void app_beacon_scanner_init(ble_beacon_scanner_init_t * p_init)
{
    m_beacon_scanner.evt_handler = p_init->evt_handler;
    m_beacon_scanner.error_handler= p_init->error_handler;
}

void app_beacon_scanner_start(void)
{
    uint32_t err_code;

    m_beacon_scanner.keep_running = true;
    m_beacon_scanner.is_running   = true;

    err_code = sd_radio_session_open(m_timeslot_callback);
    if ((err_code != NRF_SUCCESS) && (m_beacon_scanner.error_handler != NULL))
    {
        m_beacon_scanner.error_handler(err_code);
    }
    
    err_code = sd_radio_request(m_request_earliest(NRF_RADIO_PRIORITY_NORMAL));
    if ((err_code != NRF_SUCCESS) && (m_beacon_scanner.error_handler != NULL))
    {
        m_beacon_scanner.error_handler(err_code);
    }
}

void app_beacon_scanner_stop(void)
{
    m_beacon_scanner.keep_running = false;
}

