/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "stm32c0xx_nucleo.h"
#include "ux_device_descriptors.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* the minimum baudrate */
#define MIN_BAUDRATE     9600
#define APP_CDC_ACM_READ_STATE_TX_START  (UX_STATE_APP_STEP + 0)
#define APP_CDC_ACM_READ_STATE_TX_WAIT   (UX_STATE_APP_STEP + 1)

#define CDC_CLASS_TYPE         CLASS_TYPE_CDC_ACM
#define CDC_INTERFACE_TYPE_1   0
#define CDC_INTERFACE_TYPE_2   1
#define CDC_DATA_INTERFACE_CLASS  0x0A
/* Enable CDC speed test and printing when defined */
#define ENABLE_CDC_SPEED_TEST 1 
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
UX_SLAVE_CLASS_CDC_ACM  *cdc_acm;
UX_SLAVE_CLASS_CDC_ACM  *cdc_acm_2;

/* Data to send over USB CDC are stored in this buffer */
uint8_t UserRx1BufferFS[APP_RX_DATA_SIZE];
uint8_t UserTx1BufferFS[APP_TX_DATA_SIZE];
uint8_t UserRx2BufferFS[APP_RX_DATA_SIZE];
uint8_t UserTx2BufferFS[APP_TX_DATA_SIZE];
uint8_t CDCRx1BufferFS[CDC_RX_DATA_SIZE];
uint8_t CDCRx2BufferFS[CDC_RX_DATA_SIZE];
UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER CDC_VCP_LineCoding =
{
  115200, /* baud rate */
  0x00,   /* stop bits-1 */
  0x00,   /* parity - none */
  0x08    /* nb. of bits 8 */
};

static UINT write_state_1 = UX_STATE_RESET;
static UINT read_state_1 = UX_STATE_RESET;
static UINT write_state_2 = UX_STATE_RESET;
static UINT read_state_2 = UX_STATE_RESET;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void USBD_CDC_VCP_Config(UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER *);
UX_SLAVE_CLASS_CDC_ACM* Get_CDC_ACM_Handle_By_Index(UX_SLAVE_DEVICE *device, uint8_t cdc_index);
/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_CDC_ACM_Activate
  *         This function is called when insertion of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Activate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Activate */
  /* Save the CDC instance */
  cdc_acm = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;
  /* USER CODE END USBD_CDC_ACM_Activate */

  return;
}


/**
  * @brief  USBD_CDC_ACM_Deactivate
  *         This function is called when extraction of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Deactivate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Deactivate */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);
  /* USER CODE END USBD_CDC_ACM_Deactivate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_ParameterChange
  *         This function is invoked to manage the CDC ACM class requests.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_ParameterChange(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_ParameterChange */
  cdc_acm = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;
  ULONG request;
  UX_SLAVE_TRANSFER *transfer_request;
  UX_SLAVE_DEVICE *device;

  /* Get the pointer to the device */
  device = &_ux_system_slave -> ux_system_slave_device;

  /* Get the pointer to the transfer request associated with the control endpoint */
  transfer_request = &device -> ux_slave_device_control_endpoint.ux_slave_endpoint_transfer_request;

  request = *(transfer_request -> ux_slave_transfer_request_setup + UX_SETUP_REQUEST);

  switch (request)
  {
    case UX_SLAVE_CLASS_CDC_ACM_SET_LINE_CODING :

      /* Get the Line Coding parameters */
      if (ux_device_class_cdc_acm_ioctl(cdc_acm, UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_CODING,
                                        &CDC_VCP_LineCoding) != UX_SUCCESS)
      {
        Error_Handler();
      }

      /* Check if baudrate < 9600) then set it to 9600 */
      if (CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate < MIN_BAUDRATE)
      {
        CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate = MIN_BAUDRATE;

        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }
      else
      {
        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }

      break;

    case UX_SLAVE_CLASS_CDC_ACM_GET_LINE_CODING :

      /* Set the Line Coding parameters */
      if (ux_device_class_cdc_acm_ioctl(cdc_acm, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
                                        &CDC_VCP_LineCoding) != UX_SUCCESS)
      {
        Error_Handler();
      }

      break;

    case UX_SLAVE_CLASS_CDC_ACM_SET_CONTROL_LINE_STATE :
    default :
      break;
  }

  /* USER CODE END USBD_CDC_ACM_ParameterChange */

  return;
}


