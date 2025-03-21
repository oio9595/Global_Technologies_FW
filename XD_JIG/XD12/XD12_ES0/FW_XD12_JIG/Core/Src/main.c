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
#define __XDIC_MAIN_C__
#include <stdint.h>
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <float.h>
#include "config.h"

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

#define UART_CMD_LENGTH     60

#define RX_BUFF_SIZE        32
#define RX_PACKET_SIZE      64

typedef struct
{
    uint16_t length;
    char buffer[RX_PACKET_SIZE];
}
rx_packet_t;

typedef struct
{
    uint16_t RxInCnt;
    uint16_t RxOutCnt;
    rx_packet_t Rxbuff[RX_BUFF_SIZE];
}RX_UART_t;

typedef enum
{
    BTN_KEY_POWER = 0,
    BTN_KEY_B1,
    BTN_KEY_TRIM,
    BTN_KEY_MAX,
}BTN_KEY_T;

static RX_UART_t gt_rx_uart;

static uint8_t gn_key_detection_flag;
static uint32_t gn_key_state[BTN_KEY_MAX];

static uint32_t gn_timeout_trimming_procedure_run;
static uint32_t gn_timeout_btn_key_detection;
static uint32_t gn_timeout_dimming_test_procedure_run;

static uint8_t gn_xd12_vsync_tick = 10;
static uint8_t gn_xd12_fault_test_tick = 250;

static double gf_freq_avg;
static double gf_freq_min;
static double gf_freq_max;

static uint32_t gu32_input_freq_capture[FREQ_IN_IC_LENGTH];
uint8_t gu8_freq_input_dma_flg;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM12_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM5_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
void sys_tick_handler(void)
{
    if(gn_timeout_trimming_procedure_run)
    {
        --gn_timeout_trimming_procedure_run;
    }

    if(gn_timeout_btn_key_detection)
    {
        --gn_timeout_btn_key_detection;
    }
    
    if(gn_timeout_dimming_test_procedure_run)
    {
        --gn_timeout_dimming_test_procedure_run;
    }

    if(XD12_get_dimming_init_done_flag())
    {        
        if(gn_xd12_vsync_tick)
        {
            --gn_xd12_vsync_tick;
        }
    }

    if(XD12_get_fault_test_run_flag())
    {        
        if(gn_xd12_fault_test_tick)
        {
            --gn_xd12_fault_test_tick;
        }
    }    
}

void set_input_freq_init(void)
{
    gf_freq_avg = 0;
    gf_freq_min = DBL_MAX;
    gf_freq_max = 0;

    gu8_freq_input_dma_flg = 0;
}

uint32_t get_input_freq(void)
{
    return (uint32_t)(gf_freq_avg + 0.5);
}

void calc_input_freq(void)
{
    if(gu8_freq_input_capture_activated == 1)
    {
        double f_freq = 0;
        double f_freq_avg = 0;
        uint32_t delta = 0;
        uint32_t n_count = 0;

        for(uint32_t i=1 ; i<(FREQ_IN_IC_LENGTH - 1) ; ++i)
        {
            if(gu32_input_freq_capture[i+1] > gu32_input_freq_capture[i+0])
            {
                delta = (gu32_input_freq_capture[i+1] - gu32_input_freq_capture[i+0]);
            }
            else
            {
                delta = (0xFFFFFFFF - gu32_input_freq_capture[i+0]) + gu32_input_freq_capture[i+1] + 1;
            }
            // f_freq = (90000000.0 / delta);
            f_freq = (73750000.0 / delta);
            f_freq_avg += f_freq;
            
            ++n_count;
        }

        gf_freq_avg = (f_freq_avg / n_count);
        if(gf_freq_min > gf_freq_avg)
        {
            gf_freq_min = gf_freq_avg;
        }
        if(gf_freq_max < gf_freq_avg)
        {
            gf_freq_max = gf_freq_avg;
        }

        gu8_freq_input_dma_flg = 0;
        
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, 512);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);
    }
}

