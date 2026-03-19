/** @file JigBd_IF.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __JIG_BD_IF_C__

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "config.h"
#include "xdic.h"
#include "JigBd_IF.h"
#include "uart.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LD_DATA_14B_MAX             (0x3FFF)

#define TIM1_PERIOD                 (179)
#define _LOGIC_0_                   (uint16_t)(((TIM1_PERIOD + 1U) * 1.0f / 3.0f) - 1U + 0.5f)
#define _LOGIC_1_                   (uint16_t)(((TIM1_PERIOD + 1U) * 2.0f / 3.0f) - 1U + 0.5f)

#define BIT_1_RATIO                 (2.0f)
#define BIT_0_RATIO                 (1.0f)
#define BIT_RATIO_SUM               (3.0f)

#define D_ERR_RATE                  (0.3)
#define XD_LOGIC_0_DUTY             ((double)XDIC_SERIAL_CLK_CNT_LOW / (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))
#define XD_LOGIC_1_DUTY             ((double)XDIC_SERIAL_CLK_CNT_HIGH / (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))

#define LOGIC_0_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_0_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_0_DUTY * (1 + D_ERR_RATE))))
#define LOGIC_1_COUNT_MIN(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 - D_ERR_RATE))))
#define LOGIC_1_COUNT_MAX(N)        ((uint16_t)((N) * (XD_LOGIC_1_DUTY * (1 + D_ERR_RATE))))

#define SERIAL_DECODE_MASK_CODE     (uint32_t)(0x0F)
#define SERIAL_DECODE_MASK_ID       (uint32_t)(0x1F)
#define SERIAL_DECODE_MASK_DATA     (uint32_t)(0xFFF)

#define BOOT_PATTERN_CHANGE_COUNT   (120U)
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_LED_BUFFER_UPDATE_TYPE_T
{
    LED_UPDATE_COLOR = 0,   // By Color
    LED_UPDATE_PATTERN,     // By Pattern
    LED_UPDATE_PIXEL,       // By Pattern
    LED_UPDATE_MAX,
} led_buffer_update_type_t;

typedef enum tag_BOOT_STATE_T
{
    BOOT_STATE_PATTERN = 0U,
    BOOT_STATE_COLOR,
    BOOT_STATE_NORMAL
} boot_state_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_serialize_tx_buffer[XDIC_LD_TRANS_BITS + 10U] = {0};
static uint16_t gn_serialize_rx_risingBuffer[400] = {0};
static uint16_t gn_serialize_rx_fallingBuffer[400] = {0};

volatile uint16_t gn_xd_rx_timeout;
volatile bool gb_xd_timeout_event;
volatile bool gb_pwm_dma_tx_flag;
volatile bool gb_pwm_is_rx_flag;

static uint16_t gn_mcu_serializer_bit_0;
static uint16_t gn_mcu_serializer_bit_1;

static led_color_type_t gt_led_color;

static uint8_t gn_led_brightness_percent; // 0 ~ 100

static uint8_t gn_led_pixel; // 0 ~ 100

static led_buffer_update_type_t gt_led_buffer_update_type;

static led_pattern_t gt_led_pattern;

static led_ld_buffer_t gt_led_ld_buffer[LED_MAP_SIZE] = {0};

const static uint8_t gt_led_id_map[LED_MAP_SIZE] =
{
      1U,   0U,  17U,  16U,  33U,  32U,
     49U,  48U,  65U,  64U,  81U,  80U,
     97U,  96U, 113U, 112U, 129U, 128U,
      3U,   2U,  19U,  18U,  35U,  34U,
     51U,  50U,  67U,  66U,  83U,  82U,
     99U,  98U, 115U, 114U, 131U, 130U,
      5U,   4U,  21U,  20U,  37U,  36U,
     53U,  52U,  69U,  68U,  85U,  84U,
    101U, 100U, 117U, 116U, 133U, 132U,
      7U,   6U,  23U,  22U,  39U,  38U,
     55U,  54U,  71U,  70U,  87U,  86U,
    103U, 102U, 119U, 118U, 135U, 134U,
      9U,   8U,  25U,  24U,  41U,  40U,
     57U,  56U,  73U,  72U,  89U,  88U,
    105U, 104U, 121U, 120U, 137U, 136U,
     11U,  10U,  27U,  26U,  43U,  42U,
     59U,  58U,  75U,  74U,  91U,  90U,
    107U, 106U, 123U, 122U, 139U, 138U,
     13U,  12U,  29U,  28U,  45U,  44U,
     61U,  60U,  77U,  76U,  93U,  92U,
    109U, 108U, 125U, 124U, 141U, 140U,
     15U,  14U,  31U,  30U,  47U,  46U,
     63U,  62U,  79U,  78U,  95U,  94U,
    111U, 110U, 127U, 126U, 143U, 142U,
};
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
static inline uint16_t LD_DATA(uint16_t percent)
{
    return (uint16_t)((((uint32_t)percent * LD_DATA_14B_MAX) + 50U) / 100U);
}

void LED_Select_Color(uint8_t color)
{
    gt_led_buffer_update_type = LED_UPDATE_COLOR;
    if ((led_color_type_t)color < LED_COLOR_MAX)
    {
        gt_led_color = (led_color_type_t)color;
    }
    else
    {
        gt_led_color = LED_COLOR_ALL;
        Print(LOG_INFO, "\r\n Invalid Input. Use: led_color (0 ~ %u)]\r\n", LED_COLOR_MAX - 1U);
    }
}

void LED_Select_Brightness(uint8_t brightness)
{
    if (brightness <= 100U)
    {
        gn_led_brightness_percent = brightness;
    }
    else
    {
        gn_led_brightness_percent = 0U;
    }
}

void LED_Select_Brightness_Up(void)
{
    if (gn_led_brightness_percent < 10U)
    {
        gn_led_brightness_percent += 1U;
    }
    else if (gn_led_brightness_percent < 90U)
    {
        gn_led_brightness_percent += 10U;
    }
    else
    {
        gn_led_brightness_percent = 100U;
    }
}

void LED_Select_Brightness_Down(void)
{
    if (gn_led_brightness_percent > 1U)
    {
        if (gn_led_brightness_percent <= 10U)
        {
            gn_led_brightness_percent -= 1U;
        }
        else
        {
            gn_led_brightness_percent -= 10U;
        }
    }
    else
    {
        gn_led_brightness_percent = 0U;
    }
}

void LED_Select_Pattern(uint8_t pattern)
{
    gt_led_buffer_update_type = LED_UPDATE_PATTERN;
    if ((led_pattern_t)pattern < LED_PATTERN_MAX)
    {
        gt_led_pattern = (led_pattern_t)pattern;
    }
    else
    {
        gt_led_pattern = LED_PATTERN_P0;
        Print(LOG_INFO, "\r\n Invalid Input. Use: led_pattern (0 ~ %u)]\r\n", LED_PATTERN_MAX - 1U);
    }
}

void LED_Select_Pixel(uint8_t pixel)
{
    gt_led_buffer_update_type = LED_UPDATE_PIXEL;
    if (gn_led_pixel < LED_MAP_SIZE)
    {
        gn_led_pixel = pixel;
    }
    else
    {
        gn_led_pixel = 0;
        Print(LOG_INFO, "\r\n Invalid Input. Use: led_pixel (0 ~ %u)]\r\n", LED_MAP_SIZE - 1U);
    }
}

static void LED_Update_Buffer_By_Color(void)
{
    switch (gt_led_color)
    {
        case LED_COLOR_ALL :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            break;
        case LED_COLOR_RED :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = 0U;
                gt_led_ld_buffer[map_idx].ld_b = 0U;
            }
            break;
        case LED_COLOR_GREEN :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0U;
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = 0U;
            }
            break;
        case LED_COLOR_BLUE :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0U;
                gt_led_ld_buffer[map_idx].ld_g = 0U;
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            break;
        default :
            Print(LOG_ERROR, "\r\n Invalid LED Buffer Update By Color\r\n");
            Error_Handler();
            break;
    }
}

static void LED_Update_Buffer_By_Pattern(void)
{
    switch (gt_led_pattern)
    {
        case LED_PATTERN_P0 :
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if (((row & 1U) == 0U))
                {
                    if ((col & 1U) == 0U)
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0U;
                        gt_led_ld_buffer[map_idx].ld_g = 0U;
                        gt_led_ld_buffer[map_idx].ld_b = 0U;
                    }
                }
                else
                {
                    if ((col & 1U) != 0U)
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0U;
                        gt_led_ld_buffer[map_idx].ld_g = 0U;
                        gt_led_ld_buffer[map_idx].ld_b = 0U;
                    }
                }
            }
            break;
        case LED_PATTERN_P1 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if (((row & 1U) == 0U))
                {
                    if ((col & 1U) != 0U)
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0U;
                        gt_led_ld_buffer[map_idx].ld_g = 0U;
                        gt_led_ld_buffer[map_idx].ld_b = 0U;
                    }
                }
                else
                {
                    if ((col & 1U) == 0U)
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0U;
                        gt_led_ld_buffer[map_idx].ld_g = 0U;
                        gt_led_ld_buffer[map_idx].ld_b = 0U;
                    }
                }
            }
            break;
        case LED_PATTERN_P2 :
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                //uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if ((col & 1U) == 0U)
                {
                    gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                }
                else
                {
                    gt_led_ld_buffer[map_idx].ld_r = 0U;
                    gt_led_ld_buffer[map_idx].ld_g = 0U;
                    gt_led_ld_buffer[map_idx].ld_b = 0U;
                }
            }
            break;
        case LED_PATTERN_P3 :
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                //uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if ((col & 1U) == 1U)
                {
                    gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                }
                else
                {
                    gt_led_ld_buffer[map_idx].ld_r = 0U;
                    gt_led_ld_buffer[map_idx].ld_g = 0U;
                    gt_led_ld_buffer[map_idx].ld_b = 0U;
                }
            }
            break;
        case LED_PATTERN_P4 :
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                //uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if ((row & 1U) == 0U)
                {
                    gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                }
                else
                {
                    gt_led_ld_buffer[map_idx].ld_r = 0U;
                    gt_led_ld_buffer[map_idx].ld_g = 0U;
                    gt_led_ld_buffer[map_idx].ld_b = 0U;
                }
            }
            break;
        case LED_PATTERN_P5 :
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t row = map_idx / LED_MAP_COL; // 0 ~ 8
                //uint8_t col = map_idx % LED_MAP_COL; // 0 ~ 15
                if ((row & 1U) == 1U)
                {
                    gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                }
                else
                {
                    gt_led_ld_buffer[map_idx].ld_r = 0U;
                    gt_led_ld_buffer[map_idx].ld_g = 0U;
                    gt_led_ld_buffer[map_idx].ld_b = 0U;
                }
            }
            break;
        default :
            Print(LOG_ERROR, "\r\n Invalid LED Buffer Update By Pattern\r\n");
            Error_Handler();
            break;
    }
}

static void LED_Update_Buffer_By_Pixel(void)
{
    for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        gt_led_ld_buffer[map_idx].ld_r = 0U;
        gt_led_ld_buffer[map_idx].ld_g = 0U;
        gt_led_ld_buffer[map_idx].ld_b = 0U;
    }
    gt_led_ld_buffer[gn_led_pixel].ld_r = LD_DATA(gn_led_brightness_percent);
    gt_led_ld_buffer[gn_led_pixel].ld_g = LD_DATA(gn_led_brightness_percent);
    gt_led_ld_buffer[gn_led_pixel].ld_b = LD_DATA(gn_led_brightness_percent);
}

void LED_Update_Buffer(void)
{
    static boot_state_t boot_state = BOOT_STATE_PATTERN;
    static uint8_t boot_pattern = 0U;
    static uint8_t boot_color = 1U;
    static uint16_t boot_vsync_count = 0U;

    switch(boot_state)
    {
        case BOOT_STATE_PATTERN :
            if (++boot_vsync_count > BOOT_PATTERN_CHANGE_COUNT)
            {
                boot_vsync_count = 0U;
                if (++boot_pattern >= LED_PATTERN_MAX)
                {
                    boot_pattern = 0U;
                    boot_state = BOOT_STATE_COLOR;
                }
            }
            LED_Select_Pattern(boot_pattern);
            LED_Update_Buffer_By_Pattern();
            break;
        case BOOT_STATE_COLOR :
            if (++boot_vsync_count > BOOT_PATTERN_CHANGE_COUNT)
            {
                boot_vsync_count = 0U;
                if (++boot_color >= LED_COLOR_MAX)
                {
                    boot_color = 0U;
                    boot_state = BOOT_STATE_NORMAL;
                }
            }
            LED_Select_Color(boot_color);
            LED_Update_Buffer_By_Color();
            break;
        case BOOT_STATE_NORMAL :
            switch (gt_led_buffer_update_type)
            {
                case LED_UPDATE_COLOR :
                    LED_Update_Buffer_By_Color();
                    break;
                case LED_UPDATE_PATTERN :
                    LED_Update_Buffer_By_Pattern();
                    break;
                case LED_UPDATE_PIXEL :
                    LED_Update_Buffer_By_Pixel();
                    break;
                default:
                    Print(LOG_ERROR, "\r\n Invalid LED Buffer Update Type\r\n");
                    Error_Handler();
                    break;
            }
            break;
        default :
            Print(LOG_ERROR, "\r\n Invalid Boot State Type\r\n");
            break;
    }
}

void us_delay(uint32_t n_delay)
{
    LL_TIM_EnableCounter(TIM12);
    while(TIM12->CNT < n_delay)
    {
    }
    LL_TIM_DisableCounter(TIM12);
    TIM12->CNT = 0U;
}

static void MCU_IF_Serializer_Init_Param(void)
{
    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);

    gn_mcu_serializer_bit_0 = (uint16_t)(((pwm_period + 1U) * BIT_0_RATIO / BIT_RATIO_SUM) - 1U + 0.5f);
    gn_mcu_serializer_bit_1 = (uint16_t)(((pwm_period + 1U) * BIT_1_RATIO / BIT_RATIO_SUM) - 1U + 0.5f);

#if 0
    SyncGen_Pattern[0U] = gn_mcu_serializer_bit_1;
    SyncGen_Pattern[1U] = gn_mcu_serializer_bit_0;
    SyncGen_Pattern[2U] = gn_mcu_serializer_bit_0;
    SyncGen_Pattern[3U] = gn_mcu_serializer_bit_1;
#endif
}

void JigBD_IF_Link_DMA_With_Buffer(void)
{
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&(TIM1->CCR1)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)gn_serialize_tx_buffer);

    MCU_IF_Serializer_Init_Param();
}

static inline uint16_t Get_Nth_Bit(uint32_t x, int n)
{
    return (x >> (n - 1U)) & 1U;
}

static float Decode_Input_Response(uint32_t* pdata, uint16_t len)
{
    uint32_t freq_count_sum = 0U;
    uint32_t count = 0U;

    float f_tim1_count_avg = 0.0f;
    float f_input_frequency = 0.0f;
    uint16_t logic_1_duty_cnt_min = 0U;
    uint16_t logic_1_duty_cnt_max = 0U;

    const uint16_t* p_rising = gn_serialize_rx_risingBuffer;
    const uint16_t* p_falling = gn_serialize_rx_fallingBuffer;

    for (uint16_t i = 1U ; i < len ; ++i)
    {
        freq_count_sum += p_rising[i];
        ++count;
    }

    if (count != 0U)
    {
        f_tim1_count_avg = ((float)freq_count_sum / count);

        logic_1_duty_cnt_min = (uint16_t)LOGIC_1_COUNT_MIN(f_tim1_count_avg);
        logic_1_duty_cnt_max = (uint16_t)LOGIC_1_COUNT_MAX(f_tim1_count_avg);
    }

    if (pdata != 0U)
    {
        uint32_t n_xdic_response = 0U;

        for (volatile uint16_t j = 0U ; j < len ; ++j)
        {
            if ((p_falling[j] > logic_1_duty_cnt_min) && (p_falling[j] < logic_1_duty_cnt_max))
            {
                n_xdic_response |= (1U << ((len - 1U) - j));
            }
        }

        *pdata = n_xdic_response;
    }

    if (f_tim1_count_avg > 0.0f)
    {
        f_input_frequency = (APB1_TIM_FREQ / f_tim1_count_avg);
    }

    return f_input_frequency;
}

static inline void Serialize_Tx_Start(uint32_t len)
{
    gb_pwm_dma_tx_flag = true;

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, len);
    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_1);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
    LL_TIM_EnableCounter(TIM1);
}

static inline bool Serialize_Rx_Start(uint32_t len)
{
    bool b_xd_timeout_event = false;
    gn_xd_rx_timeout = XD_TIMEOUT_MS;

    /* output enable set HIGH */
    PWM_SWITCH_HI();

    LL_TIM_SetCounter(TIM2, 0);

    LL_TIM_EnableCounter(TIM2);

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, len);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, len);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    while(LL_DMA_IsActiveFlag_TC6(DMA1) == 0U)
    {
        if (gn_xd_rx_timeout == 0U)
        {
            b_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, 0);
    LL_DMA_ClearFlag_TC6(DMA1);

    while(LL_DMA_IsActiveFlag_TC5(DMA1) == 0U)
    {
        if (gn_xd_rx_timeout == 0U)
        {
            b_xd_timeout_event = true;
            break;
        }
    }
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, 0);
    LL_DMA_ClearFlag_TC5(DMA1);

    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_DisableCounter(TIM2);

    /* output enable set LOW */
    PWM_SWITCH_LO();

    return b_xd_timeout_event;
}

