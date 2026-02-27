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
#define _LOGIC_0_                   (uint16_t)(((TIM1_PERIOD + 1) * 1.0f / 3.0f) - 1 + 0.5f)
#define _LOGIC_1_                   (uint16_t)(((TIM1_PERIOD + 1) * 2.0f / 3.0f) - 1 + 0.5f)

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

#define SERIAL_DECODE_MASK_CODE     (0x0F)
#define SERIAL_DECODE_MASK_ID       (0x1F)
#define SERIAL_DECODE_MASK_DATA     (0xFFF)

#define LD_DATA(x)                  ((uint16_t)(((float)(x) / 100.0f) * LD_DATA_14B_MAX + 0.5f))
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
    LED_COLOR_ALL = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_MAX,
} led_color_type_t;

typedef enum
{
    LED_UPDATE_COLOR = 0,   // By Color
    LED_UPDATE_PATTERN,     // By Pattern
    LED_UPDATE_MAX,
} led_buffer_update_type_t;

typedef enum
{
    LED_PATTERN_P0 = 0, // Checkerboard pattern 1
    LED_PATTERN_P1,     // Checkerboard pattern 2
    LED_PATTERN_P2,     //
    LED_PATTERN_P3,
    LED_PATTERN_P4,
    LED_PATTERN_MAX,
} led_pattern_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_serialize_tx_buffer[XDIC_LD_TRANS_BITS + 10] = {0, };
static uint16_t gn_serialize_rx_risingBuffer[400] = {0, };
static uint16_t gn_serialize_rx_fallingBuffer[400] = {0, };

volatile uint16_t gn_xd_rx_timeout;
volatile bool gb_xd_timeout_event;
volatile bool gb_pwm_dma_tx_flag;
volatile bool gb_pwm_is_rx_flag;

static led_color_type_t gt_led_color;

static uint8_t gn_led_brightness_percent; // 0 ~ 100

static led_buffer_update_type_t gt_led_buffer_update_type;

static led_pattern_t gt_led_pattern;

static led_ld_buffer_t gt_led_ld_buffer[LED_MAP_SIZE];

const uint8_t gt_led_id_map[LED_MAP_SIZE] =
{
      1,   0,   9,   8,  17,  16,
     25,  24,  33,  32,  41,  40,
     49,  48,  57,  56,  65,  64,
      3,   2,  11,  10,  19,  18,
     27,  26,  35,  34,  43,  42,
     51,  50,  59,  58,  67,  66,
      5,   4,  13,  12,  21,  20,
     29,  28,  37,  36,  45,  44,
     53,  52,  61,  60,  69,  68,
      7,   6,  15,  14,  23,  22,
     31,  30,  39,  38,  47,  46,
     55,  54,  63,  62,  71,  70,
     73,  72,  81,  80,  89,  88,
     97,  96, 105, 104, 113, 112,
    121, 120, 129, 128, 137, 136,
     75,  74,  83,  82,  91,  90,
     99,  98, 107, 106, 115, 114,
    123, 122, 131, 130, 139, 138,
     77,  76,  85,  84,  93,  92,
    101, 100, 109, 108, 117, 116,
    125, 124, 133, 132, 141, 140,
     79,  78,  87,  86,  95,  94,
    103, 102, 111, 110, 119, 118,
    127, 126, 135, 134, 143, 142,
};
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
void LED_Select_Color(uint8_t color)
{
    gt_led_buffer_update_type = LED_UPDATE_COLOR;
    if (color < LED_COLOR_MAX)
    {
        gt_led_color = (led_color_type_t)(color % LED_COLOR_MAX);
    }
    else
    {
        gt_led_color = LED_COLOR_ALL;
        Print(LOG_INFO, "\r\n Invalid Input. Use: led_color (0 ~ %u)]\r\n", LED_COLOR_MAX - 1);
    }
}

void LED_Select_Brightness(uint8_t brightness)
{
    if (brightness <= 100)
    {
        gn_led_brightness_percent = brightness;
    }
    else
    {
        gn_led_brightness_percent = 0;
    }
}

void LED_Select_Brightness_Up(void)
{
    if (gn_led_brightness_percent > 90)
    {
        gn_led_brightness_percent = 100;
    }
    else
    {
        gn_led_brightness_percent += 10;
    }
}

void LED_Select_Brightness_Down(void)
{
    if (gn_led_brightness_percent > 10)
    {
        gn_led_brightness_percent -= 10;
    }
    else
    {
        gn_led_brightness_percent = 0;
    }
}

