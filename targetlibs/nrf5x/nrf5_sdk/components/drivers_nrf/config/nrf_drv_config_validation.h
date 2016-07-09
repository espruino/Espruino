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

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((SPI0_ENABLED + SPIS0_ENABLED + TWI0_ENABLED + TWIS0_ENABLED) > 1)
#error "Peripherals overlap. SPI0, SPIS0, TWI0, TWIS0 - only one of these can be enabled."
#endif

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((SPI1_ENABLED + SPIS1_ENABLED + TWI1_ENABLED + TWIS1_ENABLED) > 1)
#error "Peripherals overlap. SPI1, SPIS1, TWI1, TWIS1 - only one of these can be enabled."
#endif

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((SPI2_ENABLED + SPIS2_ENABLED) > 1)
#error "Peripherals overlap. SPI2, SPIS2 - only one of these can be enabled."
#endif

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((COMP_ENABLED + LPCOMP_ENABLED) > 1)
#error "COMP and LPCOMP cannot be enabled together. Peripherals overlap."
#endif

#else //NRF51

#if (TWIS0_ENABLED + TWIS1_ENABLED) > 0
#error "TWIS not present in nRF51."
#endif

#if SPIS0_ENABLED > 0
#error "SPIS0 instance not present in nRF51."
#endif 

#if (SPI2_ENABLED + SPIS2_ENABLED) > 0
#error "SPI2/SPIS2 instance not present in nRF51."
#endif 

#if RTC2_ENABLED
#error "RTC2 not present in NRF51."
#endif

#if (TIMER3_ENABLED + TIMER4_ENABLED) > 0
#error "TIMER3 and TIMER4 not present in nRF51."
#endif

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((SPI0_ENABLED + TWI0_ENABLED) > 1)
#error "Peripherals overlap. SPI0, TWI0 - only one of these can be enabled."
#endif

#if (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((SPI1_ENABLED + SPIS1_ENABLED + TWI1_ENABLED) > 1)
#error "Peripherals overlap. SPI1, SPIS1, TWI1 - only one of these can be enabled."
#endif

#if SAADC_ENABLED > 0
#error "SAADC not present in nRF51."
#endif

#if I2S_ENABLED > 0
#error "I2S not present in nRF51."
#endif
#if COMP_ENABLED > 0
#error "COMP not present in nRF51."
#endif

#endif //NRF51

#endif // NRF_DRV_CONFIG_VALIDATION_H
