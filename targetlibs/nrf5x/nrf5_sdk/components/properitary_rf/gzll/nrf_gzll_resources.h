/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#ifndef NRF_GZLL_RESOURCES_H__
#define NRF_GZLL_RESOURCES_H__

#include <stdint.h>

#ifndef GAZELL_ALTERNATIVE_RESOURCES
	#define GZLL_PPI_CHANNELS_USED    0x00000007uL /**< PPI channels utilized by Gazell (not available to th spplication). */
	#define GZLL_TIMERS_USED          0x00000004uL /**< Timers used by Gazell. */
	#define GZLL_SWI_USED             0x00000001uL /**< Software interrupts used by Gazell */
#else
	#define GZLL_PPI_CHANNELS_USED    0x00000700uL /**< PPI channels utilized by Gazell (not available to th spplication). */
	#define GZLL_TIMERS_USED          0x00000001uL /**< Timers used by Gazell. */
	#define GZLL_SWI_USED             0x00000002uL /**< Software interrupts used by Gazell */
#endif



#endif /* NRF_GZLL_RESOURCES_H__ */
