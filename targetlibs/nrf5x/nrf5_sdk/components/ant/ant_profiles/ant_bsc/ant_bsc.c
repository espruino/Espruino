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
#include "nrf_error.h"
#include "app_error.h"
#include "ant_interface.h"
#include "app_util.h"
#include "ant_bsc.h"
#include "ant_bsc_utils.h"
#include "ant_bsc_page_logger.h"
#include "app_error.h"

#define MAIN_DATA_INTERVAL          4       /**< The number of background data pages sent between main data pages.*/
#define BACKGROUND_DATA_INTERVAL    64      /**< The number of main data pages sent between background data page. 
                                                 Background data page is sent every 65th message. */
#define TX_TOGGLE_DIVISOR           4       /**< The number of messages between changing state of toggle bit. */

/**@brief BSC message data layout structure. */
typedef struct
{
    ant_bsc_page_t  page_number         : 7;
    uint8_t         toggle_bit          : 1;
    uint8_t         page_payload[7];
}ant_bsc_single_message_layout_t;

typedef struct
{
    uint8_t         page_payload[8];
}ant_bsc_combined_message_layout_t;

typedef union
{
    ant_bsc_single_message_layout_t     speed_or_cadence;
    ant_bsc_combined_message_layout_t   combined;
}ant_bsc_message_layout_t;


/**@brief Function for initializing the ANT BSC profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 *
 * @retval     NRF_SUCCESS      If initialization was successful. Otherwise, an error code is returned.
 */
static ret_code_t ant_bsc_init(ant_bsc_profile_t          * p_profile,
                               ant_channel_config_t const * p_channel_config)
{
    p_profile->channel_number = p_channel_config->channel_number;

    p_profile->page_0       = DEFAULT_ANT_BSC_PAGE0();
    p_profile->page_1       = DEFAULT_ANT_BSC_PAGE1();
    p_profile->page_2       = DEFAULT_ANT_BSC_PAGE2();
    p_profile->page_3       = DEFAULT_ANT_BSC_PAGE3();
    p_profile->page_4       = DEFAULT_ANT_BSC_PAGE4();
    p_profile->page_5       = DEFAULT_ANT_BSC_PAGE5();
    p_profile->page_comb_0  = DEFAULT_ANT_BSC_COMBINED_PAGE0();

    LOG_BSC("ANT BSC channel %u init\n\r", p_profile->channel_number);
    return ant_channel_init(p_channel_config);
}

ret_code_t ant_bsc_disp_init(ant_bsc_profile_t           * p_profile,
                             ant_channel_config_t const  * p_channel_config,
                             ant_bsc_disp_config_t const * p_disp_config)
{
    ASSERT(p_profile != NULL);
    ASSERT(p_channel_config != NULL);
    ASSERT(p_disp_config->evt_handler != NULL);

    p_profile->evt_handler   = p_disp_config->evt_handler;
    p_profile->_cb.p_disp_cb = p_disp_config->p_cb;

    p_profile->_cb.p_disp_cb->device_type = p_channel_config->device_type;

    return ant_bsc_init(p_profile, p_channel_config);
}

ret_code_t ant_bsc_sens_init(ant_bsc_profile_t           * p_profile,
                             ant_channel_config_t const  * p_channel_config,
                             ant_bsc_sens_config_t const * p_sens_config)
{
    ASSERT(p_profile != NULL);
    ASSERT(p_channel_config != NULL);
    ASSERT(p_sens_config != NULL);
    ASSERT(p_sens_config->p_cb_buffer != NULL);
    ASSERT(p_sens_config->evt_handler != NULL);

    ASSERT((p_sens_config->main_page_number == ANT_BSC_PAGE_0) 
        || (p_sens_config->main_page_number == ANT_BSC_PAGE_5)
        || (p_sens_config->main_page_number == ANT_BSC_COMB_PAGE_0));

    p_profile->evt_handler   = p_sens_config->evt_handler;
    p_profile->_cb.p_sens_cb = p_sens_config->p_cb;

    p_profile->_cb.p_sens_cb->page_1_present    = p_sens_config->page_1_present;
    p_profile->_cb.p_sens_cb->page_4_present    = p_sens_config->page_4_present;
    p_profile->_cb.p_sens_cb->main_page_number  = p_sens_config->main_page_number;
    p_profile->_cb.p_sens_cb->bkgd_page_number  = 
                            p_sens_config->page_1_present ? ANT_BSC_PAGE_1 : ANT_BSC_PAGE_2;
    p_profile->_cb.p_sens_cb->message_counter   = 0;
    p_profile->_cb.p_sens_cb->toggle_bit        = true;
    p_profile->_cb.p_sens_cb->device_type       = p_channel_config->device_type;  /* Device type is set up in channel config */

    return ant_bsc_init(p_profile, p_channel_config);
}