/* USER CODE BEGIN 1 */

/**
  * @brief  USBD_CDC_ACM_2_Activate
  *         This function is called when insertion of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_2_Activate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Activate */
  /* Save the CDC instance */
  cdc_acm_2 = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;
  /* USER CODE END USBD_CDC_ACM_Activate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_2_Deactivate
  *         This function is called when extraction of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_2_Deactivate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Deactivate */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);
  /* USER CODE END USBD_CDC_ACM_Deactivate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_2_ParameterChange
  *         This function is invoked to manage the CDC ACM class requests.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_2_ParameterChange(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_ParameterChange */
  cdc_acm_2 = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;
  ULONG request;
  UX_SLAVE_TRANSFER *transfer_request;
  UX_SLAVE_DEVICE *device;

  /* Get the pointer to the device */
  device = &_ux_system_slave -> ux_system_slave_device;

  /* Get the pointer to the transfer request associated with the control endpoint */
  transfer_request = &device -> ux_slave_device_control_endpoint.ux_slave_endpoint_transfer_request;

  request = *(transfer_request -> ux_slave_transfer_request_setup + UX_SETUP_REQUEST);

  switch (request)
  {
    case UX_SLAVE_CLASS_CDC_ACM_SET_LINE_CODING :

      /* Get the Line Coding parameters */
      if (ux_device_class_cdc_acm_ioctl(cdc_acm_2, UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_CODING,
                                        &CDC_VCP_LineCoding) != UX_SUCCESS)
      {
        Error_Handler();
      }

      /* Check if baudrate < 9600) then set it to 9600 */
      if (CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate < MIN_BAUDRATE)
      {
        CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate = MIN_BAUDRATE;

        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }
      else
      {
        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }

      break;

    case UX_SLAVE_CLASS_CDC_ACM_GET_LINE_CODING :

      /* Set the Line Coding parameters */
      if (ux_device_class_cdc_acm_ioctl(cdc_acm_2, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
                                        &CDC_VCP_LineCoding) != UX_SUCCESS)
      {
        Error_Handler();
      }

      break;

    case UX_SLAVE_CLASS_CDC_ACM_SET_CONTROL_LINE_STATE :
    default :
      break;
  }

  /* USER CODE END USBD_CDC_ACM_ParameterChange */

  return;
}

/**
  * @brief  CDC_ACM_1_Read_Single_Package_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_1_Read_Single_Package_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
  //UX_SLAVE_INTERFACE *data_interface;
  //UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state_1 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_1);
  /* 空指针保护 */
  if (cdc_acm == UX_NULL)
  {
        return; 
  }
  /* Run state machine.  */
  switch(read_state_1)
  {
    case UX_STATE_RESET:
      read_state_1 = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                (UCHAR *)UserRx1BufferFS, read_length,
                                                &actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        /* Reset state.  */
        read_state_1 = UX_STATE_RESET;
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
          read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
        }
        else
        {
          read_state_1 = UX_STATE_RESET;
        }
        return;
      }
      /* Wait.  */
      return;
    case APP_CDC_ACM_READ_STATE_TX_START:
      /* Send the data via UART */
      //printf("%*s",actual_length, UserRxBufferFS);
      if (actual_length != 0)
      {
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)UserRx1BufferFS, actual_length, COM_POLL_TIMEOUT);
      }
      read_state_1 = UX_STATE_RESET;
      return;
      /* DMA started.  */
      read_state_1 = APP_CDC_ACM_READ_STATE_TX_WAIT;
      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      /* Check the DMA transfer status.  */

      read_state_1 = UX_STATE_WAIT;
      return;
    default:
      return;
  }
}

