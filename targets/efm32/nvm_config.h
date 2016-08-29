/***************************************************************************//**
 * @file nvm_config.h
 * @brief NVM config definition
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#ifndef __NVMCONFIG_H
#define __NVMCONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "ecode.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup NVM
 * @{
 ******************************************************************************/

/*******************************************************************************
 ****************************   CONFIGURATION   ********************************
 ******************************************************************************/

/** Without this define the wear pages are no longer supported */
#define NVM_FEATURE_WEAR_PAGES_ENABLED               true

/** Include and activate the static wear leveling functionality */
#define NVM_FEATURE_STATIC_WEAR_ENABLED              true

/** The threshold used to decide when to do static wear leveling */
#define NVM_STATIC_WEAR_THRESHOLD                    100

/** Validate data against checksums on every read operation */
#define NVM_FEATURE_READ_VALIDATION_ENABLED          true

/** Validate data against checksums after every write operation */
#define NVM_FEATURE_WRITE_VALIDATION_ENABLED         true

/** Include the NVM_WearLevelGet function. */
#define NVM_FEATURE_WEARLEVELGET_ENABLED             true

/** Check if data has been updated before writing update to the NVM */
#define NVM_FEATURE_WRITE_NECESSARY_CHECK_ENABLED    true

/** define maximum number of flash pages that can be used as NVM */
#define NVM_MAX_NUMBER_OF_PAGES                      32

/** Configure extra pages to allocate for data security and wear leveling.
    Minimum 1, but the more you add the better lifetime your system will have. */
#define NVM_PAGES_SCRATCH                            1

/** Set the NVM driver page size to the size of the flash. */
#define NVM_PAGE_SIZE                                FLASH_PAGE_SIZE

/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

/** Enum describing the type of logical page we have; normal or wear. */
typedef enum
{
  nvmPageTypeNormal = 0, /**< Normal page, always rewrite. */
  nvmPageTypeWear   = 1  /**< Wear page. Can be used several times before rewrite. */
} NVM_Page_Type_t;

/** Describes the properties of an object in a page. */
typedef struct
{
  uint8_t  * location; /**< A pointer to the location of the object in RAM. */
  uint16_t size;       /**< The size of the object in bytes. */
  uint8_t  objectId;   /**< An object ID used to reference the object. Must be unique in the page. */
} NVM_Object_Descriptor_t;

/** A collection of object descriptors that make up a page. */
typedef NVM_Object_Descriptor_t   NVM_Page_t[];


/** Describes the properties of a page. */
typedef struct
{
  uint8_t           pageId;    /**< A page ID used when referring to the page. Must be unique. */
  NVM_Page_t const *page;      /**< A pointer to the list of all the objects in the page. */
  uint8_t           pageType;  /**< The type of page, normal or wear. */
} NVM_Page_Descriptor_t;

/** The list of pages registered for use. */
typedef NVM_Page_Descriptor_t   NVM_Page_Table_t[];

/** Configuration structure. */
typedef struct
{ NVM_Page_Table_t const *nvmPages;  /**< Pointer to table defining NVM pages. */
  uint8_t          const pages;      /**< Total number of physical pages. */
  uint8_t          const userPages;  /**< Number of defined (used) pages. */
  uint8_t          const *nvmArea;   /**< Pointer to nvm area in flash. */
} NVM_Config_t;


/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

NVM_Config_t const *NVM_ConfigGet(void);

/** @} (end addtogroup NVM) */
/** @} (end addtogroup EM_Drivers) */

#ifdef __cplusplus
}
#endif

#endif /* __NVMCONFIG_H */
