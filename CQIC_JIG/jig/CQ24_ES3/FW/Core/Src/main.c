/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TASK_JIG_FUNCTION_TICK      (10) //ms
#define TASK_KEY_HANDLER_TICK       (10) //ms
#define TASK_UART_HANDLER_TICK      (5) //ms

#define UART_BACKSPACE              (0x08)
#define UART_NEW_LINE               (0x0A)
#define UART_CARRIAGE_RETURN        (0x0D)
#define UART_DELETE                 (0x7F)
#define UART_SPACE                  (0x20)

#define TX_PACKET_SIZE              (50)
#define TX_BUFF_SIZE                (256) // must be power of 2

#define RX_PACKET_SIZE              (20)
#define RX_BUFF_SIZE                (32) // must be power of 2

typedef enum
{
    KEY_FUNCTION_LONG = 0,
    KEY_FUNCTION_SHORT,
    KEY_FUNCTION_MAX,
}KEY_FUNCTION_TYPE;

typedef enum
{
    KEY_RUN = 0,
    KEY_MODE,
    KEY_B1,
    KEY_MAX,
}KEY_INDEX;

static GPIO_TypeDef* key_gpio_port[KEY_MAX] =
{
    KEY_RUN_GPIO_Port,
    KEY_MODE_GPIO_Port,
    KEY_B1_GPIO_Port
};

static uint16_t key_gpio_pin[KEY_MAX] =
{
    KEY_RUN_Pin,
    KEY_MODE_Pin,
    KEY_B1_Pin
};

#define KEY_PRESSED_JUDGE_CNT   (50)
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
TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* Definitions for JigFunctionTask */
osThreadId_t JigFunctionTaskHandle;
const osThreadAttr_t JigFunctionTask_attributes = {
  .name = "JigFunctionTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for KeyHandlerTask */
osThreadId_t KeyHandlerTaskHandle;
const osThreadAttr_t KeyHandlerTask_attributes = {
  .name = "KeyHandlerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UartHandlerTask */
osThreadId_t UartHandlerTaskHandle;
const osThreadAttr_t UartHandlerTask_attributes = {
  .name = "UartHandlerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for queue_key_event */
osMessageQueueId_t queue_key_eventHandle;
const osMessageQueueAttr_t queue_key_event_attributes = {
  .name = "queue_key_event"
};
/* USER CODE BEGIN PV */
osSemaphoreId_t JigTaskSemaphore;

typedef struct
{
    uint8_t length;
    char buffer[TX_PACKET_SIZE];
}Tx_Packet;

typedef struct
{
    uint8_t TxInCnt;
    uint8_t TxOutCnt;
    Tx_Packet Txbuff[TX_BUFF_SIZE];
}Uart_Tx;
static Uart_Tx gt_uart_tx;
bool gb_uart_tx_start;

typedef struct
{
    uint8_t length;
    char buffer[RX_PACKET_SIZE];
}Rx_Packet;

typedef struct
{
    uint8_t RxInCnt;
    uint8_t RxOutCnt;
    Rx_Packet Rxbuff[RX_BUFF_SIZE];
}Uart_Rx;
static Uart_Rx gt_uart_rx;
uint8_t gn_uart_rx_temp;

const char* CQ24_CONSOLE = "CQ24 >> ";

typedef struct
{
    GPIO_PinState prev_pin_state;
    GPIO_PinState now_pin_state;
    uint8_t key_press_count;
    bool is_long_key_done;
}_key_state_;
static _key_state_ gt_key_state[KEY_MAX];
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
static void MX_TIM12_Init(void);
void JigFunction(void *argument);
void KeyHandler(void *argument);
void UartHandler(void *argument);

/* USER CODE BEGIN PFP */
static void Key_Run_Long_Function(void);
static void Key_Run_Short_Function(void);
static void Key_Mode_Long_Function(void);
static void Key_Mode_Short_Function(void);
static void Key_B1_Long_Function(void);
static void Key_B1_Short_Function(void);
static void Key_Struct_Init(void);

static uint8_t Receive_Command_Is(char* compare_msg);
static void Uart_Display_Command_List(void);
static void Uart_Rx_Decode_String(void);
static void Uart_Debug_Init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Print(const char* fmt, ...)
{
    int length = 0;
    va_list args;

    va_start(args, fmt);
    length = vsnprintf(gt_uart_tx.Txbuff[gt_uart_tx.TxInCnt].buffer, (TX_PACKET_SIZE - 1), fmt, args);
    va_end(args);

    gt_uart_tx.Txbuff[gt_uart_tx.TxInCnt].length = length;
    ++gt_uart_tx.TxInCnt;
    gt_uart_tx.TxInCnt &= (uint8_t)(TX_BUFF_SIZE - 1);
}

void Uart_Rx_Parser(void)
{
    HAL_UART_Transmit(&huart2, &gn_uart_rx_temp, 1, 10);

    if (gn_uart_rx_temp == UART_NEW_LINE || gn_uart_rx_temp == UART_CARRIAGE_RETURN)
    {
        char enter = UART_NEW_LINE;
        HAL_UART_Transmit(&huart2, (const uint8_t*)&enter, 1, 10);
        ++gt_uart_rx.RxInCnt;
        gt_uart_rx.RxInCnt &= (uint8_t)(RX_BUFF_SIZE - 1);

        gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length = 0;
    }
    else if (gn_uart_rx_temp == UART_BACKSPACE)
    {
        char backspace[2] = { UART_SPACE, UART_BACKSPACE };
        HAL_UART_Transmit(&huart2, (const uint8_t*)backspace, 2, 10);
        if (gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length)
        {
            --gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length;
            gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].buffer[gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length] = 0;
        }
    }
    else if (gn_uart_rx_temp == UART_DELETE)
    {
        char delete[3] = { UART_BACKSPACE, UART_SPACE, UART_BACKSPACE };
        HAL_UART_Transmit(&huart2, (const uint8_t*)delete, 3, 10);
        while (gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length)
        {
            HAL_UART_Transmit(&huart2, (const uint8_t*)delete, 3, 10);
            --gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length;
            gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].buffer[gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length] = 0;
        }
    }
    else
    {
        if (gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length < RX_PACKET_SIZE)
        {
            gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].buffer[gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length] = gn_uart_rx_temp;
            ++gt_uart_rx.Rxbuff[gt_uart_rx.RxInCnt].length;
        }
        else
        {
            Print("\tRx Buffer Size Limit...\r\n");
        }
    }

    HAL_UART_Receive_DMA(&huart2, &gn_uart_rx_temp, 1);
}

