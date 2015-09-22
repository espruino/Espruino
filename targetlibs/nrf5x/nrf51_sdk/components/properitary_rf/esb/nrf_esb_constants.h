/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 32956 $
 */


/**
 * @file
 * @brief Enhanced ShockBurst constants and default values.
 *
 * NOTE! Changing values here has no effect. They are only provided as a reference.
 */

#ifndef NRF_ESB_CONSTANTS_H__
#define NRF_ESB_CONSTANTS_H__

/**
 * @addtogroup esb_02_api
  * @{
*/


/*****************************************************************************/
/** @name Hardware resources used by Enhanced ShockBurst */
/*****************************************************************************/
#define NRF_ESB_HIGH_IRQ_PRIORITY 0        ///< Interrupt priority the ESB timer and the radio
#define NRF_ESB_LOW_IRQ_PRIORITY 1         ///< Interrupt priority for ESB callback functions.

#ifndef USE_SD_HW_RESOURCES
#define NRF_ESB_SWI_IRQn SWI0_IRQn              ///< Software interrupt # used for callback functions.
#define NRF_ESB_SWI_IRQ_HANDLER SWI0_IRQHandler ///< Software interrupt handler used for callback functions.

#define NRF_ESB_TIMER NRF_TIMER2                               ///< Timer to be used as flywheel timer.
#define NRF_ESB_TIMER_PERPOWER_Msk POWER_PERPOWER_TIMER2_Msk   ///< PERPOWER mask for the timer.
#define NRF_ESB_TIMER_IRQn TIMER2_IRQn                         ///< Interrupt # for the timer.
#define NRF_ESB_TIMER_IRQ_HANDLER TIMER2_IRQHandler            ///< Interrupt handler for the timer.           

// In addition, ESB uses the radio peripheral and radio interrupts.
               
/*
 * PPI configuration 
 */
#define NRF_ESB_PPI_EEP0 (NRF_PPI -> CH0_EEP)   ///< ESB PPI event endpoint 0.
#define NRF_ESB_PPI_TEP0 (NRF_PPI -> CH0_TEP)   ///< ESB PPI task endpoint 0.
#define NRF_ESB_PPI_EEP1 (NRF_PPI -> CH1_EEP)   ///< ESB PPI event endpoint 1.
#define NRF_ESB_PPI_TEP1 (NRF_PPI -> CH1_TEP)   ///< ESB PPI task endpoint 1.
#define NRF_ESB_PPI_EEP2 (NRF_PPI -> CH2_EEP)   ///< ESB PPI event endpoint 2.
#define NRF_ESB_PPI_TEP2 (NRF_PPI -> CH2_TEP)   ///< ESB PPI task endpoint 2.

#define NRF_ESB_PPI_CHEN_MSK_0_AND_1 (0x03)     ///< Channel enable/disable mask for PPI endpoint 0 and 1.
#define NRF_ESB_PPI_CHEN_MSK_2 (0x04)           ///< Channel enable/disable mask for PPI endpoint 2.
#else
#define NRF_ESB_SWI_IRQn SWI1_IRQn              ///< Software interrupt # used for callback functions.
#define NRF_ESB_SWI_IRQ_HANDLER SWI1_IRQHandler ///< Software interrupt handler used for callback functions.

#define NRF_ESB_TIMER NRF_TIMER0                               ///< Timer to be used as flywheel timer.
#define NRF_ESB_TIMER_PERPOWER_Msk POWER_PERPOWER_TIMER0_Msk   ///< PERPOWER mask for the timer.
#define NRF_ESB_TIMER_IRQn TIMER0_IRQn                         ///< Interrupt # for the timer.
#define NRF_ESB_TIMER_IRQ_HANDLER TIMER0_IRQHandler            ///< Interrupt handler for the timer.           

// In addition, ESB uses the radio peripheral and radio interrupts.
               
/*
 * PPI configuration 
 */
#define NRF_ESB_PPI_EEP0 (NRF_PPI -> CH8_EEP)   ///< ESB PPI event endpoint 0.
#define NRF_ESB_PPI_TEP0 (NRF_PPI -> CH8_TEP)   ///< ESB PPI task endpoint 0.
#define NRF_ESB_PPI_EEP1 (NRF_PPI -> CH9_EEP)   ///< ESB PPI event endpoint 1.
#define NRF_ESB_PPI_TEP1 (NRF_PPI -> CH9_TEP)   ///< ESB PPI task endpoint 1.
#define NRF_ESB_PPI_EEP2 (NRF_PPI -> CH10_EEP)   ///< ESB PPI event endpoint 2.
#define NRF_ESB_PPI_TEP2 (NRF_PPI -> CH10_TEP)   ///< ESB PPI task endpoint 2.

