/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#ifndef NRF_DRV_VALIDATION__
#define NRF_DRV_VALIDATION__

#if TIMER1_ENABLED
    #if (TIMER1_CONFIG_BIT_WIDTH == TIMER_BITMODE_BITMODE_24Bit) || (TIMER1_CONFIG_BIT_WIDTH == TIMER_BITMODE_BITMODE_32Bit)
        #error "TIMER1 instance does not support such BIT_WIDTH"
    #endif
#endif
#endif //NRF_DRV_VALIDATION__
