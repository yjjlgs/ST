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
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
UX_SLAVE_CLASS_CDC_ACM  *cdc_acm;

/* Data to send over USB CDC are stored in this buffer */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER CDC_VCP_LineCoding =
{
  115200, /* baud rate */
  0x00,   /* stop bits-1 */
  0x00,   /* parity - none */
  0x08    /* nb. of bits 8 */
};

static UINT write_state = UX_STATE_RESET;
static UINT read_state = UX_STATE_RESET;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void USBD_CDC_VCP_Config(UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER *);
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
  * @brief  CDC_ACM_Read_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_Read_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
  UX_SLAVE_INTERFACE *data_interface;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
  data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
  cdc_acm =  data_interface->ux_slave_interface_class_instance;
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;

  /* Run state machine.  */
  switch(read_state)
  {
    case UX_STATE_RESET:
      read_state = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                (UCHAR *)UserRxBufferFS, read_length,
                                                &actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        /* Reset state.  */
        read_state = UX_STATE_RESET;
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
          read_state = APP_CDC_ACM_READ_STATE_TX_START;
        }
        else
        {
          read_state = UX_STATE_RESET;
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
        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)UserRxBufferFS, actual_length, COM_POLL_TIMEOUT);
      }
      read_state = APP_CDC_ACM_READ_STATE_TX_WAIT;
      /* DMA started.  */

      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      /* Check the DMA transfer status.  */

      read_state = UX_STATE_WAIT;
      return;
    default:
      return;
  }
}

/**
  * @brief  CDC_ACM_Write_Task.
  * @param  none
  * @retval none
  */
VOID CDC_ACM_Write_Task(VOID)
{
  UX_SLAVE_DEVICE    *device;
  UX_SLAVE_INTERFACE *data_interface;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  ULONG actual_length;
  //ULONG buffptr;
  //ULONG buffsize;
  UINT ux_status = UX_SUCCESS;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state = UX_STATE_RESET;
    return;
  }

  /* Get Data interface */
  data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
  cdc_acm =  data_interface->ux_slave_interface_class_instance;

    switch(write_state) {
        case UX_STATE_RESET:
            if (tx_pending) {
                tx_pending = 0;
                ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UserTxBufferFS, 64, &actual_length);
                //printf("TX start: %02x %d\r\n", ux_status, actual_length);
                if (ux_status == UX_STATE_WAIT) {
                    write_state = UX_STATE_WAIT;
                }
            }
            break;
        case UX_STATE_WAIT:
            ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UX_NULL, 0, &actual_length);
            //printf("TX wait: %02x %d\r\n", ux_status, actual_length);
            if (ux_status <= UX_STATE_NEXT) {
                write_state = UX_STATE_RESET;
            }
            break;
        default:
            write_state = UX_STATE_RESET;
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

/* USER CODE END 1 */