/**@brief Function for setting the next background page number.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 */
__STATIC_INLINE void next_bkgd_page_number_set(ant_bsc_profile_t * p_profile)
{
    /* Determine the last background page*/
    ant_bsc_page_t last_bkgd_page = 
                p_profile->_cb.p_sens_cb->page_4_present ? ANT_BSC_PAGE_4 : ANT_BSC_PAGE_3;

    /* Switch the background page according to user settings */
    ++(p_profile->_cb.p_sens_cb->bkgd_page_number);
    if (p_profile->_cb.p_sens_cb->bkgd_page_number > last_bkgd_page)
    {
        p_profile->_cb.p_sens_cb->bkgd_page_number =
                    p_profile->_cb.p_sens_cb->page_1_present ? ANT_BSC_PAGE_1 : ANT_BSC_PAGE_2;
    }
}

/**@brief Function for getting next page number to send.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @return     Next page number.
 */
static ant_bsc_page_t next_page_number_get(ant_bsc_profile_t * p_profile)
{
    ant_bsc_page_t page_number;

    /* This is a single speed or cadence sensor - switch data page */
    if (p_profile->_cb.p_sens_cb->message_counter < (BACKGROUND_DATA_INTERVAL))
    {
        /* Return main page */
        page_number  = p_profile->_cb.p_sens_cb->main_page_number;
    }
    else
    {
        /* Return background page */
        page_number  = p_profile->_cb.p_sens_cb->bkgd_page_number;
    }

    /* Set page toggle bit */
    if ((p_profile->_cb.p_sens_cb->message_counter % TX_TOGGLE_DIVISOR) == 0)
    {
        p_profile->_cb.p_sens_cb->toggle_bit ^= 1;
    }

    /* Update message counter, wrap when counter equals 64 + 4 */
    ++(p_profile->_cb.p_sens_cb->message_counter);
    if (p_profile->_cb.p_sens_cb->message_counter == (BACKGROUND_DATA_INTERVAL + MAIN_DATA_INTERVAL))
    {
        p_profile->_cb.p_sens_cb->message_counter = 0;
        /* Set new background data page number */
        next_bkgd_page_number_set(p_profile);
    }

    return page_number;
}

/**@brief Function for encoding BSC message.
 *
 * @param[in]  p_profile         Pointer to the profile instance.
 * @param[out] p_message_payload Pointer to the message payload structure.
 *
 * @note Assume to be called each time when Tx window will occur.
 */
static void sens_message_encode(ant_bsc_profile_t * p_profile, uint8_t * p_message_payload)
{
    ant_bsc_message_layout_t * p_bsc_message_payload = (ant_bsc_message_layout_t *)p_message_payload;
    ant_bsc_evt_t              bsc_sens_event;

    if (p_profile->_cb.p_sens_cb->device_type == BSC_COMBINED_DEVICE_TYPE)
    {
        LOG_BSC("%-30s \"Combined Speed & Cadence Page\"\n\r", "BSC TX Page:");
        ant_bsc_combined_page_0_encode(p_bsc_message_payload->combined.page_payload,
                                       &(p_profile->page_comb_0));
        bsc_sens_event = (ant_bsc_evt_t) ANT_BSC_COMB_PAGE_0_UPDATED;
    }
    else
    {
        p_bsc_message_payload->speed_or_cadence.page_number = next_page_number_get(p_profile);
        p_bsc_message_payload->speed_or_cadence.toggle_bit  = p_profile->_cb.p_sens_cb->toggle_bit;
        LOG_BSC("%-30s %u\n\r", "BSC TX Page number:",
                p_bsc_message_payload->speed_or_cadence.page_number);

        ant_bsc_page_0_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                              &(p_profile->page_0));
        bsc_sens_event = (ant_bsc_evt_t) p_bsc_message_payload->speed_or_cadence.page_number;

        switch(p_bsc_message_payload->speed_or_cadence.page_number)
        {
            case ANT_BSC_PAGE_0:
                // No implementation needed
                break;
            case  ANT_BSC_PAGE_1:
                ant_bsc_page_1_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_1));
                break;
            case ANT_BSC_PAGE_2:
                ant_bsc_page_2_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_2));
                break;
            case ANT_BSC_PAGE_3:
                ant_bsc_page_3_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_3));
                break;
            case ANT_BSC_PAGE_4:
                ant_bsc_page_4_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_4));
                break;
            case ANT_BSC_PAGE_5:
                ant_bsc_page_5_encode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_5));
                break;
            default:
                // No implementation needed
                break;
        }
    }
    LOG_BSC("\r\n");

    p_profile->evt_handler(p_profile, bsc_sens_event);
}

