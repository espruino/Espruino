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

#include <string.h>
#include "ant_bpwr_page_1.h"
#include "ant_bpwr_page_logger.h"
#include "app_util.h"
#include "nordic_common.h"

/**@brief bicycle power page 1 data layout structure. */
typedef struct
{
    uint8_t calibration_id; ///< Calibration request type
    union
    {
        struct
        {
            uint8_t reserved[6]; ///< Unused, fill by 0xFF.
        } general_calib_request;
        struct
        {
            uint8_t auto_zero_status; ///< Status of automatic zero feature of power sensor.
            uint8_t reserved[5];      ///< Unused, fill by 0xFF.
        } auto_zero_config;
        struct
        {
            uint8_t auto_zero_status; ///< Status of automatic zero feature of power sensor.
            uint8_t reserved[3];      ///< Unused, fill by 0xFF.
            uint8_t data[2];          ///< Calibration Data.
        } general_calib_response;
        struct
        {
            uint8_t enable      : 1;
            uint8_t status      : 1;
            uint8_t reserved0   : 6; ///< Unused, fill by 0x00.
            uint8_t reserved1[5];    ///< Unused, fill by 0xFF.
        } auto_zero_support;
        struct
        {
            uint8_t manufac_spec[6]; ///< Manufacture Specyfic Data.
        } custom_calib;
    } data;
} ant_bpwr_page1_data_layout_t;


static void page1_data_log(ant_bpwr_page1_data_t const * p_page_data)
{
    LOG_PAGE1("Calibration id:                   %u\n\r", p_page_data->calibration_id);

    switch (p_page_data->calibration_id)
    {
        case ANT_BPWR_CALIB_ID_MANUAL:
            // No implementation needed
            break;

        case ANT_BPWR_CALIB_ID_MANUAL_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_FAILED:
            LOG_PAGE1("General calibration data:         %u\n\r", p_page_data->data.general_calib);

        /* fall through */
        case ANT_BPWR_CALIB_ID_AUTO:
        /* fall through */
        case ANT_BPWR_CALIB_ID_AUTO_SUPPORT:

            switch (p_page_data->auto_zero_status)
            {
                case ANT_BPWR_AUTO_ZERO_NOT_SUPPORTED:
                    LOG_PAGE1("Auto zero not supported\n\r");
                    break;

                case ANT_BPWR_AUTO_ZERO_OFF:
                    LOG_PAGE1("Auto zero off\n\r");
                    break;

                case ANT_BPWR_AUTO_ZERO_ON:
                    LOG_PAGE1("Auto zero on\n\r");
                    break;
            }
            break;

        case ANT_BPWR_CALIB_ID_CTF:
            LOG_PAGE1("Not supported\n\r");
            break;

        case ANT_BPWR_CALIB_ID_CUSTOM_REQ:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_REQ_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE_SUCCESS:
            LOG_PAGE1("Manufacture specyfic:            ");

            for (uint8_t i = 0; i < sizeof (p_page_data->data.custom_calib); i++)
            {
                LOG_PAGE1(" %u", p_page_data->data.custom_calib[i]);
            }
            LOG_PAGE1("\n\r");
            break;

        default: // shouldn't occur
            LOG_PAGE1("Unsupported calibration ID\n\r");
            break;
    }
}


