#include "ant_parameters.h"
#include "bsp.h"
#include "ant_state_indicator.h"
#include "app_error.h"

/**
 * @addtogroup ant_sdk_state_indicator ANT channel state indicator module.
 * @{
 */
     
static uint8_t m_related_channel;  ///< ANT channel number linked to indication
static uint8_t m_channel_type;     ///< type of linked ANT channel


void ant_state_indicator_init( uint8_t channel, uint8_t channel_type)
{
    m_related_channel   = channel;
    m_channel_type      = channel_type;

    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}


uint32_t ant_state_indicator_channel_opened(void)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (m_channel_type)
    {
        case CHANNEL_TYPE_SLAVE_RX_ONLY:
            err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
            break;
        case CHANNEL_TYPE_MASTER:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            break;
    }

    return err_code;
}

void ant_state_indicator_evt_handle(ant_evt_t * p_ant_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    if (m_related_channel != p_ant_evt->channel)
        return;

    switch (m_channel_type)
    {
        case CHANNEL_TYPE_SLAVE_RX_ONLY:
            switch (p_ant_evt->event)
            {
                case EVENT_RX:
                    err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
                    break;

                case EVENT_RX_FAIL:
                    err_code = bsp_indication_set(BSP_INDICATE_RCV_ERROR);
                    break;

                case EVENT_RX_FAIL_GO_TO_SEARCH:
                    err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
                    break;

                case EVENT_CHANNEL_CLOSED:
                case EVENT_RX_SEARCH_TIMEOUT:
                    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                    break;
            }
            break;

        case CHANNEL_TYPE_MASTER:
            switch (p_ant_evt->event)
            {
                case EVENT_TX:
                    break;
            }
            break;
    }
    APP_ERROR_CHECK(err_code);
}
/**
 *@}
 */