/**
  * @brief  CDC_ACM_1_Read_Multiple_Package_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_1_Read_Multiple_Package_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
//   UX_SLAVE_INTERFACE *data_interface;
//   UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;
  static uint32_t cdc_rx_accumulated_len = 0;
  static uint8_t cdc_rx_packing = 0; // 0: 0: Idle, 1: Combining packages
  static uint8_t data_null_count = 0;
  /* Timestamp and stats for a frame */
  static uint32_t cdc_frame_start_ts = 0;
  static uint32_t cdc_frame_end_ts = 0;
  static char cdc_stats_msg[96];

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    actual_length = 0;
    cdc_rx_packing = 0;
    cdc_rx_accumulated_len = 0;
    read_state_1 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
//   data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
//   cdc_acm =  data_interface->ux_slave_interface_class_instance;
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_1);
  /* 空指针保护 */
  if (cdc_acm == UX_NULL)
  {
        return; 
  }
  /* Run state machine.  */
  switch(read_state_1)
  {
    case UX_STATE_RESET:
      
      cdc_rx_accumulated_len = 0;
      cdc_rx_packing = 0;
      read_state_1 = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      actual_length = 0;
      //memset(UserRx1BufferFS, 0, APP_RX_DATA_SIZE);
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                //(UCHAR *)UserRx1BufferFS + cdc_rx_accumulated_len,
                                                &CDCRx1BufferFS[0],
                                                read_length,
                                                &actual_length);
      //printf("status: %d, %d\r\n", status, actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        
        if (cdc_rx_accumulated_len > 0) {
          //The buffer contains data and is in the sending state
          read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
        } else {
            //No data in the buffer, reset status
            cdc_rx_packing = 0;
            cdc_rx_accumulated_len = 0;
            read_state_1 = UX_STATE_RESET;
        }
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
                  /* First data of a new frame: record start timestamp */
                  if (cdc_rx_accumulated_len == 0)
                  {
        #if defined(ENABLE_CDC_SPEED_TEST)
                    cdc_frame_start_ts = HAL_GetTick();
        #endif
                  }

          /* Empty data count cleared */
          data_null_count = 0;
          /* If there is already data in the buffer, update the bundling status */
          if (cdc_rx_accumulated_len > 0) {
            cdc_rx_packing = 1;
          }
          memcpy(&UserRx1BufferFS[cdc_rx_accumulated_len], &CDCRx1BufferFS[0], actual_length);
          cdc_rx_accumulated_len += actual_length;
          
          //read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
          //Check if the remaining space can still receive another packet
          if ((APP_RX_DATA_SIZE - cdc_rx_accumulated_len) < read_length) {
              //There is less than 64 remaining space, please send directly
              read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
              return;
          }
        }
        else
        {
          read_state_1 = UX_STATE_RESET;
        }
        return;
      }
      /* Wait.  */
      //USB idle and with data, send directly
      if (status == UX_STATE_WAIT && cdc_rx_accumulated_len > 0 ) {
        if(actual_length == 0)
        {
          // Empty data count
          data_null_count++;
          // If there are more than 15 consecutive empty data, it means that the transmission is complete, and the data in the buffer can be sent directly. 
          // This is to avoid the problem of incomplete data caused by the idle timeout of USB.
          if(data_null_count > 15)
          {
            // Accumulate two empty data and send them
            data_null_count = 0;
            read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
          }
          return;
        }else{
          read_state_1 = UX_STATE_WAIT;
        }
        
          
      }
      return;
    case APP_CDC_ACM_READ_STATE_TX_START:
    //   /* Send the data via UART */


    /*Print masking*/
      /* Record end timestamp and print stats */
