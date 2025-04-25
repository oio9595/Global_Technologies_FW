/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#define __APIC_MAIN_C__
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "include.h"

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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

osThreadId JIG_MAINHandle;
osThreadId KeyDetectTaskHandle;
osThreadId KeyHandlerTaskHandle;
osThreadId myTaskDebugUartHandle;
osMessageQId queue_key_eventHandle;
/* USER CODE BEGIN PV */

//osMessageQId Queue_Handle;
//osPoolId Pool_ID;
#define UART_CMD_LENGTH     60
typedef struct {
    uint8_t len;
    uint8_t param[UART_CMD_LENGTH];
}MessageQ;


/* Place following data in section METAINFO */
#pragma default_variable_attributes = __root @ "METAINFO"
const uint16_t gn_metainfo[8] =
{
    0xAAAA,
    VERSION_MAJOR,
    VERSION_MINOR,
    VERSION_REVISION,
    BUILD_YEAR,
    (BUILD_MONTH + 1),
    BUILD_DAY,
    0x5555
};
/* Stop placing data in section METAINFO */
#pragma default_variable_attributes =

uint8_t gn_i2c_setting_type;    /* 0 : I2C, 1 : GPIO */

uint8_t gn_apic_test_type;      /* 0 : normal test, 1 : 1 : trimming */

static uint8_t gn_uart_temp_rx;

static uint8_t gn_uart_msg_queue_index;
static MessageQ gt_uart_queue[16];
static MessageQ* gp_uart_queue;

QueueHandle_t xQueue = NULL;

char msg_buffer[256];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM4_Init(void);
void JigTestMainTask(void const * argument);
void KeyDetector(void const * argument);
void KeyHandler(void const * argument);
void TaskDebugUart(void const * argument);

/* USER CODE BEGIN PFP */
static void DAC_SPI_CS_LO()
{
    LATCH__WritePin(DAC_CS_LS_CP, DAC_CS_LS_PORT, LATCH_PIN_RESET);
}

static void DAC_SPI_CS_HI()
{
    LATCH__WritePin(DAC_CS_LS_CP, DAC_CS_LS_PORT, LATCH_PIN_SET);
}

static void spi_dac_init(void)
{
    uint16_t spi_buffer[1] = {0, };

    spi_buffer[0] = DAC_OP_MODE_WTM;

    DAC_SPI_CS_LO();
    HAL_SPI_Transmit(&hspi3, (uint8_t *)spi_buffer, 1, 1);
    DAC_SPI_CS_HI();

    spi_buffer[0] = DAC_OP_BROADCAST;

    DAC_SPI_CS_LO();
    HAL_SPI_Transmit(&hspi3, (uint8_t *)spi_buffer, 1, 1);
    DAC_SPI_CS_HI();
}

void AQIC_Apply_Voltage(float f_voltage, uint8_t ch)
{
    uint16_t channel = ch;
    uint16_t gn_spi_buffer[1] = {0};
    uint16_t voltage_to_dac = (uint16_t)((f_voltage * 4095) / 5.0f);

    if(voltage_to_dac > 4095)
    {
        voltage_to_dac = 4095;
    }

    if (channel != AQIC_D_ALL)
    {
        gn_spi_buffer[0] = (channel<<12) | voltage_to_dac;
        DAC_SPI_CS_LO();
        HAL_SPI_Transmit(&hspi3, (uint8_t*)gn_spi_buffer, 1, 1);
        DAC_SPI_CS_HI();
    }

    else
    {
        for(channel = 0 ; channel < 2 ; channel++)
        {
            gn_spi_buffer[0] = (channel<<12) | voltage_to_dac;
            DAC_SPI_CS_LO();
            HAL_SPI_Transmit(&hspi3, (uint8_t*)gn_spi_buffer, 1, 1);
            DAC_SPI_CS_HI();
        }
    }
}

