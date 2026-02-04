/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"

#include "options.h"
#include "sys.h"
#include "gpio.h"
#include "uarts.h"
#include "CDC.h"

#include "usb_device.h"
#include "usbd_hid.h"

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

// MIDI Message arrays: 4 Bytes
// 1st byte : MSBs --> Cable Number 0, LSBs --> duplicate MSBs of MIDI opcode (!??!)
uint8_t midiNoteOn[4] = { 0x09, 0x93, 65, 99 };
uint8_t midiNoteOff[4]= { 0x08, 0x83, 65, 99 };


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

#ifdef USE_CDC
void cmd_handler( int c )
{
switch	( c )
	{
	case '&' :
		CDC_printf( "HCLK %u\n", SystemCoreClock );
		break;
	case '$' :
		report_interrupts();
		break;
	default:	// simple echo
		CDC_printf( "cmd '%c'\n", ((c>=' ')?(c):('?')) );
	}
}
#endif

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  // HAL_Init() flattened to 4 statements :

	/* Configure Flash prefetch */
	// __HAL_FLASH_PREFETCH_BUFFER_ENABLE();	// enabled by default
	/* Set Interrupt Group Priority */
	// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	__NVIC_SetPriorityGrouping(3);	// group 4 est represente par 3 (4 bits for pre-emption priority, 0 bits for subpriority)
	/* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
	// HAL_InitTick(TICK_INT_PRIORITY);	// ridicule, il devra le refaire apres avoir configure la PLL
	/* Init the low level hardware */
	// HAL_MspInit();			// toxic weak function, but 3 LL-translated lines were put in gpio.c

  /* Configure the system clock */
  SystemClock_Config();
  SystemCoreClockUpdate();
  systick_init( 1000 );		// priorite 7

  /* Initialize peripherals */
  gpio_init();

#ifdef USE_CDC
// config UART (interrupt handler doit etre pret!!)
gpio_uart2_init();
UART2_init( 38400 );
CDC_init();	// on doit faire cela avant d'entrer dans la main loop,
CDC.verbose = 1;
CDC_printf("Hello je suis imposant\n");
#endif

  MX_USB_DEVICE_Init();

  USB_DP_pullup( 1 );
  HAL_Delay(5000);

  /* Infinite loop */
  int c;
  while (1)
  	{
	// Make sure the USB functions are not BUSY before sending the MIDI Message
	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOn, 4); LED_ON();
	HAL_Delay(500);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOff, 4); LED_OFF();
	HAL_Delay(500);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  #ifdef HSE_EXT
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  #else
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  #endif
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  #define FLASH_LATENCY_2            FLASH_ACR_LATENCY_1       /*!< FLASH Two Latency cycles */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