#if defined(ENABLE_CDC_SPEED_TEST)
      cdc_frame_end_ts = HAL_GetTick();
      if (cdc_frame_start_ts == 0)
      {
        /* if start ts not set, treat as instantaneous */
        cdc_frame_start_ts = cdc_frame_end_ts;
      }
      uint32_t duration_ms = (cdc_frame_end_ts - cdc_frame_start_ts);
      uint32_t bytes = cdc_rx_accumulated_len;
      uint32_t rate_bps = 0;
      uint32_t rate_kb = 0;
      if (duration_ms > 0)
      {
        rate_bps = (bytes * 1000u) / duration_ms; /* bytes per second */
        rate_kb = rate_bps / 1024u; /* KB/s */
      }
      int n = snprintf(cdc_stats_msg, sizeof(cdc_stats_msg), "CDC frame: %lu bytes, %lu ms, %lu KB/s\r\n",
                       (unsigned long)bytes, (unsigned long)duration_ms, (unsigned long)rate_kb);
      if (n > 0)
      {
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)cdc_stats_msg, (uint16_t)strnlen(cdc_stats_msg, sizeof(cdc_stats_msg)), 0xff);
      }
      /* reset frame tracking */
      cdc_frame_start_ts = 0;
      cdc_frame_end_ts = 0;
#endif
       HAL_UART_Transmit(&hcom_uart[COM1], &UserRx1BufferFS[0], cdc_rx_accumulated_len, 0xff);
       read_state_1 = UX_STATE_RESET;
     return;
      /* DMA started.  */
      read_state_1 = APP_CDC_ACM_READ_STATE_TX_WAIT;

      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      // DMA TF FLAG
      read_state_1 = UX_STATE_RESET;
      return;
    default:
      return;
  }
}

/**
  * @brief  CDC_ACM_1_Write_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_1_Write_Task(VOID)
{
  UX_SLAVE_DEVICE    *device;
//   UX_SLAVE_INTERFACE *data_interface;
//   UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  ULONG actual_length = 0;
  /* TX timing/stats */
  static uint32_t tx_frame_start_ts = 0;
  static uint32_t tx_frame_end_ts = 0;
  static uint32_t tx_frame_bytes = 0;
  static char tx_stats_msg[96];
  //ULONG buffptr;
  //ULONG buffsize;
  UINT ux_status = UX_SUCCESS;
  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    write_state_1 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface */
//   data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
//   cdc_acm =  data_interface->ux_slave_interface_class_instance;
    UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_1);

    switch(write_state_1) {
        case UX_STATE_RESET:
            if (tx_pending_1) {
                tx_pending_1 = 0;
          /* record start timestamp and set bytes to APP_TX_DATA_SIZE before write call */
#if defined(ENABLE_CDC_SPEED_TEST)
          tx_frame_start_ts = HAL_GetTick();
          tx_frame_bytes = APP_TX_DATA_SIZE;
#endif
                ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UserTx1BufferFS, APP_TX_DATA_SIZE, &actual_length);
                //printf("TX_1 start: %02x %d\r\n", ux_status, actual_length);
                if (ux_status == UX_STATE_WAIT) {
                    write_state_1 = UX_STATE_WAIT;
                }
            }
            break;
        case UX_STATE_WAIT:
            ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UX_NULL, 0, &actual_length);
            //printf("TX_1 wait: %02x %d\r\n", ux_status, actual_length);
            if (ux_status <= UX_STATE_NEXT) {
#if defined(ENABLE_CDC_SPEED_TEST)
          /* record end timestamp and print stats if we had a start */
          if (tx_frame_start_ts != 0) {
            tx_frame_end_ts = HAL_GetTick();
            uint32_t duration_ms = (tx_frame_end_ts - tx_frame_start_ts);
            uint32_t bytes = tx_frame_bytes;
            uint32_t rate_bps = 0;
            uint32_t rate_kb = 0;
            if (duration_ms > 0) {
              rate_bps = (bytes * 1000u) / duration_ms; /* bytes per second */
              rate_kb = rate_bps / 1024u; /* KB/s */
            }
            int n = snprintf(tx_stats_msg, sizeof(tx_stats_msg), "CDC TX: %lu bytes, %lu ms, %lu KB/s\r\n",
                     (unsigned long)bytes, (unsigned long)duration_ms, (unsigned long)rate_kb);
            if (n > 0) {
              HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)tx_stats_msg, (uint16_t)strnlen(tx_stats_msg, sizeof(tx_stats_msg)), 0xff);
            }
            tx_frame_start_ts = 0;
            tx_frame_end_ts = 0;
            tx_frame_bytes = 0;
          }