void MCU_IF_Write_XDIC(uint8_t in_addr, uint16_t in_data)
{
    uint16_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? gn_mcu_serializer_bit_1 : gn_mcu_serializer_bit_0);
        }

        for (uint8_t k = 0 ; k < SERIAL_DATA_SIZE ; ++k)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_data, SERIAL_DATA_SIZE - k)) ? gn_mcu_serializer_bit_1 : gn_mcu_serializer_bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}
    us_delay(XDIC_WRITE_DELAY);
}

uint16_t MCU_IF_Read_XDIC(uint8_t in_addr)
{
    uint32_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? gn_mcu_serializer_bit_1 : gn_mcu_serializer_bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    DEBUG_HI();
    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}

    uint32_t n_response = 0U;

    DEBUG_LO();
    if (true == Serialize_Rx_Start(XDIC_READ_RECV_BITS))
    {
        Print(LOG_ERROR, "Rx Timeout!!! [addr - 0x%02X]\r\n", in_addr);
    }
    else
    {
        DEBUG_HI();
        float f_frequency = Decode_Input_Response(&n_response, XDIC_READ_RECV_BITS);
        Print(LOG_DEBUG, "Received DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, ID : 0x%02X, DATA - 0x%03X, FULL - 0x%06X]\r\n", in_addr, f_frequency,
            ((n_response >> 17U) & SERIAL_DECODE_MASK_CODE), ((n_response >> 12U) & SERIAL_DECODE_MASK_ID), ((n_response >> 0U) & SERIAL_DECODE_MASK_DATA), n_response);
    }

    us_delay(XDIC_READ_DELAY);
    DEBUG_LO();

    return (uint16_t)(n_response & SERIAL_DECODE_MASK_DATA);
}