static uint8_t Receive_Command_Is(char* compare_msg)
{
    uint8_t ret = 0x40;

    ret = strcmp(compare_msg, gt_uart_rx.Rxbuff[gt_uart_rx.RxOutCnt].buffer);

    //Print("msg : %s, comp : %s, ret : %u\r\n", gt_uart_rx.Rxbuff[gt_uart_rx.RxOutCnt].buffer, compare_msg, ret);

    return ret;
}

static void Uart_Display_Command_List(void)
{
    Print("\t [ cq_w1 (h'addr) (h'data) ]\r\n");
    Print("\t [ cq_r1 (h'addr)          ]\r\n");
    Print("\t [ cq_w2 (h'addr) (h'data) ]\r\n");
    Print("\t [ cq_r2 (h'addr)          ]\r\n");
}

static void Uart_Rx_Decode_String(void)
{
    uint32_t u32_param[6] = {0, };
    char* str = gt_uart_rx.Rxbuff[gt_uart_rx.RxOutCnt].buffer;

    if (!Receive_Command_Is("reset"))
    {
        NVIC_SystemReset();
    }
    else if ((!Receive_Command_Is("help")) || (!Receive_Command_Is("?")))
    {
        Uart_Display_Command_List();
    }
    else if (!Receive_Command_Is("vsync_start"))
    {
        Print("Vsync start!! \r\n");
        CQ24_Set_Vsync(true);
    }
    else if (!Receive_Command_Is("vsync_stop"))
    {
        Print("Vsync stop!! \r\n");
        CQ24_Set_Vsync(false);
    }
    else if (!Receive_Command_Is("mclk_start"))
    {
        Print("MCLK start!! \r\n");
        CQ24_Set_MCLK(true);
    }
    else if (!Receive_Command_Is("mclk_stop"))
    {
        Print("MCLK stop!! \r\n");
        CQ24_Set_MCLK(false);
    }
    else if (sscanf(str, "cq_w1 %x %x", &u32_param[0], &u32_param[1]))
    {
        CQ24_Set_CMD1_Target_Reg(CQ24_WR, u32_param[0], u32_param[1]);
    }
    else if (sscanf(str, "cq_r1 %x", &u32_param[0]))
    {
        CQ24_Set_CMD1_Target_Reg(CQ24_RD, u32_param[0], 0);
    }
    else if (sscanf(str, "cq_w2 %x %x", &u32_param[0], &u32_param[1]))
    {
        CQ24_Set_CMD2_Target_Reg(CQ24_WR, u32_param[0], u32_param[1]);
    }
    else if (sscanf(str, "cq_r2 %x", &u32_param[0]))
    {
        CQ24_Set_CMD2_Target_Reg(CQ24_RD, u32_param[0], 0);
    }
    else if (sscanf(str, "duty %u", &u32_param[0]))
    {
        CQ24_Set_CMD3_Duty((uint16_t)u32_param[0]);
    }
    else if (sscanf(str, "ld_i %u", &u32_param[0]))
    {
        CQ24_Set_CMD4_LD_I((uint16_t)u32_param[0]);
    }
    else
    {
        Print("\tUnknown CMD!! \r\n");
    }

    Print("%s", CQ24_CONSOLE);
}