void LED_Select_Pattern(uint8_t pattern)
{
    gt_led_buffer_update_type = LED_UPDATE_PATTERN;
    if (pattern < LED_PATTERN_MAX)
    {
        gt_led_pattern = (led_pattern_t)(pattern % LED_PATTERN_MAX);
    }
    else
    {
        gt_led_pattern = LED_PATTERN_P0;
        Print(LOG_INFO, "\r\n Invalid Input. Use: led_pattern (0 ~ %u)]\r\n", LED_PATTERN_MAX - 1);
    }
}

void LED_Update_Buffer_By_Color(void)
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
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);;
                gt_led_ld_buffer[map_idx].ld_g = 0;
                gt_led_ld_buffer[map_idx].ld_b = 0;
            }
            break;
        case LED_COLOR_GREEN :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0;
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);;
                gt_led_ld_buffer[map_idx].ld_b = 0;
            }
            break;
        case LED_COLOR_BLUE :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0;
                gt_led_ld_buffer[map_idx].ld_g = 0;
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);;
            }
            break;
    }
}

void LED_Update_Buffer_By_Pattern(void)
{
    switch (gt_led_pattern)
    {
        case LED_PATTERN_P0 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t vertical = map_idx / LED_MAP_HORIZONTAL;
                uint8_t horizontal = map_idx % LED_MAP_HORIZONTAL;
                if (((vertical % 2) == 0))
                {
                    if (horizontal % 2) // Odd Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else // Even Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0;
                        gt_led_ld_buffer[map_idx].ld_g = 0;
                        gt_led_ld_buffer[map_idx].ld_b = 0;
                    }
                }
                else
                {
                    if (horizontal % 2) // Odd Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0;
                        gt_led_ld_buffer[map_idx].ld_g = 0;
                        gt_led_ld_buffer[map_idx].ld_b = 0;
                    }
                    else // Even Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                }
            }
            break;
        case LED_PATTERN_P1 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                uint8_t vertical = map_idx / LED_MAP_HORIZONTAL;
                uint8_t horizontal = map_idx % LED_MAP_HORIZONTAL;
                if (((vertical % 2) == 0))
                {
                    if (horizontal % 2) // Odd Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0;
                        gt_led_ld_buffer[map_idx].ld_g = 0;
                        gt_led_ld_buffer[map_idx].ld_b = 0;
                    }
                    else // Even Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                }
                else
                {
                    if (horizontal % 2) // Odd Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                    }
                    else // Even Horizontal
                    {
                        gt_led_ld_buffer[map_idx].ld_r = 0;
                        gt_led_ld_buffer[map_idx].ld_g = 0;
                        gt_led_ld_buffer[map_idx].ld_b = 0;
                    }
                }
            }
            break;
        case LED_PATTERN_P2 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            break;
        case LED_PATTERN_P3 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            break;
        case LED_PATTERN_P4 :
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            break;
    }
}

void LED_Update_Buffer(void)
{
    switch (gt_led_buffer_update_type)
    {
        case LED_UPDATE_COLOR :
            LED_Update_Buffer_By_Color();
            break;
        case LED_UPDATE_PATTERN :
            LED_Update_Buffer_By_Pattern();
            break;
        default:
            break;
    }
}

void us_delay(uint16_t us_delay)
{
    uint16_t start = LL_TIM_GetCounter(TIM12);
    while ((uint16_t)(LL_TIM_GetCounter(TIM12) - start) < us_delay)
    {
    }
}

void JigBD_IF_Link_DMA_With_Buffer(void)
{
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)(&(TIM1->CCR1)));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, (uint32_t)gn_serialize_tx_buffer);
}

static inline uint16_t Get_Nth_Bit(uint32_t x, int n)
{
    return (x >> (n - 1)) & 1U;
}

static float Decode_Input_Response(uint32_t* pdata, uint16_t len)
{
    uint32_t freq_count_sum = 0;
    uint32_t count = 0;

    float f_tim1_count_avg = 0;
    float f_input_frequency = 0;
    uint16_t logic_1_duty_cnt_min = 0;
    uint16_t logic_1_duty_cnt_max = 0;

    const uint16_t* p_rising = gn_serialize_rx_risingBuffer;
    const uint16_t* p_falling = gn_serialize_rx_fallingBuffer;

    for (uint16_t i = 1 ; i < len ; ++i)
    {
        freq_count_sum += p_rising[i];
        ++count;
    }

    if (count)
    {
        f_tim1_count_avg = ((float)freq_count_sum / count);

        logic_1_duty_cnt_min = (uint16_t)LOGIC_1_COUNT_MIN(f_tim1_count_avg);
        logic_1_duty_cnt_max = (uint16_t)LOGIC_1_COUNT_MAX(f_tim1_count_avg);
    }

    if (pdata)
    {
        uint32_t n_xdic_response = 0;

        for (volatile uint16_t j = 0 ; j < len ; ++j)
        {
            if ((p_falling[j] > logic_1_duty_cnt_min) && (p_falling[j] < logic_1_duty_cnt_max))
            {
                n_xdic_response |= (1U << ((len - 1) - j));
            }
        }

        *pdata = n_xdic_response;
    }

    if (f_tim1_count_avg > 0)
    {
        f_input_frequency = (APB1_TIM_FREQ / f_tim1_count_avg);
    }

    return f_input_frequency;
}