void change_i2c_setting(uint8_t i2c_type)
{
    if(gn_i2c_setting_type != i2c_type)
    {
        gn_i2c_setting_type = i2c_type;

        snprintf(msg_buffer, sizeof msg_buffer, "Change to %s\r\n", (gn_i2c_setting_type == I2C_USED_I2C) ? "I2C_I2C" : "I2C_GPIO" );
        print(msg_buffer);

        if(gn_i2c_setting_type == I2C_USED_I2C)
        {
            hi2c1.Instance = I2C1;
            hi2c1.Init.ClockSpeed = 100000;
            hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
            hi2c1.Init.OwnAddress1 = 0;
            hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
            hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
            hi2c1.Init.OwnAddress2 = 0;
            hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
            hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
            if (HAL_I2C_Init(&hi2c1) != HAL_OK)
            {
                Error_Handler();
            }
        }
        else
        {
            HAL_I2C_DeInit(&hi2c1);
            /* GPIO Ports Clock Enable */
            __HAL_RCC_GPIOB_CLK_ENABLE();
            GPIO_InitTypeDef GPIO_InitStruct = {0};

            /*Configure GPIO pin Output Level */
            HAL_GPIO_WritePin(GPIOB, MCU_I2C_SCL_CS_Pin| MCU_I2C_SDA_G_Pin, GPIO_PIN_RESET);

            /*Configure GPIO pins : MCU_I2C_SCL_CS_Pin MCU_I2C_SDA_G_Pin */
            GPIO_InitStruct.Pin = MCU_I2C_SCL_CS_Pin | MCU_I2C_SDA_G_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        }
    }
}

void us_delay(uint32_t us_delay)
{
    HAL_TIM_Base_Start(&htim4);
    while (__HAL_TIM_GET_COUNTER(&htim4) < us_delay)
    {
    }
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    HAL_TIM_Base_Stop(&htim4);
}