static void Uart_Debug_Init(void)
{
    Print("--------------------------------------\r\n");
    Print("    [GT-CQ24(xxx) TEST/TRIM JIG]\r\n");
    Print("--------------------------------------\r\n");
    Print("-Project\t: STM32 CQ24 JIG\r\n");
    Print("-Author\t\t: xxx@glbltech.com\r\n");
    Print("-Build\t\t: %s\r\n", __DATE__);
    Print("--------------------------------------\r\n");
    Print("%s", CQ24_CONSOLE);

    HAL_UART_Receive_DMA(&huart2, &gn_uart_rx_temp, 1);
}

static void (*Key_Function_Pointer[KEY_MAX][KEY_FUNCTION_MAX])(void) =
{
    { Key_Run_Long_Function,    Key_Run_Short_Function },
    { Key_Mode_Long_Function,   Key_Mode_Short_Function },
    { Key_B1_Long_Function,     Key_B1_Short_Function },
};

static void Key_Run_Long_Function(void)
{
    Print("\r\n key run long \r\n");
}
static void Key_Run_Short_Function(void)
{
    CQ24_Init();
    Print("\r\n key run short \r\n");
}

static void Key_Mode_Long_Function(void)
{
    Print("\r\n key mode long \r\n");
}
static void Key_Mode_Short_Function(void)
{
    Print("\r\n key mode short \r\n");
}

static void Key_B1_Long_Function(void)
{
    Print("\r\n key b1 long \r\n");
}
static void Key_B1_Short_Function(void)
{
    Print("\r\n key b1 short \r\n");
}
static void Key_Struct_Init(void)
{
    for (KEY_INDEX key_idx = KEY_RUN ; key_idx < KEY_MAX ; ++key_idx)
    {
        gt_key_state[key_idx].now_pin_state = GPIO_PIN_SET;
        gt_key_state[key_idx].prev_pin_state = GPIO_PIN_SET;
        gt_key_state[key_idx].key_press_count = 0;
        gt_key_state[key_idx].is_long_key_done = false;
    }
}

void CQ24_Set_MCLK(bool en)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (en == true)
    {
        GPIO_InitStruct.Pin = MCLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
        HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
    }
    else
    {
        /*Configure GPIO pin : MCLK_Pin */
        GPIO_InitStruct.Pin = MCLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);
    }
}

