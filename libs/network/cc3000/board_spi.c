/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * CC3000 WiFi SPI wrappers
 * ----------------------------------------------------------------------------
 */

#include "hci.h"
#include "spi.h"

#include "jshardware.h"
#include "board_spi.h"

#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5)

#define SPI_HEADER_SIZE         (5)

#define     eSPI_STATE_POWERUP               (0)
#define     eSPI_STATE_INITIALIZED           (1)
#define     eSPI_STATE_IDLE                  (2)
#define     eSPI_STATE_READ_IRQ              (6)
#define     eSPI_STATE_READ_FIRST_PORTION    (7)
#define     eSPI_STATE_READ_EOT              (8)

#define READ                    3
#define WRITE                   1
#define HI(value)               (((value) & 0xFF00) >> 8)
#define LO(value)               ((value) & 0x00FF)

volatile bool cc3000_ints_enabled = false;
volatile bool cc3000_in_interrupt = false;

typedef struct
{
    gcSpiHandleRx  SPIRxHandler;
    unsigned short usTxPacketLength;
    unsigned short usRxPacketLength;
    unsigned long  ulSpiState;
    unsigned char *pTxPacket;
    unsigned char *pRxPacket;

}tSpiInformation;


tSpiInformation sSpiInformation;


// buffer for 5 bytes of SPI HEADER
unsigned char tSpiReadHeader[] = {READ, 0, 0, 0, 0};


void SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
void SpiPauseSpi(void);
void cc3000_spi_resume(void);
void cc3000_continue_read(void);
#define ASSERT_CS()          jshPinSetValue(WLAN_CS_PIN, 0)
#define DEASSERT_CS()        jshPinSetValue(WLAN_CS_PIN, 1)


// The magic number that resides at the end of the TX/RX buffer (1 byte after
// the allocated size) for the purpose of detection of the overrun. The location
// of the memory where the magic number resides shall never be written. In case
// it is written - the overrun occurred and either receive function or send
// function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

char spi_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];


void  cc3000_spi_open(void)
{
  // SPI config
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.pinSCK =  WLAN_CLK_PIN;
  inf.pinMISO = WLAN_MISO_PIN;
  inf.pinMOSI = WLAN_MOSI_PIN;
  inf.baudRate = 100000; // FIXME - just slow for debug
  inf.spiMode = SPIF_SPI_MODE_1;  // Mode 1   CPOL= 0  CPHA= 1
  jshSPISetup(WLAN_SPI, &inf);

  // WLAN CS, EN and WALN IRQ Configuration
  jshSetPinStateIsManual(WLAN_CS_PIN, false);
  jshPinOutput(WLAN_CS_PIN, 1); // de-assert CS
  jshSetPinStateIsManual(WLAN_EN_PIN, false);
  jshPinOutput(WLAN_EN_PIN, 0); // disable WLAN
  jshSetPinStateIsManual(WLAN_IRQ_PIN, true);
  jshPinSetState(WLAN_IRQ_PIN, JSHPINSTATE_GPIO_IN_PULLUP); // flip into read mode with pullup
  jshPinWatch(WLAN_IRQ_PIN, true); // watch IRQ pin

  // wait a little (ensure that WLAN takes effect)
  jshDelayMicroseconds(500*1000); // force a 500ms delay! FIXME
}

void cc3000_spi_close(void)
{
    if (sSpiInformation.pRxPacket)
        sSpiInformation.pRxPacket = 0;
    //  Disable Interrupt
    cc3000_irq_disable();
}

void SpiOpen(gcSpiHandleRx pfRxHandler)
{
	sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;
	sSpiInformation.SPIRxHandler = pfRxHandler;
    sSpiInformation.usTxPacketLength = 0;
    sSpiInformation.pTxPacket = NULL;
    sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
    sSpiInformation.usRxPacketLength = 0;
    spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
    wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

	// Enable interrupt
    tSLInformation.WlanInterruptEnable();
}

long
SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
    // workaround for first transaction
    ASSERT_CS();

    // 50 microsecond delay
     jshDelayMicroseconds(50);

    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    jshDelayMicroseconds(50);

    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    DEASSERT_CS();

    jshDelayMicroseconds(10000);

    return(0);
}