#endif
                write_state_1 = UX_STATE_RESET;
        }
            break;
        default:
            write_state_1 = UX_STATE_RESET;
            break;
    }
}

/**
  * @brief  CDC_ACM_2_Read_Single_Package_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_2_Read_Single_Package_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
  //UX_SLAVE_INTERFACE *data_interface;
  //UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state_2 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_2);
  /* 空指针保护 */
  if (cdc_acm == UX_NULL)
  {
        return; 
  }
  /* Run state machine.  */
  switch(read_state_2)
  {
    case UX_STATE_RESET:
      read_state_2 = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                (UCHAR *)UserRx2BufferFS, read_length,
                                                &actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        /* Reset state.  */
        read_state_2 = UX_STATE_RESET;
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
          read_state_2 = APP_CDC_ACM_READ_STATE_TX_START;
        }
        else
        {
          read_state_2 = UX_STATE_RESET;
        }
        return;
      }
      /* Wait.  */
      return;
    case APP_CDC_ACM_READ_STATE_TX_START:
      /* Send the data via UART */
      //printf("%*s",actual_length, UserRxBufferFS);
      if (actual_length != 0)
      {
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)UserRx2BufferFS, actual_length, COM_POLL_TIMEOUT);
      }
      read_state_2 = UX_STATE_RESET;
      return;
      /* DMA started.  */
      read_state_2 = APP_CDC_ACM_READ_STATE_TX_WAIT;
      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      /* Check the DMA transfer status.  */

      read_state_2 = UX_STATE_WAIT;
      return;
    default:
      return;
  }
}

/**
  * @brief  CDC_ACM_2_Read_Multiple_Package_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_2_Read_Multiple_Package_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
//   UX_SLAVE_INTERFACE *data_interface;
//   UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;
  static uint32_t cdc_rx_accumulated_len = 0;
  static uint8_t cdc_rx_packing = 0; // 0: 0: Idle, 1: Combining packages
  static uint8_t data_null_count = 0;
  /* Timestamp and stats for a frame */
  static uint32_t cdc_frame_start_ts = 0;
  static uint32_t cdc_frame_end_ts = 0;
  static char cdc_stats_msg[96];

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    actual_length = 0;
    cdc_rx_packing = 0;
    cdc_rx_accumulated_len = 0;
    read_state_2 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
