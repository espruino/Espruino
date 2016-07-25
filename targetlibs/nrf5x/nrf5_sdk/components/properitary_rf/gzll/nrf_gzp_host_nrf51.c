/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic 
 * Semiconductor ASA.Terms and conditions of usage are described in detail 
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *              
 * $LastChangedRevision: 133 $
 */


#include "nrf_gzp.h"
#include "nrf_nvmc.h"

/** 
 * @file
 * @brief Implementation of Gazell Pairing Library (gzp), nRF51 specific Host functions.
 * @defgroup gzp_source_host_nrf51 Gazell Pairing Host nRF51 specific implementation
 * @{
 * @ingroup gzp_04_source
 */


void gzp_host_chip_id_read(uint8_t *dst, uint8_t n)
{
  uint8_t i;
  uint8_t random_number;
  
  if( *((uint8_t*)(GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 1)) == 0xff)
  {
    nrf_nvmc_write_byte((GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 1) , 0x00);
  
    for(i = 0; i < n; i++) 
    {
      gzp_random_numbers_generate(&random_number, 1);
      nrf_nvmc_write_byte((GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 2 + i) , random_number);
    }
  }
  
  for(i = 0; i < n; i++) 
  {
    *(dst++) = *((uint8_t*)(GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 2 + i));
  }
}

/** @} */