static void JigTestMainTask(void);
static void KeyHandler(void);
static void KeyDeterctor(void);
static void TaskDebugUart(void);

void print(const char *fmt, ...)
{
    int len = 0;
    char buffer[4096] = {0, };
    va_list ap;
    
    va_start(ap, fmt);
    len = vsprintf(buffer, fmt, ap);
    va_end(ap);
    for(int i=0 ; i<len ; ++i)
    {
        /* Loop until the end of transmission */
        while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
        /* Echo received character on TX */
        LL_USART_TransmitData8(USART2, (uint8_t)buffer[i]);
    }
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

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM12_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_TIM5_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  
    print("\r\nstart %s TRIMMING_JIG application....\r\n", TARGET_CHIP_NAME);
    print("Build time : %s / %s\r\n", __DATE__, __TIME__ );
    
    /* for GUI App */
    print("XD12_TYPE2 IS SELECTED!\r\n");

    TargetIC_IF_PowerOn_Init();

    ads114s08_init();

    LL_TIM_EnableCounter(TIM1); /* PWM Input or Output for ... */
    LL_TIM_EnableCounter(TIM5); /* for Freq Input */

    gn_key_state[BTN_KEY_POWER] = LL_GPIO_IsInputPinSet(KEY_POWER_GPIO_Port, KEY_POWER_Pin);
    gn_key_state[BTN_KEY_B1]    = LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin);
    gn_key_state[BTN_KEY_TRIM]  = LL_GPIO_IsInputPinSet(KEY_TRIMMING_GPIO_Port, KEY_TRIMMING_Pin);

    /* DMA2_Stream2_IRQn interrupt configuration */
    NVIC_SetPriority(DMA2_Stream2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        JigTestMainTask();
        KeyHandler();
        TaskDebugUart();
        KeyDeterctor();
        if (!gn_xd12_vsync_tick)
        {
            JigBd_IF_SyncGen_Command();
            XD12_tx_LD_Data(XD12_get_PWM());
            if (XD12_get_ch_change_done_flag())
            {
                XD12_set_ch_change_done_flag(0);
                XD12_set_LD_tx_done_flag(1);
            }
            gn_xd12_vsync_tick = 10;
        }
        if (!gn_xd12_fault_test_tick)
        {
            XD12_get_fault_status();
            gn_xd12_fault_test_tick = 250;
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 295;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLLI2SCLK, RCC_MCODIV_5);
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
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 295;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
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

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**ADC1 GPIO Configuration
  PB0   ------> ADC1_IN8
  */
  GPIO_InitStruct.Pin = XD12_ADO_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(XD12_ADO_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_8, LL_ADC_SAMPLINGTIME_480CYCLES);
  /* USER CODE BEGIN ADC1_Init 2 */

    LL_ADC_Enable(ADC1);

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

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA6   ------> SPI1_MISO
  PA7   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_16BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV4;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI1_Init 2 */

    //LL_SPI_Enable(SPI1);

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

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**SPI2 GPIO Configuration
  PC1   ------> SPI2_MOSI
  PC2   ------> SPI2_MISO
  PB10   ------> SPI2_SCK
  */
  GPIO_InitStruct.Pin = ADC_SPI_MOSI_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(ADC_SPI_MOSI_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ADC_SPI_MISO_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(ADC_SPI_MISO_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ADC_SPI_CLK_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(ADC_SPI_CLK_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI2, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

  /* TIM1 DMA Init */

  /* TIM1_CH1 Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_1, LL_DMA_CHANNEL_6);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_1, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_1, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_1, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_1, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_1, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_1, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_1, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_1);

  /* USER CODE BEGIN TIM1_Init 1 */

    /* TIM1_CH2 Init */
    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_6);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_2, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_2);

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 129;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM1);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 0;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_ENABLE;
  LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM1_Init 2 */

    LL_TIM_IC_SetActiveInput(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_FALLING);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&(TIM1->CCR1)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)gu16_pwm_risingBuffer);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)(&(TIM1->CCR2)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)gu16_pwm_fallingBuffer);

    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
    //LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_1);

    LL_TIM_EnableDMAReq_CC1(TIM1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);

    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_2);
    //LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_2);

    LL_TIM_DisableDMAReq_CC2(TIM1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);

    LL_DMA_ClearFlag_FE1(DMA2);
    LL_DMA_ClearFlag_FE2(DMA2);
    LL_DMA_ClearFlag_HT1(DMA2);
    LL_DMA_ClearFlag_HT2(DMA2);
    LL_DMA_ClearFlag_TC1(DMA2);
    LL_DMA_ClearFlag_TC2(DMA2);

  /* USER CODE END TIM1_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM1 GPIO Configuration
  PA8   ------> TIM1_CH1
  */
  GPIO_InitStruct.Pin = PWM_OUT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(PWM_OUT_GPIO_Port, &GPIO_InitStruct);

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

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 8;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 81;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 40;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**TIM3 GPIO Configuration
  PC6   ------> TIM3_CH1
  */
  GPIO_InitStruct.Pin = BUZZER_PWM_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(BUZZER_PWM_GPIO_Port, &GPIO_InitStruct);

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

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM5 GPIO Configuration
  PA0-WKUP   ------> TIM5_CH1
  */
  GPIO_InitStruct.Pin = DOUT_Q4_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(DOUT_Q4_GPIO_Port, &GPIO_InitStruct);

  /* TIM5 DMA Init */

  /* TIM5_CH1 Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_2, LL_DMA_CHANNEL_6);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_2, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_2, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_WORD);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_WORD);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_2);

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM5, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM5);
  LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM5);
  LL_TIM_IC_SetActiveInput(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM5, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
  /* USER CODE BEGIN TIM5_Init 2 */

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)(&(TIM5->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_2, (uint32_t)gu32_input_freq_capture);

    LL_TIM_EnableDMAReq_CC1(TIM5);
    LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH1);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_2);

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  TIM_InitStruct.Prescaler = 179;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 6828;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM8, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM8);
  LL_TIM_SetClockSource(TIM8, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM8, LL_TIM_CHANNEL_CH2);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 5;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM8, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM8, LL_TIM_CHANNEL_CH2);
  LL_TIM_SetTriggerOutput(TIM8, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM8);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 0;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_ENABLE;
  LL_TIM_BDTR_Init(TIM8, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM8_Init 2 */

  LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);

  /* USER CODE END TIM8_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**TIM8 GPIO Configuration
  PC7   ------> TIM8_CH2
  */
  GPIO_InitStruct.Pin = VSYNC_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(VSYNC_GPIO_Port, &GPIO_InitStruct);

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

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  TIM_InitStruct.Prescaler = 72;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM12, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM12);
  LL_TIM_SetClockSource(TIM12, LL_TIM_CLOCKSOURCE_INTERNAL);
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */

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

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.BaudRate = 921600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
  /* USER CODE BEGIN USART2_Init 2 */

    /* Enable RXNE and Error interrupts */
    LL_USART_EnableIT_RXNE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  /* DMA interrupt init */
  /* DMA1_Stream2_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Stream2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  NVIC_SetPriority(DMA2_Stream1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

  /**/
  LL_GPIO_SetOutputPin(GPIOC, LTC_HIGH_CURRENT_Pin|ENABLE_SELECT1_Pin|CNT_MR_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOA, LTC_LOW_CURRENT_Pin|LTC_MID_CURRENT_Pin|ADC_CS_Pin|ENABLE_SELECT2_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOB, ENABLE_SELECT4_Pin|ENABLE_SELECT3_Pin|VLED_CTR_9V_Pin|XD12_VCC_EN_Pin
                          |ADC_RESET_Pin|NSCS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, XD12_5_7V_Pin|XD12_5_0V_Pin|TP_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, ADC_START_Pin|LED_TRIM_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LTC_DISCHARGE_GPIO_Port, LTC_DISCHARGE_Pin);

  /**/
  GPIO_InitStruct.Pin = B1_Pin|KEY_TRIMMING_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LTC_HIGH_CURRENT_Pin|ENABLE_SELECT1_Pin|LTC_DISCHARGE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LTC_LOW_CURRENT_Pin|LTC_MID_CURRENT_Pin|ADC_CS_Pin|ENABLE_SELECT2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ENABLE_SELECT4_Pin|ENABLE_SELECT3_Pin|XD12_5_7V_Pin|XD12_5_0V_Pin
                          |VLED_CTR_9V_Pin|XD12_VCC_EN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12|LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = KEY_POWER_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(KEY_POWER_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MCLK_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ADC_START_Pin|LED_TRIM_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CNT_MR_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(CNT_MR_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = XD12_TYPE_DETECT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(XD12_TYPE_DETECT_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = TP_Pin|ADC_RESET_Pin|NSCS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE4);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_4;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(ADC_DRDY_GPIO_Port, ADC_DRDY_Pin, LL_GPIO_PULL_UP);

  /**/
  LL_GPIO_SetPinMode(ADC_DRDY_GPIO_Port, ADC_DRDY_Pin, LL_GPIO_MODE_INPUT);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(EXTI4_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void JigTestMainTask(void)
{
    if(gn_timeout_trimming_procedure_run == 0)
    {
        trimming_procedure_run();
        gn_timeout_trimming_procedure_run = 1;
    }
    
    if(gn_timeout_dimming_test_procedure_run == 0)
    {
        dimming_procedure_run();
        gn_timeout_dimming_test_procedure_run = 1;
    }
}

static void KeyHandler(void)
{
    if(gn_key_detection_flag)
    {
        if((gn_key_detection_flag & (1U << KEY_POWER_UP)) == (1U << KEY_POWER_UP))
        {
            gn_key_detection_flag &= ~(1U << KEY_POWER_UP);
        }
        if((gn_key_detection_flag & (1U << KEY_B1_UP)) == (1U << KEY_B1_UP))
        {
            gn_key_detection_flag &= ~(1U << KEY_B1_UP);
            XD12_tx_LD_Data(XD12_get_PWM());
        }
        if((gn_key_detection_flag & (1U << KEY_TRIMMING_UP)) == (1U << KEY_TRIMMING_UP))
        {
            gn_key_detection_flag &= ~(1U << KEY_TRIMMING_UP);
        }
    }
}

static void KeyDeterctor(void)
{
    if(gn_timeout_btn_key_detection == 0)
    {
        uint32_t key_state;

        key_state = LL_GPIO_IsInputPinSet(KEY_POWER_GPIO_Port, KEY_POWER_Pin);
        if(gn_key_state[BTN_KEY_POWER] != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
                print("\r\n-XDIC_ACTIVATE_AND_START-\r\n");
                gn_key_detection_flag |= (1U << KEY_POWER_UP);
            }
            gn_key_state[BTN_KEY_POWER] = key_state;            
        }

        key_state = LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin);
        if(gn_key_state[BTN_KEY_B1] != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
                print("\r\n B1_Pin \r\n");
                gn_key_detection_flag |= (1U << KEY_B1_UP);
            }
            gn_key_state[BTN_KEY_B1] = key_state;
        }

        key_state = LL_GPIO_IsInputPinSet(KEY_TRIMMING_GPIO_Port, KEY_TRIMMING_Pin);
        if(gn_key_state[BTN_KEY_TRIM] != key_state)
        {
            if(key_state == GPIO_PIN_SET)
            {
                print("\r\n KEY_TRIMMING_Pin \r\n");
                gn_key_detection_flag |= (1U << KEY_TRIMMING_UP);
            }
            gn_key_state[BTN_KEY_TRIM] = key_state;
        }

        gn_timeout_btn_key_detection = 20;
    }
}

