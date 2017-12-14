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

/**@file
 *
 * @defgroup sdk_bootloader_info Information
 * @{
 * @ingroup sdk_bootloader
 */

#ifndef NRF_BOOTLOADER_INFO_H__
#define NRF_BOOTLOADER_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdm.h"
#endif

/** @brief External definitions of symbols for the start of the application image.
 */
#if (__LINT__ == 1)
    // No implementation
#elif defined ( __CC_ARM )
    extern uint32_t* Image$$ER_IROM1$$Base   __attribute__((used));
#elif defined ( __GNUC__ )
    extern uint32_t * __isr_vector;
#elif defined ( __ICCARM__ )
    extern void * __vector_table;
#else
    #error Not a valid compiler/linker for application image symbols.
#endif


/** @brief Macro for getting the start address of the application image.
 *
 * This macro is valid only when absolute placement is used for the application
 * image. The macro is not a compile time symbol. It cannot be used as a
 * constant expression, for example, inside a static assert or linker script
 * at-placement.
 */
#if (__LINT__ == 1)
    #define BOOTLOADER_START_ADDR        (0x3AC00)
#elif BOOTLOADER_START_ADDR
    // Bootloader start address is defined at project level
#elif defined (__CC_ARM)
    #define BOOTLOADER_START_ADDR        (uint32_t)&Image$$ER_IROM1$$Base
#elif defined (__GNUC__)
    #define BOOTLOADER_START_ADDR        (uint32_t)&__isr_vector
#elif defined (__ICCARM__)
    #define BOOTLOADER_START_ADDR        (uint32_t)&__vector_table
#else
    #error Not a valid compiler/linker for BOOTLOADER_START_ADDR.
#endif


/**
 * @brief Bootloader start address in UICR.
 *
 * Register location in UICR where the bootloader start address is stored.
 *
 * @note If the value at the given location is 0xFFFFFFFF, the bootloader address is not set.
 */
#define NRF_UICR_BOOTLOADER_START_ADDRESS       (NRF_UICR_BASE + 0x14)


#ifndef MAIN_APPLICATION_START_ADDR


#ifdef SOFTDEVICE_PRESENT

/** @brief  Main application start address (if the project uses a SoftDevice).
 *
 * @note   The start address is equal to the end address of the SoftDevice.
 */
#define MAIN_APPLICATION_START_ADDR             (SD_SIZE_GET(MBR_SIZE))

#else

/** @brief  Main application start address if the project does not use a SoftDevice.
 *
 * @note   The MBR is required for the @ref sdk_bootloader to function.
 */
#define MAIN_APPLICATION_START_ADDR             (MBR_SIZE)

#endif

#endif // #ifndef MAIN_APPLICATION_START_ADDR


#ifdef __cplusplus
}
#endif

#endif // #ifndef NRF_BOOTLOADER_INFO_H__
/** @} */
