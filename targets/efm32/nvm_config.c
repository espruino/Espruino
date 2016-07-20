/***************************************************************************//**
 * @file nvm_config.c
 * @brief NVM config implementation
 * @version 4.1.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stddef.h>
#include "nvm.h"
#include "nvm_config.h"

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

/*******************************************************************************
 ***********************   DATA SPECIFICATION START   **************************
 ******************************************************************************/

/**< Example data objects */
uint32_t colorTable[]           = { 0x000000, 0xff0000, 0x00ff00, 0x0000ff, 0xffff00,
                                    0x00ffff, 0xff00ff, 0xc0c0c0, 0xffffff };
uint8_t coefficientTable[]      = { 11, 12, 13, 14, 15 };
uint8_t primeNumberTable[]      = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41,
                                    43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
                                    101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
                                    151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
                                    199, 211, 223, 227, 229, 233, 239, 241, 251 };
uint8_t bonusTable[]            = { 39, 38, 37, 36 };
uint8_t privateKeyTable[]       = { 49, 48, 47, 46 };
uint8_t transformTable[]        = { 59, 58, 57, 56 };
int32_t safetyTable[]           = { -71, 72, -73, 74, -75, -76, -77, -78, 79, -70 };
uint8_t bigEmptyTable[450];
uint32_t singleVariable         = 68;


/* Example object IDs. These IDs should have names that relate to the data
 * objects defined in nvm_config.c. */
typedef enum
{
  COLOR_TABLE_ID,
  COEFFICIENT_TABLE_ID,
  PRIME_NUMBER_TABLE_ID,
  BONUS_TABLE_ID,
  PRIVATE_KEY_TABLE_ID,
  TRANSFORM_TABLE_ID,
  SINGLE_VARIABLE_ID,
  SAFETY_TABLE_ID,
  BIG_EMPTY_TABLE_ID
} NVM_Object_Ids;

/* Example page IDs.
 * These IDs should have names that relate to the pages defined in nvm_config.c. */
typedef enum
{
  MY_PAGE_1,
  MY_PAGE_2,
  MY_PAGE_3,
  MY_PAGE_4
} NVM_Page_Ids;

/* Page definition 1.
 * Combine objects with their ID, and put them in a page. */
NVM_Page_t const myPage1 =
{
/*{ Pointer to object,            Size of object,           Object ID}, */
  { (uint8_t *) colorTable,       sizeof(colorTable),       COLOR_TABLE_ID },
  { (uint8_t *) bonusTable,       sizeof(bonusTable),       BONUS_TABLE_ID },
  { (uint8_t *) &singleVariable,  sizeof(singleVariable),   SINGLE_VARIABLE_ID },
  { (uint8_t *) bigEmptyTable,    sizeof(bigEmptyTable),    BIG_EMPTY_TABLE_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. This is required. */
};

/* Page definition 2.
 * Combine objects with their ID, and put them in a page.
 * This page contains only one object, since it is going to be
 * used as a wear page. */
NVM_Page_t const myPage2 =
{
/*{ Pointer to object,            Size of object,           Object ID}, */
  { (uint8_t *) safetyTable,      sizeof(safetyTable),      SAFETY_TABLE_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. This is required. */
};

/* Page definition 3.
 * Combine objects with their ID, and put them in a page. */
NVM_Page_t const myPage3 =
{
/*{ Pointer to object,            Size of object,           Object ID}, */
  { (uint8_t *) coefficientTable, sizeof(coefficientTable), COEFFICIENT_TABLE_ID },
  { (uint8_t *) privateKeyTable,  sizeof(privateKeyTable),  PRIVATE_KEY_TABLE_ID },
  { (uint8_t *) &singleVariable,  sizeof(singleVariable),   SINGLE_VARIABLE_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. This is required. */
};

/* Page definition 4.
 * Combine objects with their ID, and put them in a page. */
NVM_Page_t const myPage4 =
{
/*{ Pointer to object,            Size of object,           Object ID}, */
  { (uint8_t *) primeNumberTable, sizeof(primeNumberTable), PRIME_NUMBER_TABLE_ID },
  { (uint8_t *) transformTable,   sizeof(transformTable),   TRANSFORM_TABLE_ID },
  { (uint8_t *) &singleVariable,  sizeof(singleVariable),   SINGLE_VARIABLE_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. This is required. */
};

/* Register all pages into the page table.
 * Assosiate each page to the page ID, and define the type of page. */
NVM_Page_Table_t const nvmPages =
{
/*{ Page ID,   Page pointer, Page type}, */
  { MY_PAGE_1, &myPage1, nvmPageTypeNormal },
  { MY_PAGE_2, &myPage2, nvmPageTypeWear },
  { MY_PAGE_3, &myPage3, nvmPageTypeNormal },
  { MY_PAGE_4, &myPage4, nvmPageTypeNormal }
};

/*******************************************************************************
 ************************   DATA SPECIFICATION END   ***************************
 ******************************************************************************/

/** The code below should not be changed.
 *
 *  Set the storage location in memory. This area should be reserved
 *  using the linker and needs to be aligned with the physical page
 *  grouping of the device.
 *
 *  For the internal flash in the Gecko and Tiny Gecko MCUs, the flash pages are
 *  512 bytes long. This means that the start location must be a multiple of
 *  512 bytes, and that an area equal to 512 bytes * the number of pages and
 *  scratch page must be reserved here.
 *
 *  This can be allocated in the IAR linker file using:
 *
 *    place at address mem:0x1f000 { readonly section NVM_PAGES };
 *
 *  and specifying the pages in the C code using:
 *
 *    const uint8_t nvmData[NVM_PAGE_SIZE * NUMBER_OF_PAGES] @ "NVM_PAGES";
 *
 */
#define NUMBER_OF_USER_PAGES  (sizeof(nvmPages) / sizeof(NVM_Page_Descriptor_t))
#define NUMBER_OF_PAGES       (NVM_PAGES_SCRATCH + NUMBER_OF_USER_PAGES)

/// @endcond

#ifdef __ICCARM__
#pragma data_alignment = NVM_PAGE_SIZE
static const uint8_t nvmData[NVM_PAGE_SIZE * NUMBER_OF_PAGES] @ ".text";   /**< Set storage size and location */
#else
static const uint8_t nvmData[NVM_PAGE_SIZE * NUMBER_OF_PAGES] \
  __attribute__ ((__aligned__(NVM_PAGE_SIZE))) = { 0xFF };                 /**< Set storage size and location */
#endif


static NVM_Config_t const nvmConfig =
{
  &nvmPages,            /**< Page table */
  NUMBER_OF_PAGES,      /**< Total number of pages */
  NUMBER_OF_USER_PAGES, /**< Wear leveling pages */
  nvmData               /**< NVM data */
};                      /**< Top-level configuration data */

/***************************************************************************//**
 * @brief
 *   Return a pointer to the config data.
 *
 * @return
 *   A pointer to the configuration
 ******************************************************************************/
NVM_Config_t const *NVM_ConfigGet(void)
{
  return( &nvmConfig );
}