static uint8_t comm_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if(gt_rx_uart.RxInCnt != gt_rx_uart.RxOutCnt)
    {
        *pData = gt_rx_uart.Rxbuff + gt_rx_uart.RxOutCnt;

        ++gt_rx_uart.RxOutCnt;
        gt_rx_uart.RxOutCnt &= (uint16_t)(RX_BUFF_SIZE -1);

        ret = 1;
    }

    return ret;
}

static void TaskDebugUart(void)
{
    rx_packet_t* p_data = NULL;

    if(comm_get_rx_packet(&p_data))
    {
        char str_in[UART_CMD_LENGTH+1] = {0, };
        uint32_t u32_recv_param[6] = {0, };
        double lf_recv_param[6] = {0,};

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if(Command_is_( "help" ))
        {
            print("Help is not supported\n\r");
        }
        else if(Command_is_( CMD_GUI_TEST ))
        {
    #ifdef DBG_TEST
            _test_IF_TEST();
    #endif //DBG_TEST
        }
        else if(Command_is_( CMD_GUI_KEY_POWER_UP ))
        {
            print("KEY_POWER_UP\r\n");
            gn_key_detection_flag |= (1U << KEY_POWER_UP);
        }
        else if(Command_is_( CMD_GUI_KEY_TRIMMING_UP ))
        {
            print("KEY_TRIMMING_UP\r\n");
            gn_key_detection_flag |= (1U << KEY_TRIMMING_UP);
        }
        else if(Command_is_( CMD_GUI_RESET ))
        {
            print("CMD_GUI_RESET\r\n");
            NVIC_SystemReset();
        }
        else if(Command_is_( CMD_GUI_READ_REGISTER_ALL ))
        {
            print("CMD_GUI_READ_REGISTER_ALL\r\n");
            TargetIC_IF_Read_Resgister_All(_GUI_SUPPORT_);
        }
        else if(Command_is_( CMD_GUI_ACTIVATE ))
        {
            print("\r\nstart %s TRIMMING_JIG application....\r\n", TARGET_CHIP_NAME);
            print("CMD_GUI_ACTIVATE\r\n");
            Trim_IF_GUI_SET_Activate();
        }
        else if(Command_Param_is_(CMD_GUI_WRITE_REGISTER, "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            TargetIC_IF_Write_REGISTER(u32_recv_param[0], u32_recv_param[1]);
        }
        else if(Command_Param_is_(CMD_GUI_WRITE_OTP_ENABLE, "%x", &u32_recv_param[0]))
        {
            Trim_IF_GUI_OTP_WRITE_enable(u32_recv_param[0]);
        }
        else if(Command_Param_is_(CMD_GUI_WITHOUT_SCREEN, "%x", &u32_recv_param[0]))
        {
            Trim_IF_GUI_WITHOUT_SCREEN(u32_recv_param[0]);
        }
        else if(Command_Param_is_(CMD_GUI_NO_TRIM_MODE, "%d", &u32_recv_param[0]))
        {
            print("CMD_GUI_NO_TRIM_MODE\r\n");
            Trim_IF_GUI_NO_TRIM_enable(u32_recv_param[0]);
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_OSC, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_TARGET_MIN], &lf_recv_param[TRIM_PARA_TARGET_MAX], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_OSC\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_OSC_FREQUENCY][0]);
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_VREF, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_TARGET_MIN], &lf_recv_param[TRIM_PARA_TARGET_MAX], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_VREF\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_VREF_CTL][0]);
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_GLB, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_TARGET_MIN], &lf_recv_param[TRIM_PARA_TARGET_MAX], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_GLB\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_ICTL_GLB][0]);
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_OFFSET, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_TARGET_MIN], &lf_recv_param[TRIM_PARA_TARGET_MAX], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_OFFSET\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_OFS1_CHS][0]);
        }      
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_GAIN, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_TARGET_MIN], &lf_recv_param[TRIM_PARA_TARGET_MAX], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_GAIN\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_OFS2_CHS][0]);
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_PARAM_SCREEN, "%lf %lf %lf %lf", 
            &lf_recv_param[TRIM_PARA_SCR_START], &lf_recv_param[TRIM_PARA_SCR_END], 
            &lf_recv_param[TRIM_PARA_P1], &lf_recv_param[TRIM_PARA_P2])
            )
        {
            p_gui_param p = trim_get_param_gui();
            print("CMD_GUI_TRIM_PARAM_SCREEN\r\n");
            Trim_IF_GUI_TRIM_PARA_Set(lf_recv_param, &p[TRIM_SCREENING][0]) ;
        }
        else if(Command_Param_is_(CMD_GUI_TRIM_START, "%d", &u32_recv_param[0]))
        {
            print("CMD_GUI_TRIMSTART\r\n");
            Trim_IF_GUI_SET_TrimStart(u32_recv_param[0]);
        }

