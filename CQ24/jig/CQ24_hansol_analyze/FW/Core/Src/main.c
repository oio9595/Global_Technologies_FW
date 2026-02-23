/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "common.h"
#include "cqic_test.h"
#include "log.h"
#include "user_flash.h"
#include "fnd.h"
#include "ADS124S08.h"
#include "cqic.h"

#include "my_i2c.h"
#include "trimming.h"
//#include "type.h"

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
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart2;

osThreadId JIG_MAINHandle;
osThreadId KeyDetectTaskHandle;
osThreadId KeyHandlerTaskHandle;
osThreadId myTaskDebugUartHandle;
osMessageQId queue_key_eventHandle;
osTimerId BeepPlayTimerHandle;
/* USER CODE BEGIN PV */

//osMessageQId Queue_Handle;
//osPoolId Pool_ID;
#define UART_CMD_LENGTH     60
typedef struct {
    uint8_t len;
    uint8_t param[UART_CMD_LENGTH];
}MessageQ;

static uint8_t gn_uart_msg_queue_index;
static MessageQ gt_uart_queue[16];
static MessageQ* gp_uart_queue;

QueueHandle_t xQueue = NULL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM12_Init(void);
void JigTaskMainTask(void const * argument);
void KeyDetector(void const * argument);
void KeyHandler(void const * argument);
void TaskDebugUart(void const * argument);
void TimerCallback(void const * argument);

/* USER CODE BEGIN PFP */
void PlayBeep(beep_freq_t t_type, uint16_t n_time);
void dac_set_voltage(float f_voltage, uint8_t ch);

extern void set_ok_led(uint8_t en);
extern void cqic_init(void);
extern void cqic_trim_data_to_otp(void);

uint8_t trimming_stage = 0;

volatile uint8_t vcc_delay = 0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static power_state_t gt_power_state;
auto_manual_t gt_auto_manual_state;
extern test_mode_t gt_jig_test_mode;
extern cqic_test_step_t gt_jig_trimming_step;
extern uint16_t gt_cqic_test_step;
extern uint8_t cqic_test_mode;
extern volatile uint16_t gt_jig_check_vsync;

typedef struct _beep_info_
{
    beep_freq_t t_beep_type;
    beep_play_t t_play_step;
    uint16_t n_beep_count;
}beep_into_t;

static beep_into_t gt_beep_info;
static uint32_t gn_beep_freq_tim12_period[BEEP_MAX] =
{
    tim12_c4_arr,
    tim12_d4_arr,
    tim12_e4_arr,
    tim12_f4_arr,
    tim12_g4_arr,
    tim12_a4_arr,
    tim12_b4_arr,
    tim12_c5_arr,
};

static uint8_t gn_debug_rx_data[1];
uint8_t gn_cqic_test_type;      /* 0 : normal test, 1 : 1 : trimming */

void PlayBeep(beep_freq_t t_type, uint16_t n_time)
{

#if 1
    //snprintf(msg_buffer, sizeof msg_buffer, "===> PlayBeep()... %u %u => %u %u %u \r\n", t_type, n_time, gt_beep_info.t_beep_type, gt_beep_info.t_play_step, gt_beep_info.n_beep_count);
    //print(msg_buffer);

    if(gt_beep_info.t_play_step == BEEP_PLAYING || gt_beep_info.t_play_step == BEEP_NG_PLAYING)
    {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
        osTimerStop(BeepPlayTimerHandle);
    }
#endif
    gt_beep_info.t_beep_type = t_type;

    if(KEY_TEST_NG_BEEP == t_type)
    {
        gt_beep_info.t_play_step = BEEP_NG_START;
    }
    else
    {
        gt_beep_info.t_play_step = BEEP_START;
    }
    gt_beep_info.n_beep_count = n_time;

    osTimerStart(BeepPlayTimerHandle, 1);
}

void dac_set_voltage(float f_voltage, uint8_t ch)
{
	return;
}

void GT_TIM5_Callback(void)
{
	HAL_TIM_Base_Stop_IT(&htim5);
	VSYNC_OUT_LO();
}

