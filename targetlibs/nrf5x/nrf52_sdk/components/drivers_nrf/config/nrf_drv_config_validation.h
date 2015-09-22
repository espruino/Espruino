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

#ifndef NRF_DRV_CONFIG_VALIDATION_H
#define NRF_DRV_CONFIG_VALIDATION_H

#ifdef NRF52

#if (TWI0_ENABLED+SPI0_ENABLED+TWIS0_ENABLED)>1
#error "TWI0, SPI0 or TWIS0 cannot be enabled together. Instances overlaps."
#endif

#if (TWI1_ENABLED+SPI1_ENABLED+TWIS1_ENABLED)>1
#error "TWI1, SPI1 or TWIS1 cannot be enabled together. Instances overlaps."
#endif

#else //NRF51

#if (TWIS0_ENABLED + TWIS1_ENABLED) > 0
#error "TWIS not present in NRF51."
#endif

#if SPI2_ENABLED > 0
#error "SPI2 instance not present in NRF51."
#endif 

#if (TIMER3_ENABLED + TIMER4_ENABLED) > 0
#error "TIMER3 and TIMER4 not present in NRF51"
#endif

#if (TWI0_ENABLED+SPI0_ENABLED)>1
#error "TWI0, SPI0 or TWIS0 cannot be enabled together. Peripherals overlaps."
#endif

#if (TWI1_ENABLED+SPI1_ENABLED)>1
#error "TWI1, SPI1 or TWIS1 cannot be enabled together. Peripherals overlaps."
#endif

#if SAADC_ENABLED > 0
#error "SAADC not present in NRF51."
#endif

#endif //NRF51

#endif //NRF_DRV_CONFIG_VALIDATION_H
