/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @brief The memory simulator interface.
 * This file is based on implementation originally made by Dynastream Innovations Inc. - August 2012
 * @defgroup ant_fs_client_main ANT-FS client device simulator
 * @{
 * @ingroup nrf_ant_fs_client
 *
 * @brief The ANT-FS client device simulator.
 *
 */

#ifndef MEM_H__
#define MEM_H__

#include <stdint.h>
#include <stdbool.h>
#include "antfs.h"

/**@brief Function for writing data to file system.
 *
 * @param[in] index        The file index.
 * @param[in] offset       The write data offset.
 * @param[in] p_data       The data to be written.
 * @param[in] size         The number of bytes to be written.
 *
 * @retval true Operation success.
 * @retval true Operation failure.
 */
bool mem_file_write(uint16_t index, uint32_t offset, const void * p_data, uint32_t size);

/**@brief Function for reading data from file system.
 *
 * @param[in] index        The file index, 0 for directory.
 * @param[in] offset       The read data offset.
 * @param[out] p_data      The buffer where data is read.
 * @param[in] size         The number of bytes to read.
 */
void mem_file_read(uint16_t index, uint32_t offset, void * p_data, uint32_t size);

/**@brief Function for erasing file from file system.
 *
 * @param[in] index        The file index.
 *
 * @retval true Operation success.
 * @retval true Operation failure.
 */
bool mem_file_erase(uint16_t index);

/**@brief Function for retrieving file information from directory.
 *
 * @param[in] index        The file index, which information to retrieve.
 * @param[out] p_file_info The container where information is read.
 *
 * @retval true Operation success.
 * @retval true Operation failure.
 */
bool mem_file_info_get(uint16_t index, antfs_dir_struct_t * p_file_info);

#endif  // MEM_H__

/**
 *@}
 **/