void GT_TIM2_Callback(void)
{
	HAL_TIM_Base_Start_IT(&htim5);
	VSYNC_OUT_HI();
	gt_jig_check_vsync = 1;
}

void cq_set_mclk_cs_float(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Configure GPIO pin : MCLK_Pin */
    GPIO_InitStruct.Pin = MCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_RESET);
}

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

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */

  print("\r\nstart CQ24 TRIMMING_JIG application....\r\n");

  snprintf(msg_buffer, sizeof msg_buffer, "\r\nBuild time : %s / %s\r\n", __DATE__, __TIME__ );
  print(msg_buffer);

  HAL_TIM_Base_Stop_IT(&htim5); /* vsync_out hold timer */

#ifdef EXTERNAL_CLK_DEF	/* for external clock */
  HAL_TIM_Base_Stop_IT(&htim2);	/* vsync_out timer */
#else
  HAL_TIM_Base_Start_IT(&htim2);	/* vsync_out timer */
#endif

	// HAL_TIM_Base_Start_IT(&htim2);	/* vsync_out timer */ Function TEST

  ads114s08_set_reset(ADC_CS_ALL);
  HAL_Delay(2);   /* Delay time : 1ms[4096tCLK(tCLK = 1 / fCLK(4.096MHz)], first SCLK rising edge after RESET rising edge (or 7th SCLK falling edge of RESET command) */
  ads114s08_init();

  //cqic_init();	/* cqic init */
    cq_set_mclk_cs_float();

	set_cqic_5_0V_on();
    vcc_delay = 10;

#if 1
  HAL_UART_Receive_IT(&huart2, gn_debug_rx_data, 1);
  HAL_TIM_Base_Start(&htim4);
#endif

  gt_cqic_test_step = TEST_STEP_NONE;
  gt_power_state = PWR_OFF;

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of BeepPlayTimer */
  osTimerDef(BeepPlayTimer, TimerCallback);
  BeepPlayTimerHandle = osTimerCreate(osTimer(BeepPlayTimer), osTimerPeriodic, (void*) &gt_beep_info);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
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
  osThreadDef(JIG_MAIN, JigTaskMainTask, osPriorityNormal, 0, 512);
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
  HAL_Delay(300);

  /* for UI I/F*/
  snprintf(msg_buffer, sizeof msg_buffer, "Build time : %s / %s\r\n", __DATE__, __TIME__ );
  print(msg_buffer);

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
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 194;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 4;
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
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLLI2SCLK, RCC_MCODIV_4);
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_PLLI2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 194;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 6;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 4;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  hspi2.Init.DataSize = SPI_DATASIZE_16BIT;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = 538888;//646666;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 646;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM12 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM12_Init(void)
{

  /* USER CODE BEGIN TIM12_Init 0 */

  /* USER CODE END TIM12_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 89;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 3866;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1933;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */
  HAL_TIM_MspPostInit(&htim12);
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  HAL_GPIO_WritePin(GPIOA, ADC1_START_Pin|AQIC_PWR_CTL_Pin|CQ24_5_0V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_TRIM_Pin|TEST_1_Pin|CQ24_5_7V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, VSYNC_IN_Pin|TEST_2_Pin|ADC2_START_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ADC1_CS_Pin|CQ24_VCC_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NSCS2_Pin|ADC_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ADC2_CS_GPIO_Port, ADC2_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : B1_Pin ADC1_DRDY_Pin */
  GPIO_InitStruct.Pin = B1_Pin|ADC1_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ADC1_START_Pin NSCS2_Pin CQ24_5_0V_Pin ADC_RESET_Pin */
  GPIO_InitStruct.Pin = ADC1_START_Pin|NSCS2_Pin|CQ24_5_0V_Pin|ADC_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : AQIC_PWR_CTL_Pin */
  GPIO_InitStruct.Pin = AQIC_PWR_CTL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AQIC_PWR_CTL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_TRIM_Pin TEST_1_Pin NSCS1_Pin CQ24_5_7V_Pin */
  GPIO_InitStruct.Pin = LED_TRIM_Pin|TEST_1_Pin|NSCS1_Pin|CQ24_5_7V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC2_DRDY_Pin */
  GPIO_InitStruct.Pin = ADC2_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC2_DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : VSYNC_IN_Pin ADC1_CS_Pin TEST_2_Pin CQ24_VCC_EN_Pin
                           ADC2_START_Pin */
  GPIO_InitStruct.Pin = VSYNC_IN_Pin|ADC1_CS_Pin|TEST_2_Pin|CQ24_VCC_EN_Pin
                          |ADC2_START_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_MODE_Pin */
  GPIO_InitStruct.Pin = KEY_MODE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_MODE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MCLK_Pin */
  GPIO_InitStruct.Pin = MCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_RUN_Pin */
  GPIO_InitStruct.Pin = KEY_RUN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_RUN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC2_CS_Pin */
  GPIO_InitStruct.Pin = ADC2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ADC2_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        HAL_UART_Transmit(huart, gn_debug_rx_data, 1, 5);

        if((gn_debug_rx_data[0] == '\n') || (gn_debug_rx_data[0] == '\r'))
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
        else if(gn_debug_rx_data[0] == 0x08)
        {
            if(gt_uart_queue[gn_uart_msg_queue_index].len)
            {
                uint8_t temp[2] = {' ', 0x08 };

                --gt_uart_queue[gn_uart_msg_queue_index].len;
                HAL_UART_Transmit(huart, temp, 2, 5);
            }
        }
        else
        {
            gt_uart_queue[gn_uart_msg_queue_index].param[gt_uart_queue[gn_uart_msg_queue_index].len] = gn_debug_rx_data[0];
            ++gt_uart_queue[gn_uart_msg_queue_index].len;
        }
        HAL_UART_Receive_IT(&huart2, gn_debug_rx_data, 1);
    }
}

