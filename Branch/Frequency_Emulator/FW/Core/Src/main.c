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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PRINT_BUFF_SIZE     (400U)
#define RX_PACKET_SIZE      (64U)
#define RX_BUFF_SIZE        (32U)

#define ANSI_FONT_NONE      "\033[0m"
#define ANSI_FONT_RED       "\033[31m"
#define ANSI_FONT_GREEN     "\033[32m"
#define ANSI_FONT_YELLOW    "\033[33m"
#define ANSI_FONT_BLUE      "\033[34m"
#define ANSI_FONT_MAGENTA   "\033[35m"

#define TIM8_BASE_FREQ      (180000000U) /* 180MHz */
#define TIM8_PRESCALE       (29U)
#define TIM8_FREQ           (TIM8_BASE_FREQ / (TIM8_PRESCALE + 1U)) /* 6MHz */
#define TIM8_PERIOD         (49999U)

typedef void (*func_t)(void);

typedef struct tag_KEY_INFO
{
    uint32_t now_state;
    uint32_t prev_state;
    uint8_t press_cnt;
    uint8_t odd_even;
    func_t function[2];
} key_info_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static key_info_t gt_key_info = {
    .now_state = 1,
    .prev_state = 1,
    .press_cnt = 0,
    .odd_even = 0,
    .function = { NULL, NULL },
};

typedef struct
{
    uint16_t length;
    char buffer[RX_PACKET_SIZE];
} rx_packet_t;

typedef struct
{
    uint16_t RxInCnt;
    uint16_t RxOutCnt;
    rx_packet_t Rxbuff[RX_BUFF_SIZE];
} RX_UART_t;

static uint8_t gn_key_task_tick;
static RX_UART_t gt_rx_uart;
static LOG_LV_T gt_log_lv = LOG_INFO;

/*
static void key_func_1(void)
{
}

static void key_func_2(void)
{
}
*/

static void btn_process(void)
{
    if (gn_key_task_tick == 0)
    {
        gt_key_info.now_state = LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin);
        if (gt_key_info.now_state != gt_key_info.prev_state)
        {
            if (gt_key_info.now_state == 0)
            {
            }
            else
            {
                // Button Released
                if (gt_key_info.press_cnt >= 30)
                {
                    if (gt_key_info.function[gt_key_info.odd_even] != NULL)
                    {
                        gt_key_info.function[gt_key_info.odd_even]();
                    }
                    gt_key_info.odd_even ^= 1;
                }
                gt_key_info.press_cnt = 0;
            }
            gt_key_info.prev_state = gt_key_info.now_state;
        }
        else
        {
            if (gt_key_info.now_state == 0)
            {
                // Button Pressed
                ++gt_key_info.press_cnt;
            }
            else
            {
                // nothing
            }
        }
        gn_key_task_tick = 10; //10ms
    }
}

static void freq_start(void)
{
    uint16_t default_freq = 5000; //5kHz
    LL_TIM_SetCounter(TIM8, 0U);
    LL_TIM_SetAutoReload(TIM8, TIM8_FREQ / default_freq - 1U);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
}

static void freq_stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0U);
}

static void freq_change(uint16_t in_freq)
{
    uint16_t arr = (TIM8_FREQ / in_freq - 1U);
    LL_TIM_SetAutoReload(TIM8, arr);
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM12_Init(void);
static void MX_TIM8_Init(void);
/* USER CODE BEGIN PFP */
static void TaskDebugUart(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void sys_tick_handler(void)
{
}

void print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;
        char msg_buffer[PRINT_BUFF_SIZE] = {0, };
        char color_buffer[PRINT_BUFF_SIZE] = {0, };

        va_list ap;
        va_start(ap, fmt);
        vsnprintf(msg_buffer, PRINT_BUFF_SIZE - 1, fmt, ap);
        va_end(ap);

        // 컬러 적용
        if (log_lv > LOG_INFO)
        {
            snprintf(color_buffer, (PRINT_BUFF_SIZE - 1), "%s%s%s", ANSI_FONT_RED, msg_buffer, ANSI_FONT_NONE);
            len = strlen(color_buffer);
        }
        else
        {
            len = strlen(msg_buffer);
            memcpy(color_buffer, msg_buffer, len + 1);
        }

        for (uint16_t i = 0 ; i < len ; ++i)
        {
            while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
            LL_USART_TransmitData8(USART2, (uint8_t)color_buffer[i]);
        }
    }
}

static void comm_print_help(void)
{
    print(LOG_INFO, "\n\r------------------ Command Help -----------------------------");

    print(LOG_INFO, "\n\r  help / ?\t\t : Show command help");

    print(LOG_INFO, "\n\r  log_lv [d]\t\t : Set log level (0 ~ LOG_MAX-1)");
    print(LOG_INFO, "\n\r  reset\t\t\t : Reset MCU");

    print(LOG_INFO, "\n\r--------------------------------------------------------------\n\r");
}

