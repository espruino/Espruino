/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#ifndef NRF_SOC_H__
#define NRF_SOC_H__

#include <stdint.h>
#include "nrf.h"

/**@brief The interrupt priorities available to the application while the softdevice is active. */
typedef uint8_t nrf_app_irq_priority_t;

/**@brief Enable External Interrupt.
 * @note Corresponds to NVIC_EnableIRQ in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in] IRQn See the NVIC_EnableIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt was enabled.
 */
uint32_t sd_nvic_EnableIRQ(IRQn_Type IRQn);

/**@brief  Disable External Interrupt.
 * @note Corresponds to NVIC_DisableIRQ in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in] IRQn See the NVIC_DisableIRQ documentation in CMSIS
 *
 * @retval ::NRF_SUCCESS The interrupt was disabled.
 */
uint32_t sd_nvic_DisableIRQ(IRQn_Type IRQn);

/**@brief  Get Pending Interrupt.
 * @note Corresponds to NVIC_GetPendingIRQ in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in]   IRQn          See the NVIC_GetPendingIRQ documentation in CMSIS.
 * @param[out]  p_pending_irq Return value from NVIC_GetPendingIRQ.
 *
 * @retval ::NRF_SUCCESS The interrupt is available for the application.
 */
uint32_t sd_nvic_GetPendingIRQ(IRQn_Type IRQn, uint32_t * p_pending_irq);

/**@brief  Set Pending Interrupt.
 * @note Corresponds to NVIC_SetPendingIRQ in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in] IRQn See the NVIC_SetPendingIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt is set pending.
 */
uint32_t sd_nvic_SetPendingIRQ(IRQn_Type IRQn);

/**@brief  Clear Pending Interrupt.
 * @note Corresponds to NVIC_ClearPendingIRQ in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in] IRQn See the NVIC_ClearPendingIRQ documentation in CMSIS.
 *
 * @retval ::NRF_SUCCESS The interrupt pending flag is cleared.
 */
uint32_t sd_nvic_ClearPendingIRQ(IRQn_Type IRQn);

/**@brief Set Interrupt Priority.
 * @note Corresponds to NVIC_SetPriority in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 * @pre{priority is valid and not reserved by the stack}
 *
 * @param[in] IRQn      See the NVIC_SetPriority documentation in CMSIS.
 * @param[in] priority  A valid IRQ priority for use by the application.
 *
 * @retval ::NRF_SUCCESS The interrupt and priority level is available for the application.
 */
uint32_t sd_nvic_SetPriority(IRQn_Type IRQn, nrf_app_irq_priority_t priority);

/**@brief Get Interrupt Priority.
 * @note Corresponds to NVIC_GetPriority in CMSIS.
 *
 * @pre{IRQn is valid and not reserved by the stack}
 *
 * @param[in]  IRQn         See the NVIC_GetPriority documentation in CMSIS.
 * @param[out] p_priority   Return value from NVIC_GetPriority.
 *
 * @retval ::NRF_SUCCESS The interrupt priority is returned in p_priority.
 */
uint32_t sd_nvic_GetPriority(IRQn_Type IRQn, nrf_app_irq_priority_t * p_priority);

/**@brief System Reset.
 * @note Corresponds to NVIC_SystemReset in CMSIS.
 *
 * @retval ::NRF_ERROR_SOC_NVIC_SHOULD_NOT_RETURN
 */
uint32_t sd_nvic_SystemReset(void);

/**@brief Enters critical region.
 *
 * @post Application interrupts will be disabled.
 * @sa sd_nvic_critical_region_exit
 *
 * @param[out]  p_is_nested_critical_region  1: If in a nested critical region.
 *                                           0: Otherwise.
 *
 * @retval ::NRF_SUCCESS
 */
uint32_t sd_nvic_critical_region_enter(uint8_t * p_is_nested_critical_region);

/**@brief Exit critical region.
 *
 * @pre Application has entered a critical region using ::sd_nvic_critical_region_enter.
 * @post If not in a nested critical region, the application interrupts will restored to the state before ::sd_nvic_critical_region_enter was called. 
 *
 * @param[in] is_nested_critical_region If this is set to 1, the critical region won't be exited. @sa sd_nvic_critical_region_enter.
 *
 * @retval ::NRF_SUCCESS
 */
uint32_t sd_nvic_critical_region_exit(uint8_t is_nested_critical_region);

/**@brief Waits for an application event.
 *
 * An application event is either an application interrupt or a pended interrupt when the
 * interrupt is disabled. When the interrupt is enabled it will be taken immediately since
 * this function will wait in thread mode, then the execution will return in the application's
 * main thread. When an interrupt is disabled and gets pended it will return to the application's
 * thread main. The application must ensure that the pended flag is cleared using
 * ::sd_nvic_ClearPendingIRQ in order to sleep using this function. This is only necessary for
 * disabled interrupts, as the interrupt handler will clear the pending flag automatically for
 * enabled interrupts.
 *
 * In order to wake up from disabled interrupts, the SEVONPEND flag has to be set in the Cortex-M0
 * System Control Register (SCR). @sa CMSIS_SCB
 *
 * @note If an application interrupt has happened since the last time sd_app_evt_wait was
 *       called this function will return immediately and not go to sleep. This is to avoid race
 *       conditions that can occur when a flag is updated in the interrupt handler and processed
 *       in the main loop.
 *
 * @post An application interrupt has happened or a interrupt pending flag is set.
 *
 * @retval ::NRF_SUCCESS
 */
uint32_t sd_app_evt_wait(void);

#endif /* NRF_SOC_H__ */
