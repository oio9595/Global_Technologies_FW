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
#include "config.h"
#include "xdic.h"
#include "xc24.h"
#include "vsync_task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRINT_BUFF_SIZE     256
#define RX_BUFF_SIZE        32
#define RX_PACKET_SIZE      32

enum tag_PROTOCOL_LIST
{
    SOP = 0xA5,
    EOP = 0x5A,
    CMD_INIT = 0x00,
    CMD_QUIT = 0xFF,
    CMD_STATUS = 0xF0,
    CMD_CURRENT = 0x01,
    CMD_BAR_ON_SELECT = 0x10,
    CMD_BAR_OFF_SELECT = 0x20,
    CMD_BLK_ON_SELECT = 0x40,
    CMD_BLK_OFF_SELECT = 0x80,
} protocol_list_t;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct tag_RX_PACKET
{
    uint8_t sop;
    uint8_t length;
    uint8_t command;
    uint8_t data;
    uint8_t checksum;
    uint8_t eop;
} rx_packet_t;

typedef struct tag_RING_BUFFER
{
    uint8_t head;
    uint8_t tail;
    char buffer[RX_BUFF_SIZE];
} ring_buffer_t;
static ring_buffer_t gt_ring_buffer; // ring buffer

typedef struct tag_RX_PACKET_BUFFER
{
    uint8_t idx;
    rx_packet_t packet_buffer[RX_PACKET_SIZE];
} rx_packet_buffer_t;
static rx_packet_buffer_t gt_rx_packet_buffer;

#if 0
    static LOG_LV_T gt_log_lv = LOG_PC;
#else
    static LOG_LV_T gt_log_lv = LOG_RS232;
#endif

static uint8_t gn_uart_rx_timeout;
static bool gb_uart_rx_timeout_enable;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM12_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM8_Init(void);
/* USER CODE BEGIN PFP */
static void TaskDebugUart(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void us_delay(uint16_t us_delay)
{
    LL_TIM_EnableCounter(TIM12);
    while(LL_TIM_GetCounter(TIM12) < us_delay)
    {
    }
    LL_TIM_DisableCounter(TIM12);
    LL_TIM_SetCounter(TIM12, 0);
}

void sys_tick_handler(void)
{
    if (gn_xc_spi_timeout)
    {
        --gn_xc_spi_timeout;
    }
    if (gn_uart_rx_timeout)
    {
        --gn_uart_rx_timeout;
    }
}

void print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;
        char msg_buffer[PRINT_BUFF_SIZE] = {0, };

        if (log_lv > gt_log_lv)
        {
            char temp_buffer[PRINT_BUFF_SIZE] = {0, };
            snprintf(temp_buffer, PRINT_BUFF_SIZE - 1, "%s%s%s", ANSI_FONT_RED, fmt, ANSI_FONT_NONE);
            fmt = temp_buffer;
        }

        va_list ap;
        va_start(ap, fmt);
        len = vsnprintf(msg_buffer, (PRINT_BUFF_SIZE - 1), fmt, ap);
        va_end(ap);

        for (int i = 0 ; i < len ; ++i)
        {
            /* Loop until the end of transmission */
            while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
            /* Echo received character on TX */
            LL_USART_TransmitData8(USART2, (uint8_t)msg_buffer[i]);
        }
    }
}

void comm_init(void)
{
    print(LOG_PC, "\n\r--------------------------------------");
    print(LOG_PC, "\n\r   [GT LED Module Control BD]");
    print(LOG_PC, "\n\r--------------------------------------");
    print(LOG_PC, "\n\r - Author: xxx@glbltech.com");
    print(LOG_PC, "\n\r - Build : %s", __DATE__);
    print(LOG_PC, "\n\r--------------------------------------\r\n");
}

__STATIC_INLINE void UART_PutChar(uint8_t data)
{
    /* Echo received character on TX */
    LL_USART_TransmitData8(USART2, (uint8_t)data);
    /* Loop until the end of transmission */
    while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
}