void ant_bpwr_page_1_encode(uint8_t                     * p_page_buffer,
                            ant_bpwr_page1_data_t const * p_page_data)
{
    ant_bpwr_page1_data_layout_t * p_outcoming_data = (ant_bpwr_page1_data_layout_t *)p_page_buffer;

    page1_data_log(p_page_data);

    p_outcoming_data->calibration_id = p_page_data->calibration_id;

    switch (p_page_data->calibration_id)
    {
        case ANT_BPWR_CALIB_ID_MANUAL:
            memset(p_outcoming_data->data.general_calib_request.reserved, 0xFF,
                   sizeof (p_outcoming_data->data.general_calib_request.reserved));
            break;

        case ANT_BPWR_CALIB_ID_AUTO:
            memset(p_outcoming_data->data.auto_zero_config.reserved, 0xFF,
                   sizeof (p_outcoming_data->data.auto_zero_config.reserved));
            p_outcoming_data->data.auto_zero_config.auto_zero_status =
                p_page_data->auto_zero_status;
            break;

        case ANT_BPWR_CALIB_ID_MANUAL_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_FAILED:
            memset(p_outcoming_data->data.general_calib_response.reserved, 0xFF,
                   sizeof (p_outcoming_data->data.general_calib_response.reserved));
            p_outcoming_data->data.general_calib_response.auto_zero_status =
                p_page_data->auto_zero_status;
            UNUSED_PARAMETER(uint16_encode(p_page_data->data.general_calib,
                                           p_outcoming_data->data.general_calib_response.data));
            break;

        case ANT_BPWR_CALIB_ID_CTF:
            LOG_PAGE1("Not supported\n\r");
            break;

        case ANT_BPWR_CALIB_ID_AUTO_SUPPORT:
            memset(p_outcoming_data->data.auto_zero_support.reserved1, 0xFF,
                   sizeof (p_outcoming_data->data.auto_zero_support.reserved1));
            p_outcoming_data->data.auto_zero_support.reserved0 = 0x00;
            p_outcoming_data->data.auto_zero_support.enable    =
                (p_page_data->auto_zero_status == ANT_BPWR_AUTO_ZERO_NOT_SUPPORTED) ? false : true;
            p_outcoming_data->data.auto_zero_support.status =
                (p_page_data->auto_zero_status == ANT_BPWR_AUTO_ZERO_ON) ? true : false;
            break;

        case ANT_BPWR_CALIB_ID_CUSTOM_REQ:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_REQ_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE_SUCCESS:
            memcpy(p_outcoming_data->data.custom_calib.manufac_spec,
                   (void *)p_page_data->data.custom_calib,
                   sizeof (p_page_data->data.custom_calib));
            break;

        default: // shouldn't occur
            break;
    }
}


void ant_bpwr_page_1_decode(uint8_t const         * p_page_buffer,
                            ant_bpwr_page1_data_t * p_page_data)
{
    ant_bpwr_page1_data_layout_t const * p_incoming_data =
        (ant_bpwr_page1_data_layout_t *)p_page_buffer;

    p_page_data->calibration_id = (ant_bpwr_calib_id_t)p_incoming_data->calibration_id;

    switch (p_incoming_data->calibration_id)
    {
        case ANT_BPWR_CALIB_ID_MANUAL:
            // No implementation needed
            break;

        case ANT_BPWR_CALIB_ID_AUTO:
            /* fall through */
            p_page_data->auto_zero_status =
                (ant_bpwr_auto_zero_status_t)p_incoming_data->data.auto_zero_config.auto_zero_status;
            break;

        case ANT_BPWR_CALIB_ID_MANUAL_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_FAILED:
            p_page_data->auto_zero_status =
                (ant_bpwr_auto_zero_status_t)p_incoming_data->data.general_calib_response.
                auto_zero_status;
            p_page_data->data.general_calib = uint16_decode(
                p_incoming_data->data.general_calib_response.data);
            break;

        case ANT_BPWR_CALIB_ID_CTF:
            LOG_PAGE1("Not supported\n\r");
            break;

        case ANT_BPWR_CALIB_ID_AUTO_SUPPORT:

            if (p_incoming_data->data.auto_zero_support.enable == false)
            {
                p_page_data->auto_zero_status = ANT_BPWR_AUTO_ZERO_NOT_SUPPORTED;
            }
            else if (p_incoming_data->data.auto_zero_support.status)
            {
                p_page_data->auto_zero_status = ANT_BPWR_AUTO_ZERO_ON;
            }
            else
            {
                p_page_data->auto_zero_status = ANT_BPWR_AUTO_ZERO_OFF;
            }
            break;

        case ANT_BPWR_CALIB_ID_CUSTOM_REQ:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_REQ_SUCCESS:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE:
        /* fall through */
        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE_SUCCESS:
            memcpy((void *)p_page_data->data.custom_calib,
                   p_incoming_data->data.custom_calib.manufac_spec,
                   sizeof (p_page_data->data.custom_calib));
            break;

        default: // shouldn't occur
            break;
    }

    page1_data_log(p_page_data);
}