#if 1   /* delate - for testing */        
        else if(Command_is_( "init_test" ))
        {
            print("\r\nVCC 5V\r\n");
            JigBd_IF_5_0V_power(PWR_ON_5_0V);
            print("\r\nVCC Enable\r\n");
            JigBd_IF_VCC_EN_power(PWR_ON);
            print("\r\nVLED Enable\r\n");
            JigBd_IF_Vled_9V_power(PWR_ON);
            
            HAL_Delay(10);

            print("\r\nUNUSE_XC24\r\n");
            USE_XC24(false);

            print("read registrer all\r\n");
            TargetIC_IF_Initialize();
            
            print("xd12 test enable\r\n");
            TargetIC_IF_TRIM_Init_VREF();

            print("xd12 max curr vref 0xFFF\r\n");
            TargetIC_IF_MAX_CURR_VREF_Init();

            print("gain - charge & low\r\n");
            JigBd_IF_current_discharge(CHARGE);
            JigBd_IF_change_current_gain(GAIN_LOW);
            
            print("ch select - 0\r\n");
            JigBd_IF_Select_Output_Ch(0);
        }
        else if(Command_is_( "idgen" ))
        {
            print("\r\nXD IDGen\r\n");
            JigBd_IF_IdGen_Command();
        }
        else if(Command_is_( "test_en" ))
        {
            print("\r\nXD TEST_EN to checksing OSC & VREF\r\n");
            TargetIC_IF_TRIM_Init_VREF();
        }
        else if(Command_is_( "r_all" ))
        {
            print("read registrer all\r\n");
            // TargetIC_IF_Initialize();
            TargetIC_IF_Read_Resgister_All(0);
        }       
        else if(Command_Param_is_("r", "%x", &u32_recv_param[0]))
        {
            uint16_t ret = JigBd_IF_Read_Command((uint8_t)u32_recv_param[0]);
            print("\r\nXD Read : 0x%02X : 0x%04X\r\n", u32_recv_param[0], ret);
        }
        else if(Command_Param_is_("w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            print("\r\nXD Write : 0x%02X - 0x%02X\r\n", (uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
            JigBd_IF_Write_Command((uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
            
            uint16_t ret = JigBd_IF_Read_Command((uint8_t)u32_recv_param[0]);
            print("\r\nXD Read : 0x%02X : 0x%04X\r\n", u32_recv_param[0], ret);
        }
        else if(Command_is_( "ic_start" ))
        {
            LL_GPIO_ResetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);
            print("\r\nStart timer for freq input capture\r\n");
            JigBd_IF_TIM_Capture_Start();
        }
        else if(Command_is_( "ic_stop" ))
        {
            JigBd_IF_TIM_Capture_Stop();
            print("\r\nStop timer for freq input capture : %1.6f(%u) [kHz] => %1.6f [MHz]\r\n", gf_freq_avg/1000, JigBd_IF_Freq_Get(), ((gf_freq_avg/1000000) * TIM_CAPTURE_EXT_PRECALER));
            LL_GPIO_SetOutputPin(CNT_MR_GPIO_Port, CNT_MR_Pin);
        }
        else if(Command_Param_is_("ch_sel", "%d", &u32_recv_param[0]))
        {
            print("\r\nOutput CH sel : %u\r\n", u32_recv_param[0]);
            JigBd_IF_Select_Output_Ch(u32_recv_param[0]);
        }

        else if(Command_Param_is_("din", "%d", &u32_recv_param[0]))
        {
            print("\r\nD_IN : %u\r\n", u32_recv_param[0]);
            TargetIC_IF_LD_FIX_Set(u32_recv_param[0]);
        }

        else if(Command_is_( "vcc on" ))
        {
            print("\r\nVCC Enable\r\n");
            JigBd_IF_VCC_EN_power(PWR_ON);
        }
        else if(Command_is_( "vcc off" ))
        {
            print("\r\nVCC Disable\r\n");
            JigBd_IF_VCC_EN_power(PWR_OFF);
        }
        else if(Command_is_( "vcc 5.7" ))
        {
            print("\r\nVCC 5.7V\r\n");
            JigBd_IF_5_0V_power(PWR_ON_5_7V);
        }
        else if(Command_is_( "vcc 5" ))
        {
            print("\r\nVCC 5V\r\n");
            JigBd_IF_5_0V_power(PWR_ON_5_0V);
        }
        else if(Command_is_( "vled on" ))
        {
            print("\r\nVLED Enable\r\n");
            JigBd_IF_Vled_9V_power(PWR_ON);
        }
        else if(Command_is_( "vled off" ))
        {
            print("\r\nVLED Disable\r\n");
            JigBd_IF_Vled_9V_power(PWR_OFF);
        }
        else if(Command_is_( "gain_low" ))
        {
            print("\r\nSet Current Gain Low\r\n");
            XD12_set_dimming_gain(GAIN_LOW);
        }
        else if(Command_is_( "gain_mid" ))
        {
            print("\r\nSet Current Gain Mid\r\n");
            XD12_set_dimming_gain(GAIN_MID);
        }
        else if(Command_is_( "gain_high" ))
        {
            print("\r\nSet Current Gain High\r\n");
            XD12_set_dimming_gain(GAIN_HIGH);
        }
        else if(Command_is_( "gain_charge" ))
        {
            print("\r\nSet Current Charge\r\n");
            JigBd_IF_current_discharge(CHARGE);
        }
        else if(Command_is_( "gain_discharge" ))
        {
            print("\r\nSet Current Discharge\r\n");
            JigBd_IF_current_discharge(DISCHARGE);
        }
        else if(Command_is_( "adc" ))
        {
            print("\r\nADC RUN\r\n");    
            JigBd_IF_VREF_ADC_StartStop();
        }

        else if(Command_is_( "ui:use_xc" ))
        {
            USE_XC24(true);
            print("\r\nSUPPORT_XC24\r\n");
        }
        else if(Command_is_( "ui:not_use_xc" ))
        {
            USE_XC24(false);
            print("\r\nNOT_SUPPORT_XC24\r\n");
        }
        else if(Command_is_( "ui:init_xc" ))
        {
            print("\r\nINIT_XC24\r\n");
            XC24_Initialize();
        }
        else if(Command_is_( "verify_done" ))
        {
            print("\r\nGO-TO-TRIM_OUTPUT_INIT\r\n");
            Trim_IF_GUI_SET_Verify_Done();
        }
        else if(Command_is_( "dimming_test_init" ))
        {
            print("\r\nXD12_dimming_test_init\r\n");
            XD12_dimming_test_init();
        }
        else if(Command_Param_is_("pwm", "%d", &u32_recv_param[0]))
        {
            print("\r\nSet - pwm : %4u\r\n", u32_recv_param[0]);
            XD12_set_PWM(u32_recv_param[0]);
        }
        else if(Command_is_( "pwm" ))
        {
            print("\r\npwm : %4u\r\n", XD12_get_PWM());
        }
        else if(Command_Param_is_("ldim", "%d", &u32_recv_param[0]))
        {
            print("\r\nSet - ldim : %4u\r\n", u32_recv_param[0]);
            XD12_set_ldim(u32_recv_param[0]);
        }
        else if(Command_is_( "ldim" ))
        {
            print("\r\nldim : %4u\r\n", XD12_get_ldim());
        }
#if 1
        else if(Command_Param_is_("short_test", "%d", &u32_recv_param[0]))
        {
            print("\r\nSet - short_level : %2u\r\n", u32_recv_param[0]);
            XD12_set_short_level(u32_recv_param[0]);
        }
        else if(Command_Param_is_("fb_test", "%d", &u32_recv_param[0]))
        {
            print("\r\nSet - fb_level : %2u\r\n", u32_recv_param[0]);
            XD12_set_FB_level(u32_recv_param[0]);
        }
        else if(Command_is_("fault_test_stop"))
        {
            print("\r\nXD12 Fault Test Stop\r\n");
            XD12_stop_fault_test();
        }
        else if(Command_is_("ui:gdim"))
        {
            print("\r\nXD12 Dimming Test Strat\r\n");
            XD12_start_dimming_test();
        }
        else if(Command_is_("data_trans"))
        {
            print("\r\nXD12 Dimming Test Strat\r\n");
            while(1)
            {
                JigBd_IF_Write_Command(XD12_ADDR_DELAY_SIZE, 0);
                HAL_Delay(1);
            }
        }

        else if(Command_Param_is_("step", "%d", &u32_recv_param[0]))
        {
            print("\r\nSet - step : %2u\r\n", u32_recv_param[0]);
        }

#endif
#endif
        else
        {
            print("\r\nWhat?\n\r");
        }
    }
}