//   data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
//   cdc_acm =  data_interface->ux_slave_interface_class_instance;
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_2);
  /* 空指针保护 */
  if (cdc_acm == UX_NULL)
  {
        return; 
  }
  /* Run state machine.  */
  switch(read_state_2)
  {
    case UX_STATE_RESET:
      
      cdc_rx_accumulated_len = 0;
      cdc_rx_packing = 0;
      read_state_2 = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      actual_length = 0;
      //memset(UserRx1BufferFS, 0, APP_RX_DATA_SIZE);
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                //(UCHAR *)UserRx1BufferFS + cdc_rx_accumulated_len,
                                                &CDCRx2BufferFS[0],
                                                read_length,
                                                &actual_length);
      //printf("status: %d, %d\r\n", status, actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        
        if (cdc_rx_accumulated_len > 0) {
          //The buffer contains data and is in the sending state
          read_state_2 = APP_CDC_ACM_READ_STATE_TX_START;
        } else {
            //No data in the buffer, reset status
            cdc_rx_packing = 0;
            cdc_rx_accumulated_len = 0;
            read_state_2 = UX_STATE_RESET;
        }
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
                  /* First data of a new frame: record start timestamp */
                  if (cdc_rx_accumulated_len == 0)
                  {
        #if defined(ENABLE_CDC_SPEED_TEST)
                    cdc_frame_start_ts = HAL_GetTick();
        #endif
                  }

          /* Empty data count cleared */
          data_null_count = 0;
          /* If there is already data in the buffer, update the bundling status */
          if (cdc_rx_accumulated_len > 0) {
            cdc_rx_packing = 1;
          }
          memcpy(&UserRx2BufferFS[cdc_rx_accumulated_len], &CDCRx2BufferFS[0], actual_length);
          cdc_rx_accumulated_len += actual_length;
          
          //read_state_1 = APP_CDC_ACM_READ_STATE_TX_START;
          //Check if the remaining space can still receive another packet
          if ((APP_RX_DATA_SIZE - cdc_rx_accumulated_len) < read_length) {
              //There is less than 64 remaining space, please send directly
            read_state_2 = APP_CDC_ACM_READ_STATE_TX_START;
              return;
          }
        }
        else
        {
          read_state_2 = UX_STATE_RESET;
        }
        return;
      }
      /* Wait.  */
      //USB idle and with data, send directly
      if (status == UX_STATE_WAIT && cdc_rx_accumulated_len > 0 ) {
        if(actual_length == 0)
        {
          // Empty data count
          data_null_count++;
          // If there are more than 15 consecutive empty data, it means that the transmission is complete, and the data in the buffer can be sent directly. 
          // This is to avoid the problem of incomplete data caused by the idle timeout of USB.
          if(data_null_count > 15)
          {
            // Accumulate two empty data and send them
            data_null_count = 0;
            read_state_2 = APP_CDC_ACM_READ_STATE_TX_START;
          }
          return;
        }else{
          read_state_2 = UX_STATE_WAIT;
        }
        
          
      }
      return;
    case APP_CDC_ACM_READ_STATE_TX_START:
    //   /* Send the data via UART */


    /*Print masking*/
      /* Record end timestamp and print stats */
#if defined(ENABLE_CDC_SPEED_TEST)
      cdc_frame_end_ts = HAL_GetTick();
      if (cdc_frame_start_ts == 0)
      {
        /* if start ts not set, treat as instantaneous */
        cdc_frame_start_ts = cdc_frame_end_ts;
      }
      uint32_t duration_ms = (cdc_frame_end_ts - cdc_frame_start_ts);
      uint32_t bytes = cdc_rx_accumulated_len;
      uint32_t rate_bps = 0;
      uint32_t rate_kb = 0;
      if (duration_ms > 0)
      {
        rate_bps = (bytes * 1000u) / duration_ms; /* bytes per second */
        rate_kb = rate_bps / 1024u; /* KB/s */
      }
      int n = snprintf(cdc_stats_msg, sizeof(cdc_stats_msg), "CDC frame: %lu bytes, %lu ms, %lu KB/s\r\n",
                       (unsigned long)bytes, (unsigned long)duration_ms, (unsigned long)rate_kb);
      if (n > 0)
      {
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)cdc_stats_msg, (uint16_t)strnlen(cdc_stats_msg, sizeof(cdc_stats_msg)), 0xff);
      }
      /* reset frame tracking */
      cdc_frame_start_ts = 0;
      cdc_frame_end_ts = 0;
