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

#ifndef SDK_VALIDATION_H
#define SDK_VALIDATION_H

#include "nrf_peripherals.h"
#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Validate peripheral availibility

#if ((defined(SAADC_ENABLED)) && (SAADC_ENABLED > 0) && (!defined(SAADC_PRESENT)))
#error "SAADC not present in selected MCU."
#endif

#if ((defined(ADC_ENABLED)) && (ADC_ENABLED > 0) && (!defined(ADC_PRESENT)))
#error "SAADC not present in selected MCU."
#endif

#if ((defined(I2S_ENABLED)) && (I2S_ENABLED > 0) && (!defined(I2S_PRESENT)))
#error "I2S not present in selected MCU."
#endif

#if ((defined(COMP_ENABLED)) && (COMP_ENABLED > 0) && (!defined(COMP_PRESENT)))
#error "COMP not present in selected MCU."
#endif

#if ((defined(LPCOMP_ENABLED)) && (LPCOMP_ENABLED > 0) && (!defined(LPCOMP_PRESENT)))
#error "LPCOMP not present in selected MCU."
#endif

#if ((defined(SPIS0_ENABLED)) && (SPIS0_ENABLED > 0) && (!defined(SPIS_PRESENT)))
#error "SPIS0 instance not present in selected MCU."
#endif

#if ((defined(EGU_ENABLED)) && (EGU_ENABLED > 0) && (!defined(EGU_PRESENT)))
#error "EGU instance not present in selected MCU."
#endif

#if ((defined(NFC_HAL_ENABLED)) && (NFC_HAL_ENABLED > 0) && (!defined(NFCT_PRESENT)))
#error "NFC TAG not present in selected MCU."
#endif

// Validate count of instances

#if ((defined(RTC2_ENABLED)) && (RTC2_ENABLED > 0) && (RTC_COUNT < 2))
#error "RTC2 not present in selected MCU."
#endif

#if ((defined(TWIS0_ENABLED) || defined(TWIS1_ENABLED)) &&\
    ((TWIS0_ENABLED + TWIS1_ENABLED) > 0) &&\
    (!defined(TWIS_PRESENT)))
#error "TWIS not present in selected MCU."
#endif

#if ((defined(SPIS2_ENABLED)) && (SPIS2_ENABLED > 0) && (SPIS_COUNT < 2))
#error "SPI2/SPIS2 instance not present in selected MCU."
#endif

#if ((defined(TIMER3_ENABLED) || defined(TIMER4_ENABLED)) &&\
    ((TIMER3_ENABLED + TIMER4_ENABLED ) > 0) &&\
    (TIMER_COUNT < 5))
#error "TIMER3 and TIMER4 not present in selected MCU."
#endif

// Validate peripheral sharing feature
#if ((defined(PERIPHERAL_RESOURCE_SHARING_ENABLED)) && (!PERIPHERAL_RESOURCE_SHARING_ENABLED))

#if ((defined(TWIM0_ENABLED) && defined(TWIS0_ENABLED)) &&\
    ((TWIM0_ENABLED + TWIS0_ENABLED) > 1))
#error "Peripherals overlap. TWIM0, TWIS0 - only one of these can be enabled."
#endif

#if ((defined(TWIM0_ENABLED) && defined(SPIM0_ENABLED)) &&\
    ((TWIM0_ENABLED + SPIM0_ENABLED) > 1))
#error "Peripherals overlap. TWIM0, SPIM0 - only one of these can be enabled."
#endif

#if ((defined(TWIM0_ENABLED) && defined(SPIS0_ENABLED)) &&\
    ((TWIM0_ENABLED + SPIS0_ENABLED) > 1))
#error "Peripherals overlap. TWIM0, SPIS0 - only one of these can be enabled."
#endif

#if ((defined(TWIM0_ENABLED) && defined(SPI0_ENABLED)) &&\
    ((TWIM0_ENABLED + SPI0_ENABLED) > 1))
#error "Peripherals overlap. TWIM0, SPI0 - only one of these can be enabled."
#endif

#if ((defined(TWIM0_ENABLED) && defined(TWI0_ENABLED)) &&\
    ((TWIM0_ENABLED + TWI0_ENABLED) > 1))
#error "Peripherals overlap. TWIM0, TWI0 - only one of these can be enabled."
#endif

#if ((defined(TWIS0_ENABLED) && defined(SPIM0_ENABLED)) &&\
    ((TWIS0_ENABLED + SPIM0_ENABLED) > 1))
