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

#include "ble_dtm_hw.h"
#include "ble_dtm.h"
#include <stdbool.h>
#include <string.h>
#include "nrf.h"


void dtm_turn_off_test()
{
}


void dtm_constant_carrier()
{
NRF_RADIO->MODECNF0 = (RADIO_MODECNF0_RU_Default << RADIO_MODECNF0_RU_Pos) |
                      (RADIO_MODECNF0_DTX_Center << RADIO_MODECNF0_DTX_Pos);
}


uint32_t dtm_radio_validate(int32_t m_tx_power, uint8_t m_radio_mode)
{
    // Initializing code below is quite generic - for BLE, the values are fixed, and expressions
    // are constant. Non-constant values are essentially set in radio_prepare().
    if (!(m_tx_power == RADIO_TXPOWER_TXPOWER_0dBm     ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Pos4dBm  ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg30dBm ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg20dBm ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg16dBm ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg12dBm ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg8dBm  ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg4dBm  ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Pos3dBm  ||
          m_tx_power == RADIO_TXPOWER_TXPOWER_Neg40dBm
          ) ||
        (m_radio_mode > RADIO_MODE_MODE_Ble_1Mbit) // Values 0 - 2: Proprietary mode, 3 (last valid): BLE
        )
    {
        return DTM_ERROR_ILLEGAL_CONFIGURATION;
    }

    return DTM_SUCCESS;
}


bool dtm_hw_set_timer(NRF_TIMER_Type ** mp_timer, IRQn_Type * m_timer_irq, uint32_t new_timer)
{
    if (new_timer == 0)
    {
        *mp_timer    = NRF_TIMER0;
        *m_timer_irq = TIMER0_IRQn;
    }
    else if (new_timer == 1)
    {
        *mp_timer    = NRF_TIMER1;
        *m_timer_irq = TIMER1_IRQn;
    }
    else if (new_timer == 2)
    {
        *mp_timer    = NRF_TIMER2;
        *m_timer_irq = TIMER2_IRQn;
    }
    else if (new_timer == 3)
    {
        *mp_timer    = NRF_TIMER3;
        *m_timer_irq = TIMER3_IRQn;
    }
    else if (new_timer == 4)
    {
        *mp_timer    = NRF_TIMER4;
        *m_timer_irq = TIMER4_IRQn;
    }
    else
    {
        // Parameter error: Only TIMER 0, 1, 2, 3 and 4 provided by nRF52
        return false;
    }
    // New timer has been selected:
    return true;
}
