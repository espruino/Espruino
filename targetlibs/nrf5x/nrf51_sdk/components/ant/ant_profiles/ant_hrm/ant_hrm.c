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
#include "nrf_assert.h"
#include "app_error.h"
#include "ant_interface.h"
#include "app_util.h"
#include "ant_hrm.h"
#include "ant_hrm_utils.h"
#include "ant_hrm_page_logger.h"
#include "app_error.h"

#define BACKGROUND_DATA_INTERVAL    64      /**< The number of main data pages sent between background data page. 
                                                 Background data page is sent every 65th message. */
#define TX_TOGGLE_DIVISOR           4       /**< The number of messages between changing state of toggle bit. */

/**@brief HRM message data layout structure. */
typedef struct
{
    ant_hrm_page_t  page_number         : 7;
    uint8_t         toggle_bit          : 1;
    uint8_t         page_payload[7];
}ant_hrm_message_layout_t;

/**@brief HRM TX control block. */
typedef struct
{
    uint8_t         toggle_bit          : 1;
    ant_hrm_page_t  main_page_number    : 7;
    uint8_t         page_1_present      : 1;
    ant_hrm_page_t  ext_page_number     : 7;
    uint8_t         message_counter;
}ant_hrm_tx_cb_t;

/*lint -save -e452 */
STATIC_ASSERT( sizeof(ant_hrm_tx_cb_t) == HRM_TX_CB_SIZE);
/*lint -restore */ 

uint32_t ant_hrm_init(ant_hrm_profile_t * p_profile, ant_channel_config_t const * p_channel_config,
                      ant_hrm_tx_config_t const * p_tx_config)
{
    ASSERT(p_profile != NULL);
    ASSERT(p_channel_config != NULL);
    
    p_profile->channel_number = p_channel_config->channel_number;

    if (p_tx_config != NULL)
    {
        p_profile->p_cb           = p_tx_config->p_cb_buffer;
        ant_hrm_tx_cb_t * p_hrm_cb   = (ant_hrm_tx_cb_t *)p_profile->p_cb;

        p_hrm_cb->page_1_present = p_tx_config->page_1_present;

        ASSERT((p_tx_config->main_page_number == ANT_HRM_PAGE_0) || (p_tx_config->main_page_number == ANT_HRM_PAGE_4));
        p_hrm_cb->main_page_number  = p_tx_config->main_page_number;

        p_hrm_cb->ext_page_number = p_hrm_cb->page_1_present ? ANT_HRM_PAGE_1 : ANT_HRM_PAGE_2;
        p_hrm_cb->message_counter = 0;
        p_hrm_cb->toggle_bit      = true;
    }

    p_profile->page_0 = (ant_hrm_page0_data_t)DEFAULT_ANT_HRM_PAGE0();
    p_profile->page_1 = (ant_hrm_page1_data_t)DEFAULT_ANT_HRM_PAGE1();
    p_profile->page_2 = (ant_hrm_page2_data_t)DEFAULT_ANT_HRM_PAGE2();
    p_profile->page_3 = (ant_hrm_page3_data_t)DEFAULT_ANT_HRM_PAGE3();
    p_profile->page_4 = (ant_hrm_page4_data_t)DEFAULT_ANT_HRM_PAGE4();

    LOG_HRM("ANT HRM channel %u init\n\r", p_profile->channel_number);
    return ant_channel_init(p_channel_config);
}

uint32_t ant_hrm_open(ant_hrm_profile_t * p_profile)
{
    LOG_HRM("ANT HRM channel %u open\n\r", p_profile->channel_number);
    return sd_ant_channel_open(p_profile->channel_number);
}

/**@brief Function for encoding HRM message.
 *
 * @note Assume to be call each time when Tx window will occur.
 */