#define NRF_ESB_PPI_CHEN_MSK_0_AND_1 (0x300)     ///< Channel enable/disable mask for PPI endpoint 0 and 1.
#define NRF_ESB_PPI_CHEN_MSK_2 (0x400)           ///< Channel enable/disable mask for PPI endpoint 2.

#endif

#define NRF_ESB_CONST_PIPE_COUNT 8              ///< Number of TX pipes (at least one for each Device-Host pairs).
#define NRF_ESB_CONST_FIFO_LENGTH 3             ///< Maximum number of packets allowed in a TX or RX FIFO.
#define NRF_ESB_CONST_MAX_TOTAL_PACKETS 6       ///< Maximum number of packets available for reservation at any one time.
#define NRF_ESB_CONST_MAX_PAYLOAD_LENGTH 32     ///< Maximum allowed payload length in bytes. 
#define NRF_ESB_CONST_CALLBACK_QUEUE_LENGTH 10  ///< Maximum number of notifications allowed in the callback queue.
/** @} */


/*****************************************************************************/
/** @name Constant pipe and FIFO configuration */
/*****************************************************************************/
#define NRF_ESB_CONST_PIPE_COUNT 8              ///< Number of transmission pipes (at least one for each Device-Host pairs).
#define NRF_ESB_CONST_QUEUE_LENGTH 3            ///< Maximum number of packets allowed in a TX or RX queue.
#define NRF_ESB_CONST_MAX_TOTAL_PACKETS 6       ///< Maximum number of packets available for reservation at any one time.
#define NRF_ESB_CONST_MAX_PAYLOAD_LENGTH 32     ///< Maximum allowed payload length in bytes. 
#define NRF_ESB_CONST_CALLBACK_QUEUE_LENGTH 10  ///< Maximum number of notifications allowed in the callback queue.

/** @} */

/*****************************************************************************/
/** @name Default parameters */
/*****************************************************************************/

/*
Corresponds to Legacy nRFgo SDK ESB addresses:
Address pipe 0  {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}
Address pipe 1  {0xC2, 0xC2, 0xC2, 0xC2, 0xC2}
Address pipe 2  {0xC3, 0xC2, 0xC2, 0xC2, 0xC2}
Address pipe 3  {0xC4, 0xC2, 0xC2, 0xC2, 0xC2}
*/

#define NRF_ESB_DEFAULT_BASE_ADDRESS_0 0xE7E7E7E7                  ///< Default base address 0.
#define NRF_ESB_DEFAULT_BASE_ADDRESS_1 0xC2C2C2C2                  ///< Default base address 1.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_0 0xE7                         ///< Default prefix address pipe 0.    
#define NRF_ESB_DEFAULT_PREFIX_BYTE_1 0xC2                         ///< Default prefix address pipe 1.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_2 0xC3                         ///< Default prefix address pipe 2.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_3 0xC4                         ///< Default prefix address pipe 3.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_4 0xC5                         ///< Default prefix address pipe 4.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_5 0xC6                         ///< Default prefix address pipe 5.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_6 0xC7                         ///< Default prefix address pipe 6.
#define NRF_ESB_DEFAULT_PREFIX_BYTE_7 0xC8                         ///< Default prefix address pipe 7.
#define NRF_ESB_DEFAULT_BASE_ADDRESS_LENGTH NRF_ESB_BASE_ADDRESS_LENGTH_4B  ///< Default on-air base address length.
#define NRF_ESB_DEFAULT_CRC_LENGTH NRF_ESB_CRC_LENGTH_1_BYTE       ///< Default CRC length.
#define NRF_ESB_DEFAULT_ENABLED_PRX_PIPES 0xFF                     ///< Default enabled RX pipes.
#define NRF_ESB_DEFAULT_MAX_NUMBER_OF_RETRANSMITS 15               ///< Default number of retransmits.
#define NRF_ESB_DEFAULT_OUTPUT_POWER NRF_ESB_OUTPUT_POWER_0_DBM    ///< Default TX output power.
#define NRF_ESB_DEFAULT_DATARATE NRF_ESB_DATARATE_2_MBPS           ///< Default datarate.
#define NRF_ESB_DEFAULT_RETRANSMIT_DELAY 600                       ///< Default retransmit delay.   
#define NRF_ESB_DEFAULT_CHANNEL  (10)
#define NRF_ESB_DEFAULT_XOSC_CTL NRF_ESB_XOSC_CTL_AUTO             ///< Default setting for controlling the XOSC

/** @} */

/** @} */

#endif
