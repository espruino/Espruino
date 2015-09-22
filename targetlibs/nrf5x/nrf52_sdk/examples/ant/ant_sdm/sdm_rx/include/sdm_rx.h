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
 * @brief Stride and Distance Monitor RX definitions.
 * @defgroup ant_sdm_rx_module_definitions Definitions
 * @{
 * @ingroup ant_sdm_rx
 *
 * @brief Stride and Distance Monitor RX definitions.
 */

#ifndef SDM_RX_H__
#define SDM_RX_H__

/** 
 * @addtogroup ant_sdm_rx_page_2_status_byte_definitions Data page 2 - Status byte definitions
 * @{
 */
// SDM Use status position and filter mask. 
#define SDM_STATUS_USE_STATE_POS          0x00                               /**< Use state bit position. */
#define SDM_STATUS_USE_STATE_MASK         (0x3u << SDM_STATUS_USE_STATE_POS) /**< Use state bit mask. */

// SDM Use state status bits. 
#define SDM_STATUS_USE_STATE_INACTIVE     (0x00 << SDM_STATUS_USE_STATE_POS) /**< State inactive. */
#define SDM_STATUS_USE_STATE_ACTIVE       (0x1u << SDM_STATUS_USE_STATE_POS) /**< State active. */
 
// SDM Health status position and filter mask. 
#define SDM_STATUS_HEALTH_POS             0x2u                               /**< Health state bit position. */
#define SDM_STATUS_HEALTH_MASK            (0x3u << SDM_STATUS_HEALTH_POS)    /**< Health bit mask. */

// SDM Health status bits. 
#define SDM_STATUS_HEALTH_OK              (0x00 << SDM_STATUS_HEALTH_POS)    /**< Health status OK. */
#define SDM_STATUS_HEALTH_ERROR           (0x1u << SDM_STATUS_HEALTH_POS)    /**< Health status error. */
#define SDM_STATUS_HEALTH_WARNING         (0x2u << SDM_STATUS_HEALTH_POS)    /**< Health status warning. */
 
// SDM Battery status position and filter mask. 
#define SDM_STATUS_BATTERY_POS            0x4u                               /**< Battery state bit position. */
#define SDM_STATUS_BATTERY_MASK           (0x3u << SDM_STATUS_BATTERY_POS)   /**< Battery bit mask. */

// SDM Battery status bits. 
#define SDM_STATUS_BATTERY_NEW            (0x00 << SDM_STATUS_BATTERY_POS)   /**< Battery status new. */
#define SDM_STATUS_BATTERY_STATUS_GOOD    (0x1u << SDM_STATUS_BATTERY_POS)   /**< Battery status good. */
#define SDM_STATUS_BATTERY_STATUS_OK      (0x2u << SDM_STATUS_BATTERY_POS)   /**< Battery status ok. */
#define SDM_STATUS_BATTERY_STATUS_LOW     (0x3u << SDM_STATUS_BATTERY_POS)   /**< Battery status low. */
 
// SDM Location status position and filter mask. 
#define SDM_STATUS_LOCATION_POS           0x6u                               /**< Location state bit position. */
#define SDM_STATUS_LOCATION_MASK          (0x3u << SDM_STATUS_LOCATION_POS)  /**< Location bit mask. */

// SDM Location status bits. 
#define SDM_STATUS_LOCATION_LACES         (0x00 << SDM_STATUS_LOCATION_POS)  /**< Located at laces. */
#define SDM_STATUS_LOCATION_MIDSOLE       (0x1u << SDM_STATUS_LOCATION_POS)  /**< Located at midsole. */
#define SDM_STATUS_LOCATION_OTHER         (0x2u << SDM_STATUS_LOCATION_POS)  /**< Located at other place. */
#define SDM_STATUS_LOCATION_ANKLE         (0x3u << SDM_STATUS_LOCATION_POS)  /**< located at ankle. */
/** @} */ 

/** 
 * @addtogroup sdm_rx_page_number_definitions Page number definitions
 * @{
 */
// Page number definitions. 
#define SDM_PAGE_1                        1u                     /**< Page number 1. */
#define SDM_PAGE_2                        2u                     /**< Page number 2. */
#define SDM_PAGE_80                       80u                    /**< Page number 80. */
#define SDM_PAGE_81                       81u                    /**< Page number 81. */
#define SDM_PAGE_EVENT_RX_FAIL            255u                   /**< Custom page number used for tracking RX failures. */
/** @} */ 

/** 
 * @addtogroup sdm_rx_data_buffer_indices Data buffer indices
 * @{ 
 */
// Page lookup indices. 
#define SDM_PAGE_1_INDEX_STRIDE_COUNT     6u                     /**< Index for stride count in page 1. */
#define SDM_PAGE_2_INDEX_CADENCE          3u                     /**< Index for cadence in page 1. */
#define SDM_PAGE_2_INDEX_STATUS           7u                     /**< Index for status in page 2. */
#define SDM_PAGE_80_INDEX_HW_REVISION     3u                     /**< Index for hardware revision in page 80. */
#define SDM_PAGE_80_INDEX_MANUFACTURER_ID 4u                     /**< Index for manufacturer ID in page 80. */
#define SDM_PAGE_80_INDEX_MODEL_NUMBER    6u                     /**< Index for model number in page 80. */
#define SDM_PAGE_81_INDEX_SW_REVISION     3u                     /**< Index for software revision in page 81. */
#define SDM_PAGE_81_INDEX_SERIAL_NUMBER   4u                     /**< Index for serial number in page 81. */

#define SDM_PAGE_INDEX_PAGE_NUM           0u                     /**< Index for page number in all pages. */

#define ANT_EVENT_BUFFER_INDEX_PAGE_NUM   0x03u                  /**< Index for page number byte in ANT protocol message. */
/** @} */ 

/** @} */ 

#endif // SDM_RX_H__

/**
 *@}
 **/