void GUI_Reset_test_state()
{
    gn_cqic_test_type ^= 0x01; //TRIMMING MODE
    gt_power_state ^= 0x01;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_JigTaskMainTask */
/**
  * @brief  Function implementing the JIG_MAIN thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_JigTaskMainTask */
void JigTaskMainTask(void const * argument)
{
  /* USER CODE BEGIN 5 */

    print("\r\n Start JigTaskMainTask Task\r\n");

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);

    if(gt_power_state == PWR_ON) 	/*  power on */
    {
        test_procedure_run();
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
    uint16_t key_event=KEY_NONE;

#if (osCMSIS < 0x20000U)
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 20;
#endif
    GPIO_PinState key_state;

    volatile GPIO_PinState key_mode_state = GPIO_PIN_SET;
    volatile GPIO_PinState key_test_start_state = GPIO_PIN_SET;
    volatile GPIO_PinState key_b1_state = GPIO_PIN_SET;

    print("\n\r Start KeyDeterctor Task");

    /* Infinite loop */
    for(;;)
    {
#if (osCMSIS < 0x20000U)
        osDelayUntil(&xLastWakeTime, xFrequency);
#else
        osDelayUntil(20);
#endif

        key_state = HAL_GPIO_ReadPin(KEY_MODE_GPIO_Port, KEY_MODE_Pin);
        if(key_mode_state != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
                key_event = KEY_MODE_UP;
                PlayBeep(KEY_PUSH_RELEASE_BEEP, KEY_BEEP_TIME);
#if (osCMSIS < 0x20000U)
                osMessagePut(queue_key_eventHandle, key_event, 0U);
#else
                osMessageQueuePut(queue_key_eventHandle, &key_event, 0U, 0U);
#endif
            }
            key_mode_state = key_state;
        }

        key_state = HAL_GPIO_ReadPin(KEY_RUN_GPIO_Port, KEY_RUN_Pin);
        if(key_test_start_state != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
#if 1
                key_event = KEY_RUN_UP;
                PlayBeep(KEY_PUSH_RELEASE_BEEP, KEY_BEEP_TIME);
#if (osCMSIS < 0x20000U)
                osMessagePut(queue_key_eventHandle, key_event, 0U);
#else
                osMessageQueuePut(queue_key_eventHandle, &key_event, 0U, 0U);
#endif
#else
                print("\r\n-CQIC_ACTIVATE_AND_START-\r\n");
#endif
            }
            key_test_start_state = key_state;
        }

        key_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
        if(key_b1_state != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
                key_event = KEY_B1_UP;
                PlayBeep(KEY_PUSH_RELEASE_BEEP, KEY_BEEP_TIME);
#if (osCMSIS < 0x20000U)
                osMessagePut(queue_key_eventHandle, key_event, 0U);
#else
                osMessageQueuePut(queue_key_eventHandle, &key_event, 0U, 0U);
#endif
            }
            key_b1_state = key_state;
        }
    }
  /* USER CODE END KeyDeterctor */
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

    print("\r\n Start KeyHandler Task\r\n");

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
            switch(key_event)
            {
            	case KEY_MODE_UP:
					if(gt_cqic_test_step == TEST_STEP_NONE)
					{
						// gn_cqic_test_type ^= 0x01;

						// snprintf(msg_buffer, sizeof msg_buffer, "\n\r CQIC TEST MODE [%s]\n\r",(gn_cqic_test_type == 1 ? "TRIMMING" : "FUNCTIONAL"));
						// print(msg_buffer);

						print("\r\n-CQIC_ACTIVATE_AND_START-\r\n");
					}
					break;
				case KEY_RUN_UP:
					//if(gt_cqic_test_step == TEST_STEP_NONE)
					{
						gt_power_state ^= 0x01;
						set_ok_led(gt_power_state);

						//snprintf(msg_buffer, sizeof msg_buffer, "POWER STATE [%s]\n\r",(gt_power_state == 1 ? "ON" : "OFF"));
						//print(msg_buffer);

                        if(gt_power_state)
                        {
                            gt_cqic_test_step = MODE_TEST_START;
                        }
                        else
                        {
                            set_cqic_5_0V_off();
                            set_cqic_power_off();
                            HAL_TIM_Base_Stop_IT(&htim2);
                            HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_RESET);
                            uint16_t mosi_temp = 0x00;
                            spi_write(&mosi_temp, 1);
                        }

						if(gt_power_state==0)
                        {
                            print("\n\r POWER STATE OFF \n\r");
// jun
#if 1
                            GPIO_InitTypeDef GPIO_InitStruct = {0};

                            GPIO_InitStruct.Pin = MCLK_Pin;
                            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                            GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                            HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

                            HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_RESET);
#endif
                        }
					}
					break;
				case KEY_B1_UP:
					//if((gt_cqic_test_step == TEST_STEP_NONE) && (gt_power_state == PWR_OFF))
					if(gt_power_state == PWR_OFF)
					{
						if(gn_cqic_test_type == 1)	//trimming test mode
						{
#if 1
							if(trimming_stage==0)
							{
								trimming_stage = 1;
								gt_cqic_test_step = TRIMMING_STEP05_00;	// mode test
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 1 = MODE 2.5V stage(%d) step(%d)",trimming_stage, gt_cqic_test_step );
							}
							else if(trimming_stage == 1)
							{
								trimming_stage = 2;
								gt_cqic_test_step = TRIMMING_STEP00_00; // LDO 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 2 = VREFO 4.5V stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 2)
							{
								trimming_stage = 3;
								gt_cqic_test_step = TRIMMING_STEP02_00; // DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 3 = 12BIT V_REF OFFSET(Delta) stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 3)
							{
								trimming_stage = 4;
								gt_cqic_test_step = TRIMMING_STEP03_00; // DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 4 = 12BIT DAC OFFSET stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 4)
							{
								trimming_stage = 5;
								gt_cqic_test_step = TRIMMING_STEP04_00; // DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 5 = SLOP [12BIT DAC 0~4095 stage(%d) step(%d)]", trimming_stage, gt_cqic_test_step);
							}
 							else if(trimming_stage == 5)
							{
								trimming_stage = 0;
								gt_cqic_test_step = TRIMMING_STEP_OTP_WRITE; // OTP WRITE MODE
								snprintf(msg_buffer, sizeof msg_buffer, "TRIMMING STEP - 6 = OTP WRITE state(%d), step(%d)", trimming_stage, gt_cqic_test_step);
							}

							print(msg_buffer);
              print("\r\n");
#else
							if(trimming_stage == 0)
							{
								trimming_stage = 1;
								gt_cqic_test_step = TRIMMING_STEP00_00;		// LDO 1.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = REF LDO 1.57V stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 1)
							{
								trimming_stage = 2;
								gt_cqic_test_step = TRIMMING_STEP01_10;// LDO 1.50V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = REF LDO 1.50V stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}

							else if(trimming_stage == 2)
							{
								trimming_stage = 3;
								gt_cqic_test_step = TRIMMING_STEP12_00;// DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = 10BIT DAC Linear stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}

							else if(trimming_stage == 3)
							{
								trimming_stage = 4;
								gt_cqic_test_step = TRIMMING_STEP02_00;// DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = 10BIT DAC Delta stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 4)
							{
								trimming_stage = 5;
								gt_cqic_test_step = TRIMMING_STEP03_00;// DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = 12BIT DAC Linear stage(%d) step(%d)", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage == 5)
							{
								trimming_stage = 6;
								gt_cqic_test_step = TRIMMING_STEP04_00;// DAC 4.5V trimming mode
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = SLOP [12BIT DAC 0~4095 stage(%d) step(%d)]", trimming_stage, gt_cqic_test_step);
							}
							else if(trimming_stage==6)
							{
								trimming_stage = 7;
								gt_cqic_test_step = TRIMMING_STEP05_00;	// mode test
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r TRIMMING STEP = MODE");
							}
 							else if(trimming_stage == 7)
							{
								trimming_stage = 0;
								gt_cqic_test_step = TRIMMING_STEP_OTP_WRITE;  // OTP WRITE MODE
								snprintf(msg_buffer, sizeof msg_buffer, "\n\r OTP WRITE state(%d), step(%d)", trimming_stage, gt_cqic_test_step);
							}
							print(msg_buffer);
#endif
						}
						else	//funtional test mode
						{
#if 0
							if(++cqic_test_mode > 5)
								cqic_test_mode = 0;
#else
							cqic_test_mode = 0;
							gt_cqic_test_step = MODE_TEST_START;
#endif
							snprintf(msg_buffer, sizeof msg_buffer, "\n\r cqic functional test mode [%d]", cqic_test_mode);
							print(msg_buffer);
						}
					}
					break;

                default: /* unknown key event */
                    break;
            }
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
    float f_voltage = 0.0f;
    float f_pwm_voltage = 0.0f;
    uint32_t recv_reg_addr = 0;
    uint32_t recv_reg_val = 0;

	cqic_cmd1_type cmd1 = {0, };
    cqic_cmd2_type cmd2 = {0, };

    print("Start TaskDebugUart Task\r\n");

  /* Infinite loop */
  for(;;)
  {
    if(xQueueReceive(xQueue, &pMsgQ, (TickType_t)portMAX_DELAY) == pdPASS)
    {
        if(UART_CMD_LENGTH < pMsgQ->len)
        {
            pMsgQ->len = pMsgQ->len;
        }

        memcpy(str_in, pMsgQ->param, pMsgQ->len);
        pMsgQ->len = 0;

        if((!(strcmp(str_in, "help"))) || (!(strcmp(str_in, "?"))))
        {
        }
        else if(!(strcmp(str_in, "info")))
        {
        }
        else if(strncmp(str_in, CMD_GUI_RESET, strlen(CMD_GUI_RESET)) == 0)
        {
            print("CMD_GUI_RESET\r\n");
            NVIC_SystemReset();
        }
        else if(strncmp(str_in, CMD_GUI_ACTIVATE, strlen(CMD_GUI_ACTIVATE)) == 0)
        {
            gt_cqic_test_init();
        }
        else if(strncmp(str_in, CMD_GUI_READ_REGISTER_ALL, strlen(CMD_GUI_READ_REGISTER_ALL)) == 0)
        {
            GUI_SEND_reg_all();
        }
        else if(sscanf(str_in, "ui:1wreg %x %x", &recv_reg_addr, &recv_reg_val) == 2)
        {
            GUI_write_reg_cmd1(recv_reg_addr, recv_reg_val);
        }
        else if(sscanf(str_in, "ui:2wreg %x %x", &recv_reg_addr, &recv_reg_val) == 2)
        {
            GUI_write_reg_cmd2(recv_reg_addr, recv_reg_val);
        }
        else if(sscanf(str_in, "ui:2wreg %x %x", &recv_reg_addr, &recv_reg_val) == 2)
        {
            GUI_write_reg_cmd2(recv_reg_addr, recv_reg_val);
        }
        else if(sscanf(str_in, "ui:din %u", &gb_NO_TRIM) == 1)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gb_NO_TRIM:%d\r\n ", gb_NO_TRIM);
            print(msg_buffer);
        }
        else if(sscanf(str_in, "ui:ctrim1 %f %f %f %f", &gf_MODE_DAC_Target, &gf_MODE_DAC_Step, &gf_VREFP_Target, &gf_VREFP_Step) == 4)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gf_MODE_DAC_Target:%f \r\n gf_MODE_DAC_Step:%f \r\n gf_VREFP_Target:%f \r\n gf_VREFP_Step:%f ",
                      gf_MODE_DAC_Target, gf_MODE_DAC_Step, gf_VREFP_Target, gf_VREFP_Step);
            print(msg_buffer);
        }
        else if(sscanf(str_in, "ui:ctrim2 %u %f %u %u %f", &gui_GAIN_DAC_DAC_Input, &gf_GAIN_DAC_Step, &gui_GAIN_DAC_vHigh, &gui_GAIN_DAC_vLow, &gf_GAIN_DAC_Delta) == 5)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gui_GAIN_DAC_DAC_Input:%d \r\n gf_GAIN_DAC_Step:%f \r\n gui_GAIN_DAC_vHigh:%d \r\n gui_GAIN_DAC_vLow:%d \r\n gf_GAIN_DAC_Delta:%f\r\n",
                      gui_GAIN_DAC_DAC_Input, gf_GAIN_DAC_Step, gui_GAIN_DAC_vHigh, gui_GAIN_DAC_vLow, gf_GAIN_DAC_Delta);
            print(msg_buffer);
        }
        else if(sscanf(str_in, "ui:ctrim3 %u %f %u %f", &gui_DATA_DAC_DAC_Input, &gf_DATA_DAC_Step, &gui_DATA_DAC_Target_Offset_DAC, &gf_DATA_DAC_Voltage) == 4)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gui_DATA_DAC_DAC_Input:%d \r\n gf_DATA_DAC_Step:%f \r\n gui_DATA_DAC_Target_Offset_DAC:%d \r\n gf_DATA_DAC_Voltage:%f \r\n",
                      gui_DATA_DAC_DAC_Input, gf_DATA_DAC_Step, gui_DATA_DAC_Target_Offset_DAC, gf_DATA_DAC_Voltage);
            print(msg_buffer);
        }
        else if(sscanf(str_in, "ui:cslop %d %d %d %d", &gui_SLOPE_GAIN_DAC_INPUT, &gui_TRIM_RANGE_MIN, &gui_TRIM_RANGE_MAX, &gb_OTP_WRITE) == 4)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gui_SLOPE_GAIN_DAC_INPUT:%d gui_TRIM_RANGE_MIN:%d gui_TRIM_RANGE_MAX:%d gb_OTP_WRITE:%d\r\n",
                                                      gui_SLOPE_GAIN_DAC_INPUT, gui_TRIM_RANGE_MIN, gui_TRIM_RANGE_MAX, gb_OTP_WRITE);
            print(msg_buffer);
        }
        else if(sscanf(str_in, "ui:gtrim %d", &gui_TRIM_regi_changed) == 1)
        {
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n gui_TRIM_regi_changed:%d \r\n", gui_TRIM_regi_changed);
            print(msg_buffer);

            if(gb_NO_TRIM == TRUE)
            {
                gt_cqic_test_step = TRIMMING_STEP04_00;
            }
            else
            {
                gt_cqic_test_step = TRIMMING_STEP05_00;
            }

            gn_cqic_test_type ^= 0x01; //TRIMMING MODE
            gt_power_state ^= 0x01;
        }
        else if(!(strcmp(str_in, "cq_r_all")))
        {
            cqic_reg_read_all();
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "cq_reset")))
        {
            cmd1.u.cmd_id = CMD_01;
            cmd1.u.rw = SPI_WR;
            cmd1.u.addr = 0x00;
            cmd1.u.data = 0x001;
            cqic_write(CQIC_1, &cmd1.value, 1);
            HAL_Delay(10);

            cmd1.u.cmd_id = CMD_01;
            cmd1.u.rw = SPI_WR;
            cmd1.u.addr = 0x00;
            cmd1.u.data = 0x000;
            cqic_write(CQIC_1, &cmd1.value, 1);
            HAL_Delay(3);
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "cq_otp_disable")))
        {
            cmd2.u.cmd_id	= CMD_02;
            cmd2.u.rw		= SPI_WR;
            cmd2.u.addr 	= OTP_PROTECTION;
            cmd2.u.data 	= 0xA5A5;
            cqic_write(CQIC_1, cmd2.value, 2);
            HAL_Delay(1);
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "vref %d", &recv_param) == 1)
        {
            cmd2.u.cmd_id	= CMD_02;
        	cmd2.u.addr 	= V_ANA_GAIN;
            cmd2.u.rw		= SPI_WR;
            cmd2.u.data 	= recv_param;
            cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
            HAL_Delay(1);
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "cq_w1 %x %x", &recv_reg_addr, &recv_reg_val) == 2)
        {
            gn_cq_write_target_cmd1_addr = recv_reg_addr;
            gn_cq_write_target_cmd1_data = recv_reg_val;
            gb_cq_write_cmd1_flag = true;
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "cq_w2 %x %x", &recv_reg_addr, &recv_reg_val) == 2)
        {
            gn_cq_write_target_cmd2_addr = recv_reg_addr;
            gn_cq_write_target_cmd2_data = recv_reg_val;
            gb_cq_write_cmd2_flag = true;
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "cq_r %x", &recv_reg_addr) == 1)
        {
            gn_cq_read_target_addr = recv_reg_addr;
            gb_cq_read_flag = true;
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "duty")))
        {
            uint16_t* p_duty_addr = get_duty_addr();
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n duty - 0x%02X", ((*(p_duty_addr + 1)) & 0xFF));
            print(msg_buffer);
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "duty %x", &recv_param) == 1)
        {
            uint16_t* p_duty_addr = get_duty_addr();
            for (uint16_t i = 0 ; i < CQ_DUTY_SIZE ; ++i)
            {
                *(p_duty_addr + i + 1) = (recv_param << 8 | recv_param);
            }
            #if 0
                for (uint16_t i = 0 ; i < 10 ; ++i)
                {
                    *(p_duty_addr + i + 1 +  50) = 0;
                    *(p_duty_addr + i + 1 + 110) = 0;
                }
            #endif
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "ldim")))
        {
            uint16_t* p_pwm_addr = get_ldim_addr();
            snprintf(msg_buffer, sizeof msg_buffer, "\r\n ldim - 0x%04X", *(p_pwm_addr + 1));
            print(msg_buffer);
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "ldim %x", &recv_param) == 1)
        {
            uint16_t* p_pwm_addr = get_ldim_addr();
            for (uint16_t i = 0 ; i < CQ_LDIM_SIZE ; ++i)
            {
                *(p_pwm_addr + i) = recv_param;
            }
            print("\n\rOK \0");
        }
