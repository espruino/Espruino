/* Copyright (c) 2013, Nordic Semiconductor ASA
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

#include "uart.h"
#include "nRF51.h"
#include "nRF51_bitfields.h"
#include "board_config.h"

static const uint32_t m_baudrates[UART_BAUD_TABLE_MAX_SIZE] = UART_BAUDRATE_DEVISORS_ARRAY;

void uart_init(uart_baudrate_t const baud_rate)
{
  NRF_UART0->PSELRTS = BOARD_UART0_RTS;
  NRF_UART0->PSELTXD = BOARD_UART0_TX;
  NRF_UART0->PSELCTS = BOARD_UART0_CTS;
  NRF_UART0->PSELRXD = BOARD_UART0_RX;

  NRF_UART0->ENABLE           = 0x04;
  NRF_UART0->BAUDRATE         = m_baudrates[baud_rate];
  NRF_UART0->CONFIG           = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos); 
  NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;
  NRF_UART0->EVENTS_RXDRDY    = 0;
}

bool hal_uart_chars_available(void)
{
  return (NRF_UART0->EVENTS_RXDRDY == 1);
}

void sendchar(uint8_t ch)
{
  uart_putchar(ch);
}

void uart_putchar(uint8_t ch)
{
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->TXD = ch;
  while(NRF_UART0->EVENTS_TXDRDY != 1){}  // Wait for TXD data to be sent
  NRF_UART0->EVENTS_TXDRDY = 0;
}

uint8_t uart_getchar(void)
{
  while(NRF_UART0->EVENTS_RXDRDY != 1){}  // Wait for RXD data to be received
  NRF_UART0->EVENTS_RXDRDY = 0;
  return (uint8_t)NRF_UART0->RXD;
}

void uart_write_buf(uint8_t const *buf, uint32_t len)
{
  while(len--)
    uart_putchar(*buf++);
}