long
cc3000_spi_write(unsigned char *pUserBuffer, unsigned short usLength)
{

    cc3000_irq_disable();

    unsigned char ucPad = 0;

    // Figure out the total length of the packet in order to figure out if there
    // is padding or not
    if(!(usLength & 0x0001))
    {
        ucPad++;
    }

    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

    usLength += (SPI_HEADER_SIZE + ucPad);

    // The magic number that resides at the end of the TX/RX buffer (1 byte after
    // the allocated size) for the purpose of detection of the overrun. If the
    // magic number is overwritten - buffer overrun occurred - and we will stuck
    // here forever!
    if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
    {
        while (1)
            ;
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
    {
        while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED)
            ;
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED)
    {
        // This is time for first TX/RX transactions over SPI: the IRQ is down -
        // so need to send read buffer size command
        SpiFirstWrite(pUserBuffer, usLength);
    }
    else
    {
        // Assert the CS line and wait till SSI IRQ line is active and then
        // initialize write operation
        ASSERT_CS();
        while (cc3000_read_irq_pin()) ; // wait for the IRQ line to go low
        SpiWriteDataSynchronous(pUserBuffer, usLength);
        DEASSERT_CS();
    }

    cc3000_irq_enable();

    return(0);
}

void
SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
  int bSend = 0, bRecv = 0;
  while (bSend<size || bRecv<size) {
    int r = jshSPISend(WLAN_SPI, (bSend<size)?data[bSend]:-1);
    bSend++;
    if (bSend>0 && r>=0) bRecv++;
  }

  jshDelayMicroseconds(10); // because of final clock pulse
}


void
SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
  int bSend = 0, bRecv = 0;
  while (bSend<size || bRecv<size) {
    int r = jshSPISend(WLAN_SPI, (bSend<size)?READ:-1);
    bSend++;
    if (bSend>0 && r>=0) data[bRecv++] = r;
  }
  jshDelayMicroseconds(10); // because of final clock pulse

}

void
SpiReadHeader(void)
{
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

void SpiReadDataCont(void) {
    long data_to_recv;
    unsigned char *evnt_buff, type;

    //determine what type of packet we have
    evnt_buff =  sSpiInformation.pRxPacket;
    data_to_recv = 0;
    STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET,
                                    type);

    switch(type)
    {
    case HCI_TYPE_DATA:
        {
            // We need to read the rest of data..
            STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
                                             HCI_DATA_LENGTH_OFFSET, data_to_recv);
            if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1))
            {
                data_to_recv++;
            }

            if (data_to_recv)
            {
                SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
            }
            break;
        }
    case HCI_TYPE_EVNT:
        {
            // Calculate the rest length of the data
            STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
                                            HCI_EVENT_LENGTH_OFFSET, data_to_recv);
            data_to_recv -= 1;

            // Add padding byte if needed
            if ((HEADERS_SIZE_EVNT + data_to_recv) & 1)
            {

                data_to_recv++;
            }

            if (data_to_recv)
            {
                SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
            }

            sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
            break;
        }
    }
}


void
SpiPauseSpi(void) {
}

void
cc3000_spi_resume(void) {
}

void cc3000_irq_handler_x(void)
{
  if (tSLInformation.usEventOrDataReceived) return; // there's already an interrupt that we haven't handled

  cc3000_in_interrupt = true;
  if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
  {
      //This means IRQ line was low call a callback of HCI Layer to inform
      //on event
      sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
  }
  else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE)
  {
      sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;

      /* IRQ line goes down - we are start reception */
      ASSERT_CS();

      SpiReadHeader();

      sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;

      // The header was read - continue with  the payload read
      SpiReadDataCont();
      // All the data was read - finalize handling by switching to the task
      //  and calling from task Event Handler
      // Trigger Rx processing
      SpiPauseSpi();
      DEASSERT_CS();

      // The magic number that resides at the end of the TX/RX buffer (1 byte after
      // the allocated size) for the purpose of detection of the overrun. If the
      // magic number is overwritten - buffer overrun occurred - and we will stuck
      // here forever!
      if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
      {
          while (1)
              ;
      }

      sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
      sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
  }
  cc3000_in_interrupt = false;
}

void cc3000_irq_handler(void)
{
  if (!cc3000_ints_enabled) return; // no ints enabled
  cc3000_irq_handler_x();
}

long cc3000_read_irq_pin(void)
{
    return jshPinGetValue(WLAN_IRQ_PIN);
}

void cc3000_irq_enable(void) {
  cc3000_check_irq_pin();
  cc3000_ints_enabled = true;
}

void cc3000_irq_disable(void) {
  cc3000_ints_enabled = false;
}

void cc3000_check_irq_pin() {
  if (!cc3000_in_interrupt && !cc3000_read_irq_pin()) {
    bool ints = cc3000_ints_enabled;
    cc3000_ints_enabled = false;
    cc3000_irq_handler_x();
    cc3000_ints_enabled = ints;
  }
}