void MCU_IF_Write_LD(void)
{
    uint16_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0U ; i < LED_MAP_SIZE ; ++i)
    {
        uint8_t led_map_index = gt_led_id_map[i];
        uint16_t LD_data_R = gt_led_ld_buffer[led_map_index].ld_r;
        uint16_t LD_data_G = gt_led_ld_buffer[led_map_index].ld_g;
        uint16_t LD_data_B = gt_led_ld_buffer[led_map_index].ld_b;

        if (((LD_data_R & 0x1FFU) == 0U) && (LD_data_R != 0U))
        {
            --LD_data_R;
        }
        if (((LD_data_G & 0x1FFU) == 0U) && (LD_data_G != 0U))
        {
            --LD_data_G;
        }
        if (((LD_data_B & 0x1FFU) == 0U) && (LD_data_B != 0U))
        {
            --LD_data_B;
        }

        uint32_t LD_data_GB = ((uint32_t)(LD_data_G << 14U)) | ((uint32_t)(LD_data_B << 0U));
        if ((i % 6U) == 0U)
        {
            gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
            gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
            gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
            gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        }
        for (uint8_t k = 0U ; k < SERIAL_ODD_SIZE ; ++k) // For Red
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint32_t)LD_data_R, SERIAL_ODD_SIZE - k)) ? gn_mcu_serializer_bit_1 : gn_mcu_serializer_bit_0);
        }
        for (uint8_t k = 0U ; k < SERIAL_EVEN_SIZE ; ++k) // For Green & Blue
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint32_t)LD_data_GB, SERIAL_EVEN_SIZE - k)) ? gn_mcu_serializer_bit_1 : gn_mcu_serializer_bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    Serialize_Tx_Start(pwm_length);
}