/**@brief Function for setting payload for ANT message and sending it.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 */
static void ant_message_send(ant_bsc_profile_t * p_profile)
{
    uint8_t p_message_payload[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    uint32_t err_code;

    sens_message_encode(p_profile, p_message_payload);
    err_code = sd_ant_broadcast_message_tx(p_profile->channel_number,
                                           sizeof(p_message_payload),
                                           p_message_payload);
    APP_ERROR_CHECK(err_code);
}

void ant_bsc_sens_evt_handler(ant_bsc_profile_t * p_profile,
                              ant_evt_t * p_ant_event)
{
    if (p_ant_event->channel == p_profile->channel_number)
    {
        switch (p_ant_event->event)
        {
            case EVENT_TX:
                ant_message_send(p_profile);
                break;

            default:
                break;
        }
    }
}

ret_code_t ant_bsc_disp_open(ant_bsc_profile_t * p_profile)
{
    ASSERT(p_profile != NULL);

    LOG_BSC("ANT BSC channel %u open\n\r", p_profile->channel_number);
    return sd_ant_channel_open(p_profile->channel_number);
}

ret_code_t ant_bsc_sens_open(ant_bsc_profile_t * p_profile)
{
    ASSERT(p_profile != NULL);

    // Fill tx buffer for the first frame
    ant_message_send(p_profile);

    LOG_BSC("ANT BSC channel %u open\n\r", p_profile->channel_number);
    return sd_ant_channel_open(p_profile->channel_number);
}

/**@brief Function for decoding BSC message.
 *
 * @param[in,out] p_profile         Pointer to the profile instance.
 * @param[in]     p_message_payload Pointer to the message payload structure.
 *
 * @note Assume to be call each time when Rx window will occur.
 */
static void disp_message_decode(ant_bsc_profile_t * p_profile, uint8_t * p_message_payload)
{
    const ant_bsc_message_layout_t * p_bsc_message_payload = 
                                    (ant_bsc_message_layout_t *)p_message_payload;
    ant_bsc_evt_t                    bsc_disp_event;

    if (p_profile->_cb.p_disp_cb->device_type == BSC_COMBINED_DEVICE_TYPE)
    {
        LOG_BSC("%-30s \"Combined Speed & Cadence Page\"\n\r", "BSC RX Page Number:");
        ant_bsc_combined_page_0_decode(p_bsc_message_payload->combined.page_payload,
                                       &(p_profile->page_comb_0));
        bsc_disp_event = (ant_bsc_evt_t) ANT_BSC_COMB_PAGE_0_UPDATED;
    }
    else
    {
        LOG_BSC("%-30s %u\n\r", "BSC RX Page Number:",
                p_bsc_message_payload->speed_or_cadence.page_number);
        ant_bsc_page_0_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                              &(p_profile->page_0)); // Page 0 is present in each message
        bsc_disp_event = (ant_bsc_evt_t) p_bsc_message_payload->speed_or_cadence.page_number;

        switch (p_bsc_message_payload->speed_or_cadence.page_number)
        {
            case ANT_BSC_PAGE_0:
                // No implementation needed
                break;

            case ANT_BSC_PAGE_1:
                ant_bsc_page_1_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_1));
                break;

            case ANT_BSC_PAGE_2:
                ant_bsc_page_2_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_2));
                break;

            case ANT_BSC_PAGE_3:
                ant_bsc_page_3_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_3));
                break;

            case ANT_BSC_PAGE_4:
                ant_bsc_page_4_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_4));
                break;

            case ANT_BSC_PAGE_5:
                ant_bsc_page_5_decode(p_bsc_message_payload->speed_or_cadence.page_payload,
                                      &(p_profile->page_5));
                break;

            default:
                // No implementation needed
                break;
        }
    }
    LOG_BSC("\r\n");

    p_profile->evt_handler(p_profile, bsc_disp_event);
}

void ant_bsc_disp_evt_handler(ant_bsc_profile_t * p_profile,
                              ant_evt_t * p_ant_event)
{
    if (p_ant_event->channel == p_profile->channel_number)
    {
        ANT_MESSAGE * p_message = (ANT_MESSAGE *)p_ant_event->msg.evt_buffer;
        switch (p_ant_event->event)
        {
            case EVENT_RX:

                if (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                    || p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID
                    || p_message->ANT_MESSAGE_ucMesgID == MESG_BURST_DATA_ID)
                {
                    disp_message_decode(p_profile, p_message->ANT_MESSAGE_aucPayload);
                }
                break;
            default:
                break;
        }
    }
}