#error "Peripherals overlap. TWIS0, SPIM0 - only one of these can be enabled."
#endif

#if ((defined(TWIS0_ENABLED) && defined(SPIS0_ENABLED)) &&\
    ((TWIS0_ENABLED + SPIS0_ENABLED) > 1))
#error "Peripherals overlap. TWIS0, SPIS0 - only one of these can be enabled."
#endif

#if ((defined(TWIS0_ENABLED) && defined(SPI0_ENABLED)) &&\
    ((TWIS0_ENABLED + SPI0_ENABLED) > 1))
#error "Peripherals overlap. TWIS0, SPI0 - only one of these can be enabled."
#endif

#if ((defined(TWIS0_ENABLED) && defined(TWI0_ENABLED)) &&\
    ((TWIS0_ENABLED + TWI0_ENABLED) > 1))
#error "Peripherals overlap. TWIS0, TWI0 - only one of these can be enabled."
#endif

#if ((defined(SPIM0_ENABLED) && defined(SPIS0_ENABLED)) &&\
    ((SPIM0_ENABLED + SPIS0_ENABLED) > 1))
#error "Peripherals overlap. SPIM0, SPIS0 - only one of these can be enabled."
#endif

#if ((defined(SPIM0_ENABLED) && defined(SPI0_ENABLED)) &&\
    ((SPIM0_ENABLED + SPI0_ENABLED) > 1))
#error "Peripherals overlap. SPIM0, SPI0 - only one of these can be enabled."
#endif

#if ((defined(SPIM0_ENABLED) && defined(TWI0_ENABLED)) &&\
    ((SPIM0_ENABLED + TWI0_ENABLED) > 1))
#error "Peripherals overlap. SPIM0, TWI0 - only one of these can be enabled."
#endif

#if ((defined(SPIS0_ENABLED) && defined(SPI0_ENABLED)) &&\
    ((SPIS0_ENABLED + SPI0_ENABLED) > 1))
#error "Peripherals overlap. SPIS0, SPI0 - only one of these can be enabled."
#endif

#if ((defined(SPIS0_ENABLED) && defined(TWI0_ENABLED)) &&\
    ((SPIS0_ENABLED + TWI0_ENABLED) > 1))
#error "Peripherals overlap. SPIS0, TWI0 - only one of these can be enabled."
#endif

#if ((defined(SPI0_ENABLED) && defined(TWI0_ENABLED)) &&\
    ((SPI0_ENABLED + TWI0_ENABLED) > 1))
#error "Peripherals overlap. SPI0, TWI0 - only one of these can be enabled."
#endif

#if ((defined(TWIM1_ENABLED) && defined(TWIS1_ENABLED)) &&\
    ((TWIM1_ENABLED + TWIS1_ENABLED) > 1))
#error "Peripherals overlap. TWIM1, TWIS1 - only one of these can be enabled."
#endif

#if ((defined(TWIM1_ENABLED) && defined(SPIM1_ENABLED)) &&\
    ((TWIM1_ENABLED + SPIM1_ENABLED) > 1))
#error "Peripherals overlap. TWIM1, SPIM1 - only one of these can be enabled."
#endif

#if ((defined(TWIM1_ENABLED) && defined(SPIS1_ENABLED)) &&\
    ((TWIM1_ENABLED + SPIS1_ENABLED) > 1))
#error "Peripherals overlap. TWIM1, SPIS1 - only one of these can be enabled."
#endif

#if ((defined(TWIM1_ENABLED) && defined(SPI1_ENABLED)) &&\
    ((TWIM1_ENABLED + SPI1_ENABLED) > 1))
#error "Peripherals overlap. TWIM1, SPI1 - only one of these can be enabled."
#endif

#if ((defined(TWIM1_ENABLED) && defined(TWI1_ENABLED)) &&\
    ((TWIM1_ENABLED + TWI1_ENABLED) > 1))
#error "Peripherals overlap. TWIM1, TWI1 - only one of these can be enabled."
#endif

#if ((defined(TWIS1_ENABLED) && defined(SPIM1_ENABLED)) &&\
    ((TWIS1_ENABLED + SPIM1_ENABLED) > 1))
#error "Peripherals overlap. TWIS1, SPIM1 - only one of these can be enabled."
#endif

#if ((defined(TWIS1_ENABLED) && defined(SPIS1_ENABLED)) &&\
    ((TWIS1_ENABLED + SPIS1_ENABLED) > 1))
