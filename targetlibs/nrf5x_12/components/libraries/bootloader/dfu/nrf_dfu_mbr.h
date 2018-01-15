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
 * @defgroup sdk_nrf_dfu_mbr MBR functions
 * @{
 * @ingroup  sdk_nrf_dfu
 */

#ifndef NRF_DFU_MBR_H__
#define NRF_DFU_MBR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Function for copying the bootloader using an MBR command.
 *
 * @param[in] p_src         Source address of the bootloader data to copy.
 * @param[in] len           Length of the data to copy in bytes.
 *
 * @return  This function will return only if the command request could not be run.
 *          See @ref sd_mbr_command_copy_bl_t for possible return values.
 */
uint32_t nrf_dfu_mbr_copy_bl(uint32_t * p_src, uint32_t len);


/** @brief Function for copying the SoftDevice using an MBR command.
 *
 * @param[in]   p_dst       Target of the SoftDevice copy.
 * @param[in]   p_src       Source address of the SoftDevice image to copy.
 * @param[in]   len         Length of the data to copy in bytes.
 *
 * @retval NRF_SUCCESS indicates that the contents of the memory blocks where copied correctly.
 * @retval NRF_ERROR_INVALID_LENGTH Invalid len
 * @retval NRF_ERROR_NO_MEM if UICR.NRFFW[1] is not set (i.e. is 0xFFFFFFFF).
 * @retval NRF_ERROR_INVALID_PARAM if an invalid command is given.
 * @retval NRF_ERROR_INTERNAL indicates that the contents of the memory blocks where not verified correctly after copying.
 */
uint32_t nrf_dfu_mbr_copy_sd(uint32_t * p_dst, uint32_t * p_src, uint32_t len);


/** @brief Function for initializing the SoftDevice using an MBR command.
 *
 * @retval  NRF_SUCCESS     If the SoftDevice was copied successfully.
 *                          Any other return value indicates that the SoftDevice
 *                          could not be copied.
 */
uint32_t nrf_dfu_mbr_init_sd(void);


/** @brief Function for comparing source and target using an MBR command.
 *
 * @param[in]   p_ptr1      First pointer to data to compare.
 * @param[in]   p_ptr2      Second pointer to data to compare.
 * @param[in]   len         Length of the data to compare in bytes.
 *
 * @retval NRF_SUCCESS    If the content of both memory blocks is equal.
 * @retval NRF_ERROR_NULL If the content of the memory blocks differs.
 */
uint32_t nrf_dfu_mbr_compare(uint32_t * p_ptr1, uint32_t * p_ptr2, uint32_t len);


/** @brief Function for setting the address of the vector table using an MBR command.
 *
 * @param[in]   address Address of the new vector table.
 *
 * @retval  NRF_SUCCESS     If the address of the new vector table was set. Any other
 *                          return value indicates that the address could not be set.
 */
uint32_t nrf_dfu_mbr_vector_table_set(uint32_t address);


#ifdef __cplusplus
}
#endif

#endif // NRF_DFU_MBR_H__

/** @} */
