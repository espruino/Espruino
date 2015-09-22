/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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
/** @file
* @addtogroup nrf_radio_test_example_main
* @{
*/

#include "radio_test.h"
#include <stdbool.h>
#include "nrf.h"

static uint8_t packet[256];

static uint8_t mode_;
static uint8_t txpower_;
static uint8_t channel_start_;
static uint8_t channel_end_;
static uint8_t channel_;
static bool    sweep_tx_;

/**
 * @brief Function for initializing Timer 0 in 24 bit timer mode with 1 us resolution.
*/
static void timer0_init(uint8_t delayms)
{
    NRF_TIMER0->TASKS_STOP = 1;

    // Create an Event-Task shortcut to clear Timer 1 on COMPARE[0] event.
    NRF_TIMER0->SHORTS     = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
    NRF_TIMER0->MODE       = TIMER_MODE_MODE_Timer;
    NRF_TIMER0->BITMODE    = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);
    NRF_TIMER0->PRESCALER  = 4;  // 1us resolution
    NRF_TIMER0->INTENSET   = (TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos);
  
    // Sample update needs to happen as soon as possible. The earliest possible moment is MAX_SAMPLE_LEVELS
    // ticks before changing the output duty cycle.
    NRF_TIMER0->CC[0]       = (uint32_t)delayms * 1000;
    NRF_TIMER0->TASKS_START = 1;
}


/**
 * @brief Function for generating an 8 bit random number using the internal random generator.
*/
static uint32_t rnd8(void)
{
    NRF_RNG->EVENTS_VALRDY = 0;
    while (NRF_RNG->EVENTS_VALRDY == 0)
    {
        // Do nothing.
    }
    return NRF_RNG->VALUE;
}


/**
 * @brief Function for generating a 32 bit random number using the internal random generator.
*/
static uint32_t rnd32(void)
{
    uint8_t  i;
    uint32_t val = 0;

    for (i=0; i<4; i++)
    {
        val <<= 8;
        val  |= rnd8();
    }
    return val;
}


/**
 * @brief Function for configuring the radio to use a random address and a 254 byte random payload.
 * The S0 and S1 fields are not used.
*/
static void generate_modulated_rf_packet(void)
{
    uint8_t i;

    NRF_RADIO->PREFIX0 = rnd8();
    NRF_RADIO->BASE0   = rnd32();

    // Packet configuration:
    // S1 size = 0 bits, S0 size = 0 bytes, payload length size = 8 bits
    NRF_RADIO->PCNF0  = (0UL << RADIO_PCNF0_S1LEN_Pos) |
                        (0UL << RADIO_PCNF0_S0LEN_Pos) |
                        (8UL << RADIO_PCNF0_LFLEN_Pos);
    // Packet configuration:
    // Bit 25: 1 Whitening enabled
    // Bit 24: 1 Big endian,
    // 4 byte base address length (5 byte full address length), 
    // 0 byte static length, max 255 byte payload .
    NRF_RADIO->PCNF1  = (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos) |
                        (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos) |
                        (4UL << RADIO_PCNF1_BALEN_Pos) |
                        (0UL << RADIO_PCNF1_STATLEN_Pos) |
                        (255UL << RADIO_PCNF1_MAXLEN_Pos);
    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Disabled << RADIO_CRCCNF_LEN_Pos);
    packet[0]         = 254;    // 254 byte payload.
  
    // Fill payload with random data.
    for (i = 0; i < 254; i++)
    {
        packet[i+1] = rnd8();
    }
    NRF_RADIO->PACKETPTR = (uint32_t)packet;
}


static void radio_disable(void)
{
    NRF_RADIO->SHORTS          = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TEST            = 0;
    NRF_RADIO->TASKS_DISABLE   = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0)
    {
        // Do nothing.
    }
    NRF_RADIO->EVENTS_DISABLED = 0;
}


/**
 * @brief Function for stopping Timer 0.
*/
void radio_sweep_end(void)
{
    NRF_TIMER0->TASKS_STOP = 1;
    radio_disable();
}


/**
 * @brief Function for turning on the TX carrier test mode.
*/
void radio_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel)
{
    radio_disable();
    NRF_RADIO->SHORTS     = RADIO_SHORTS_READY_START_Msk;
    NRF_RADIO->TXPOWER    = (txpower << RADIO_TXPOWER_TXPOWER_Pos);    
    NRF_RADIO->MODE       = (mode << RADIO_MODE_MODE_Pos);
    NRF_RADIO->FREQUENCY  = channel;
    NRF_RADIO->TEST       = (RADIO_TEST_CONST_CARRIER_Enabled << RADIO_TEST_CONST_CARRIER_Pos) \
                            | (RADIO_TEST_PLL_LOCK_Enabled << RADIO_TEST_PLL_LOCK_Pos);
    NRF_RADIO->TASKS_TXEN = 1;
}


/**
 * @brief Function for starting modulated TX carrier by repeatedly sending a packet with random address and 
 * random payload.
*/
void radio_modulated_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel)
{
    radio_disable();
    generate_modulated_rf_packet();
    NRF_RADIO->SHORTS     = RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_READY_START_Msk | \
                            RADIO_SHORTS_DISABLED_TXEN_Msk;;
    NRF_RADIO->TXPOWER    = (txpower << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->MODE       = (mode << RADIO_MODE_MODE_Pos);
    NRF_RADIO->FREQUENCY  = channel;
    NRF_RADIO->TASKS_TXEN = 1;
}


/**
 * @brief Function for turning on RX carrier.
*/
void radio_rx_carrier(uint8_t mode, uint8_t channel)
{
    radio_disable();
    NRF_RADIO->SHORTS     = RADIO_SHORTS_READY_START_Msk;
    NRF_RADIO->FREQUENCY  = channel;
    NRF_RADIO->TASKS_RXEN = 1;
}


/**
 * @brief Function for turning on TX carrier sweep. This test uses Timer 0 to restart the TX carrier at different channels.
*/
void radio_tx_sweep_start(uint8_t txpower, uint8_t mode, uint8_t channel_start, uint8_t channel_end, uint8_t delayms)
{
    txpower_       = txpower;
    mode_          = mode;
    channel_start_ = channel_start;
    channel_       = channel_start;
    channel_end_   = channel_end;
    sweep_tx_      = true;
    timer0_init(delayms);
}


/**
 * @brief Function for turning on RX carrier sweep. This test uses Timer 0 to restart the RX carrier at different channels.
*/
void radio_rx_sweep_start(uint8_t mode, uint8_t channel_start, uint8_t channel_end, uint8_t delayms)
{
    mode_          = mode;
    channel_start_ = channel_start;
    channel_       = channel_start;
    channel_end_   = channel_end;
    sweep_tx_      = false;
    timer0_init(delayms);
}


/**
 * @brief Function for handling the Timer 0 interrupt used for TX/RX sweep. The carrier is started with the new channel,
 * and the channel is incremented for the next interrupt.
*/
void TIMER0_IRQHandler(void) 
{
    if (sweep_tx_)
    {
        radio_tx_carrier(txpower_, mode_, channel_);
    }
    else
    {
        radio_rx_carrier(mode_, channel_);
    }
    channel_++;
    if (channel_ > channel_end_)
    {
        channel_ = channel_start_;
    }
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
}
/**
 * @}
 */