#error "Peripherals overlap. TWIS1, SPIS1 - only one of these can be enabled."
#endif

#if ((defined(TWIS1_ENABLED) && defined(SPI1_ENABLED)) &&\
    ((TWIS1_ENABLED + SPI1_ENABLED) > 1))
#error "Peripherals overlap. TWIS1, SPI1 - only one of these can be enabled."
#endif

#if ((defined(TWIS1_ENABLED) && defined(TWI1_ENABLED)) &&\
    ((TWIS1_ENABLED + TWI1_ENABLED) > 1))
#error "Peripherals overlap. TWIS1, TWI1 - only one of these can be enabled."
#endif

#if ((defined(SPIM1_ENABLED) && defined(SPIS1_ENABLED)) &&\
    ((SPIM1_ENABLED + SPIS1_ENABLED) > 1))
#error "Peripherals overlap. SPIM1, SPIS1 - only one of these can be enabled."
#endif

#if ((defined(SPIM1_ENABLED) && defined(SPI1_ENABLED)) &&\
    ((SPIM1_ENABLED + SPI1_ENABLED) > 1))
#error "Peripherals overlap. SPIM1, SPI1 - only one of these can be enabled."
#endif

#if ((defined(SPIM1_ENABLED) && defined(TWI1_ENABLED)) &&\
    ((SPIM1_ENABLED + TWI1_ENABLED) > 1))
#error "Peripherals overlap. SPIM1, TWI1 - only one of these can be enabled."
#endif

#if ((defined(SPIS1_ENABLED) && defined(SPI1_ENABLED)) &&\
    ((SPIS1_ENABLED + SPI1_ENABLED) > 1))
#error "Peripherals overlap. SPIS1, SPI1 - only one of these can be enabled."
#endif

#if ((defined(SPIS1_ENABLED) && defined(TWI1_ENABLED)) &&\
    ((SPIS1_ENABLED + TWI1_ENABLED) > 1))
#error "Peripherals overlap. SPIS1, TWI1 - only one of these can be enabled."
#endif

#if ((defined(SPI1_ENABLED) && defined(TWI1_ENABLED)) &&\
    ((SPI1_ENABLED + TWI1_ENABLED) > 1))
#error "Peripherals overlap. SPI1, TWI1 - only one of these can be enabled."
#endif

#if ((defined(SPI2_ENABLED) && defined(SPIS2_ENABLED)) &&\
    ((SPI2_ENABLED + SPIS2_ENABLED) > 1))
#error "Peripherals overlap. SPI2, SPIS2 - only one of these can be enabled."
#endif

#if ((defined(SPIM2_ENABLED) && defined(SPIS2_ENABLED)) &&\
    ((SPI2_ENABLED + SPIS2_ENABLED) > 1))
#error "Peripherals overlap. SPIM2, SPIS2 - only one of these can be enabled."
#endif

#if ((defined(SPIM2_ENABLED) && defined(SPI2_ENABLED)) &&\
    ((SPI2_ENABLED + SPIS2_ENABLED) > 1))
#error "Peripherals overlap. SPIM2, SPI2 - only one of these can be enabled."
#endif

#endif

#ifdef NFCT_PRESENT

#if ((defined(NFC_HAL_ENABLED) && defined(CLOCK_ENABLED)) &&\
    ((NFC_HAL_ENABLED) && (!CLOCK_ENABLED)))
#error "NFC_HAL requires CLOCK to work. NFC_HAL can not be enabled without CLOCK."
#endif

#if ((defined(NFC_HAL_ENABLED) && defined(TIMER4_ENABLED)) &&\
    ((NFC_HAL_ENABLED + TIMER4_ENABLED) > 1))
#error "TIMER4 is used by NFC_HAL. NFC_HAL, TIMER4 - only one of these can be enabled."
#endif

#endif
// Complex driver validation
#ifdef LPCOMP_PRESENT

#if ((defined(COMP_ENABLED) && defined(LPCOMP_ENABLED)) &&\
    (!PERIPHERAL_RESOURCE_SHARING_ENABLED) && \
    ((COMP_ENABLED + LPCOMP_ENABLED) > 1))
#error "Peripherals overlap. SPIM2, SPI2 - only one of these can be enabled."
#endif

#endif


#ifdef __cplusplus
}
#endif

#endif // SDK_VALIDATION_H