__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Echo received character on TX */
    LL_USART_TransmitData8(USART2, (uint8_t)data);

    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
}

void comm_rx_handler(uint8_t rx)
{
    UART_PutChar(rx);

    if((rx == '\n') || (rx == '\r'))
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = 0;
        }
        else
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[(RX_PACKET_SIZE - 1)] = 0;
        }
        
        ++gt_rx_uart.RxInCnt;
        gt_rx_uart.RxInCnt &= (uint16_t)(RX_BUFF_SIZE - 1);
    }
    else if(rx == 0x08)
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length)
        {
            uint8_t temp[2] = { ' ', 0x08 };
            
            UART_PutChar(temp[0]);
            UART_PutChar(temp[1]);
            --gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
    }
    else
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = rx;
            ++gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;

            if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[0] == 0x1B)
            {
                if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[1] == 0x5B)
                {
                    if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[2] == 0x41)
                    {
                        print("\r\n\r\n");
                        /* copy */
                        for (uint8_t i = 0 ; i < 64 ; ++i)
                        {
                            if (gt_rx_uart.RxInCnt)
                            {
                                if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt-1].buffer[i])
                                {             
                                    gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i] = gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt-1].buffer[i];
                                    print("%c", gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i]);
                                }
                                else
                                {
                                    gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length = i;
                                    break;
                                }
                            }
                            else
                            {
                                if (gt_rx_uart.Rxbuff[RX_BUFF_SIZE-1].buffer[i])
                                {             
                                    gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i] = gt_rx_uart.Rxbuff[RX_BUFF_SIZE-1].buffer[i];
                                    print("%c", gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i]);
                                }
                                else
                                {
                                    gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length = i;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            //print("buffer limit...\r\n");
        }
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