void CQ24_Set_Vsync(bool en)
{
    if (en == true)
    {
        uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim2);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, (uint32_t)(period * 0.01));
        HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_4);
    }
    else
    {
        HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_4);
    }
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
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */
    Key_Struct_Init();
    Uart_Debug_Init();

    CQ24_Set_MCLK(false);
    CQ24_Set_Vsync(false);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
    JigTaskSemaphore = osSemaphoreNew(1, 0, NULL);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queue_key_event */
  queue_key_eventHandle = osMessageQueueNew (16, sizeof(uint16_t), &queue_key_event_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of JigFunctionTask */
  JigFunctionTaskHandle = osThreadNew(JigFunction, NULL, &JigFunctionTask_attributes);

  /* creation of KeyHandlerTask */
  KeyHandlerTaskHandle = osThreadNew(KeyHandler, NULL, &KeyHandlerTask_attributes);

  /* creation of UartHandlerTask */
  UartHandlerTaskHandle = osThreadNew(UartHandler, NULL, &UartHandlerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 128;
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 639999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
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
  HAL_GPIO_WritePin(GPIOC, DEBUG_Pin|LED_TRIM_Pin|TEST_1_Pin|CQ24_5_7V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ADC1_START_Pin|AQIC_PWR_CTL_Pin|CQ24_5_0V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ADC1_CS_Pin|CQ24_VCC_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TEST_2_Pin|ADC2_START_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NSCS1_GPIO_Port, NSCS1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NSCS2_Pin|ADC_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ADC2_CS_GPIO_Port, ADC2_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : KEY_B1_Pin */
  GPIO_InitStruct.Pin = KEY_B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DEBUG_Pin LED_TRIM_Pin TEST_1_Pin NSCS1_Pin
                           CQ24_5_7V_Pin */
  GPIO_InitStruct.Pin = DEBUG_Pin|LED_TRIM_Pin|TEST_1_Pin|NSCS1_Pin
                          |CQ24_5_7V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
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

  /*Configure GPIO pin : ADC1_DRDY_Pin */
  GPIO_InitStruct.Pin = ADC1_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC1_DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC2_DRDY_Pin */
  GPIO_InitStruct.Pin = ADC2_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC2_DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ADC1_CS_Pin TEST_2_Pin CQ24_VCC_EN_Pin ADC2_START_Pin */
  GPIO_InitStruct.Pin = ADC1_CS_Pin|TEST_2_Pin|CQ24_VCC_EN_Pin|ADC2_START_Pin;
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

  /* USER CODE BEGIN MX_GPIO_Init_2 */
    CQ24_NSCS_LO();
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_JigFunction */
/**
  * @brief  Function implementing the JigFunctionTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_JigFunction */
void JigFunction(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    if (osSemaphoreAcquire(JigTaskSemaphore, osWaitForever) == osOK)
    {
        CQ24_Vsync_Task();
    }
    //osDelay(TASK_JIG_FUNCTION_TICK);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_KeyHandler */
/**
* @brief Function implementing the KeyHandlerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyHandler */
void KeyHandler(void *argument)
{
  /* USER CODE BEGIN KeyHandler */
  /* Infinite loop */
  for(;;)
  {
    for (KEY_INDEX key_idx = KEY_RUN ; key_idx < KEY_MAX ; ++key_idx)
    {
        gt_key_state[key_idx].now_pin_state = HAL_GPIO_ReadPin(key_gpio_port[key_idx], key_gpio_pin[key_idx]);

        if (gt_key_state[key_idx].now_pin_state != gt_key_state[key_idx].prev_pin_state)
        {
            if (gt_key_state[key_idx].now_pin_state == GPIO_PIN_SET)
            {
                if (gt_key_state[key_idx].key_press_count < KEY_PRESSED_JUDGE_CNT)
                {
                    if (gt_key_state[key_idx].is_long_key_done == false)
                    {
                        Key_Function_Pointer[key_idx][KEY_FUNCTION_SHORT]();
                    }
                }
                gt_key_state[key_idx].key_press_count = 0;
                gt_key_state[key_idx].is_long_key_done = false;
            }
            else
            {

            }
            gt_key_state[key_idx].prev_pin_state = gt_key_state[key_idx].now_pin_state;
        }
        else
        {
            if (gt_key_state[key_idx].now_pin_state == GPIO_PIN_RESET)
            {
                ++gt_key_state[key_idx].key_press_count;
                if ((gt_key_state[key_idx].key_press_count > KEY_PRESSED_JUDGE_CNT) && !(gt_key_state[key_idx].key_press_count % KEY_PRESSED_JUDGE_CNT))
                {
                    Key_Function_Pointer[key_idx][KEY_FUNCTION_LONG]();
                    gt_key_state[key_idx].is_long_key_done = true;
                }
            }
        }
    }
    osDelay(TASK_KEY_HANDLER_TICK);
  }
  /* USER CODE END KeyHandler */
}

/* USER CODE BEGIN Header_UartHandler */
/**
* @brief Function implementing the UartHandlerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UartHandler */
void UartHandler(void *argument)
{
  /* USER CODE BEGIN UartHandler */
  /* Infinite loop */
  for(;;)
  {
    if ((!gb_uart_tx_start) && (gt_uart_tx.TxInCnt != gt_uart_tx.TxOutCnt))
    {
        HAL_UART_Transmit_DMA(&huart2, (const uint8_t*)gt_uart_tx.Txbuff[gt_uart_tx.TxOutCnt].buffer, gt_uart_tx.Txbuff[gt_uart_tx.TxOutCnt].length);
        ++gt_uart_tx.TxOutCnt;
        gt_uart_tx.TxOutCnt &= (uint8_t)(TX_BUFF_SIZE - 1);
        gb_uart_tx_start = true;
    }

    if (gt_uart_rx.RxInCnt != gt_uart_rx.RxOutCnt)
    {
        Uart_Rx_Decode_String();
        ++gt_uart_rx.RxOutCnt;
        gt_uart_rx.RxOutCnt &= (uint8_t)(RX_BUFF_SIZE - 1);
    }

    osDelay(TASK_UART_HANDLER_TICK);
  }
  /* USER CODE END UartHandler */
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
