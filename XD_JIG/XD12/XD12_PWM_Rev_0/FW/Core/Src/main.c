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

static bool gb_xd_vsync_active_flag;

static bool gb_xd_read_flag;
static uint8_t gn_xd_read_addr;

static bool gb_xd_write_flag;
static uint8_t gn_xd_write_addr;
static uint16_t gn_xd_write_data;

static uint16_t gn_xd_ld_data_table[3] = {1000, 4000, 8000};

#define XD_DELAY_GAP        ((0x3FFF + 1) / 16 - 1)

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

static uint32_t gn_timeout_trimming_procedure_run;
static uint32_t gn_timeout_screening_procedure_run;
static uint32_t gn_timeout_dimming_test_procedure_run;

static double gf_freq_avg;
static double gf_freq_min;
static double gf_freq_max;

static LOG_LV_T gt_log_lv = LOG_DEBUG;

uint8_t gu8_freq_input_dma_flg;
static uint32_t gu32_input_freq_capture[FREQ_IN_IC_LENGTH];

const char* gs_xd12_dev_max_curr_level[DEV_MAX_CURR_LEVEL_MAX] =
{
    "max_4mA",
    "max_8mA",
    "max_12mA",
    "max_16mA",
    "max_24mA",
    "max_32mA",
    "max_46mA",
    "max_64mA",
};

const char* gs_xd12_short_level[SHORT_LEVEL_MAX] =
{
    "short_3V",
    "short_4V",
    "short_6V",
    "short_8V",
    "short_13V",
    "short_16V",
    "short_24V",
    "short_36V",
};

const char* gs_xd12_fb_level[FB_LEVEL_MAX] =
{
    "FB_0.4V",
    "FB_0.5V",
    "FB_0.6V",
    "FB_0.7V",
    "FB_0.85V",
    "FB_1.0V",
    "FB_1.15V",
    "FB_1.3V",
};

const char* gs_jig_gain_level[GAIN_MAX] =
{
    "GAIN_LOW",
    "GAIN_MID",
    "GAIN_HIGH",
};