void print(const char* str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

    print("\r\nstart AQ06 TRIMMING_JIG application....\r\n");
    snprintf(msg_buffer, sizeof msg_buffer, "Build time : %s / %s\r\n", __DATE__, __TIME__ );
    print(msg_buffer);

    ads114s08_init();

    LATCH__WritePin(LTC_HIGH_CURRENT_LS_CP, LTC_HIGH_CURRENT_LS_PORT, LATCH_PIN_RESET);
    change_i2c_setting(I2C_USED_GPIO);

    /* PWM for using APIC MODE & TRIMMING */
    AQIC_Mode_Setting(MODE_DAC);
    AQIC_Apply_Voltage(0.0f, AQIC_MODE);
    AQIC_Apply_Voltage(0.0f, AQIC_D_ALL);
    AQIC_Mode_Duty(DUTY_ZERO);

    spi_dac_init();
    /* SPI DAC - 0V */
    AQIC_Select_Output_Ch(AQIC_O_MAX);

    HAL_UART_Receive_IT(&huart2, &gn_uart_temp_rx, 1);

    HAL_TIM_Base_Start(&htim4);
print("\n\r==========================================\r\n");
#ifndef MANUAL_TEST
    snprintf(msg_buffer, sizeof msg_buffer, "Test Mode : Din Auto Increase Mode");
#else
    snprintf(msg_buffer, sizeof msg_buffer, "Test Mode : Din Manual Test Mode");
#endif
    print(msg_buffer);
print("\n\r==========================================\r\n\r\n");

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  // creates a periodic timer:
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of queue_key_event */
  osMessageQDef(queue_key_event, 16, uint16_t);
  queue_key_eventHandle = osMessageCreate(osMessageQ(queue_key_event), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
    xQueue = xQueueCreate( 10, sizeof(MessageQ *));
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of JIG_MAIN */
  osThreadDef(JIG_MAIN, JigTestMainTask, osPriorityNormal, 0, 512);
  JIG_MAINHandle = osThreadCreate(osThread(JIG_MAIN), NULL);

  /* definition and creation of KeyDetectTask */
  osThreadDef(KeyDetectTask, KeyDetector, osPriorityLow, 0, 128);
  KeyDetectTaskHandle = osThreadCreate(osThread(KeyDetectTask), NULL);

  /* definition and creation of KeyHandlerTask */
  osThreadDef(KeyHandlerTask, KeyHandler, osPriorityLow, 0, 128);
  KeyHandlerTaskHandle = osThreadCreate(osThread(KeyHandlerTask), NULL);

  /* definition and creation of myTaskDebugUart */
  osThreadDef(myTaskDebugUart, TaskDebugUart, osPriorityLow, 0, 512);
  myTaskDebugUartHandle = osThreadCreate(osThread(myTaskDebugUart), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLRCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 250000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 89;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 25;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 89;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LS_OE3_Pin|LS_D1_Pin|LS_CP1_Pin|LS_D0_Pin
                          |AS1105_CLK_Pin|LTC_DISCHARGE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RXD_GPIO_Pin|LS_CP3_Pin|LS_OE2_Pin|LD2_Pin
                          |LS_D5_Pin|TXS_OE_Pin|LS_D6_Pin|LS_CP2_Pin
                          |LED_TRIM_Pin|LS_D2_Pin|LS_D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LS_D4_Pin|LS_D3_Pin|AQIC_PWR_CTL_Pin|LED_OK_Pin
                          |LED_NG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(VLED_CTR_9V_GPIO_Port, VLED_CTR_9V_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LS_OE1_GPIO_Port, LS_OE1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin PC0 KEY_TRIMMING_Pin */
  GPIO_InitStruct.Pin = B1_Pin|GPIO_PIN_0|KEY_TRIMMING_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LS_OE3_Pin LS_D1_Pin LS_CP1_Pin LS_D0_Pin
                           VLED_CTR_9V_Pin */
  GPIO_InitStruct.Pin = LS_OE3_Pin|LS_D1_Pin|LS_CP1_Pin|LS_D0_Pin
                          |VLED_CTR_9V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RXD_GPIO_Pin LS_CP3_Pin LS_OE2_Pin LD2_Pin
                           LS_D5_Pin LS_D6_Pin LS_CP2_Pin LED_TRIM_Pin
                           LS_D2_Pin LS_D7_Pin */
  GPIO_InitStruct.Pin = RXD_GPIO_Pin|LS_CP3_Pin|LS_OE2_Pin|LD2_Pin
                          |LS_D5_Pin|LS_D6_Pin|LS_CP2_Pin|LED_TRIM_Pin
                          |LS_D2_Pin|LS_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC_DRDY_Pin */
  GPIO_InitStruct.Pin = ADC_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LS_D4_Pin LS_D3_Pin AQIC_PWR_CTL_Pin LED_OK_Pin
                           LED_NG_Pin */
  GPIO_InitStruct.Pin = LS_D4_Pin|LS_D3_Pin|AQIC_PWR_CTL_Pin|LED_OK_Pin
                          |LED_NG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_POWER_Pin */
  GPIO_InitStruct.Pin = KEY_POWER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_POWER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TXS_OE_Pin */
  GPIO_InitStruct.Pin = TXS_OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TXS_OE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : AS1105_CLK_Pin LTC_DISCHARGE_Pin */
  GPIO_InitStruct.Pin = AS1105_CLK_Pin|LTC_DISCHARGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LS_OE1_Pin */
  GPIO_InitStruct.Pin = LS_OE1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LS_OE1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        HAL_UART_Transmit(huart, &gn_uart_temp_rx, 1, 5);

        if((gn_uart_temp_rx == '\n') || (gn_uart_temp_rx == '\r'))
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            gt_uart_queue[gn_uart_msg_queue_index].param[gt_uart_queue[gn_uart_msg_queue_index].len] = 0;

            gp_uart_queue = gt_uart_queue + gn_uart_msg_queue_index;

            xQueueSendFromISR(xQueue, &gp_uart_queue, &xHigherPriorityTaskWoken);
            /* Now the buffer is empty we can switch context if necessary. */
            if( xHigherPriorityTaskWoken )
            {
                /* Actual macro used here is port specific. */
                portYIELD();
            }

            ++gn_uart_msg_queue_index;
            if(gn_uart_msg_queue_index == 16)
            {
                gn_uart_msg_queue_index = 0;
            }

        }
        else if(gn_uart_temp_rx == 0x08)
        {
            if(gt_uart_queue[gn_uart_msg_queue_index].len)
            {
                uint8_t temp[2] = { ' ', 0x08 };

                --gt_uart_queue[gn_uart_msg_queue_index].len;
                HAL_UART_Transmit(huart, temp, 2, 5);
            }
        }
        else
        {
            gt_uart_queue[gn_uart_msg_queue_index].param[gt_uart_queue[gn_uart_msg_queue_index].len] = gn_uart_temp_rx;
            ++gt_uart_queue[gn_uart_msg_queue_index].len;
        }

        HAL_UART_Receive_IT(&huart2, &gn_uart_temp_rx, 1);
    }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_JigTestMainTask */
/**
  * @brief  Function implementing the JIG_MAIN thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_JigTestMainTask */
void JigTestMainTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
    print("Start JigTestMainTask Task\r\n");

  /* Infinite loop */
    for(;;)
    {
        osDelay(1);
        if(gn_apic_test_type == 0)  /* normal test */
        {
            test_procedure_run();
        }
        else                        /* trimming */
        {
            trimming_procedure_run();
        }
    }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_KeyDetector */
/**
* @brief Function implementing the KeyDetectTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyDetector */
void KeyDetector(void const * argument)
{
  /* USER CODE BEGIN KeyDetector */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END KeyDetector */
}

/* USER CODE BEGIN Header_KeyHandler */
/**
* @brief Function implementing the KeyHandlerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyHandler */
void KeyHandler(void const * argument)
{
  /* USER CODE BEGIN KeyHandler */
#if (osCMSIS < 0x20000U)
    osEvent event;
#else
    osStatus_t status;
#endif
    uint16_t key_event = 0;

    print("Start KeyHandler Task\r\n");

    /* Infinite loop */
    for(;;)
    {
#if (osCMSIS < 0x20000U)
        event = osMessageGet(queue_key_eventHandle, osWaitForever);   // wait for message
        if(event.status == osEventMessage)
        {
            key_event = (uint16_t)event.value.v;
#else
        status = osMessageQueueGet(queue_key_eventHandle, &key_event, NULL, osWaitForever);   // wait for message
        if (status == osOK)
        {
#endif
        }
    }
  /* USER CODE END KeyHandler */
}

/* USER CODE BEGIN Header_TaskDebugUart */
/**
* @brief Function implementing the myTaskDebugUart thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskDebugUart */
void TaskDebugUart(void const * argument)
{
  /* USER CODE BEGIN TaskDebugUart */
    MessageQ *pMsgQ = NULL;

    char str_in[UART_CMD_LENGTH+1] = {0, };
    uint32_t recv_param = 0;
    uint32_t recv_reg_addr = 0;
    uint32_t recv_reg_val = 0;
    uint32_t recv_trim_rang[4] = {0,};
    uint32_t recv_czmn[3] = {0,};

    float recv_trim_volt[6] = {0,};
    float f_voltage = 0.0f;
    // float f_pwm_voltage = 0.0f;

    print("Start TaskDebugUart Task\r\n");

  /* Infinite loop */
  for(;;)
  {
        if(xQueueReceive( xQueue,&pMsgQ,( TickType_t )portMAX_DELAY ) == pdPASS)
        {
            if(UART_CMD_LENGTH < pMsgQ->len)
            {
                pMsgQ->len = pMsgQ->len;
            }

            memcpy(str_in, pMsgQ->param, pMsgQ->len);
            pMsgQ->len = 0;

            if(!strncmp(str_in, "help", sizeof("help")))
            {
                print("\n\r--------------- Command Description ---------------");
                print("\n\r\t-run               : start measurement");

                print("\n\r\t-cz n              : set cz to n [n : 1 ~ 4]");
                print("\n\r\t-cz                : current cz");

                print("\n\r\t-manual volt n     : set manual voltage to n [n : 0 ~ 4.5]");
                print("\n\r\t-manual volt       : current manual voltage");

                print("\n\r\t-manual gain n     : set manual gain to High or Mid or Low");
                print("\n\r\t                     High @ n = 2, up to 80mA");
                print("\n\r\t                     Mid  @ n = 1, up to 10mA");
                print("\n\r\t                     Low  @ n = 0, up to  1mA");

                print("\n\r\t-manual gain       : current manual gain");
                print("\n\r---------------------------------------------------");
            }
            else if(!(strcmp(str_in, "info")))
            {
            }
            else if(!(strcmp(str_in, "run")) || !(strcmp(str_in, "2")))
            {
                gn_apic_test_type = 0;
                test_Procedure_Start();
            }
            else if(!(strcmp(str_in, "trim_start")) || !(strcmp(str_in, "1")))
            {
                gn_apic_test_type = 1;
                trimming_procedure_start();
            }
            else if(!(strcmp(str_in, "cz")))
            {
                snprintf(msg_buffer, sizeof msg_buffer, "\r\n cz_volt : %3.1f \r\n", gf_measure_cz_volt);
                print(msg_buffer);
            }
            else if(sscanf(str_in, "cz %d", &recv_param) == 1)
            {
                if (recv_param == 1)
                {
                    print("\r\n set cz 1 (mode = 0.5V)");
                    gf_measure_cz_volt = CZ_1;
                }
                else if (recv_param == 2)
                {
                    print("\r\n set cz 2 (mode = 1.5V)");
                    gf_measure_cz_volt = CZ_2;
                }
                else if (recv_param == 3)
                {
                    print("\r\n set cz 3 (mode = 2.5V)");
                    gf_measure_cz_volt = CZ_3;
                }
                else if (recv_param == 4)
                {
                    print("\r\n set cz 4 (mode = 3.5V)");
                    gf_measure_cz_volt = CZ_4;
                }
                else
                {
                    print("\r\n invalid cz  [CZ : 1 ~ 4]");
                    gf_measure_cz_volt = CZ_1;
                }
                snprintf(msg_buffer, sizeof msg_buffer, "\r\n cz_volt : %3.1f \r\n", gf_measure_cz_volt);
                print(msg_buffer);
            }
            else if(!(strcmp(str_in, "end point")))
            {
                snprintf(msg_buffer, sizeof msg_buffer, "\n\rEnd Point Of Measure is %3.1f[V]\r\n", gf_measure_end_point);
                print(msg_buffer);
            }
            else if(sscanf(str_in, "end point %f", &f_voltage) == 1)
            {
                if((f_voltage == 0.0f) | (f_voltage == 4.5f))
                {
                    gf_measure_end_point = 4.5f;
                }
                else
                {
                    gf_measure_end_point = f_voltage;
                }
                snprintf(msg_buffer, sizeof msg_buffer, "\n\rSet End Point Of Measure to %3.1f[V]\r\n", gf_measure_end_point);
                print(msg_buffer);
            }
            else if(!(strcmp(str_in, "manual volt")))
            {
                snprintf(msg_buffer, sizeof msg_buffer, "\n\rManual Test Voltage is %5.3f[V]\r\n", gf_manual_volt);
                print(msg_buffer);
            }
            else if(sscanf(str_in, "manual volt %f", &f_voltage) == 1)
            {
                if((f_voltage == 0.0f) | (f_voltage == 4.5f))
                {
                    gf_manual_volt = 4.5f;
                }
                else
                {
                    gf_manual_volt = f_voltage;
                }
                snprintf(msg_buffer, sizeof msg_buffer, "\n\rSet Manual Test Voltage to %5.3f[V]\r\n", gf_manual_volt);
                print(msg_buffer);
            }
            else if(!(strcmp(str_in, "manual gain")))
            {
                if (gn_manual_gain  == 0)
                {
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rManual gain is LOW\r\n");
                }
                else if (gn_manual_gain  == 1)
                {
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rManual gain is MID\r\n");
                }
                else if (gn_manual_gain  == 2)
                {
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rManual gain is HIGH\r\n");
                }
                print(msg_buffer);
            }
            else if(sscanf(str_in, "manual gain %u", &recv_param) == 1)
            {
                if (recv_param  == 0)
                {
                    gn_manual_gain = GAIN_LOW;
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rSet Manual gain to LOW\r\n");
                }
                else if (recv_param  == 1)
                {
                    gn_manual_gain = GAIN_MID;
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rSet Manual gain to MID\r\n");
                }
                else if (recv_param  == 2)
                {
                    gn_manual_gain = GAIN_HIGH;
                    snprintf(msg_buffer, sizeof msg_buffer, "\n\rSet Manual gain to HIGH\r\n");
                }
                print(msg_buffer);
            }
            else if(!(strcmp(str_in, "vled_7v_on")))
            {
                AQIC_VLED_EN(PWR_ON);
                print("\n\rOK");
            }
            else if(!(strcmp(str_in, "vled_7v_off")))
            {
                AQIC_VLED_EN(PWR_OFF);
                print("\n\rOK");
            }
            else if(!(strcmp(str_in, "trim_get_regs")))
            {
                apic_get_regs_for_trimming();
                print("\n\rOK");
            }
            else if(!(strcmp(str_in, "trim_e2p")))
            {
                apic_program_e2p();
                print("\n\rOK");
            }
            else if(sscanf(str_in, "dac_output %f", &f_voltage) == 1)
            {
                if(f_voltage < 0.0f || f_voltage > 5.0f)
                {
                    print("\n\rInvalid parameter");
                }
                else
                {
                    AQIC_Apply_Voltage(f_voltage, AQIC_D_ALL);
                    print("\n\rOK");
                }
            }
            else if(!(strcmp(str_in, "reset")))
            {
                print("RESET\r\n");
                NVIC_SystemReset();
            }
            else if(strncmp(str_in, CMD_GUI_KEY_POWER_UP, strlen(CMD_GUI_KEY_POWER_UP)) == 0)
            {
                print("KEY_POWER_UP\r\n");
                osMessagePut(queue_key_eventHandle, KEY_POWER_UP, 0U);
            }

            else if(strncmp(str_in, CMD_GUI_KEY_TRIMMING_UP, strlen(CMD_GUI_KEY_TRIMMING_UP)) == 0)
            {
                print("KEY_TRIMMING_UP\r\n");
                osMessagePut(queue_key_eventHandle, KEY_TRIMMING_UP, 0U);
            }

            else if(strncmp(str_in, CMD_GUI_RESET, strlen(CMD_GUI_RESET)) == 0)
            {
                print("CMD_GUI_RESET\r\n");
                NVIC_SystemReset();
            }

            else if(strncmp(str_in, CMD_GUI_ACTIVATE, strlen(CMD_GUI_ACTIVATE)) == 0)
            {
                print("\r\nstart AQ06 TRIMMING_JIG application....\r\n");
                print("CMD_GUI_ACTIVATE\r\n");

                GUI_SET_otp_regs(I2C_AQIC_06);
                GUI_SET_init_reg_tbl(I2C_AQIC_06);
                GUI_SET_Activate();
            }

            // else if(strncmp(str_in, CMD_GUI_TRIMSTART, strlen(CMD_GUI_TRIMSTART)) == 0)
            else if(sscanf(str_in, "ui:gtrim %d", &recv_param) == 1)
            {
                print("CMD_GUI_TRIMSTART\r\n");
                GUI_SET_TrimStart(recv_param);
            }

            else if(strncmp(str_in, CMD_GUI_READ_REGISTER_ALL, strlen(CMD_GUI_READ_REGISTER_ALL)) == 0)
            {
                GUI_SET_RegisterReadAll();
                GUI_SEND_reg_all();
            }

            else if(sscanf(str_in, "ui:gwreg %x %x", &recv_reg_addr, &recv_reg_val) == 2)
            {
                GUI_write_reg(recv_reg_addr, recv_reg_val);
                // GUI_SEND_reg_all();
            }

            else if(sscanf(str_in, "ui:gtren %x", &recv_reg_val) == 1)
            {
                GUI_trim_enable(recv_reg_val);
            }

            else if(sscanf(str_in, "ui:din %d", &recv_param) == 1)
            {
                print("CMD_GUI_DIN_MODE\r\n");
                GUI_din_mode_enable(recv_param);
            }

            else if(sscanf(str_in, "ui:gvolt %f %f %f %f %f %f",
            &recv_trim_volt[0], &recv_trim_volt[1], &recv_trim_volt[2],
            &recv_trim_volt[3], &recv_trim_volt[4], &recv_trim_volt[5] ) == 6)
            {
                GUI_trim_VoltageTbl(recv_trim_volt);
            }

            else if(sscanf(str_in, "ui:grang %u %u %u %u", &recv_trim_rang[0], &recv_trim_rang[1], &recv_trim_rang[2], &recv_trim_rang[3]) == 4)
            {
                GUI_trim_RangeTbl(recv_trim_rang);
            }

            else if(sscanf(str_in, "ui:czmn %u %u %u", &recv_czmn[0], &recv_czmn[1], &recv_czmn[2]) == 3)
            {
                GUI_CZmax_CZn_Set(recv_czmn[0], recv_czmn[1], recv_czmn[2]);
            }

            else if(sscanf(str_in, "ui:gainl %u", &recv_param) == 1)
            {
                GUI_GAIN_LEVEL(recv_param);
            }

            else
            {
                print("\n\rWhat? \0");
            }

            memset(str_in, 0x00, sizeof(str_in));
        }
  }
  /* USER CODE END TaskDebugUart */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
#if 0
    else if(TIM2 == htim->Instance)  /* 3us, for Gate pulse */
    {
        timer_for_amic_gate_pulse();
    }
    else if(TIM4 == htim->Instance)  /* 2083us, 480Hz  */
    {
        timer_for_virtual_vsync();
    }
    else if(TIM5 == htim->Instance)  /* 347us, 2083/6ch  */
    {
        timer_for_output_channel();
    }
    else if(TIM9 == htim->Instance)  /* 80us,  */
    {
        timer_for_i_t_voltage();
    }
    else if(TIM13 == htim->Instance)  /* ,  */
    {
        timer_for_end();
    }
#endif
  /* USER CODE END Callback 1 */
}

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