void MCU_IF_IdGen_Command()
{
    uint16_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}
    us_delay(XDIC_IDGEN_DELAY);
}

#if 0
uint16_t MCU_IF_Fault_Read_Command(void)
{
    uint32_t pwm_length = 0U;
    static uint32_t read_msg_error_count = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0U ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_1;
        gn_serialize_tx_buffer[pwm_length++] = gn_mcu_serializer_bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}

    uint32_t n_response = 0U;

    if (true == Serialize_Rx_Start(XDIC_FAULT_RECV_BITS))
    {
        if((read_msg_error_count % 20U) == 0U)
        {
            Print(LOG_ERROR, "Rx Timeout!!!\r\n");
        }

        ++read_msg_error_count;
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_FAULT_RECV_BITS);
        Print(LOG_DEBUG, "FAULT DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, DATA - 0x%01X]\r\n",\
        n_response, f_frequency, ((n_response >> 4U) & 0x0FU), ((n_response >> 0U) & 0x0FU));

        read_msg_error_count = 0U;
    }

    us_delay(XDIC_FAULT_RECV_DELAY);

    return (uint16_t)(n_response & 0x0FFFU);
}

void MCU_IF_SyncGen_Command()
{
    uint16_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0U ; i < XDIC_DAISY_SIZE ; ++i)
    {
        memcpy(&gn_serialize_tx_buffer[pwm_length], SyncGen_Pattern, sizeof(SyncGen_Pattern));
        pwm_length += 4U;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}
    us_delay(XDIC_SYNCGEN_DELAY);
}
#endif

/* USER CODE END */

/*** end of file ***/