const char* gs_xd12_reg_type[XD12_REG_TYPE_MAX] =
{
    "XD12_REG_TYPE_NON_TRIM",
    "XD12_REG_TYPE_TRIM",
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM12_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM5_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void sys_tick_handler(void)
{
    if(gn_timeout_trimming_procedure_run)
    {
        --gn_timeout_trimming_procedure_run;
    }
    if(gn_timeout_screening_procedure_run)
    {
        --gn_timeout_screening_procedure_run;
    }
    if(gn_timeout_dimming_test_procedure_run)
    {
        --gn_timeout_dimming_test_procedure_run;
    }
    if (gn_xd_timeout)
    {
        --gn_xd_timeout;
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
    return (uint32_t)(gf_freq_avg + 0.5f);
}

void calc_input_freq(void)
{
    if(gu8_freq_input_capture_activated == 1)
    {
        double f_freq = 0;
        double f_freq_avg = 0;
        uint32_t delta = 0;
        uint32_t n_count = 0;

        for(uint32_t i = 1 ; i < (FREQ_IN_IC_LENGTH - 1) ; ++i)
        {
            if(gu32_input_freq_capture[i + 1] > gu32_input_freq_capture[i + 0])
            {
                delta = (gu32_input_freq_capture[i + 1] - gu32_input_freq_capture[i + 0]);
            }
            else
            {
                delta = (0xFFFFFFFF - gu32_input_freq_capture[i + 0]) + gu32_input_freq_capture[i + 1] + 1;
            }

            f_freq = (TIM5_FREQ / delta);
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
    }
}

static void JigTestMainTask(void);
static void TaskDebugUart(void);

void print(LOG_LV_T log_lv, const char *fmt, ...)
{
    if (log_lv >= gt_log_lv)
    {
        int len = 0;
        char buffer[PRINT_BUFF_SIZE] = {0, };
        va_list ap;

        va_start(ap, fmt);
        len = vsnprintf(buffer, (PRINT_BUFF_SIZE - 1), fmt, ap);
        va_end(ap);
        for(int i = 0 ; i < len ; ++i)
        {
            /* Loop until the end of transmission */
            while (RESET == LL_USART_IsActiveFlag_TXE(USART2));
            /* Echo received character on TX */
            LL_USART_TransmitData8(USART2, (uint8_t)buffer[i]);
        }
    }
}

void comm_print_help()
{
    print(LOG_INFO, "\n\r--Command------------------Description------------------------");

    print(LOG_INFO, "\n\r  ");
    print(LOG_INFO, "\n\r  xd_idgen\t\t : Send idgen cmd to XD12");
    print(LOG_INFO, "\n\r  xd_test_en\t\t : Set mode to Trim of XD12");
    print(LOG_INFO, "\n\r  xd_reg_type [x]\t : Set register type of XD12 [Normal, OTP, Trim]");
    print(LOG_INFO, "\n\r  xd_r [x]\t\t : Read [x] address normal register of XD12 (Input Type : HEX)");
    print(LOG_INFO, "\n\r  xd_rt [x]\t\t : Read [x] address trim register of XD12 (Input Type : HEX)");
    print(LOG_INFO, "\n\r  xd_r_all\t\t : Read all register of XD12");
    print(LOG_INFO, "\n\r  xd_w [x] [y]\t\t : Write [y] to address normal [x] of XD12 (Input Type : HEX)");
    print(LOG_INFO, "\n\r  xd_wt [x] [y]\t\t : Write [y] to address trim [x] of XD12 (Input Type : HEX)");
    print(LOG_INFO, "\n\r  xd_ld_fix [x]\t\t : Write [x] to LD_FIX_1 & LD_FIX_2 of XD12 [0 ~ 127]");
    print(LOG_INFO, "\n\r  xd_short_level [x]\t : Write [x] to short_level bit of fault_level register of XD12 [0 ~ 7]");
    print(LOG_INFO, "\n\r  xd_fb_level [x]\t : Write [x] to fb_level bit of fault_level register of XD12 [0 ~ 7]");
    print(LOG_INFO, "\n\r  xd_max_curr [x]\t : Write [x] to dev_max_current bit of fault_level register of XD12 [0 ~ 7]");
    print(LOG_INFO, "\n\r  xd_ldim [x]\t\t : Write [x] to Local Dimming Data of XD12 [0 ~ 65535]");

    print(LOG_INFO, "\n\r  xd_temp [x]\t\t : Write [x] to OFS_TEMP bit of ld_control register of XD12 [0 ~ 15]");
    print(LOG_INFO, "\n\r  xd_temp_up\t\t : Increase OFS_TEMP bit of ld_control register of XD12");
    print(LOG_INFO, "\n\r  xd_temp_dw\t\t : Decrease OFS_TEMP bit of ld_control register of XD12");

    print(LOG_INFO, "\n\r  ");
    print(LOG_INFO, "\n\r  xc_init\t\t : Initialize XC24");
    print(LOG_INFO, "\n\r  xc_r [x]\t\t : Read [x] address register of XC24 - HEX");
    print(LOG_INFO, "\n\r  xc_w [x] [y]\t\t : Write [y] to address [x] of XC24 - HEX");

    print(LOG_INFO, "\n\r  ");
    print(LOG_INFO, "\n\r  jig_xd_vcc_on\t\t : XD VCC On");
    print(LOG_INFO, "\n\r  jig_xd_vcc_off\t : XD VCC Off");
    print(LOG_INFO, "\n\r  jig_xd_vcc_5\t\t : XD VCC -> 5V");
    print(LOG_INFO, "\n\r  jig_xd_vcc_5.7\t : XD VCC -> 5.7V");
    print(LOG_INFO, "\n\r  jig_xc_vcc_on\t\t : XC VCC On");
    print(LOG_INFO, "\n\r  jig_xc_vcc_off\t : XC VCC Off");
    print(LOG_INFO, "\n\r  jig_ch_sel [x]\t : Set XD CH SEL [x] [0 ~ 11]");
    print(LOG_INFO, "\n\r  jig_vled_on\t\t : VLED On");
    print(LOG_INFO, "\n\r  jig_vled_off\t\t : VLED Off");
    print(LOG_INFO, "\n\r  jig_vled_fb [x]\t : Set VLED for FB Test [x] [0 ~ 2]");
    print(LOG_INFO, "\n\r  jig_vled_fb_up\t : Increase VLED for FB Test");
    print(LOG_INFO, "\n\r  jig_vled_fb_dw\t : Decrease VLED for FB Test");
    print(LOG_INFO, "\n\r  jig_vled_short [x]\t : Set VLED for Short Test [x] [0 ~ 40]");
    print(LOG_INFO, "\n\r  jig_vled_short_up\t : Increase VLED for Short Test");
    print(LOG_INFO, "\n\r  jig_vled_short_dw\t : Decrease VLED for Short Test");
    print(LOG_INFO, "\n\r  jig_gain [x]\t\t : Set JIG Current gain [LOW, MID, HIGH]");
    print(LOG_INFO, "\n\r  jig_gain_charge\t : Set JIG Current gain charge");
    print(LOG_INFO, "\n\r  jig_gain_discharge\t : Set JIG Current gain discharge");
    print(LOG_INFO, "\n\r  jig_vref_adc_on\t : Run MCU's adc sampling (for VREF of XD12)");
    print(LOG_INFO, "\n\r  jig_vsync_start\t : Init XD/XC and start Vsync // Need External VLED");

    print(LOG_INFO, "\n\r  ");

    print(LOG_INFO, "\n\r  reset\t\t : Reset MCU");
    print(LOG_INFO, "\n\r--------------------------------------------------------------\n\r");
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
  MX_SPI2_Init();
  MX_TIM5_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  MX_TIM1_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

    print(LOG_INFO, "\r\nstart %s TRIMMING_JIG application....\r\n", TARGET_CHIP_NAME);
    print(LOG_INFO, "Build time : %s / %s\r\n", __DATE__, __TIME__ );

    /* for GUI App */
    print(LOG_INFO, "XD12 PWM IS SELECTED!\r\n");

#if (XC_TYPE == XC_TYPE_ES1)
    print(LOG_INFO, "XC24 ES1 IS SELECTED!\r\n");
#else
    print(LOG_INFO, "XC24 ES2 IS SELECTED!\r\n");
#endif

    USE_XC24(TRUE);
    print(LOG_INFO, "%s", (IS_XC24() ? "\r\n SUPPORT XC24\r\n" : "\r\n NOT SUPPORT XC24\r\n"));

    Trim_IF_Trim_Set_OTP_Enable(TRUE);
    print(LOG_INFO, "%s", (Trim_IF_Trim_Get_OTP_Enable() ? "XD OTP WRITE ENABLE\r\n" : "XD OTP WRITE DISABLE\r\n"));

    TargetIC_IF_Calculate_Trim_Spec();
    ads114s08_init();
    mcp_wiper_init();

    XC24_start_mclk_oscillation(FALSE);

    //LL_TIM_EnableCounter(TIM1); /* PWM Output for ... */
    LL_TIM_EnableCounter(TIM2); /* PWM Input for ... */
    LL_TIM_EnableCounter(TIM5); /* for Freq Input */

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
    TaskDebugUart();

    if (gb_vsync_out)
    {
        //JigBd_IF_SyncGen_Command();

        //XD12_get_fault_status();

        JigBd_IF_Write_LD_Command(XD12_get_LD_out());

        if (gb_xd_read_flag)
        {
            DEBUG_HI();
            uint16_t xd_read_value = 0;
            xd_read_value = TargetIC_IF_Read_Register(gn_xd_read_addr, XD12_REG_TYPE_NON_TRIM);
            print(LOG_INFO, "xd_r : [0x%02X - 0x%03X]\r\n", gn_xd_read_addr, xd_read_value);
            gb_xd_read_flag = false;
            DEBUG_LO();
        }

        if (gb_xd_write_flag)
        {
            DEBUG_HI();
            TargetIC_IF_Write_Register(gn_xd_write_addr, gn_xd_write_data, XD12_REG_TYPE_NON_TRIM);
            gb_xd_write_flag = false;
            DEBUG_LO();
        }

        gb_vsync_out = false;
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
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
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
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**SPI3 GPIO Configuration
  PB2   ------> SPI3_MOSI
  PC10   ------> SPI3_SCK
  PC11   ------> SPI3_MISO
  */
  GPIO_InitStruct.Pin = MCP_SPI_MOSI_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(MCP_SPI_MOSI_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MCP_SPI_CLK_Pin|MCP_SPI_MISO_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV4;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI3, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI3, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

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

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 143;
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

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&(TIM1->CCR1)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)gu16_pwm_tx_risingBuffer);

    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
    //LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_1);

    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableDMAReq_CC1(TIM1);

    LL_DMA_ClearFlag_FE1(DMA2);
    LL_DMA_ClearFlag_HT1(DMA2);
    LL_DMA_ClearFlag_TC1(DMA2);

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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM2 GPIO Configuration
  PB3   ------> TIM2_CH2
  */
  GPIO_InitStruct.Pin = PWM_IN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(PWM_IN_GPIO_Port, &GPIO_InitStruct);

  /* TIM2 DMA Init */

  /* TIM2_CH2_CH4 Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_3);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_6);

  /* USER CODE BEGIN TIM2_Init 1 */

    LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_5, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_5);

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI2FP2);
  LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);
  LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
  LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_DisableIT_TRIG(TIM2);
  LL_TIM_DisableDMAReq_TRIG(TIM2);
  LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_INDIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_FALLING);
  LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)(&(TIM2->CCR2)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gu16_pwm_rx_risingBuffer);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)(&(TIM2->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gu16_pwm_rx_fallingBuffer);

    //LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_6);
    //LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_6);

#if 1
    LL_TIM_DisableDMAReq_CC1(TIM2);
    LL_TIM_DisableDMAReq_CC2(TIM2);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
#else
    LL_TIM_EnableDMAReq_CC1(TIM2);
    LL_TIM_EnableDMAReq_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
#endif

    //LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_6);
    //LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_6);

    LL_DMA_ClearFlag_FE5(DMA1);
    LL_DMA_ClearFlag_FE6(DMA1);
    LL_DMA_ClearFlag_HT5(DMA1);
    LL_DMA_ClearFlag_HT6(DMA1);
    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_ClearFlag_TC6(DMA1);

  /* USER CODE END TIM2_Init 2 */

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

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
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
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  /* DMA interrupt init */
  /* DMA1_Stream2_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Stream2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Stream6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
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
  LL_GPIO_SetOutputPin(GPIOC, LTC_HIGH_CURRENT_Pin|MCP_SPI_CS_Pin|ENABLE_SELECT1_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOA, LTC_LOW_CURRENT_Pin|LTC_MID_CURRENT_Pin|ADC_CS_Pin|ENABLE_SELECT2_Pin);

  /**/
  LL_GPIO_SetOutputPin(GPIOB, ENABLE_SELECT4_Pin|ENABLE_SELECT3_Pin|CNT_MR_Pin|VLED_CTR_9V_Pin
                          |XD12_VCC_EN_Pin|ADC_RESET_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOC, PWM_SWITCH_Pin|XD_FB_IN_Pin);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, XD12_5_7V_Pin|XC24_VCC_EN_Pin|XC_NSCS_Pin);

  /**/
  LL_GPIO_ResetOutputPin(ADC_START_GPIO_Port, ADC_START_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);

  /**/
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LTC_HIGH_CURRENT_Pin|PWM_SWITCH_Pin|MCP_SPI_CS_Pin|ENABLE_SELECT1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
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
  GPIO_InitStruct.Pin = XD_FB_IN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(XD_FB_IN_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ENABLE_SELECT4_Pin|ENABLE_SELECT3_Pin|XD12_5_7V_Pin|VLED_CTR_9V_Pin
                          |XD12_VCC_EN_Pin|XC_NSCS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = XD_FB_OUT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(XD_FB_OUT_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CNT_MR_Pin|XC24_VCC_EN_Pin|ADC_RESET_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = MCLK_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(MCLK_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = ADC_START_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(ADC_START_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_11|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DEBUG_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DEBUG_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
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
	GPIO_InitStruct.Pin = XD_FB_IN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(XD_FB_IN_GPIO_Port, &GPIO_InitStruct);
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

    if(gn_timeout_screening_procedure_run == 0)
    {
        screening_procedure_run();
        gn_timeout_screening_procedure_run = 1;
    }

    if(gn_timeout_dimming_test_procedure_run == 0)
    {
        dimming_procedure_run();
        gn_timeout_dimming_test_procedure_run = 1;
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
        char str_in[UART_CMD_LENGTH + 1] = {0, };
        uint32_t u32_recv_param[6] = {0, };
        double lf_recv_param[6] = {0,};

        memcpy(str_in, p_data->buffer, p_data->length);
        p_data->length = 0;

        if(Command_is_("help") || Command_is_("?"))
        {
            comm_print_help();
        }
#if 1
/* ----------------- command list - jig ----------------- */
        else if(Command_is_("jig_ic_start"))
        {
            print(LOG_INFO, "\r\n Start timer for freq input capture\r\n");
            JigBd_IF_TIM_Capture_Start();
        }
        else if(Command_is_("jig_ic_stop"))
        {
            JigBd_IF_TIM_Capture_Stop();
            print(LOG_INFO, "\r\n Stop timer for freq input capture : %1.6f(%u) [kHz] => %1.6f [MHz]\r\n", gf_freq_avg/1000, JigBd_IF_Freq_Get(), ((gf_freq_avg/1000000) * TIM_CAPTURE_EXT_PRESCALER));
        }
        else if(Command_is_("jig_vref_start"))
        {
            print(LOG_INFO, "\r\n ADC RUN\r\n");
            JigBd_IF_VREF_ADC_StartStop();
        }
        else if(Command_Param_is_("jig_ch_sel", "%d", &u32_recv_param[0]))
        {
            print(LOG_INFO, "\r\n Output CH sel - [%u]\r\n", u32_recv_param[0]);
            JigBd_IF_select_output_ch(u32_recv_param[0]);
        }
        else if(Command_Param_is_("jig_xd_vcc", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                print(LOG_INFO, "\r\n XD VCC On [5V]\r\n");
                JigBd_IF_XD_VCC_EN(PWR_ON);
            }
            else
            {
                print(LOG_INFO, "\r\n XD VCC Off [0V]\r\n");
                JigBd_IF_XD_VCC_EN(PWR_OFF);
            }
        }
        else if(Command_Param_is_("jig_xd_vcc_level", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                print(LOG_INFO, "\r\n XD VCC [5.7V]\r\n");
                JigBd_IF_XD_VCC_Level(PWR_ON_5V7);
            }
            else
            {
                print(LOG_INFO, "\r\n XD VCC [5V]\r\n");
                JigBd_IF_XD_VCC_Level(PWR_ON_5V0);
            }
        }
        else if(Command_Param_is_("jig_xc_vcc", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                print(LOG_INFO, "\r\n XC VCC On\r\n");
                JigBd_IF_XC_VCC_EN(PWR_ON);
            }
            else
            {
                print(LOG_INFO, "\r\n XC VCC Off\r\n");
                JigBd_IF_XC_VCC_EN(PWR_OFF);
            }
        }
        else if(Command_Param_is_("jig_vled", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                print(LOG_INFO, "\r\n VLED_Enable\r\n");
                JigBd_IF_VLED_9V_EN(PWR_ON);
            }
            else
            {
                print(LOG_INFO, "\r\n VLED_Disable\r\n");
                JigBd_IF_VLED_9V_EN(PWR_OFF);
            }
        }
        else if(Command_Param_is_("jig_ch_sel", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] > (CH_MAX + 1))
            {
                print(LOG_ERROR, "\r\n Out of CH SEL [%d] [0 - %d]\r\n", u32_recv_param[0], CH_MAX);
            }
            else
            {
                print(LOG_INFO, "\r\n XD CH SEL - [%d]\r\n", u32_recv_param[0]);
                JigBd_IF_select_output_ch(u32_recv_param[0]);
            }
        }
        else if(Command_Param_is_("jig_vled_fb", "%lf", &lf_recv_param[0]))
        {
            if (lf_recv_param[0] > FAULT_FB_VLED_MAX)
            {
                print(LOG_ERROR, "\r\n Out of Fault FB VLED Level [%.2f] [0 - %.1f]\r\n", lf_recv_param[0], FAULT_FB_VLED_MAX);
            }
            else
            {
                print(LOG_INFO, "\r\n VLED for FB Test - [%.2lf]\r\n", lf_recv_param[0]);
                XD12_set_FB_vled(lf_recv_param[0]);
            }
        }
        else if(Command_is_("jig_vled_fb_up"))
        {
            print(LOG_INFO, "\r\n Fault FB VLED Level Increase\r\n");
            mcp_wiper_level_incr(MCP_WIPER_PORT_0);
        }
        else if(Command_is_("jig_vled_fb_dw"))
        {
            print(LOG_INFO, "\r\n Fault FB VLED Level Decrease\r\n");
            mcp_wiper_level_decr(MCP_WIPER_PORT_0);
        }
        else if(Command_Param_is_("jig_vled_short", "%lf", &lf_recv_param[0]))
        {
            if (lf_recv_param[0] > FAULT_SHORT_VLED_MAX)
            {
                print(LOG_ERROR, "\r\n Out of Fault Short VLED Level [%.2f] [0 - %.1f]\r\n", lf_recv_param[0], FAULT_SHORT_VLED_MAX);
            }
            else
            {
                print(LOG_INFO, "\r\n VLED for Short Test - [%.2lf]\r\n", lf_recv_param[0]);
                XD12_set_short_vled(lf_recv_param[0]);
            }
        }
        else if(Command_is_("jig_vled_short_up"))
        {
            print(LOG_INFO, "\r\n Fault Short VLED Level Increase\r\n");
            mcp_wiper_level_incr(MCP_WIPER_PORT_1);
        }
        else if(Command_is_("jig_vled_short_dw"))
        {
            print(LOG_INFO, "\r\n Fault Short VLED Level Decrease\r\n");
            mcp_wiper_level_decr(MCP_WIPER_PORT_1);
        }
        else if(Command_Param_is_("jig_gain", "%u", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < GAIN_MAX)
            {
                XD12_set_dimming_gain((current_gain_t)u32_recv_param[0]);
                JigBd_IF_change_current_gain(XD12_get_dimming_gain());
                print(LOG_INFO, "\r\n Set JIG Current [%s]\r\n", gs_jig_gain_level[u32_recv_param[0]]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of Jig Current Gain Level [%u] [0 - %u]\r\n", u32_recv_param[0], GAIN_MAX-1);
            }
        }
        else if(Command_Param_is_("jig_vsync", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                global_dimming_start();
                gb_xd_vsync_active_flag = true;
                print(LOG_INFO, "vsync start\r\n");
            }
            else
            {
                LL_TIM_DisableCounter(TIM8);
                LL_TIM_SetCounter(TIM8, 0);
                LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
                LL_TIM_DisableIT_UPDATE(TIM8);
                gb_xd_vsync_active_flag = false;
                print(LOG_INFO, "vsync stop\r\n");
            }
            print(LOG_INFO, "supply external VLED\r\n");
        }
        else if(Command_Param_is_("vsync", "%lf", &lf_recv_param[0]))
        {
            vsync_update_frequency((float)lf_recv_param[0]);
            print(LOG_INFO, "set vsync freq to %.3lfHz\r\n", lf_recv_param[0]);
        }
        else if(Command_Param_is_("pwm_freq", "%lf", &lf_recv_param[0]))
        {
            uint32_t tim_arr = (uint32_t)(144000/lf_recv_param[0] + 0.5f - 1);

            LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
            LL_TIM_DisableCounter(TIM1);
            LL_TIM_SetCounter(TIM1, 0);
            LL_TIM_OC_SetCompareCH1(TIM1, 0);

            LL_TIM_SetAutoReload(TIM1, tim_arr);

            LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
            LL_TIM_EnableCounter(TIM1);
            print(LOG_INFO, "set pwm freq to %.3lf kHz\r\n", lf_recv_param[0]);
        }
#endif
/* ----------------- command list - xd ----------------- */
        else if(Command_is_("xd_idgen"))
        {
            print(LOG_INFO, "\r\n XD IDGen\r\n");
            JigBd_IF_IdGen_Command();
        }
        else if(Command_is_("xd_reset"))
        {
            print(LOG_INFO, "\r\n XD Reset\r\n");
            JigBd_IF_XD12_reset_Command();
            us_tdelay(5);
        }
        else if(Command_is_("xd_syncgen"))
        {
            print(LOG_INFO, "\r\n XD Syncgen\r\n");
            JigBd_IF_SyncGen_Command();
            us_tdelay(5);
        }
        else if(Command_is_("xd_test_en"))
        {
            print(LOG_INFO, "\r\n XD TEST_EN\r\n");
            TargetIC_IF_TRIM_Init_VREF_CTL();
        }
        else if(Command_Param_is_("xd_reg_type", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < XD12_REG_TYPE_MAX)
            {
                print(LOG_INFO, "\r\n xd_reg_type - [%u] [%s]\r\n", u32_recv_param[0], gs_xd12_reg_type[u32_recv_param[0]]);
                XD12_Set_Reg_Type((XD12_REG_TYPE)u32_recv_param[0]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd_reg_type [%u] [0 - %u]\r\n", u32_recv_param[0], XD12_REG_TYPE_MAX - 1);
            }
        }
        else if(Command_is_("xd_reg_type"))
        {
            XD12_REG_TYPE XD12_REG_TYPE_T;
            XD12_REG_TYPE_T = XD12_Get_Reg_Type();
            print(LOG_INFO, "\r\n xd_reg_type - [%u] [%s]\r\n", XD12_REG_TYPE_T, gs_xd12_reg_type[XD12_REG_TYPE_T]);
        }
        else if(Command_is_("xd_r_all"))
        {
            print(LOG_INFO, "\r\n Read XD's register all\r\n");
            TargetIC_IF_XD_Regs_Read_Display();
        }
        else if(Command_Param_is_("xd_r", "%x", &u32_recv_param[0]))
        {
            if (gb_xd_vsync_active_flag)
            {
                gn_xd_read_addr = u32_recv_param[0];
                gb_xd_read_flag = true;
            }
            else
            {
                uint16_t ret = TargetIC_IF_Read_Register((uint8_t)u32_recv_param[0], XD12_REG_TYPE_NON_TRIM);
            }

            print(LOG_INFO, "\r\n OK\r\n");
        }
        else if(Command_Param_is_("xd_rt", "%x", &u32_recv_param[0]))
        {
            uint16_t ret = TargetIC_IF_Read_Register((uint8_t)u32_recv_param[0], XD12_REG_TYPE_TRIM);
            print(LOG_INFO, "\r\n XD Read : 0x%02X : 0x%04X\r\n", u32_recv_param[0], ret);
        }
        else if(Command_Param_is_("xd_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            if (gb_xd_vsync_active_flag)
            {
                gn_xd_write_addr = u32_recv_param[0];
                gn_xd_write_data = u32_recv_param[1];
                gb_xd_write_flag = true;
            }
            else
            {
                TargetIC_IF_Write_Register((uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12_REG_TYPE_NON_TRIM);
            }

            print(LOG_INFO, "\r\n XD Write : [Reg Type : %s]0x%02X - 0x%02X\r\n", gs_xd12_reg_type[XD12_REG_TYPE_NON_TRIM], (uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
            print(LOG_INFO, "\r\n OK\r\n");
        }
        else if(Command_Param_is_("xd_wt", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            print(LOG_INFO, "\r\n XD Write : [Reg Type : %s]0x%02X - 0x%02X\r\n", gs_xd12_reg_type[XD12_REG_TYPE_TRIM], (uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
            TargetIC_IF_Write_Register((uint8_t)u32_recv_param[0], (uint16_t)u32_recv_param[1], XD12_REG_TYPE_TRIM);
        }
        else if(Command_Param_is_("xd_short_level", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= REG_LIMIT_SHORT_LEVEL)
            {
                _xd12_fault_level_t xd12_fault_level;

                xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
                xd12_fault_level.short_level = u32_recv_param[0];

                TargetIC_IF_Write_Register(XD12_ADDR_FAULT_LEVEL, xd12_fault_level.val, XD12_REG_TYPE_NON_TRIM);

                print(LOG_INFO, "\r\n xd12_short_level - [%u] [%s]\r\n", u32_recv_param[0], gs_xd12_short_level[u32_recv_param[0]]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_short_level range [%u] [0 - %u]\r\n", u32_recv_param[0], REG_LIMIT_SHORT_LEVEL);
            }
        }
        else if(Command_is_("xd_short_level"))
        {
            _xd12_fault_level_t xd12_fault_level;
            xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
            print(LOG_INFO, "\r\n xd12_short_level - [%u] [%s]\r\n", xd12_fault_level.short_level, gs_xd12_short_level[xd12_fault_level.short_level]);
        }
        else if(Command_Param_is_("xd_fb_level", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= REG_LIMIT_FB_LEVEL)
            {
                _xd12_fault_level_t xd12_fault_level;

                xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
                xd12_fault_level.fb_level = u32_recv_param[0];
                TargetIC_IF_Write_Register(XD12_ADDR_FAULT_LEVEL, xd12_fault_level.val, XD12_REG_TYPE_NON_TRIM);

                print(LOG_INFO, "\r\n xd12_fb_level - [%u] [%s]\r\n", u32_recv_param[0], gs_xd12_fb_level[u32_recv_param[0]]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_fb_level range [%u] [0 - %u]\r\n", u32_recv_param[0], REG_LIMIT_FB_LEVEL);
            }
        }
        else if(Command_is_("xd_fb_level"))
        {
            _xd12_fault_level_t xd12_fault_level;
            xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
            print(LOG_INFO, "\r\n xd12_fb_level - [%u] [%s]\r\n", xd12_fault_level.fb_level, gs_xd12_fb_level[xd12_fault_level.fb_level]);
        }
        else if(Command_Param_is_("xd_max_curr", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= REG_LIMIT_DEV_MAX_CURR_LEVEL)
            {
                _xd12_fault_level_t xd12_fault_level;

                xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
                xd12_fault_level.dev_max_curr_level = u32_recv_param[0];
                TargetIC_IF_Write_Register(XD12_ADDR_FAULT_LEVEL, xd12_fault_level.val, XD12_REG_TYPE_NON_TRIM);

                print(LOG_INFO, "\r\n xd12_max_curr - [%u] [%s]\r\n", xd12_fault_level.dev_max_curr_level, gs_xd12_dev_max_curr_level[xd12_fault_level.dev_max_curr_level]);

            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_max_curr range [%u] [0 - %u]\r\n", u32_recv_param[0], REG_LIMIT_DEV_MAX_CURR_LEVEL);
            }
        }
        else if(Command_is_("xd_max_curr"))
        {
            _xd12_fault_level_t xd12_fault_level;
            xd12_fault_level.val = TargetIC_IF_Read_Register(XD12_ADDR_FAULT_LEVEL, XD12_REG_TYPE_NON_TRIM);
            print(LOG_INFO, "\r\n xd12_max_curr - [%u] [%s]\r\n", xd12_fault_level.dev_max_curr_level, gs_xd12_dev_max_curr_level[xd12_fault_level.dev_max_curr_level]);
        }
        else if(Command_Param_is_("xd_vref", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 4095)
            {
                _xd12_max_current_vref_t xd12_max_current_vref;

                xd12_max_current_vref.val = TargetIC_IF_Read_Register(XD12_ADDR_MAX_CURR_VREF, XD12_REG_TYPE_NON_TRIM);
                xd12_max_current_vref.max_curr_vref = u32_recv_param[0];
                TargetIC_IF_Write_Register(XD12_ADDR_MAX_CURR_VREF, xd12_max_current_vref.val, XD12_REG_TYPE_NON_TRIM);

                print(LOG_INFO, "\r\n xd12_vref - [%u]\r\n", xd12_max_current_vref.max_curr_vref);

            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_vref [%u] [0 - %u]\r\n", u32_recv_param[0], 4095);
            }
        }
        else if(Command_is_("xd_vref"))
        {
            _xd12_max_current_vref_t xd12_max_current_vref;
            xd12_max_current_vref.val = TargetIC_IF_Read_Register(XD12_ADDR_MAX_CURR_VREF, XD12_REG_TYPE_NON_TRIM);
            print(LOG_INFO, "\r\n xd12_vref - [%u]\r\n", xd12_max_current_vref.max_curr_vref);
        }
        else if(Command_Param_is_("xd_ldim", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 65535)
            {
                print(LOG_INFO, "\r\n Set ldim to [%u]\r\n", u32_recv_param[0]);
                XD12_set_LD_out(u32_recv_param[0]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_ldim [%u] [0 - %u]\r\n", u32_recv_param[0], 65535);
            }
        }
        else if(Command_is_("xd_ldim"))
        {
            print(LOG_INFO, "\r\n ldim - [%u]\r\n", XD12_get_LD_out());
        }

        else if(Command_Param_is_("xd_dimming_ch", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < (CH_MAX + 1))
            {
                print(LOG_INFO, "\r\n Set xd dimming ch to [%u]\r\n", u32_recv_param[0]);
                XD12_set_dimming_channel(u32_recv_param[0]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd_dimming_ch [%u] [0 - %u]\r\n", u32_recv_param[0], 12);
            }
        }
        else if(Command_is_("xd_dimming_ch"))
        {
            print(LOG_INFO, "\r\n xd_dimming_ch - [%u]\r\n", XD12_get_dimming_channel());
        }

        else if(Command_Param_is_("xd_temp", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] <= 15)
            {
                _xd12_temp_t xd12_temp;
                xd12_temp.val = TargetIC_IF_Read_Register(XD12_ADDR_TEMP, XD12_REG_TYPE_NON_TRIM);
                xd12_temp.ofs_temp = u32_recv_param[0];
                TargetIC_IF_Write_Register(XD12_ADDR_TEMP, xd12_temp.val, XD12_REG_TYPE_NON_TRIM);
                print(LOG_INFO, "\r\n xd_temp - [%u]\r\n", u32_recv_param[0]);
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of xd12_temp [%u] [0 - %u]\r\n", u32_recv_param[0], 15);
            }
        }
        else if(Command_is_("xd_temp_up"))
        {
            _xd12_temp_t xd12_temp;
            xd12_temp.val = TargetIC_IF_Read_Register(XD12_ADDR_TEMP, XD12_REG_TYPE_NON_TRIM);
            if (xd12_temp.ofs_temp < 15)
            {
                ++xd12_temp.ofs_temp;
                TargetIC_IF_Write_Register(XD12_ADDR_TEMP, xd12_temp.val, XD12_REG_TYPE_NON_TRIM);
                print(LOG_INFO, "\r\n xd12_temp_up - [%u]\r\n", xd12_temp.ofs_temp);
            }
            else
            {
                print(LOG_INFO, "\r\n xd12_temp is max!! - [%u]\r\n", xd12_temp.ofs_temp);
            }
        }
        else if(Command_is_("xd_temp_dw"))
        {
            _xd12_temp_t xd12_temp;
            xd12_temp.val = TargetIC_IF_Read_Register(XD12_ADDR_TEMP, XD12_REG_TYPE_NON_TRIM);
            if (xd12_temp.ofs_temp > 0)
            {
                --xd12_temp.ofs_temp;
                TargetIC_IF_Write_Register(XD12_ADDR_TEMP, xd12_temp.val, XD12_REG_TYPE_NON_TRIM);
                print(LOG_INFO, "\r\n xd12_temp_down - [%u]\r\n", xd12_temp.ofs_temp);
            }
            else
            {
                print(LOG_INFO, "\r\n xd12_temp is min!! - [%u]\r\n", xd12_temp.ofs_temp);
            }
        }
        else if(Command_Param_is_("xd_fbi", "%x", &u32_recv_param[0]))
        {
            if (u32_recv_param[0])
            {
                XD_FBI_HI();
            }
            else
            {
                XD_FBI_LO();
            }
            print(LOG_INFO, "\r\n XD FBI %s\r\n", (u32_recv_param[0] ? "HI" : "LO"));
        }
        else if(Command_is_("xd_fbo"))
        {
            print(LOG_INFO, "\r\n XD FBO %s\r\n", (XD_FBO_READ() ? "HI" : "LO"));
        }
        else if(Command_is_("xd_debug"))
        {
            JigBd_IF_XD_VCC_EN(PWR_ON);
            print(LOG_INFO, "\r\n xd_vcc_on\r\n");

            if(IS_XC24())
            {
                XC24_Initialize();
                print(LOG_INFO, "\r\n xc_init\r\n");
                LL_mDelay(10);
            }

            JigBd_IF_select_output_ch(CH_MAX);
            print(LOG_INFO, "\r\n jig_ch_sel_0\r\n");
            LL_mDelay(10);

            JigBd_IF_change_current_gain(GAIN_HIGH);
            print(LOG_INFO, "\r\n jig_gain_high\r\n");
            LL_mDelay(10);

            XD12_Initialize();
            TargetIC_IF_XD_Regs_Read_Display();

            JigBd_IF_VLED_9V_EN(PWR_ON);
            print(LOG_INFO, "\r\n xd_vled_on\r\n");
        }
        else if(Command_is_("xd_trim_debug"))
        {
            JigBd_IF_XD_VCC_EN(PWR_ON);
            print(LOG_INFO, "\r\n xd_vcc_on\r\n");

            if(IS_XC24())
            {
                XC24_Initialize();
                print(LOG_INFO, "\r\n xc_init\r\n");
                LL_mDelay(10);
            }

            JigBd_IF_select_output_ch(0);
            print(LOG_INFO, "\r\n jig_ch_sel_0\r\n");
            LL_mDelay(10);

            JigBd_IF_change_current_gain(GAIN_HIGH);
            print(LOG_INFO, "\r\n jig_gain_high\r\n");
            LL_mDelay(10);

            XD12_Trim_Initialize();
            TargetIC_IF_XD_Regs_Read_Display();

            JigBd_IF_VLED_9V_EN(PWR_ON);
            print(LOG_INFO, "\r\n xd_vled_on\r\n");
        }
        else if(Command_is_("xd_trim_vref"))
        {
            TargetIC_IF_TRIM_Init_VREF_CTL();
        }
        else if(Command_is_("xd_trim_osc"))
        {
            TargetIC_IF_TRIM_Init_OSC();
        }
        else if(Command_Param_is_("xd_trim_ictl_l", "%x", &u32_recv_param[0]))
        {
            TargetIC_IF_TRIM_Init_ICTL_L();
            XD12_set_CHANNEL_ENABLE_All();
            JigBd_IF_select_output_ch(u32_recv_param[0]);
            JigBd_IF_change_current_gain(GAIN_MID);
            XD12_set_max_curr_level(DEV_MAX_CURR_LEVEL_8mA);
        }
        else if(Command_Param_is_("xd_trim_ictl_h", "%x", &u32_recv_param[0]))
        {
            TargetIC_IF_TRIM_Init_ICTL_H();
            XD12_set_CHANNEL_ENABLE_All();
            JigBd_IF_select_output_ch(u32_recv_param[0]);
            JigBd_IF_change_current_gain(GAIN_HIGH);
            XD12_set_max_curr_level(DEV_MAX_CURR_LEVEL_32mA);
        }

        else if(Command_is_("xd_osc_debug"))
        {
            JigBd_IF_XD_VCC_EN(PWR_ON);
            print(LOG_INFO, "\r\n xd_vcc_on\r\n");

            if(IS_XC24())
            {
                XC24_Initialize();
                print(LOG_INFO, "\r\n xc_init\r\n");
            }

            JigBd_IF_select_output_ch(0);
            print(LOG_INFO, "\r\n jig_ch_sel_0\r\n");
            LL_mDelay(10);

            JigBd_IF_change_current_gain(GAIN_HIGH);
            print(LOG_INFO, "\r\n jig_gain_high\r\n");
            LL_mDelay(10);

            XD12_Trim_Initialize();
            TargetIC_IF_XD_Regs_Read_Display();

            JigBd_IF_VLED_9V_EN(PWR_ON);
            TargetIC_IF_TRIM_Init_OSC();
            print(LOG_INFO, "\r\n xd_vled_on\r\n");

            for (uint8_t i = 0 ; i <= REG_LIMIT_OSC ; ++i)
            {
                TargetIC_IF_Write_Register(0x01, i, XD12_REG_TYPE_TRIM);
                JigBd_IF_TIM_Capture_Start();
                LL_mDelay(150);
                JigBd_IF_TIM_Capture_Stop();

                print(LOG_INFO, "\r\n reg:%3u:osc:%f:MHz", i, JigBd_IF_Freq_Count_to_MHZ(JigBd_IF_Freq_Get()));
            }
        }

/* ----------------- command list - xc ----------------- */
        else if(Command_is_("xc_init"))
        {
            print(LOG_INFO, "\r\n INIT_XC24\r\n");
            XC24_Initialize();
        }
        else if(Command_Param_is_("xc_w", "%x %x", &u32_recv_param[0], &u32_recv_param[1]))
        {
            print(LOG_INFO, "\r\n XC Write : 0x%02X - 0x%02X\r\n", u32_recv_param[0], u32_recv_param[1]);
            XC24_write_register((uint16_t)u32_recv_param[0], (uint16_t)u32_recv_param[1]);
        }
        else if(Command_is_("xc_r_all"))
        {
            XC24_read_register_all();
        }
        else if(Command_Param_is_("xc_r", "%x", &u32_recv_param[0]))
        {
            uint16_t ret = XC24_read_register((uint8_t)u32_recv_param[0]);
            print(LOG_INFO, "\r\n XC Read : 0x%02X : 0x%04X\r\n", u32_recv_param[0], ret);
        }
/* ----------------- command list - ui ----------------- */
        else if(Command_is_("ui:gdim"))
        {
            print(LOG_INFO, "\r\n XD12 Dimming Test Start\r\n");
            XD12_start_dimming_test();
        }
        else if(Command_Param_is_("ui:use_xc", "%d", &u32_recv_param[0]))
        {
            if (u32_recv_param[0] < 2)
            {
                if (u32_recv_param[0])
                {
                    USE_XC24(true);
                    print(LOG_INFO, "\r\n SUPPORT_XC24\r\n");
                }
                else
                {
                    USE_XC24(false);
                    print(LOG_INFO, "\r\n NOT_SUPPORT_XC24\r\n");
                }
            }
            else
            {
                print(LOG_ERROR, "\r\n Out of ui:use_xc [%u] [0 - %u]\r\n", u32_recv_param[0], 1);
            }
        }
        else if(Command_is_("ui:gdebug"))
        {
            print(LOG_INFO, "\r\n XD12 debug Dimming Test Start\r\n");
            XD12_start_debug_dimming_test();
        }
        else if(Command_is_("xd_trim_start") || Command_is_("1"))
        {
            print(LOG_INFO, "\r\n XD12 OTP Write EN & Activate \r\n");
            Trim_IF_Trimming_Start();
        }
        else if(Command_is_("xd_screen_start") || Command_is_("2"))
        {
            print(LOG_INFO, "\r\n XD12 Screen Start \r\n");
            Trim_IF_Screening_Start();
        }
        else if(Command_Param_is_("log_lv", "%d", &u32_recv_param[0]))
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
        else if(Command_is_("reset"))
        {
            print(LOG_INFO, "\r\n system reset \r\n");
            NVIC_SystemReset();
        }
        else
        {
            print(LOG_INFO, "\r\nWhat?\n\r");
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
    else if(rx == UART_BACKSPACE)
    {
        if(gt_rx_uart.Rxbuff[gt_rx_uart.RxInCnt].length)
        {
            uint8_t temp[2] = {' ', UART_BACKSPACE};

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
