/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.h
  * @author  MCD Application Team
  * @brief   USBX Device CDC ACM interface header file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UX_DEVICE_CDC_ACM_H__
#define __UX_DEVICE_CDC_ACM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define APP_RX_DATA_SIZE   2048
#define APP_TX_DATA_SIZE   2048
#define CDC_RX_DATA_SIZE   64
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_CDC_ACM_Activate(VOID *cdc_acm_instance);
VOID USBD_CDC_ACM_Deactivate(VOID *cdc_acm_instance);
VOID USBD_CDC_ACM_ParameterChange(VOID *cdc_acm_instance);

/* USER CODE BEGIN EFP */
VOID USBD_CDC_ACM_2_Activate(VOID *cdc_acm_instance);
VOID USBD_CDC_ACM_2_Deactivate(VOID *cdc_acm_instance);
VOID USBD_CDC_ACM_2_ParameterChange(VOID *cdc_acm_instance);

VOID CDC_ACM_1_Read_Single_Package_Task(VOID);
VOID CDC_ACM_1_Read_Multiple_Package_Task(VOID);
VOID CDC_ACM_2_Read_Single_Package_Task(VOID);
VOID CDC_ACM_2_Read_Multiple_Package_Task(VOID);
VOID CDC_ACM_1_Write_Task(void);
VOID CDC_ACM_2_Write_Task(VOID);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern uint8_t UserRx1BufferFS[APP_RX_DATA_SIZE];
extern uint8_t UserTx1BufferFS[APP_TX_DATA_SIZE];
extern uint8_t UserRx2BufferFS[APP_RX_DATA_SIZE];
extern uint8_t UserTx2BufferFS[APP_TX_DATA_SIZE];
/* USER CODE END PD */


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_CDC_ACM_H__ */