// jun
        else if(sscanf(str_in, "cq_vcc %d", &recv_param) == 1)
        {
            if (recv_param)
            {
                set_cqic_5_0V_on();
                set_cqic_power_on();
                //HAL_Delay(vcc_delay);
                /* MCLK on, CS on */
                GPIO_InitTypeDef GPIO_InitStruct = {0, };
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
                HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_SET);
                HAL_Delay(3);
#if 0
                /* MCLK off, CS off */
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
                HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_RESET);
                HAL_Delay(6);
#endif
                /* MCLK on, CS on */
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
                HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_SET);
            }
            else
            {
                set_cqic_5_0V_off();
                set_cqic_power_off();

                GPIO_InitTypeDef GPIO_InitStruct = {0, };
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
                HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_RESET);
            }
            print("\n\rOK \0");
        }
        else if(sscanf(str_in, "cq_mclk %d", &recv_param) == 1)
        {
            if (recv_param)
            {
                GPIO_InitTypeDef GPIO_InitStruct = {0, };
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
            }
            else
            {
                GPIO_InitTypeDef GPIO_InitStruct = {0, };
                GPIO_InitStruct.Pin = MCLK_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
            }
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "cq_soft_reset")))
        {
            cmd1.u.cmd_id = CMD_01;
            cmd1.u.rw = SPI_WR;
            cmd1.u.addr = 0x00;
            cmd1.u.data = 0x001;
            cqic_write(CQIC_1, &cmd1.value, 1);

            cmd1.u.cmd_id = CMD_01;
            cmd1.u.rw = SPI_WR;
            cmd1.u.addr = 0x00;
            cmd1.u.data = 0x000;
            cqic_write(CQIC_1, &cmd1.value, 1);
            print("\n\rOK \0");
        }
        else if(!(strcmp(str_in, "cq_dump_otp")))
        {
            cmd2.u.cmd_id	= CMD_02;
            cmd2.u.rw		= SPI_WR;
            cmd2.u.addr 	= OTP_PROTECTION;
            cmd2.u.data 	= 0xA5A5;
            cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */

            cmd2.u.cmd_id	= CMD_02;
            cmd2.u.rw		= SPI_WR;
            cmd2.u.addr 	= OTP_RD_PG_CONTROL;
            cmd2.u.data 	= 0x02;
            cqic_write(CQIC_1, cmd2.value, 2);	/* set channel output enable */
            print("\n\rOK \0");
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

/* TimerCallback function */
void TimerCallback(void const * argument)
{
    /* USER CODE BEGIN TimerCallback */
    //snprintf(msg_buffer, sizeof msg_buffer, "===> TimerCallback()... %u %u %u \r\n", gt_beep_info.t_beep_type, gt_beep_info.t_play_step, gt_beep_info.n_beep_count);
    //print(msg_buffer);

    osTimerStop(BeepPlayTimerHandle);

    if(gt_beep_info.t_play_step == BEEP_START)
    {
        gt_beep_info.t_play_step = BEEP_PLAYING;

        osTimerStart(BeepPlayTimerHandle, gt_beep_info.n_beep_count);

        __HAL_TIM_SET_AUTORELOAD(&htim12, gn_beep_freq_tim12_period[gt_beep_info.t_beep_type]);
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, gn_beep_freq_tim12_period[gt_beep_info.t_beep_type] >> 1);
        HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
    }
    else if(gt_beep_info.t_play_step == BEEP_PLAYING)
    {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
        gt_beep_info.t_play_step = BEEP_STOP;
    }
    else if(gt_beep_info.t_play_step == BEEP_NG_START)
    {
        gt_beep_info.t_play_step = BEEP_NG_PLAYING;
        gt_beep_info.n_beep_count -= NG_BEEP_TIME;

        osTimerStart(BeepPlayTimerHandle, NG_BEEP_TIME);

        __HAL_TIM_SET_AUTORELOAD(&htim12, gn_beep_freq_tim12_period[gt_beep_info.t_beep_type]);
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, gn_beep_freq_tim12_period[gt_beep_info.t_beep_type] >> 1);
        HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
    }
    else if(gt_beep_info.t_play_step == BEEP_NG_PLAYING)
    {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);

        if(gt_beep_info.n_beep_count > NG_BEEP_TIME)
        {
            gt_beep_info.n_beep_count -= NG_BEEP_TIME;
            gt_beep_info.t_play_step = BEEP_NG_START;

            osTimerStart(BeepPlayTimerHandle, NG_BEEP_TIME);
        }
        else
        {
            gt_beep_info.t_play_step = BEEP_STOP;
        }
    }
    /* USER CODE END TimerCallback */
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
    if(htim->Instance == TIM2)
    {
        GT_TIM2_Callback();
    }
    if(htim->Instance == TIM5)
    {
        GT_TIM5_Callback();
    }
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
