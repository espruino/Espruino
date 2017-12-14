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

#include "nrf_bootloader_info.h"


/** @brief  This variable ensures that the linker script will write the bootloader start address
 *          to the UICR register. This value will be written in the HEX file and thus written to
 *          UICR when the bootloader is flashed into the chip.
 */
#if defined (__CC_ARM )
    #pragma push
    #pragma diag_suppress 1296
    uint32_t  m_uicr_bootloader_start_address __attribute__((at(NRF_UICR_BOOTLOADER_START_ADDRESS)))
                                                    = BOOTLOADER_START_ADDR;
    #pragma pop
#elif defined ( __GNUC__ )
    volatile uint32_t m_uicr_bootloader_start_address  __attribute__ ((section(".uicrBootStartAddress")))
                                            = BOOTLOADER_START_ADDR;
#elif defined ( __ICCARM__ )
    __root    const uint32_t m_uicr_bootloader_start_address @ NRF_UICR_BOOTLOADER_START_ADDRESS
                                            = BOOTLOADER_START_ADDR;
#endif