static void comm_init(void)
{
    print(LOG_INFO, "\n\r--------------------------------------");
    print(LOG_INFO, "\n\r    [GT Frequency Emulator]");
    print(LOG_INFO, "\n\r--------------------------------------");
    print(LOG_INFO, "\n\r - Author: xxx@glbltech.com");
    print(LOG_INFO, "\n\r - Build : %s", __DATE__);
    print(LOG_INFO, "\n\r--------------------------------------\r\n");
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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM12_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

    comm_init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    TaskDebugUart();
    btn_process();
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
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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

  /* TIM8 interrupt Init */
  NVIC_SetPriority(TIM8_UP_TIM13_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  TIM_InitStruct.Prescaler = 29;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 49999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM8, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM8);
  LL_TIM_SetClockSource(TIM8, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM8, LL_TIM_CHANNEL_CH2);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 40;
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

  /* USER CODE END TIM8_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**TIM8 GPIO Configuration
  PC7   ------> TIM8_CH2
  */
  GPIO_InitStruct.Pin = Frequency_Out_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(Frequency_Out_GPIO_Port, &GPIO_InitStruct);

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
  TIM_InitStruct.Prescaler = 89;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM12, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM12);
  LL_TIM_SetClockSource(TIM12, LL_TIM_CLOCKSOURCE_INTERNAL);
  /* USER CODE BEGIN TIM12_Init 2 */
    LL_TIM_SetCounter(TIM12, 0);
    LL_TIM_EnableCounter(TIM12);
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
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.BaudRate = 115200;
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

    /* USART RX */
    LL_USART_EnableIT_RXNE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
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
  LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);

  /**/
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_3
                          |LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_8
                          |LL_GPIO_PIN_9|LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_4|LL_GPIO_PIN_5
                          |LL_GPIO_PIN_6|LL_GPIO_PIN_7|LL_GPIO_PIN_8|LL_GPIO_PIN_9
                          |LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_10
                          |LL_GPIO_PIN_12|LL_GPIO_PIN_13|LL_GPIO_PIN_14|LL_GPIO_PIN_15
                          |LL_GPIO_PIN_3|LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6
                          |LL_GPIO_PIN_7|LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DEBUG_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DEBUG_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static uint8_t comm_get_rx_packet(rx_packet_t** pData)
{
    uint8_t ret = 0;

    if (gt_rx_uart.RxInCnt != gt_rx_uart.RxOutCnt)
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

    if (comm_get_rx_packet(&p_data))
    {
        char str_in[RX_PACKET_SIZE + 1] = {0, };
        uint32_t u32_recv_param[6] = {0, };
        double lf_recv_param[6] = {0, };

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if (Command_is_("help") || Command_is_("?"))
        {
            comm_print_help();
        }
/* ----------------- command list - jig ----------------- */
        else if (Command_is_("freq_start"))
        {
            print(LOG_INFO, "\r\n Frequency Start Generation!!");
            freq_start();
        }
        else if (Command_is_("freq_stop"))
        {
            print(LOG_INFO, "\r\n Frequency Stop Generation!!");
            freq_stop();
        }
        else if (Command_Param_is_("freq", "%d", &u32_recv_param[0]))
        {
            DEBUG_HI();
            freq_change(u32_recv_param[0]);
            DEBUG_LO();
            print(LOG_INFO, "\r\n freq to [%d Hz]\r\n", u32_recv_param[0]);
        }
        else if (Command_Param_is_("log_lv", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < LOG_MAX)
            {
                print(LOG_INFO, "\r\n log_level - [%u]\r\n", u32_recv_param[0]);
                gt_log_lv = (LOG_LV_T)u32_recv_param[0];
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of log_lv [%u] [0 - %u]\r\n", u32_recv_param[0], LOG_MAX - 1);
            }
        }
        else if (Command_is_("reset"))
        {
            print(LOG_INFO, "\r\n system reset \r\n");
            NVIC_SystemReset();
        }
        else
        {
            print(LOG_INFO, "\r\n What?\n\r");
        }
    }
}

__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Echo received character on TX */
    if ((data == '\n') || (data == '\r'))
    {
        LL_USART_TransmitData8(USART2, '\r');
        LL_USART_TransmitData8(USART2, '\n');
    }
    else
    {
        LL_USART_TransmitData8(USART2, data);
    }

    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
}

void comm_rx_handler(uint8_t rx)
{
    UART_PutChar(rx);

    if ((rx == '\n') || (rx == '\r'))
    {
        if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
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
    else if (rx == UART_BACKSPACE)
    {
        if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length)
        {
            UART_PutChar(' ');
            UART_PutChar(UART_BACKSPACE);
            --gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;
        }
    }
    else
    {
        if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length < (RX_PACKET_SIZE - 1))
        {
            gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length] = rx;
            ++gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length;

            if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[0] == 0x1B)
            {
                if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[1] == 0x5B)
                {
                    if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[2] == 0x41)
                    {
                        print(LOG_INFO, "\r\n\r\n");
                        /* copy */
                        for (uint8_t i = 0 ; i < 64 ; ++i)
                        {
                            if (gt_rx_uart.RxInCnt)
                            {
                                if (gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt-1].buffer[i])
                                {
                                    gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i] = gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt-1].buffer[i];
                                    print(LOG_INFO, "%c", gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i]);
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
                                    print(LOG_INFO, "%c", gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].buffer[i]);
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
            //print(LOG_ERROR, "buffer limit...\r\n");
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
#ifdef USE_FULL_ASSERT
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