#endif
       HAL_UART_Transmit(&hcom_uart[COM1], &UserRx2BufferFS[0], cdc_rx_accumulated_len, 0xff);
      read_state_2 = UX_STATE_RESET;
     return;
      /* DMA started.  */
      read_state_2 = APP_CDC_ACM_READ_STATE_TX_WAIT;

      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      // DMA TF FLAG
      read_state_2 = UX_STATE_RESET;
      return;
    default:
      return;
  }
}

/**
  * @brief  CDC_ACM_2_Write_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_2_Write_Task(VOID)
{
  UX_SLAVE_DEVICE    *device;
//   UX_SLAVE_INTERFACE *data_interface;
//   UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  ULONG actual_length = 0;
  /* TX timing/stats */
  static uint32_t tx_frame_start_ts = 0;
  static uint32_t tx_frame_end_ts = 0;
  static uint32_t tx_frame_bytes = 0;
  static char tx_stats_msg[96];
  //ULONG buffptr;
  //ULONG buffsize;
  UINT ux_status = UX_SUCCESS;
  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    write_state_2 = UX_STATE_RESET;
    return;
  }

  /* Get Data interface */
//   data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
//   cdc_acm =  data_interface->ux_slave_interface_class_instance;
    UX_SLAVE_CLASS_CDC_ACM *cdc_acm = Get_CDC_ACM_Handle_By_Index(device, CDC_INTERFACE_TYPE_2);

    switch(write_state_2) {
        case UX_STATE_RESET:
            if (tx_pending_2) {
                tx_pending_2 = 0;
          /* record start timestamp and set bytes to APP_TX_DATA_SIZE before write call */
#if defined(ENABLE_CDC_SPEED_TEST)
          tx_frame_start_ts = HAL_GetTick();
          tx_frame_bytes = APP_TX_DATA_SIZE;
#endif
                ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UserTx2BufferFS, APP_TX_DATA_SIZE, &actual_length);
                //printf("TX_1 start: %02x %d\r\n", ux_status, actual_length);
                if (ux_status == UX_STATE_WAIT) {
                    write_state_2 = UX_STATE_WAIT;
                }
            }
            break;
        case UX_STATE_WAIT:
            ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UX_NULL, 0, &actual_length);
            //printf("TX_1 wait: %02x %d\r\n", ux_status, actual_length);
            if (ux_status <= UX_STATE_NEXT) {
#if defined(ENABLE_CDC_SPEED_TEST)
          /* record end timestamp and print stats if we had a start */
          if (tx_frame_start_ts != 0) {
            tx_frame_end_ts = HAL_GetTick();
            uint32_t duration_ms = (tx_frame_end_ts - tx_frame_start_ts);
            uint32_t bytes = tx_frame_bytes;
            uint32_t rate_bps = 0;
            uint32_t rate_kb = 0;
            if (duration_ms > 0) {
              rate_bps = (bytes * 1000u) / duration_ms; /* bytes per second */
              rate_kb = rate_bps / 1024u; /* KB/s */
            }
            int n = snprintf(tx_stats_msg, sizeof(tx_stats_msg), "CDC TX: %lu bytes, %lu ms, %lu KB/s\r\n",
                     (unsigned long)bytes, (unsigned long)duration_ms, (unsigned long)rate_kb);
            if (n > 0) {
              HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)tx_stats_msg, (uint16_t)strnlen(tx_stats_msg, sizeof(tx_stats_msg)), 0xff);
            }
            tx_frame_start_ts = 0;
            tx_frame_end_ts = 0;
            tx_frame_bytes = 0;
          }
#endif
                write_state_2 = UX_STATE_RESET;
            }
            break;
        default:
            write_state_2 = UX_STATE_RESET;
            break;
    }
}

/**
  * @brief  USBD_CDC_VCP_Config
            Configure the COM Port with the parameters received from host.
  * @param  UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER: linecoding struct.
  * @param  CDC_VCP_LineCoding: CDC VCP line coding.
  * @retval none
  */