static void encode_tx_message(ant_hrm_profile_t * p_profile, uint8_t * p_message_payload)
{
    ant_hrm_message_layout_t * p_hrm_message_payload = (ant_hrm_message_layout_t *)p_message_payload;
    ant_hrm_tx_cb_t * p_hrm_cb                       = (ant_hrm_tx_cb_t *)p_profile->p_cb;

    ASSERT(p_hrm_cb != NULL);

    if (p_hrm_cb->message_counter == (BACKGROUND_DATA_INTERVAL))
    {
        p_hrm_message_payload->page_number  = p_hrm_cb->ext_page_number;

        p_hrm_cb->message_counter = 0;

        p_hrm_cb->ext_page_number++;
        if (p_hrm_cb->ext_page_number > ANT_HRM_PAGE_3)
        {
            p_hrm_cb->ext_page_number = p_hrm_cb->page_1_present ? ANT_HRM_PAGE_1 : ANT_HRM_PAGE_2;
        }
    }
    else
    {
        p_hrm_message_payload->page_number  = p_hrm_cb->main_page_number;
    }

    if (p_hrm_cb->message_counter % TX_TOGGLE_DIVISOR == 0)
    {
        p_hrm_cb->toggle_bit ^= 1;
    }
    
    p_hrm_cb->message_counter++;
    
    p_hrm_message_payload->toggle_bit  = p_hrm_cb->toggle_bit;

    LOG_HRM("HRM TX Page number:               %u\r\n", p_hrm_message_payload->page_number);

    ant_hrm_page_0_encode(p_hrm_message_payload->page_payload, &(p_profile->page_0)); // Page 0 is present in each message

    switch (p_hrm_message_payload->page_number)
    {
        case ANT_HRM_PAGE_0:
            // No implementation needed
            break;

        case ANT_HRM_PAGE_1:
            ant_hrm_page_1_encode(p_hrm_message_payload->page_payload, &(p_profile->page_1));
            break;

        case ANT_HRM_PAGE_2:
            ant_hrm_page_2_encode(p_hrm_message_payload->page_payload, &(p_profile->page_2));
            break;

        case ANT_HRM_PAGE_3:
            ant_hrm_page_3_encode(p_hrm_message_payload->page_payload, &(p_profile->page_3));
            break;

        case ANT_HRM_PAGE_4:
            ant_hrm_page_4_encode(p_hrm_message_payload->page_payload, &(p_profile->page_4));
            break;

        default:
            APP_ERROR_HANDLER(NRF_ERROR_NOT_FOUND);
            break;
    }
    LOG_HRM("\r\n");
}

void ant_hrm_tx_evt_handle(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event)
{
    if (p_ant_event->channel == p_profile->channel_number)
    {
        uint8_t p_message_payload[8];
        uint32_t err_code;
        switch (p_ant_event->event)
        {
            case EVENT_TX:
                encode_tx_message(p_profile, p_message_payload);
                err_code = sd_ant_broadcast_message_tx(p_profile->channel_number, sizeof(p_message_payload), p_message_payload);
                APP_ERROR_CHECK(err_code);
                break;

            default:
                break;
        }
    }
}

/**@brief Function for encoding HRM message.
 *
 * @note Assume to be call each time when Rx window will occur.
 */
static void decode_rx_message(ant_hrm_profile_t * p_profile, uint8_t * p_message_payload)
{
    const ant_hrm_message_layout_t * p_hrm_message_payload  = (ant_hrm_message_layout_t *)p_message_payload;

    LOG_HRM("HRM RX Page Number:               %u\r\n", p_hrm_message_payload->page_number);

    ant_hrm_page_0_decode(p_hrm_message_payload->page_payload, &(p_profile->page_0)); // Page 0 is present in each message

    switch (p_hrm_message_payload->page_number)
    {
        case ANT_HRM_PAGE_0:
            // No implementation needed
            break;

        case ANT_HRM_PAGE_1:
            ant_hrm_page_1_decode(p_hrm_message_payload->page_payload, &(p_profile->page_1));
            break;

        case ANT_HRM_PAGE_2:
            ant_hrm_page_2_decode(p_hrm_message_payload->page_payload, &(p_profile->page_2));
            break;

        case ANT_HRM_PAGE_3:
            ant_hrm_page_3_decode(p_hrm_message_payload->page_payload, &(p_profile->page_3));
            break;

        case ANT_HRM_PAGE_4:
            ant_hrm_page_4_decode(p_hrm_message_payload->page_payload, &(p_profile->page_4));
            break;

        default:
            break;
    }
    LOG_HRM("\r\n");
}

void ant_hrm_rx_evt_handle(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event)
{
    if (p_ant_event->channel == p_profile->channel_number)
    {
        ANT_MESSAGE * p_message = (ANT_MESSAGE *)p_ant_event->evt_buffer;
        switch (p_ant_event->event)
        {
            case EVENT_RX:
                switch(p_message->ANT_MESSAGE_ucMesgID)
                {
                    case MESG_BROADCAST_DATA_ID:
                    case MESG_ACKNOWLEDGED_DATA_ID:
                    case MESG_BURST_DATA_ID:
                        decode_rx_message(p_profile, p_message->ANT_MESSAGE_aucPayload);
                    break;

                    default:
                        // No implementation needed
                    break;
                }
                break;
            default:
                break;
        }
    }
}
