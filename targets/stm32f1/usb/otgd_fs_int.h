/**
  ******************************************************************************
  * @file    otgd_fs_int.h
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Endpoint interrupt's service routines prototypes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_INT_H
#define __USB_INT_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef STM32F10X_CL

/* Interrupt Handlers functions */
uint32_t OTGD_FS_Handle_Sof_ISR(void);
uint32_t OTGD_FS_Handle_RxStatusQueueLevel_ISR(void);
uint32_t OTGD_FS_Handle_GInNakEff_ISR(void);
uint32_t OTGD_FS_Handle_GOutNakEff_ISR(void);
uint32_t OTGD_FS_Handle_EarlySuspend_ISR(void);
uint32_t OTGD_FS_Handle_USBSuspend_ISR(void);
uint32_t OTGD_FS_Handle_UsbReset_ISR(void);
uint32_t OTGD_FS_Handle_EnumDone_ISR(void);
uint32_t OTGD_FS_Handle_IsoOutDrop_ISR(void);
uint32_t OTGD_FS_Handle_EOPF_ISR(void);
uint32_t OTGD_FS_Handle_EPMismatch_ISR(void);
uint32_t OTGD_FS_Handle_InEP_ISR(void);
uint32_t OTGD_FS_Handle_OutEP_ISR(void);
uint32_t OTGD_FS_Handle_IncomplIsoIn_ISR(void);
uint32_t OTGD_FS_Handle_IncomplIsoOut_ISR(void);
uint32_t OTGD_FS_Handle_Wakeup_ISR(void);

#endif /* STM32F10X_CL */

/* External variables --------------------------------------------------------*/

#endif /* __USB_INT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