static VOID USBD_CDC_VCP_Config(UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER
                                *CDC_VCP_LineCoding)
{
  /* Deinitialization UART */
//   if (HAL_UART_DeInit(uart_handler) != HAL_OK)
//   {
//     /* Deinitialization Error */
//     Error_Handler();
//   }

  /* Check stop bit parameter */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_stop_bit)
  {
    case 0:

      /* Set the UART Stop bit to 1 */
    //   uart_handler->Init.StopBits = UART_STOPBITS_1;

      break;

    case 2:

      /* Set the UART Stop bit to 2 */
    //   uart_handler->Init.StopBits = UART_STOPBITS_2;

      break;

    default :

      /* By default set the UART Stop bit to 1 */
    //   uart_handler->Init.StopBits = UART_STOPBITS_1;

      break;
  }

  /* Check parity parameter */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_parity)
  {
    case 0:

      /* Set the UART parity bit to none */
    //   uart_handler->Init.Parity = UART_PARITY_NONE;

      break;

    case 1:

      /* Set the UART parity bit to ODD */
    //   uart_handler->Init.Parity = UART_PARITY_ODD;

      break;

    case 2:

      /* Set the UART parity bit to even */
    //   uart_handler->Init.Parity = UART_PARITY_EVEN;

      break;

    default :

      /* By default set the UART parity bit to none */
    //   uart_handler->Init.Parity = UART_PARITY_NONE;

      break;
  }

  /* Set the UART data type : only 8bits and 9bits is supported */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_data_bit)
  {
    case 0x07:

      /* With this configuration a parity (Even or Odd) must be set */
    //   uart_handler->Init.WordLength = UART_WORDLENGTH_8B;

      break;

    case 0x08:

    //   if (uart_handler->Init.Parity == UART_PARITY_NONE)
    //   {
    //     uart_handler->Init.WordLength = UART_WORDLENGTH_8B;
    //   }
    //   else
    //   {
    //     uart_handler->Init.WordLength = UART_WORDLENGTH_9B;
    //   }

      break;

    default :

    //   uart_handler->Init.WordLength = UART_WORDLENGTH_8B;

      break;
  }

//   /* Get the UART baudrate from VCP */
//   uart_handler->Init.BaudRate = CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_baudrate;

//   /* Set the UART Hw flow control to none */
//   uart_handler->Init.HwFlowCtl = UART_HWCONTROL_NONE;

//   /* Set the UART mode */
//   uart_handler->Init.Mode = UART_MODE_TX_RX;

//   /* Set the UART sampling */
//   uart_handler->Init.OverSampling = UART_OVERSAMPLING_16;

//   /* Initialization UART */
//   if (HAL_UART_Init(uart_handler) != HAL_OK)
//   {
//     /* Initialization Error */
//     Error_Handler();
//   }

//   /* Start reception: provide the buffer pointer with offset and the buffer size */
//   HAL_UART_Receive_IT(uart_handler, (uint8_t *)(UserTxBufferFS + UserTxBufPtrIn), 1);
}

UX_SLAVE_CLASS_CDC_ACM* Get_CDC_ACM_Handle_By_Index(UX_SLAVE_DEVICE *device, uint8_t cdc_index)
{
    uint16_t cdc_control_if = USBD_Get_Interface_Number(CDC_CLASS_TYPE, cdc_index);
    uint16_t cdc_data_if = cdc_control_if + 1;

    UX_SLAVE_INTERFACE *interface = device->ux_slave_device_first_interface;
    while (interface != UX_NULL) {
        if ((interface->ux_slave_interface_descriptor.bInterfaceClass == CDC_DATA_INTERFACE_CLASS) &&
            (interface->ux_slave_interface_descriptor.bInterfaceNumber == cdc_data_if)) {
            return (UX_SLAVE_CLASS_CDC_ACM*)interface->ux_slave_interface_class_instance;
        }
        interface = interface->ux_slave_interface_next_interface;
    }
    return UX_NULL;
}
/* USER CODE END 1 */
