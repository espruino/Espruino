/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

#include "mem.h"
#include <stdio.h>
#include <string.h>

#define MEM_DIR_SIZE          9u                     /**< Example directory size. */
#define MEM_DIR_INVALID_INDEX 0xFFFFu                /**< Defined invalid index value. */

// Sample constant directory structure.
typedef struct
{
    antfs_dir_header_t header;                       /**< Directory header. */
    antfs_dir_struct_t directory_file[MEM_DIR_SIZE]; /**< Array of directory entry structures. */
} directory_file_t;

// Sample constant directory.
static const directory_file_t m_directory =
{
    {
        ANTFS_DIR_STRUCT_VERSION,                             // Version 1, length of each subsequent entry = 16 bytes, system time not used.
        sizeof(antfs_dir_struct_t),
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        {9, 0,        4, 0, 0, 0xA0, 0x00000000, 0x000296BC}, // Index 9, data type 0, identifier 4, read and erase, 0 bytes in length.
        {7, 1,        0, 0, 0, 0x60, 0x00000000, 0x00007E71}, // Index 7, data type 1, identifier 0, write and erase, 0 bytes in length.
        {8, 1,        1, 0, 0, 0x60, 0x00000000, 0x00007E73}, // Index 8, data type 1, identifier 1, write and erase, 0 bytes in length.
        {1, 0x80,     1, 0, 0, 0x80, 0x000000DA, 0x00007E63}, // Index 1, data type 128, identifier 1, read and write, 218 bytes in length.
        {2, 0x80,     2, 0, 0, 0xC0, 0x0000014E, 0x00007E65}, // Index 2, data type 128, identifier 2, read and write, 334 bytes in length.
        {3, 0x80,     3, 0, 0, 0xC0, 0x0000037B, 0x00007E67}, // Index 3, data type 128, identifier 3, read and write, 891 bytes in length.
        {4, 0x80,     3, 0, 0, 0xC0, 0x0000037B, 0x00007E6A}, // Index 4, data type 128, identifier 3, read and write, 891 bytes in length.
        {5, 0x80,     3, 0, 0, 0xF0, 0x0001D4C0, 0x00007E6C}, // Index 5, data type 128, identifier 3, read, write, erase, archive, 120000 bytes in length.
        {6, 0x80,  0x0A, 0, 0, 0x80, 0x00000000, 0x00007E6F}  // Index 6, data type 128, identifier 10, read-only, 0 bytes in length.
    }
};


/**@brief Function for getting an array index of a particular directory structure matching the
 *        requested file index.
 *
 * @param[in] index The file index identifier used for lookup.
 *
 * @return Upon success the array index, otherwise MEM_DIR_INVALID_INDEX.
 */
static uint32_t index_lookup(uint32_t index)
{
    if (index)
    {
        uint32_t idx;
        for (idx = 0; idx < MEM_DIR_SIZE; idx++)
        {
            if (m_directory.directory_file[idx].data_file_index == index)
            {
                return idx;
            }
        }

    }

    return MEM_DIR_INVALID_INDEX;
}


bool mem_file_write(uint16_t index, uint32_t offset, const void * p_data, uint32_t size)
{
    if ((index < MEM_DIR_SIZE + 1u) && (index > 0))
    {
        const uint32_t array_index = index_lookup(index);
        if (array_index != MEM_DIR_INVALID_INDEX)
        {
            uint32_t loop_count = size / 8u;
#ifndef TRACE_MEM_WRITE_OFF
            uint8_t * p_trace = (uint8_t *)p_data;
#endif // TRACE_MEM_WRITE_OFF
            while (loop_count)
            {
#ifndef TRACE_MEM_WRITE_OFF // Do not define this if you want to trace out the upload buffer content.
                printf("%#x-%#x-%#x-%#x-%#x-%#x-%#x-%#x\n",
                    p_trace[0], p_trace[1], p_trace[2], p_trace[3],
                    p_trace[4], p_trace[5], p_trace[6], p_trace[7]);
#endif // TRACE_MEM_WRITE_OFF
                --loop_count;
            }

            return true;
        }
        else
        {
            return false;
        }

    }

    return false;
}


void mem_file_read(uint16_t index, uint32_t offset, void * p_data, uint32_t size)
{
    if (index == 0)
    {
        // Directory.

        uint8_t * p_directory = (uint8_t*) &m_directory;
        memcpy((uint8_t*)p_data, p_directory + offset, size);
    }
    else
    {
        // Fake data (no actual files stored in memory for this reference design).

        uint32_t idx;
        uint8_t* p_data_access = (uint8_t*)p_data;
        for (idx = 0; idx < size; idx++)
        {
            p_data_access[idx] = ((uint8_t)(offset + idx));
        }
    }
}


bool mem_file_erase(uint16_t index)
{
    if ((index < MEM_DIR_SIZE + 1u) && (index > 0))
    {
        const uint32_t array_index = index_lookup(index);
        if (array_index != MEM_DIR_INVALID_INDEX)
        {
            // Erase file. This function is implementation specific. There are no actual files in
            // memory for this reference design, so there is nothing to erase.
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}


bool mem_file_info_get(uint16_t index, antfs_dir_struct_t * p_file_info)
{
    if (index < (MEM_DIR_SIZE + 1u))
    {
        if (index == 0)
        {
            // Requested directory.

            // Set can download flag.
            p_file_info->general_flags = 0x80u;

            p_file_info->file_size_in_bytes =
                // Header + directory structures.
                (uint32_t)(MEM_DIR_SIZE + 1u) * sizeof(antfs_dir_struct_t);

            // Directory is index 0
            p_file_info->data_file_index = 0;
        }
        else
        {
            // Requested a file.

            const uint32_t array_index = index_lookup(index);
            memcpy(p_file_info,
                   &m_directory.directory_file[array_index],
                   sizeof(antfs_dir_struct_t));
        }

        return true;
    }

    return false;
}
