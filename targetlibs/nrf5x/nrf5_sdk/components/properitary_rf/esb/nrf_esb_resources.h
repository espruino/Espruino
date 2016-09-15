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

#ifndef NRF_ESB_RESOURCES_H__
#define NRF_ESB_RESOURCES_H__

#include <stdint.h>

#ifndef ESB_ALTERNATIVE_RESOURCES
	#define ESB_PPI_CHANNELS_USED    0x00000007uL /**< PPI channels utilized by ESB (not available to th spplication). */
	#define ESB_TIMERS_USED          0x00000004uL /**< Timers used by ESB. */
	#define ESB_SWI_USED             0x00000001uL /**< Software interrupts used by ESB */
#else
	#define ESB_PPI_CHANNELS_USED    0x00000700uL /**< PPI channels utilized by ESB (not available to th spplication). */
	#define ESB_TIMERS_USED          0x00000001uL /**< Timers used by ESB. */
	#define ESB_SWI_USED             0x00000002uL /**< Software interrupts used by ESB */
#endif



#endif /* NRF_ESB_RESOURCES_H__ */