void MCU_IF_Write_XDIC(uint8_t in_addr, uint16_t in_data)
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? bit_1 : bit_0);
        }

        for (uint8_t k = 0 ; k < SERIAL_DATA_SIZE ; ++k)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_data, SERIAL_DATA_SIZE - k)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    us_delay(XDIC_RESET_DELAY);
}

uint16_t MCU_IF_Read_XDIC(uint8_t in_addr)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;

        for (uint8_t j = 0 ; j < SERIAL_ADDR_SIZE ; ++j)
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint16_t)in_addr, SERIAL_ADDR_SIZE - j)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;
    gb_pwm_is_rx_flag = true;

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}

    Serialize_Rx_Start(XDIC_READ_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        Print(LOG_ERROR, "Rx Timeout!!! [addr - 0x%02X]\r\n", in_addr);
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_READ_RECV_BITS);
        Print(LOG_DEBUG, "Received DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, ID : 0x%02X, DATA - 0x%03X, FULL - 0x%04X]\r\n", in_addr, f_frequency,
            ((n_response >> 17) & SERIAL_DECODE_MASK_CODE), ((n_response >> 12) & SERIAL_DECODE_MASK_ID), ((n_response >> 0) & SERIAL_DECODE_MASK_DATA), n_response);
    }

    us_delay(XDIC_READ_DELAY);

    return (uint16_t)(n_response & SERIAL_DECODE_MASK_DATA);
}

void MCU_IF_Write_LD(void)
{
    uint16_t pwm_length = 0;
    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    gn_serialize_tx_buffer[pwm_length++] = 0;

    for (uint8_t i = 0 ; i < LED_MAP_SIZE ; ++i)
    {
        uint8_t led_map_index = gt_led_id_map[i];
        uint16_t LD_data_R = gt_led_ld_buffer[led_map_index].ld_r;
        uint16_t LD_data_G = gt_led_ld_buffer[led_map_index].ld_g;
        uint16_t LD_data_B = gt_led_ld_buffer[led_map_index].ld_b;

        if ((LD_data_R & 0x1FF) == 0 && LD_data_R != 0) --LD_data_R;
        if ((LD_data_G & 0x1FF) == 0 && LD_data_G != 0) --LD_data_G;
        if ((LD_data_B & 0x1FF) == 0 && LD_data_B != 0) --LD_data_B;

        uint32_t LD_data_GB = ((uint32_t)LD_data_G << 14U) | ((uint32_t)LD_data_B << 0U);
        if (i % 6 == 0)
        {
            gn_serialize_tx_buffer[pwm_length++] = bit_1;
            gn_serialize_tx_buffer[pwm_length++] = bit_1;
            gn_serialize_tx_buffer[pwm_length++] = bit_1;
            gn_serialize_tx_buffer[pwm_length++] = bit_1;
        }
        for (uint8_t k = 0 ; k < SERIAL_ODD_SIZE ; ++k) // For Red
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint32_t)LD_data_R, SERIAL_ODD_SIZE - k)) ? bit_1 : bit_0);
        }
        for (uint8_t k = 0 ; k < SERIAL_EVEN_SIZE ; ++k) // For Green & Blue
        {
            gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint32_t)LD_data_GB, SERIAL_EVEN_SIZE - k)) ? bit_1 : bit_0);
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);
    #if 0

    while (gb_pwm_dma_tx_flag) {}

    #endif
}

uint16_t MCU_IF_Fault_Read_Command(void)
{
    uint32_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Rx_Start(XDIC_FAULT_RECV_BITS);

    uint32_t n_response = 0;

    if (gb_xd_timeout_event)
    {
        Print(LOG_ERROR, "Rx Timeout!!!\r\n");
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_FAULT_RECV_BITS);
        Print(LOG_DEBUG, "FAULT DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, DATA - 0x%01X]\r\n",\
        n_response, f_frequency, ((n_response >> 4) & 0x0F), ((n_response >> 0) & 0x0F));
    }

    us_delay(XDIC_FAULT_READ_DELAY);

    return (uint16_t)(n_response & 0x0FFF);
}

void MCU_IF_IdGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    us_delay(XDIC_IDGEN_DELAY);
}

void MCU_IF_SyncGen_Command()
{
    uint16_t pwm_length = 0;

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_0;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    us_delay(XDIC_SYNCGEN_DELAY);
}
/* USER CODE END */

/*** end of file ***/