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

#include "rtc1.h"

void RTC1_IRQHandler(void)
{
  if ( (NRF_RTC1->EVENTS_COMPARE[0] == 1)
      &&
      ((NRF_RTC1->INTENCLR & (1 << RTC_INTENCLR_COMPARE0_Pos)) != 0 ) )
  {
    NRF_RTC1->INTENCLR = (1 << RTC_INTENCLR_COMPARE0_Pos);
    NRF_RTC1->EVENTS_COMPARE[0] = 0;
    timeout_RTC1 = true;
  }
}

void rtc1_timeout_set_ms(uint32_t ms_value)
{
  NRF_RTC1->INTENCLR = (1 << RTC_INTENCLR_COMPARE0_Pos);
  timeout_RTC1 = false;
  NRF_RTC1->CC[0] = NRF_RTC1->COUNTER + ms_value * 32768 / 1000;
  NRF_RTC1->EVENTS_COMPARE[0] = 0;
  NRF_RTC1->INTENSET = (1 << (RTC_INTENSET_COMPARE0_Pos));// | (1 << (RTC_INTENSET_OVRFLW_Enabled));
}

bool rtc1_timeout(void)
{
  return timeout_RTC1;
}

