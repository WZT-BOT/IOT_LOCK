/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "OLED.h"
#include "motor.h"
#include "TTP229.h"
#include "flash.h"
#include "Store.h"
#include "fm383c.h"
#include "ADC.h"
#include "L9100S.h"
#include "Hall.h"
#include "esp8266.h"
#include "Delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern ADC_HandleTypeDef hadc1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
	uint16_t Mode=0;
	
	uint8_t my_uart1_redata=0;
	uint8_t my_uart1_txdata=0x01;
	uint8_t my_uart2_redata=0;
	uint8_t my_uart2_txdata=0x01;
	uint16_t finger_ID;
	
	int Identify_Result = 0;
	extern uint16_t ADC_Value;
	extern uint16_t PassWord;
	extern uint16_t USART_RX_STA;       
	extern uint16_t USART2_RX_STA;   			
	extern uint8_t num;
	extern uint16_t num_2;
	extern uint8_t USART_RX_BUF[USART_REC_LEN];
	extern uint8_t USART2_RX_BUF[USART2_REC_LEN];
	extern	unsigned char Blue_LED[16];
	extern	unsigned char Red_LED[16];
	extern	unsigned char Green_LED[16];
	extern unsigned char	PS_GetImage[12];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	OLED_Init();
	Store_Init();
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_UART_Receive_IT(&huart1,&my_uart1_redata,1);
	HAL_UART_Receive_IT(&huart2,&my_uart2_redata,1);
	HAL_TIM_Base_Start_IT(&htim3);
	init_wifi();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		OLED_Initial_interface();
		KeyDown();
		Mode = PassWord;
		switch(Mode)
			{
				case('C'):	
				{
					for(int i=0;i<4;i++)
					{
						Identify_Result = FPM383C_Identify(1000);
						Clear_Usart_Rx();
						Delay_ms(1000);
						if(Identify_Result)
						{
								Delay_ms(500);
								OPEN_DOOR();															 								//����
								break;			
						}
					}
					Identify_Result = 0;
					OLED_Clear();
				}break;
				case('D'):	
				{
					//FingerPrint_Insert();
					Password_Insert();
				}break;
				case('E'):	
				{
					for(int i=0;i<4;i++)
					{
						Identify_Result = FPM383C_Identify(1000);
						Delay_ms(1000);
						if(Identify_Result)
						break;
					}
					if(Identify_Result)
					{
						FingerPrint_Insert();
						Identify_Result = 0;
					}

				}break;
				case('F'):	
				{
					wifi();
				}break;
				case('G'):	
				{
					
					OLED_Clear();
					OLED_Initial_interface();
					PassWord = 0;
					Mode =0;
					
				}break;
	
		 }
  }
  /* USER CODE END 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	if(huart == &huart1)
	{
		HAL_UART_Receive_DMA(&huart1,&USART_RX_BUF[num++],sizeof(USART_RX_BUF[num++]));
		if(num>USART_REC_LEN)		 
		{
			num=USART_REC_LEN;
		}
			USART_RX_STA = 1;
		
	}
	if(huart == &huart2)
	{
		HAL_UART_Receive_IT(&huart2,&USART2_RX_BUF[num_2++],sizeof(USART2_RX_BUF[num_2++]));
		//HAL_UART_Receive_DMA(&huart2,&USART2_RX_BUF[num_2++],sizeof(USART2_RX_BUF[num_2++]));
		if(num_2>USART2_REC_LEN)		 
		{
			num_2 = USART2_REC_LEN;
		}
		USART2_RX_STA = 1;	
	}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
