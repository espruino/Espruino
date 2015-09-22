/* Copyright (c) 2014, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef RTC1_H__
#define RTC1_H__

/** @file
    
    @brief @{
        Timer RTC1 functions.
    @}
*/
#include <stdbool.h>
#include "nrf51.h"
#include "nrf51_bitfields.h"

static volatile bool timeout_RTC1;
static volatile uint32_t timer_count;

static void rtc1_init(void)
{
    NVIC_DisableIRQ(RTC1_IRQn);

    NRF_RTC1->TASKS_STOP                = 1;
    NRF_RTC1->TASKS_CLEAR               = 1;
    NRF_RTC1->EVENTS_COMPARE[0] = 0;
    NRF_RTC1->EVENTS_COMPARE[1] = 0;
    NRF_RTC1->EVENTS_COMPARE[2] = 0;
    NRF_RTC1->EVENTS_COMPARE[3] = 0;
    NRF_RTC1->PRESCALER                   = 0; // Wrap on 512 seconds
    NRF_RTC1->INTENCLR                    = (RTC_INTENCLR_COMPARE3_Clear << RTC_INTENCLR_COMPARE3_Pos) |
                                                                (RTC_INTENCLR_COMPARE2_Clear << RTC_INTENCLR_COMPARE2_Pos) |
                                                                (RTC_INTENCLR_COMPARE1_Clear << RTC_INTENCLR_COMPARE1_Pos) |
                                                                (RTC_INTENCLR_COMPARE0_Clear << RTC_INTENCLR_COMPARE0_Pos) |
                                                                (RTC_INTENCLR_OVRFLW_Clear     << RTC_INTENCLR_OVRFLW_Pos)     |
                                                                (RTC_INTENCLR_TICK_Clear         << RTC_INTENCLR_TICK_Pos) ;
    NRF_RTC1->EVTENCLR                    = (RTC_EVTENCLR_COMPARE3_Clear << RTC_EVTENCLR_COMPARE3_Pos) |
                                                                (RTC_EVTENCLR_COMPARE2_Clear << RTC_EVTENCLR_COMPARE2_Pos) |
                                                                (RTC_EVTENCLR_COMPARE1_Clear << RTC_EVTENCLR_COMPARE1_Pos) |
                                                                (RTC_EVTENCLR_COMPARE0_Clear << RTC_EVTENCLR_COMPARE0_Pos) |
                                                                (RTC_EVTENCLR_OVRFLW_Clear     << RTC_EVTENCLR_OVRFLW_Pos)     |
                                                                (RTC_EVTENCLR_TICK_Clear         << RTC_EVTENCLR_TICK_Pos) ;
    NRF_RTC1->EVTENSET                    = (1 << (RTC_EVTEN_COMPARE0_Pos));
    NRF_RTC1->EVTENSET                    = (1 << (RTC_EVTEN_COMPARE3_Pos));

    NRF_CLOCK->TASKS_LFCLKSTOP            = 1;
    NRF_CLOCK->EVENTS_LFCLKSTARTED        = 0;
    NRF_CLOCK->LFCLKSRC                   = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->TASKS_LFCLKSTART           = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
    }

    timeout_RTC1 = false;
    NVIC_SetPriority(RTC1_IRQn, 1);  // APP HIGH
    NVIC_ClearPendingIRQ(RTC1_IRQn);
    NVIC_EnableIRQ(RTC1_IRQn);       // Enable Interrupt for the RTC in the core.
    NRF_RTC1->TASKS_START = 1;
}

void RTC1_IRQHandler(void);
void rtc1_timeout_set_ms(uint32_t ms_value);
bool rtc1_timeout(void);
#endif /* RTC1_H__ */
