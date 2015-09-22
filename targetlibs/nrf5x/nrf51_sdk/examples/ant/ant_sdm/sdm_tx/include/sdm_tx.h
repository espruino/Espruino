/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @brief Stride and Distance Monitor TX definitions.
 * @defgroup ant_sdm_tx_module_definitions Definitions
 * @{
 * @ingroup ant_sdm_tx
 *
 * @brief Stride and Distance Monitor TX definitions. 
 */
 
#ifndef SDM_TX_H__
#define SDM_TX_H__

/**@addtogroup sdm_tx_page_2_status_byte_definitions Data page 2 - status byte definitions
 * @{
 */
// SDM Use status position and filter mask 
#define SDM_STATUS_USE_STATE_POS          0x0                                /**< Use state bit position. */ 
#define SDM_STATUS_USE_STATE_MASK         (0x3u << SDM_STATUS_USE_STATE_POS) /**< Use state bit mask. */

// SDM Use state status bits 
#define SDM_STATUS_USE_STATE_INACTIVE     (0x0  << SDM_STATUS_USE_STATE_POS) /**< State inactive. */
#define SDM_STATUS_USE_STATE_ACTIVE       (0x1u << SDM_STATUS_USE_STATE_POS) /**< State active. */
 
// SDM Health status position and filter mask 
#define SDM_STATUS_HEALTH_POS             0x2                                /**< Health state bit position. */
#define SDM_STATUS_HEALTH_MASK            (0x3u << SDM_STATUS_HEALTH_POS)    /**< Health bit mask. */

// SDM Health status bits 
#define SDM_STATUS_HEALTH_OK              (0x0  << SDM_STATUS_HEALTH_POS)    /**< Health status OK. */
#define SDM_STATUS_HEALTH_ERROR           (0x1u << SDM_STATUS_HEALTH_POS)    /**< Health status error. */
#define SDM_STATUS_HEALTH_WARNING         (0x2u << SDM_STATUS_HEALTH_POS)    /**< Health status warning. */
 
// SDM Battery status position and filter mask 
#define SDM_STATUS_BATTERY_POS            0x4u                               /**< Battery state bit position. */
#define SDM_STATUS_BATTERY_MASK           (0x3u << SDM_STATUS_BATTERY_POS)   /**< Battery bit mask. */

// SDM Battery status bits 
#define SDM_STATUS_BATTERY_NEW            (0x0  << SDM_STATUS_BATTERY_POS)   /**< Battery status new. */
#define SDM_STATUS_BATTERY_STATUS_GOOD    (0x1u << SDM_STATUS_BATTERY_POS)   /**< Battery status good. */
#define SDM_STATUS_BATTERY_STATUS_OK      (0x2u << SDM_STATUS_BATTERY_POS)   /**< Battery status ok. */
#define SDM_STATUS_BATTERY_STATUS_LOW     (0x3u << SDM_STATUS_BATTERY_POS)   /**< Battery status low. */
 
// SDM Location status position and filter mask 
#define SDM_STATUS_LOCATION_POS           0x6u                               /**< Location state bit position. */
#define SDM_STATUS_LOCATION_MASK          (0x3u << SDM_STATUS_LOCATION_POS)  /**< Location bit mask. */

// SDM Location status bits 
#define SDM_STATUS_LOCATION_LACES         (0x0  << SDM_STATUS_LOCATION_POS)  /**< Located at laces. */
#define SDM_STATUS_LOCATION_MIDSOLE       (0x1u << SDM_STATUS_LOCATION_POS)  /**< Located at midsole. */
#define SDM_STATUS_LOCATION_OTHER         (0x2u << SDM_STATUS_LOCATION_POS)  /**< Located at other place. */
#define SDM_STATUS_LOCATION_ANKLE         (0x3u << SDM_STATUS_LOCATION_POS)  /**< Located at ankle. */
/** @} */ 

/**@addtogroup sdm_tx_configurable_common_page_data Configurable common page data
 * @{ 
 */
#define SDM_PAGE_80_HW_REVISION           1u                                 /**< Hardware revision. */
#define SDM_PAGE_80_MANUFACTURER_ID       0xABCDu                            /**< Manufacturer ID. */
#define SDM_PAGE_80_MODEL_NUMBER          0x1234u                            /**< Model number. */
#define SDM_PAGE_81_SW_REVISION           1u                                 /**< Software revision. */
#define SDM_PAGE_81_SERIAL_NUMBER         0xFFFFFFFFu                        /**< Serial number. */
/** @} */ 

/**@addtogroup sdm_tx_page_number_definitions Page number definitions
 * @{
 */
// Page number definitions 
#define SDM_PAGE_1                        1u                                 /**< Page number 1. */
#define SDM_PAGE_2                        2u                                 /**< Page number 2. */
#define SDM_PAGE_80                       80u                                /**< Page number 80. */
#define SDM_PAGE_81                       81u                                /**< Page number 81. */
/** @} */ 

/**@addtogroup sdm_tx_data_buffer_indices Data buffer indices
 * @{ 
 */
// Data buffer lookup indices 
#define SDM_PAGE_1_INDEX_STRIDE_COUNT     6u                                 /**< Index for stride count in page 1. */
#define SDM_PAGE_2_INDEX_STATUS           7u                                 /**< Index for status in page 2. */
#define SDM_PAGE_80_INDEX_HW_REVISION     3u                                 /**< Index for hardware revision in page 80. */
#define SDM_PAGE_80_INDEX_MANUFACTURER_ID 4u                                 /**< Index for manufacturer ID in page 80. */
#define SDM_PAGE_80_INDEX_MODEL_NUMBER    6u                                 /**< Index for model number in page 80. */
#define SDM_PAGE_81_INDEX_SW_REVISION     3u                                 /**< Index for software revision in page 81. */
#define SDM_PAGE_81_INDEX_SERIAL_NUMBER   4u                                 /**< Index for serial number in page 81. */

// Index for page number in message buffer 
#define SDM_PAGE_INDEX_PAGE_NUM           0                                  /**< Index for page number in all pages. */
/** @} */ 

/** @} */ 

#endif // SDM_TX_H__

/**
 *@}
 **/