uint8_t comm_rx_calc_checksum(rx_packet_t* p_packet)
{
    return (uint8_t)(p_packet->sop + p_packet->length + p_packet->command + p_packet->data);
}

void comm_rx_push(uint8_t rx)
{
    gt_ring_buffer.buffer[gt_ring_buffer.head++] = rx;
    gt_ring_buffer.head &= (RX_BUFF_SIZE - 1);
}

uint8_t comm_rx_pop(void)
{
    uint8_t ret = gt_ring_buffer.buffer[gt_ring_buffer.tail++];
    gt_ring_buffer.tail &= (RX_BUFF_SIZE - 1);
    return ret;
}

static uint8_t comm_get_rx_available(void)
{
    if (gt_ring_buffer.head > gt_ring_buffer.tail)
    {
        return gt_ring_buffer.head - gt_ring_buffer.tail;
    }
    else
    {
        return (gt_ring_buffer.head + RX_BUFF_SIZE) - gt_ring_buffer.tail;
    }
}

static void comm_tx_response(uint8_t status)
{
    uint8_t response_ok[6] = { SOP, 0x01, CMD_STATUS, 0xA5, 0x00, 0x5A };
    uint8_t response_ng[6] = { SOP, 0x01, CMD_STATUS, 0x5A, 0x00, 0x5A };

    response_ok[4] = response_ok[0] + response_ok[1] + response_ok[2] + response_ok[3];
    response_ng[4] = response_ng[0] + response_ng[1] + response_ng[2] + response_ng[3];

    if (status)
    {
        for (int i = 0 ; i < 6 ; ++i)
        {
            UART_PutChar(response_ok[i]);
        }
    }
    else
    {
        for (int i = 0 ; i < 6 ; ++i)
        {
            UART_PutChar(response_ng[i]);
        }
    }
}

static uint8_t comm_get_rx_packet(rx_packet_t* p_packet)
{
    uint8_t ret = 0;
    rx_packet_t empty_packet = {0, };

    if (gt_ring_buffer.head != gt_ring_buffer.tail)
    {
        if (gt_ring_buffer.buffer[gt_ring_buffer.tail] != SOP)
        {
            ++gt_ring_buffer.tail;
            gt_ring_buffer.tail &= (RX_BUFF_SIZE - 1);
            ret = 0;
        }
        else // If SOP
        {
            if (!gb_uart_rx_timeout_enable)
            {
                gn_uart_rx_timeout = 5;
                gb_uart_rx_timeout_enable = true;
            }

            if (comm_get_rx_available() >= 6) // Minimum packet size
            {
                empty_packet.sop = comm_rx_pop();
                empty_packet.length = comm_rx_pop();
                empty_packet.command = comm_rx_pop();
                empty_packet.data = comm_rx_pop();
                empty_packet.checksum = comm_rx_pop();
                empty_packet.eop = comm_rx_pop();

                uint8_t checksum = comm_rx_calc_checksum(&empty_packet);
                if ((checksum == empty_packet.checksum) && (empty_packet.eop == EOP))
                {
                    p_packet->sop = empty_packet.sop;
                    p_packet->length = empty_packet.length;
                    p_packet->command = empty_packet.command;
                    p_packet->data = empty_packet.data;
                    p_packet->checksum = empty_packet.checksum;
                    p_packet->eop = empty_packet.eop;

                    comm_tx_response(1);
                    ret = 1;
                }
                else
                {
                    comm_tx_response(0);
                }
                gb_uart_rx_timeout_enable = false;
            }

            if (0 == gn_uart_rx_timeout) // Timeout
            {
                comm_tx_response(0);
                gt_ring_buffer.tail = gt_ring_buffer.head;
                gb_uart_rx_timeout_enable = false;
                ret = 0;
            }
        }
    }

    return ret;
}

static void TaskDebugUart(void)
{
    rx_packet_t* p_packet = gt_rx_packet_buffer.packet_buffer + gt_rx_packet_buffer.idx;

    if (comm_get_rx_packet(p_packet))
    {
        #if 0
            print(LOG_PC, "Recv Packet: {[0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]}\r\n",
                p_packet->sop, p_packet->length, p_packet->command, p_packet->data, p_packet->checksum, p_packet->eop);
        #endif

        switch(p_packet->command)
        {
            case CMD_INIT:
                print(LOG_PC, "CMD_INIT\r\n");
                XC24_Init();
                XDIC_Init();
                Vsync_Timer_Start();
                break;
            case CMD_QUIT:
                print(LOG_PC, "CMD_QUIT\r\n");
                NVIC_SystemReset();
                break;
            case CMD_STATUS:
                print(LOG_PC, "CMD_STATUS\r\n");
                break;
            case CMD_CURRENT:
                LED_Current_Select((float)(p_packet->data));
                print(LOG_PC, "CMD_CURRENT: %d\r\n", p_packet->data);
                break;
            case CMD_BAR_ON_SELECT:
                LED_BAR_On_Select(p_packet->data);
                print(LOG_PC, "CMD_BAR_ON_SELECT: %u\r\n", p_packet->data);
                break;
            case CMD_BAR_OFF_SELECT:
                LED_BAR_Off_Select(p_packet->data);
                print(LOG_PC, "CMD_BAR_OFF_SELECT: %u\r\n", p_packet->data);
                break;
            case CMD_BLK_ON_SELECT:
                LED_BLK_On_Select(p_packet->data);
                print(LOG_PC, "CMD_BLK_ON_SELECT: %u\r\n", p_packet->data);
                break;
            case CMD_BLK_OFF_SELECT:
                LED_BLK_Off_Select(p_packet->data);
                print(LOG_PC, "CMD_BLK_OFF_SELECT: %u\r\n", p_packet->data);
                break;
        }

        ++gt_rx_packet_buffer.idx;
        gt_rx_packet_buffer.idx &= (RX_PACKET_SIZE - 1);
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
  MX_USART2_UART_Init();
  MX_TIM12_Init();
  MX_SPI1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

    XC24_Start_MCLK_Oscillation(false);
    comm_init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    TaskDebugUart();
    XDIC_Vsync_Task();
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
  RCC_OscInitStruct.PLL.PLLN = 144;
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
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 400;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
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
  GPIO_InitStruct.Pin = XC_SPI_CLK_Pin|XC_SPI_MISO_Pin|XC_SPI_MOSI_Pin;
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
  NVIC_SetPriority(TIM8_UP_TIM13_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  TIM_InitStruct.Prescaler = 29;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 39999;
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
  TIM_InitStruct.Prescaler = 71;
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

  /* USART2 DMA Init */

  /* USART2_TX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_4);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_6);

  /* USART2_RX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_5, LL_DMA_CHANNEL_4);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_5);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Stream5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Stream6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);

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
  LL_GPIO_ResetOutputPin(GPIOB, XC24_5V5_Pin|XDIC_5_7V_Pin|XC24_VCC_EN_Pin|XC24_NSCS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);

  /**/
  LL_GPIO_SetOutputPin(XDIC_VCC_EN_GPIO_Port, XDIC_VCC_EN_Pin);

  /**/
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_3
                          |LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_8
                          |LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_4|LL_GPIO_PIN_8
                          |LL_GPIO_PIN_9|LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12
                          |LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_10|LL_GPIO_PIN_12
                          |LL_GPIO_PIN_13|LL_GPIO_PIN_15|LL_GPIO_PIN_3|LL_GPIO_PIN_4
                          |LL_GPIO_PIN_7|LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = XC24_5V5_Pin|XC24_VCC_EN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = XDIC_5_7V_Pin|XDIC_VCC_EN_Pin|XC24_NSCS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = XC_MCLK_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(XC_MCLK_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DEBUG_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DEBUG_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
    LL_GPIO_SetOutputPin(XC24_VCC_EN_GPIO_Port, XC24_VCC_EN_Pin);
  /* USER CODE END MX_GPIO_Init_2 */
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
