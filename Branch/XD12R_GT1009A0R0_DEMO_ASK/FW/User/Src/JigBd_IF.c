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
//#define TIME_SAVE

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

#define GRAY_BLEND_RATIO            (40UL) /* 0~100, 0: max only, 100: gray only */

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_COLOR_COUNT                  (10U)
#else
    #define PATTERN_CHANGE_COLOR_COUNT                  (30U)
#endif

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_RAINBOW_LEFT_TO_RIGHT_COUNT  (10U)
#else
    #define PATTERN_CHANGE_RAINBOW_LEFT_TO_RIGHT_COUNT  (10U)
#endif
#define TABLE_SIZE_RAINBOW_LEFT_TO_RIGHT                (LED_MAP_COL)

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_RAINBOW_UP_TO_DOWN_COUNT  (10U)
#else
    #define PATTERN_CHANGE_RAINBOW_UP_TO_DOWN_COUNT  (10U)
#endif
#define TABLE_SIZE_RAINBOW_UP_TO_DOWN               (LED_MAP_ROW)

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_PIXEL_RAINBOW_FLOW1_COUNT  (60U)
#else
    #define PATTERN_CHANGE_PIXEL_RAINBOW_FLOW1_COUNT  (360U)
#endif

#define RAINBOW_PHASE_STEP_TICKS (1U) /* 120Hz에서 3이면 초당 40회 phase 갱신 */
#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_PIXEL_RAINBOW_FLOW2_COUNT  (60U)
#else
    #define PATTERN_CHANGE_PIXEL_RAINBOW_FLOW2_COUNT  (510U)
#endif

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_BLUE_CYAN_WAVE_COUNT  (60U)
#else
    #define PATTERN_CHANGE_BLUE_CYAN_WAVE_COUNT  (360U)
#endif

#if defined(TIME_SAVE)
    #define PATTERN_CHANGE_COLUMN_COLOR_SHIFT_COUNT  (5U)
#else
    #define PATTERN_CHANGE_COLUMN_COLOR_SHIFT_COUNT  (3U)
#endif

#define DEMO_RAINDROP_COUNT                 (16U)
#define PATTERN_CHANGE_RAIN_AND_TAIL_COUNT  (360U)

#define DEMO_METEOR_COUNT                   (8U)
#define DEMO_METEOR_TAIL_LENGTH             (8U)
#define DEMO_METEOR_MOVE_DIVIDER            (2U)
#define DEMO_METEOR_SPAWN_GAP               (8U)
#define PATTERN_CHANGE_METEOR_COUNT         (360U)

#define PATTERN_CHANGE_FIREWORKS_COUNT      (360U)

#define DEMO_BALL_RADIUS                    (1U)
#define DEMO_BALL_MOVE_DIVIDER              (4U) /* 120Hz 기준 초당 30회 이동 */
#define PATTERN_CHANGE_BALL_BOUNCE_COUNT    (360U)

#define PATTERN_CHANGE_SPARKLE_COUNT    (240U)

#define PATTERN_CHANGE_PIXEL_ART_COUNT  (240U)

#define PATTERN_CHANGE_LEFT_TURN_COUNT  (240U)

#define PATTERN_CHANGE_RIGHT_TURN_COUNT  (240U)

#define BANNER_SCROLL_TICKS_PER_COLUMN  (9U)
#define PATTERN_CHANGE_GT_BANNER_COUNT  (91U * BANNER_SCROLL_TICKS_PER_COLUMN + 60U) // banner + 0.5s

//const static float LED_BRT_COMP_RED   = 0.32f;
//const static float LED_BRT_COMP_GREEN = 0.80f;
//const static float LED_BRT_COMP_BLUE  = 1.00f;

const static float LED_BRT_COMP_RED   = 1.00f;
const static float LED_BRT_COMP_GREEN = 1.00f;
const static float LED_BRT_COMP_BLUE  = 1.00f;

/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_OPERATE_STATE
{
    OPERATE_STATE_RGB = 0U,
    OPERATE_STATE_ONLY_B,
    OPERATE_STATE_MAX
} operate_state_t;
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

static led_pattern_t gt_led_pattern;
static operate_state_t gt_operate_state;

static led_ld_buffer_t gt_led_ld_buffer[LED_MAP_SIZE] = {0};

static uint16_t gn_led_brightness_percent = 100U;

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

const static uint16_t gn_rainbow_left_to_right_color_table[TABLE_SIZE_RAINBOW_LEFT_TO_RIGHT][3] =
{
    {255, 0, 0},
    {255, 64, 0},
    {255, 128, 0},
    {255, 192, 0},
    {255, 255, 0},
    {128, 255, 0},
    {0, 255, 0},
    {0, 255, 128},
    {0, 255, 255},
    {0, 128, 255},
    {0, 0, 255},
    {32, 0, 192},
    {64, 0, 160},
    {128, 0, 128},
    {192, 0, 192},
    {255, 0, 128}
};

const static uint16_t gn_rainbow_up_to_down_color_table[TABLE_SIZE_RAINBOW_UP_TO_DOWN][3] =
{
    {255, 0, 0},
    {255, 128, 0},
    {255, 255, 0},
    {128, 255, 0},
    {0, 255, 0},
    {0, 255, 255},
    {0, 0, 255},
    {64, 0, 192},
    {128, 0, 128}
};

#if 0 // original table wjj
static const uint32_t g_shift_color[8] =
{
    0x000000U,  /* Black */
    0xFF0000U,  /* Red */
    0x00FF00U,  /* Green */
    0x0000FFU,  /* Blue */
    0xFFFFFFU,  /* White */
    0x00FFFFU,  /* Cyan */
    0xFF00FFU,  /* Magenta */
    0xFFFF00U,  /* Yellow */
};
#else
static const uint32_t g_shift_color[8] =
{
    0x03045E,//0x00D2FF,
    0x023E8A,//0x0080FF,
    0x0077B6,//0x7A00FF,
    0x0096C7,//0xFF00D2,
    0x00B4D8,//0xFF3B30,
    0x48CAE4,//0xFF9500,
    0x90E0EF,//0xFFCC00,
    0xADE8F4,//0x00E676,
};
#endif
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
static inline uint16_t LD_DATA(uint16_t percent)
{
    return (uint16_t)((((uint32_t)percent * LD_DATA_14B_MAX) + 50U) / 100U);
}

void LED_Set_Operation_State(uint8_t operation_state)
{
    if (operation_state < OPERATE_STATE_MAX)
    {
        gt_operate_state = operation_state;
    }
}

void LED_Set_Pattern(uint8_t pattern)
{
    if (pattern < LED_PATTERN_MAX)
    {
        gt_led_pattern = pattern;
    }
}

static void LED_Update_Buffer_Rainbow_Left_to_Right(uint32_t* p_vsync_tick)
{
    static uint32_t rainbow_shift = 0U;
    static uint32_t repeat = 0U;
    const uint32_t table_size = TABLE_SIZE_RAINBOW_LEFT_TO_RIGHT;

    for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        uint16_t col = (map_idx % table_size); // 0 ~ 15
        uint16_t table_idx = (col + table_size - (rainbow_shift % table_size)) % table_size;
        gt_led_ld_buffer[map_idx].ld_r = gn_rainbow_left_to_right_color_table[table_idx][0] << 6U;
        gt_led_ld_buffer[map_idx].ld_g = gn_rainbow_left_to_right_color_table[table_idx][1] << 6U;
        gt_led_ld_buffer[map_idx].ld_b = gn_rainbow_left_to_right_color_table[table_idx][2] << 6U;
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_RAINBOW_LEFT_TO_RIGHT_COUNT) == 0U)
    {
        ++rainbow_shift;
        if (rainbow_shift >= table_size)
        {
            ++repeat;
            if (repeat >= 3U)
            {
                repeat = 0U;
                rainbow_shift = 0U;
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P8;
            }
        }
    }
}

static void LED_Update_Buffer_Rainbow_Up_to_Down(uint32_t* p_vsync_tick)
{
    static uint32_t rainbow_shift = 0U;
    static uint32_t repeat = 0U;
    const uint32_t table_size = TABLE_SIZE_RAINBOW_UP_TO_DOWN;

    for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        uint16_t row = (map_idx / LED_MAP_COL); // 0 ~ 15
        uint16_t table_idx = (row + table_size - (rainbow_shift % table_size)) % table_size;
        gt_led_ld_buffer[map_idx].ld_r = gn_rainbow_up_to_down_color_table[table_idx][0] << 6U;
        gt_led_ld_buffer[map_idx].ld_g = gn_rainbow_up_to_down_color_table[table_idx][1] << 6U;
        gt_led_ld_buffer[map_idx].ld_b = gn_rainbow_up_to_down_color_table[table_idx][2] << 6U;
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_RAINBOW_UP_TO_DOWN_COUNT) == 0U)
    {
        ++rainbow_shift;
        if (rainbow_shift >= table_size)
        {
            ++repeat;
            if (repeat >= 3U)
            {
                repeat = 0U;
                rainbow_shift = 0U;
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P9;
            }
        }
    }
}

static void LED_Update_Buffer_Pixel_Rainbow_Flow1(uint32_t* p_vsync_tick)
{
    uint8_t x = 0U;
    uint8_t y = 0U;
    uint8_t hue = 0U;

    static uint8_t rainbow_phase = 0U;

    for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        x = map_idx % LED_MAP_COL;
        y = map_idx / LED_MAP_COL;
        hue = (uint8_t)(256U - rainbow_phase + ((uint16_t)y * 16U) + ((uint16_t)x * 37U));

        uint8_t r = 0U;
        uint8_t g = 0U;
        uint8_t b = 0U;

        if (hue < 85U)
        {
            r = (uint8_t)(255U - (hue * 3U));
            g = (uint8_t)(hue * 3U);
            b = 0U;
        }
        else if (hue < 170U)
        {
            hue = (uint8_t)(hue - 85U);
            r = 0U;
            g = (uint8_t)(255U - (hue * 3U));
            b = (uint8_t)(hue * 3U);
        }
        else
        {
            hue = (uint8_t)(hue - 170U);
            r = (uint8_t)(hue * 3U);
            g = 0U;
            b = (uint8_t)(255U - (hue * 3U));
        }

        gt_led_ld_buffer[map_idx].ld_r = r << 6U;
        gt_led_ld_buffer[map_idx].ld_g = g << 6U;
        gt_led_ld_buffer[map_idx].ld_b = b << 6U;
    }

    rainbow_phase += 4U;

    if ((*p_vsync_tick % PATTERN_CHANGE_PIXEL_RAINBOW_FLOW1_COUNT) == 0U)
    {
        rainbow_phase = 0U;
        *p_vsync_tick = 0U;
        gt_led_pattern = LED_PATTERN_P10; // Switch to Pixel Rainbow Flow2 after Flow1 completes
    }
}

static void LED_Update_Buffer_Pixel_Rainbow_Flow2(uint32_t* p_vsync_tick)
{
    static uint8_t rainbow_phase = 0U;

    for (uint16_t map_idx = 0U; map_idx < LED_MAP_SIZE; ++map_idx)
    {
        uint8_t row = (uint8_t)(map_idx / LED_MAP_COL);
        uint8_t col = (uint8_t)(map_idx % LED_MAP_COL);

        /* 열을 따라 무지개를 만들고, 행 오프셋으로 흐름에 대각선 변화를 준다. */
        uint8_t hue = (uint8_t)(rainbow_phase + (col * 16U) + (row * 5U));
        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (hue < 85U)
        {
            r = (uint8_t)(255U - (hue * 3U));
            g = (uint8_t)(hue * 3U);
            b = 0U;
        }
        else if (hue < 170U)
        {
            uint8_t segment = (uint8_t)(hue - 85U);
            r = 0U;
            g = (uint8_t)(255U - (segment * 3U));
            b = (uint8_t)(segment * 3U);
        }
        else
        {
            uint8_t segment = (uint8_t)(hue - 170U);
            r = (uint8_t)(segment * 3U);
            g = 0U;
            b = (uint8_t)(255U - (segment * 3U));
        }

        /* RGB 0..255 값을 LED 밝기와 LD_DATA 스케일에 맞춘다. */
        uint16_t red_percent = (uint16_t)(
            ((uint32_t)r * gn_led_brightness_percent + 127U) / 255U);
        uint16_t green_percent = (uint16_t)(
            ((uint32_t)g * gn_led_brightness_percent + 127U) / 255U);
        uint16_t blue_percent = (uint16_t)(
            ((uint32_t)b * gn_led_brightness_percent + 127U) / 255U);

        gt_led_ld_buffer[map_idx].ld_r = LD_DATA(red_percent);
        gt_led_ld_buffer[map_idx].ld_g = LD_DATA(green_percent);
        gt_led_ld_buffer[map_idx].ld_b = LD_DATA(blue_percent);
    }

    if ((*p_vsync_tick % RAINBOW_PHASE_STEP_TICKS) == 0U)
    {
        ++rainbow_phase;
    }


    if ((*p_vsync_tick % PATTERN_CHANGE_PIXEL_RAINBOW_FLOW2_COUNT) == 0U)
    {
        rainbow_phase = 0U;
        *p_vsync_tick = 0U;
        gt_led_pattern = LED_PATTERN_P11; // Switch to Blue/Cyan Wave after Flow2 completes
    }
}

static void LED_Update_Buffer_Blue_Cyan_Wave(uint32_t* p_vsync_tick)
{
    static uint8_t  blue_cyan_wave_phase = 0;   /* Wave가 이동하는 위치 */

    uint8_t x;
    uint8_t y;
    uint8_t level;
    uint8_t local_brightness;
    uint16_t pos;
    uint32_t rgb;

    for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        x = map_idx % LED_MAP_COL;
        y = map_idx / LED_MAP_COL;

        pos = (uint16_t)(256U - blue_cyan_wave_phase + ((uint16_t)x * 16U) + ((uint16_t)y * 16U));
        level = (uint8_t)pos;

        /* 삼각파 형태의 밝기 Envelope */
        if (level < 128U)
        {
            local_brightness = (uint8_t)(level * 2U);
        }
        else
        {
            local_brightness = (uint8_t)((255U - level) * 2U);
        }

        /* 완전히 꺼지지 않도록 최소 밝기를 약간 남긴다. */
        const uint8_t g_brightness = 128U;
        local_brightness = (uint8_t)(26U + ((uint16_t)local_brightness * 229U / 255U));
        local_brightness = (uint8_t)(((uint16_t)local_brightness * g_brightness + 127U) / 255U);

        /* Wave 중심은 White/Cyan, 바깥은 Blue */
        if      (level < 64U)  rgb = 0x0000FFU;
        else if (level < 112U) rgb = 0x00FFFFU;
        else if (level < 144U) rgb = 0xFFFFFFU;
        else if (level < 192U) rgb = 0x00FFFFU;
        else                   rgb = 0x0000FFU;

        uint32_t r = (rgb >> 16) & 0xFFU;
        uint32_t g = (rgb >> 8)  & 0xFFU;
        uint32_t b = rgb & 0xFFU;

        r = ((r * local_brightness) + 127U) / 255U;
        g = ((g * local_brightness) + 127U) / 255U;
        b = ((b * local_brightness) + 127U) / 255U;

        gt_led_ld_buffer[map_idx].ld_r = r << 6U;
        gt_led_ld_buffer[map_idx].ld_g = g << 6U;
        gt_led_ld_buffer[map_idx].ld_b = b << 6U;
    }

    blue_cyan_wave_phase += 6U;

    if ((*p_vsync_tick % PATTERN_CHANGE_BLUE_CYAN_WAVE_COUNT) == 0U)
    {
        blue_cyan_wave_phase = 0U;
        gt_led_pattern = LED_PATTERN_P12; // Switch to Column Color Shift after Blue/Cyan Wave completes
        *p_vsync_tick = 0U;
    }
}

static void LED_Update_Buffer_Column_Color_Shift(uint32_t* p_vsync_tick)
{
    static uint16_t shift_step = 0;
    static uint8_t repeat = 0U;

    uint8_t x;
    uint8_t y;
    uint8_t strip;
    uint8_t color_index;
    uint32_t rgb;

    for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
    {
        x = map_idx % LED_MAP_COL;
        y = map_idx / LED_MAP_COL;

        /*
         * Matrix Width가 8/16/32 등 달라도 8개 Color Strip에 정규화한다.
         */
        strip = (uint8_t)(((uint16_t)x * 8U) / LED_MAP_COL);

        /*
         * g_shift_step을 빼면 색 배열이 화면 오른쪽 방향으로 한 칸씩 이동한다.
         */
        color_index = (uint8_t)((strip + 8U - shift_step) % 8U);

        rgb = g_shift_color[color_index];
        const uint8_t g_brightness = 128U;

        uint32_t r = 0U;
        uint32_t g = 0U;
        uint32_t b = 0U;

        if (rgb != 0U)
        {
            r = (rgb >> 16) & 0xFFU;
            g = (rgb >> 8)  & 0xFFU;
            b = rgb & 0xFFU;

            r = ((r * g_brightness) + 127U) / 255U;
            g = ((g * g_brightness) + 127U) / 255U;
            b = ((b * g_brightness) + 127U) / 255U;
        }

        gt_led_ld_buffer[map_idx].ld_r = r << 6U;
        gt_led_ld_buffer[map_idx].ld_g = g << 6U;
        gt_led_ld_buffer[map_idx].ld_b = b << 6U;
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_COLUMN_COLOR_SHIFT_COUNT) == 0U)
    {
        ++shift_step;
        *p_vsync_tick = 0U;

        if (shift_step > 8)
        {
            shift_step = 0U;
            repeat++;

            if (repeat > 9)
            {
                shift_step = 0U;
                repeat = 0U;
                gt_led_pattern = LED_PATTERN_P13;
            }
        }
    }
}

static void LED_Rain_Set_Pixel(uint16_t map_idx, uint32_t rgb, uint8_t level)
{
    uint32_t r = (rgb >> 16U) & 0xFFU;
    uint32_t g = (rgb >> 8U) & 0xFFU;
    uint32_t b = rgb & 0xFFU;

    /* level과 전체 밝기를 적용하고 LD 데이터 범위로 변환한다. */
    r = ((r * level + 127U) / 255U * gn_led_brightness_percent + 50U) / 100U;
    g = ((g * level + 127U) / 255U * gn_led_brightness_percent + 50U) / 100U;
    b = ((b * level + 127U) / 255U * gn_led_brightness_percent + 50U) / 100U;

    uint16_t ld_r = LD_DATA((uint16_t)r);
    uint16_t ld_g = LD_DATA((uint16_t)g);
    uint16_t ld_b = LD_DATA((uint16_t)b);

    /* 빗방울이 겹치면 더 밝은 채널 값을 유지한다. */
    if (ld_r > gt_led_ld_buffer[map_idx].ld_r)
        gt_led_ld_buffer[map_idx].ld_r = ld_r;
    if (ld_g > gt_led_ld_buffer[map_idx].ld_g)
        gt_led_ld_buffer[map_idx].ld_g = ld_g;
    if (ld_b > gt_led_ld_buffer[map_idx].ld_b)
        gt_led_ld_buffer[map_idx].ld_b = ld_b;
}

static void LED_Update_Buffer_Rain_and_Tail(uint32_t* p_vsync_tick)
{
    static uint8_t raindrop_x[DEMO_RAINDROP_COUNT];
    static int16_t raindrop_y[DEMO_RAINDROP_COUNT];
    static uint8_t raindrop_length[DEMO_RAINDROP_COUNT];
    static uint8_t raindrop_speed[DEMO_RAINDROP_COUNT];
    static uint8_t raindrop_speed_count[DEMO_RAINDROP_COUNT];
    static uint32_t random_seed = 0x12345678U;
    static bool initialized = false;

    static const uint32_t tail_rgb[] =
    {
        0xF0FFFFU, /* Head: 밝은 시안 */
        0x90E8FFU,
        0x40A8FFU,
        0x1260D0U,
        0x082050U,
        0x041020U
    };

    static const uint8_t tail_level[] = {100U, 72U, 48U, 30U, 16U, 8U};

    if (!initialized)
    {
        for (uint8_t i = 0U; i < DEMO_RAINDROP_COUNT; ++i)
        {
            raindrop_x[i] = (uint8_t)(((uint16_t)i * LED_MAP_COL) / DEMO_RAINDROP_COUNT);
            raindrop_y[i] = -(int16_t)(i % 8U);
            raindrop_length[i] = 4U;
            raindrop_speed[i] = 1U;
            raindrop_speed_count[i] = 0U;
        }
        initialized = true;
    }

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    for (uint8_t i = 0U; i < DEMO_RAINDROP_COUNT; ++i)
    {
        for (uint8_t tail = 0U; tail < raindrop_length[i]; ++tail)
        {
            int16_t y = raindrop_y[i] - (int16_t)tail;

            if ((y < 0) || (y >= (int16_t)LED_MAP_ROW) ||
                (raindrop_x[i] >= LED_MAP_COL))
            {
                continue;
            }

            uint8_t color_idx = (tail < 6U) ? tail : 5U;
            uint16_t map_idx =
                (uint16_t)(y * (int16_t)LED_MAP_COL + raindrop_x[i]);

            LED_Rain_Set_Pixel(map_idx,
                tail_rgb[color_idx], tail_level[color_idx]);
        }

        /* 빗방울이 바닥에 닿은 프레임에 양옆으로 짧은 물보라를 표시한다. */
        if (raindrop_y[i] == (int16_t)(LED_MAP_ROW - 1U))
        {
            int16_t bottom = (int16_t)(LED_MAP_ROW - 1U);

            if (raindrop_x[i] > 0U)
            {
                uint16_t idx =
                    (uint16_t)(bottom * (int16_t)LED_MAP_COL
                    + (int16_t)raindrop_x[i] - 1);
                LED_Rain_Set_Pixel(idx, 0x66CCFFU, 45U);
            }

            if ((uint16_t)raindrop_x[i] + 1U < LED_MAP_COL)
            {
                uint16_t idx =
                    (uint16_t)(bottom * (int16_t)LED_MAP_COL
                    + (int16_t)raindrop_x[i] + 1);
                LED_Rain_Set_Pixel(idx, 0x66CCFFU, 45U);
            }
        }

        if (++raindrop_speed_count[i] >= raindrop_speed[i])
        {
            raindrop_speed_count[i] = 0U;
            ++raindrop_y[i];
        }

        /* 꼬리까지 화면 밖으로 나간 빗방울을 위쪽에서 재생성한다. */
        if (raindrop_y[i] >=
            (int16_t)(LED_MAP_ROW + raindrop_length[i]))
        {
            random_seed = (random_seed * 1664525U) + 1013904223U;

            raindrop_x[i] = (uint8_t)((random_seed >> 16U) % LED_MAP_COL);
            raindrop_y[i] = -(int16_t)(4U + ((random_seed >> 8U) % 8U));
            raindrop_length[i] = (uint8_t)(4U + ((random_seed >> 24U) & 1U));
            raindrop_speed[i] = (((random_seed >> 5U) & 3U) == 0U) ? 2U : 1U;
            raindrop_speed_count[i] = 0U;
        }
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_RAIN_AND_TAIL_COUNT) == 0U)
    {
        for (uint8_t i = 0U; i < DEMO_RAINDROP_COUNT; ++i)
        {
            raindrop_x[i] = (uint8_t)(((uint16_t)i * LED_MAP_COL) / DEMO_RAINDROP_COUNT);
            raindrop_y[i] = -(int16_t)(i % 8U);
            raindrop_length[i] = 4U;
            raindrop_speed[i] = 1U;
            raindrop_speed_count[i] = 0U;
        }
        gt_led_pattern = LED_PATTERN_P14;
        *p_vsync_tick = 0U;
    }
}

static void LED_Update_Buffer_Meteor(uint32_t* p_vsync_tick)
{
    static int16_t  meteor_x[DEMO_METEOR_COUNT];
    static int16_t  meteor_y[DEMO_METEOR_COUNT];
    static uint16_t meteor_wait[DEMO_METEOR_COUNT];
    static bool initialized = false;

    static const uint8_t tail_rgb[7][3] =
    {
        {100U, 100U, 100U}, /* Head */
        {60U,   95U, 100U},
        {35U,   80U, 100U},
        {18U,   60U,  95U},
        { 8U,   38U,  75U},
        { 3U,   18U,  45U},
        { 1U,    5U,  18U}
    };

    const bool move_meteors =
        ((*p_vsync_tick % DEMO_METEOR_MOVE_DIVIDER) == 0U);

    if (!initialized)
    {
        for (uint8_t i = 0U; i < DEMO_METEOR_COUNT; ++i)
        {
            meteor_x[i] = (int16_t)(LED_MAP_COL - 1U);
            meteor_y[i] = -(int16_t)(i * 3U);
            meteor_wait[i] = (uint16_t)(i * DEMO_METEOR_SPAWN_GAP);
        }
        initialized = true;
    }

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    for (uint8_t i = 0U; i < DEMO_METEOR_COUNT; ++i)
    {
        if (meteor_wait[i] != 0U)
        {
            if (move_meteors)
            {
                --meteor_wait[i];
            }
            continue;
        }

        for (uint8_t tail = 0U; tail < DEMO_METEOR_TAIL_LENGTH; ++tail)
        {
            /* Head travels down-left; its tail extends up-right. */
            int16_t x = meteor_x[i] + (int16_t)tail;
            int16_t y = meteor_y[i] - (int16_t)tail;

            if ((x < 0) || (x >= (int16_t)LED_MAP_COL) ||
                (y < 0) || (y >= (int16_t)LED_MAP_ROW))
            {
                continue;
            }

            uint8_t color = (tail < 7U) ? tail : 6U;
            uint16_t idx = (uint16_t)(y * (int16_t)LED_MAP_COL + x);

            uint16_t red = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * tail_rgb[color][0] + 50U) / 100U));
            uint16_t green = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * tail_rgb[color][1] + 50U) / 100U));
            uint16_t blue = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * tail_rgb[color][2] + 50U) / 100U));

            /* Keep the brightest value if meteors overlap. */
            if (red > gt_led_ld_buffer[idx].ld_r)
                gt_led_ld_buffer[idx].ld_r = red;
            if (green > gt_led_ld_buffer[idx].ld_g)
                gt_led_ld_buffer[idx].ld_g = green;
            if (blue > gt_led_ld_buffer[idx].ld_b)
                gt_led_ld_buffer[idx].ld_b = blue;
        }

        if (move_meteors)
        {
            --meteor_x[i];
            ++meteor_y[i];

            if (meteor_y[i] >=
                (int16_t)(LED_MAP_ROW + DEMO_METEOR_TAIL_LENGTH))
            {
                static uint32_t random_seed = 0x12345678U;
                random_seed = (random_seed * 1664525U) + 1013904223U;

                meteor_x[i] = (int16_t)(LED_MAP_COL - 1U);
                meteor_y[i] = -(int16_t)((random_seed >> 16U) % 5U);
                meteor_wait[i] = (uint16_t)(
                    DEMO_METEOR_SPAWN_GAP + ((random_seed >> 8U) % 17U));
            }
        }
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_METEOR_COUNT) == 0U)
    {
        *p_vsync_tick = 0U;
        for (uint8_t i = 0U; i < DEMO_METEOR_COUNT; ++i)
        {
            meteor_x[i] = (int16_t)(LED_MAP_COL - 1U);
            meteor_y[i] = -(int16_t)(i * 3U);
            meteor_wait[i] = (uint16_t)(i * DEMO_METEOR_SPAWN_GAP);
        }
        gt_led_pattern = LED_PATTERN_P15;
    }
}

typedef struct tag_FIREWORKS_PARTICLE_T
{
    int16_t x_q4;
    int16_t y_q4;
    int16_t vx_q4;
    int16_t vy_q4;
    uint8_t life;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} fireworks_particle_t;

#define FIREWORKS_PARTICLE_COUNT (12U)
#define FIREWORKS_TRAIL_DECAY    (3U)
#define FIREWORKS_ROCKET_TICK_DIVIDER (2U)

static uint8_t gt_fireworks_pixels[LED_MAP_SIZE][3];
static fireworks_particle_t gt_fireworks_particles[FIREWORKS_PARTICLE_COUNT];
static uint16_t gn_fireworks_random = 0xACE1U;
static uint8_t gn_fireworks_launch_countdown;
static int16_t gn_fireworks_rocket_x_q4;
static int16_t gn_fireworks_rocket_y_q4;
static uint8_t gt_fireworks_rocket_color[3];
static bool gb_fireworks_rocket_active;
static bool gb_fireworks_reset_requested = true;
static uint8_t gn_fireworks_burst_color;

static uint16_t Fireworks_Random(void)
{
    gn_fireworks_random = (uint16_t)((gn_fireworks_random >> 1U) ^
        ((gn_fireworks_random & 1U) != 0U ? 0xB400U : 0U));
    return gn_fireworks_random;
}

static void Fireworks_Plot(int16_t x_q4, int16_t y_q4,
    uint8_t red, uint8_t green, uint8_t blue)
{
    if ((x_q4 < 0) || (x_q4 >= (int16_t)(LED_MAP_COL * 16U)) ||
        (y_q4 < 0) || (y_q4 >= (int16_t)(LED_MAP_ROW * 16U)))
    {
        return;
    }

    uint16_t pixel =
        (uint16_t)((y_q4 / 16) * LED_MAP_COL) + (uint16_t)(x_q4 / 16);

    if (red > gt_fireworks_pixels[pixel][0])
        gt_fireworks_pixels[pixel][0] = red;
    if (green > gt_fireworks_pixels[pixel][1])
        gt_fireworks_pixels[pixel][1] = green;
    if (blue > gt_fireworks_pixels[pixel][2])
        gt_fireworks_pixels[pixel][2] = blue;
}

static void LED_Update_Buffer_Fireworks(uint32_t* p_vsync_tick)
{
    static const int8_t burst_vx_q4[FIREWORKS_PARTICLE_COUNT] =
        {-24, -20, -12, 0, 12, 20, 24, 20, 12, 0, -12, -20};
    static const int8_t burst_vy_q4[FIREWORKS_PARTICLE_COUNT] =
        {-8, -18, -24, -28, -24, -18, -8, 8, 18, 24, 18, 8};
    static const uint8_t burst_colors[5][3] =
    {
        {100U, 55U, 5U},
        {100U, 12U, 2U},
        {12U, 38U, 100U},
        {5U, 90U, 85U},
        {100U, 5U, 75U}
    };

    if (gb_fireworks_reset_requested)
    {
        memset(gt_fireworks_pixels, 0, sizeof(gt_fireworks_pixels));
        memset(gt_fireworks_particles, 0, sizeof(gt_fireworks_particles));
        gn_fireworks_launch_countdown = 0U;
        gb_fireworks_rocket_active = false;
        memset(gt_fireworks_rocket_color, 0, sizeof(gt_fireworks_rocket_color));
        gn_fireworks_random = 0xACE1U;
        gb_fireworks_reset_requested = false;
    }

    /* 화면 잔광을 서서히 줄인다. */
    for (uint16_t pixel = 0U; pixel < LED_MAP_SIZE; ++pixel)
    {
        for (uint8_t channel = 0U; channel < 3U; ++channel)
        {
            gt_fireworks_pixels[pixel][channel] =
                (uint8_t)((gt_fireworks_pixels[pixel][channel]
                    * FIREWORKS_TRAIL_DECAY) / 4U);
        }
    }

    if (gn_fireworks_launch_countdown > 0U)
        --gn_fireworks_launch_countdown;

    if (!gb_fireworks_rocket_active &&
        (gn_fireworks_launch_countdown == 0U))
    {
        gn_fireworks_rocket_x_q4 =
            (int16_t)((3U + (Fireworks_Random() % 10U)) * 16U);
        gn_fireworks_rocket_y_q4 =
            (int16_t)((LED_MAP_ROW - 1U) * 16U);

        /* 발사 시 색을 한 번만 선택하고 현재 로켓 색으로 보관한다. */
        gn_fireworks_burst_color =
            (uint8_t)(Fireworks_Random() % 5U);

        for (uint8_t channel = 0U; channel < 3U; ++channel)
        {
            gt_fireworks_rocket_color[channel] =
                burst_colors[gn_fireworks_burst_color][channel];
        }

        gb_fireworks_rocket_active = true;
        gn_fireworks_launch_countdown = 30U;
    }

    if (gb_fireworks_rocket_active &&
        ((*p_vsync_tick % FIREWORKS_ROCKET_TICK_DIVIDER) == 0U))
    {
        gn_fireworks_rocket_y_q4 -= 8;

        /* 상승하는 로켓도 선택된 색을 사용한다. */
        Fireworks_Plot(gn_fireworks_rocket_x_q4, gn_fireworks_rocket_y_q4,
            gt_fireworks_rocket_color[0],
            gt_fireworks_rocket_color[1],
            gt_fireworks_rocket_color[2]);

        if (gn_fireworks_rocket_y_q4 <= (int16_t)(3U * 16U))
        {
            /* 폭발 섬광도 로켓과 동일한 색이다. */
            Fireworks_Plot(gn_fireworks_rocket_x_q4, gn_fireworks_rocket_y_q4,
                gt_fireworks_rocket_color[0],
                gt_fireworks_rocket_color[1],
                gt_fireworks_rocket_color[2]);

            for (uint8_t i = 0U; i < FIREWORKS_PARTICLE_COUNT; ++i)
            {
                gt_fireworks_particles[i].x_q4 = gn_fireworks_rocket_x_q4;
                gt_fireworks_particles[i].y_q4 = gn_fireworks_rocket_y_q4;
                gt_fireworks_particles[i].vx_q4 = burst_vx_q4[i];
                gt_fireworks_particles[i].vy_q4 = burst_vy_q4[i];
                gt_fireworks_particles[i].life = 10U;

                /* 파티클마다 발사 당시의 RGB를 저장한다. */
                gt_fireworks_particles[i].red =
                    gt_fireworks_rocket_color[0];
                gt_fireworks_particles[i].green =
                    gt_fireworks_rocket_color[1];
                gt_fireworks_particles[i].blue =
                    gt_fireworks_rocket_color[2];
            }

            gb_fireworks_rocket_active = false;
        }
    }

    for (uint8_t i = 0U; i < FIREWORKS_PARTICLE_COUNT; ++i)
    {
        fireworks_particle_t* particle = &gt_fireworks_particles[i];

        if (particle->life == 0U)
            continue;

        particle->x_q4 += particle->vx_q4;
        particle->y_q4 += particle->vy_q4;
        particle->vy_q4 += 3;
        --particle->life;

        if ((particle->x_q4 < 0) ||
            (particle->x_q4 >= (int16_t)(LED_MAP_COL * 16U)) ||
            (particle->y_q4 < 0) ||
            (particle->y_q4 >= (int16_t)(LED_MAP_ROW * 16U)))
        {
            particle->life = 0U;
            continue;
        }

        /* 저장된 자기 색으로 폭발 파티클을 그린다. */
        Fireworks_Plot(particle->x_q4, particle->y_q4,
            particle->red, particle->green, particle->blue);
    }

    /* 0..100 색상값에 전체 밝기를 적용한 뒤 14비트 LD 값으로 변환한다. */
    for (uint16_t pixel = 0U; pixel < LED_MAP_SIZE; ++pixel)
    {
        uint32_t red =
            ((uint32_t)gn_led_brightness_percent
                * gt_fireworks_pixels[pixel][0] + 50U) / 100U;
        uint32_t green =
            ((uint32_t)gn_led_brightness_percent
                * gt_fireworks_pixels[pixel][1] + 50U) / 100U;
        uint32_t blue =
            ((uint32_t)gn_led_brightness_percent
                * gt_fireworks_pixels[pixel][2] + 50U) / 100U;

        gt_led_ld_buffer[pixel].ld_r = LD_DATA((uint16_t)red);
        gt_led_ld_buffer[pixel].ld_g = LD_DATA((uint16_t)green);
        gt_led_ld_buffer[pixel].ld_b = LD_DATA((uint16_t)blue);
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_FIREWORKS_COUNT) == 0U)
    {
        gt_led_pattern = LED_PATTERN_P16;
        *p_vsync_tick = 0U;
    }
}

static void LED_Update_Buffer_Ball_Bounce(uint32_t* p_vsync_tick)
{
    static int16_t g_ball_x = (int16_t)DEMO_BALL_RADIUS;
    static int16_t g_ball_y = (int16_t)DEMO_BALL_RADIUS;
    static int8_t g_ball_dx = 1;
    static int8_t g_ball_dy = 1;
    static uint8_t g_ball_hue = 0U;

    const int16_t radius = (int16_t)DEMO_BALL_RADIUS;
    const int16_t min_x = radius;
    const int16_t max_x = (int16_t)LED_MAP_COL - 1 - radius;
    const int16_t min_y = radius;
    const int16_t max_y = (int16_t)LED_MAP_ROW - 1 - radius;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t hue = g_ball_hue;

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    /* hue 복사본만 변환해 애니메이션 상태값을 훼손하지 않도록 한다. */
    if (hue < 85U)
    {
        r = (uint8_t)(255U - hue * 3U);
        g = (uint8_t)(hue * 3U);
        b = 0U;
    }
    else if (hue < 170U)
    {
        hue = (uint8_t)(hue - 85U);
        r = 0U;
        g = (uint8_t)(255U - hue * 3U);
        b = (uint8_t)(hue * 3U);
    }
    else
    {
        hue = (uint8_t)(hue - 170U);
        r = (uint8_t)(hue * 3U);
        g = 0U;
        b = (uint8_t)(255U - hue * 3U);
    }

    for (int16_t dy = -radius; dy <= radius; ++dy)
    {
        for (int16_t dx = -radius; dx <= radius; ++dx)
        {
            if (((dx == -radius) || (dx == radius)) &&
                ((dy == -radius) || (dy == radius)))
            {
                continue;
            }

            int16_t x = g_ball_x + dx;
            int16_t y = g_ball_y + dy;

            if ((x < 0) || (x >= (int16_t)LED_MAP_COL) ||
                (y < 0) || (y >= (int16_t)LED_MAP_ROW))
            {
                continue;
            }

            uint16_t idx = (uint16_t)(y * (int16_t)LED_MAP_COL + x);

            gt_led_ld_buffer[idx].ld_r = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * r + 127U) / 255U));
            gt_led_ld_buffer[idx].ld_g = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * g + 127U) / 255U));
            gt_led_ld_buffer[idx].ld_b = LD_DATA((uint16_t)(
                ((uint32_t)gn_led_brightness_percent * b + 127U) / 255U));
        }
    }

    if ((*p_vsync_tick % DEMO_BALL_MOVE_DIVIDER) == 0U)
    {
        g_ball_x += g_ball_dx;
        g_ball_y += g_ball_dy;

        if (g_ball_x <= min_x)
        {
            g_ball_x = min_x;
            g_ball_dx = 1;
        }
        else if (g_ball_x >= max_x)
        {
            g_ball_x = max_x;
            g_ball_dx = -1;
        }

        if (g_ball_y <= min_y)
        {
            g_ball_y = min_y;
            g_ball_dy = 1;
        }
        else if (g_ball_y >= max_y)
        {
            g_ball_y = max_y;
            g_ball_dy = -1;
        }

        g_ball_hue += 3U;
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_BALL_BOUNCE_COUNT) == 0U)
    {
        gt_led_pattern = LED_PATTERN_P17;
        *p_vsync_tick = 0U;
    }
}

static void LED_Update_Buffer_Sparkle(uint32_t* p_vsync_tick)
{
    uint8_t i;
    uint8_t x;
    uint8_t y;
    uint8_t hue;
    uint32_t r;
    uint32_t rgb;

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    for (i = 0; i < 18U; i++)
    {
        static uint32_t g_random_seed = 0x12345678U;
        g_random_seed = (g_random_seed * 1664525U) + 1013904223U;
        r = g_random_seed;

        x = (uint8_t)(r % LED_MAP_COL);
        y = (uint8_t)((r >> 8) % LED_MAP_ROW);
        hue = (uint8_t)(r >> 16);

        const uint8_t g_brightness = 128U;

        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (hue < 85U)
        {
            r = (uint8_t)(255U - (hue * 3U));
            g = (uint8_t)(hue * 3U);
            b = 0U;
        }
        else if (hue < 170U)
        {
            hue = (uint8_t)(hue - 85U);
            r = 0U;
            g = (uint8_t)(255U - (hue * 3U));
            b = (uint8_t)(hue * 3U);
        }
        else
        {
            hue = (uint8_t)(hue - 170U);
            r = (uint8_t)(hue * 3U);
            g = 0U;
            b = (uint8_t)(255U - (hue * 3U));
        }

        rgb = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;

        r = (rgb >> 16) & 0xFFU;
        g = (rgb >> 8)  & 0xFFU;
        b = rgb & 0xFFU;

        r = ((r * g_brightness) + 127U) / 255U;
        g = ((g * g_brightness) + 127U) / 255U;
        b = ((b * g_brightness) + 127U) / 255U;

        uint16_t map_idx = (uint16_t)(y * LED_MAP_COL + x);
        gt_led_ld_buffer[map_idx].ld_r = r << 6U;
        gt_led_ld_buffer[map_idx].ld_g = g << 6U;
        gt_led_ld_buffer[map_idx].ld_b = b << 6U;
    }


    if ((*p_vsync_tick % PATTERN_CHANGE_SPARKLE_COUNT) == 0U)
    {
        gt_led_pattern = LED_PATTERN_P18;
        *p_vsync_tick = 0U;
    }
}

static const char g_pixelart[LED_MAP_ROW][LED_MAP_COL] =
{
    "................",
    "....KKK..KKK....",
    "...KRRRKKRRRK...",
    "...KRRRRRRRRK...",
    "...KRRRRRRRRK...",
    "....KRRRRRRK....",
    ".....KRRRRK.....",
    "......KRRK......",
    ".......KK......."
};

static const char sunset[LED_MAP_ROW][LED_MAP_COL] =
{
    "NNNNNNNNNNNNNNNN",
    "NNNNVVVVVVVVNNNN",
    "NNVVVVMMMMVVVVNN",
    "VVVVMMMMMMMMVVVV",
    "VVMMOOOOOOOOMMVV", /* 아래 설명 참고 */
    "PPPOOOOOOOOOOPPP",
    "OOOOOOOOOOOOOOOO",
    "HHHHHHHHHHHHHHHH",
    "...OOGGYYGGOO..."
};

static const char trees[LED_MAP_ROW][LED_MAP_COL] =
{
    "................",
    ".G....G....G....",
    "GGG..GGG..GGG...",
    "GGGG..GGGG..GGGG",
    "gGGg..gGGg..gGGg",
    ".GG....GG....GG.",
    ".TT....TT....TT.",
    ".TT....TT....TT.",
    "GGGGGGGGGGGGGGGG"
};

static void LED_Update_Buffer_Pixel_Art(uint32_t* p_vsync_tick)
{
    uint8_t x;
    uint8_t y;
    char p;
    uint32_t rgb;

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    const uint16_t g_brightness = 128U;
    for (y = 0; (y < LED_MAP_ROW) && (y < 16U); y++)
    {
        for (x = 0; (x < LED_MAP_COL) && (x < 16U); x++)
        {
#if 1
            p = g_pixelart[y][x];

            if      (p == 'R') rgb = 0xFF2020U;
            else if (p == 'W') rgb = 0xFFFFFFU;
            else if (p == 'K') rgb = 0x101010U;
            else               rgb = 0x000000U;
#else
#if 0
            p = sunset[y][x];
            if      (p == 'N') rgb = 0x100820U; /* 짙은 남색 */
            else if (p == 'V') rgb = 0x402080U; /* 보라 */
            else if (p == 'M') rgb = 0xA03080U; /* 자주 */
            else if (p == 'O') rgb = 0xF06020U; /* 주황 */
            else if (p == 'Y') rgb = 0xFFFF60U; /* 해 */
            else if (p == 'H') rgb = 0x101018U; /* 지평선 */
            else if (p == 'G') rgb = 0x2080D0U; /* 물빛 */
            else               rgb = 0x000000U;
#else
            p = trees[y][x];
            if (p == 'G') rgb = 0x28C840U; /* 밝은 초록 잎 */
            else if (p == 'g') rgb = 0x126028U; /* 어두운 초록 잎 */
            else if (p == 'T') rgb = 0x8B4A24U; /* 갈색 줄기 */
#endif
#endif
            uint32_t r = 0U;
            uint32_t g = 0U;
            uint32_t b = 0U;

            if (rgb != 0U)
            {
                r = (rgb >> 16) & 0xFFU;
                g = (rgb >> 8)  & 0xFFU;
                b = rgb & 0xFFU;

                r = ((r * g_brightness) + 127U) / 255U;
                g = ((g * g_brightness) + 127U) / 255U;
                b = ((b * g_brightness) + 127U) / 255U;
            }
            uint16_t map_index = (y * LED_MAP_COL) + x;
            gt_led_ld_buffer[map_index].ld_r = r << 6U;
            gt_led_ld_buffer[map_index].ld_g = g << 6U;
            gt_led_ld_buffer[map_index].ld_b = b << 6U;
        }
    }

    if ((*p_vsync_tick % PATTERN_CHANGE_PIXEL_ART_COUNT) == 0U)
    {
        gt_led_pattern = LED_PATTERN_P19;
        *p_vsync_tick = 0U;
    }
}

#define TURN_ARROW_WIDTH                 (12U)
#define TURN_ARROW_HEIGHT                (7U)
#define TURN_ARROW_MOVE_TICKS            (8U)
#define TURN_ARROW_BLINK_INTERVAL_TICKS  (60U) /* 120Hz 기준 0.5초 */
#define TURN_ARROW_BLINK_COUNT           (4U)
#define TURN_ARROW_REPEAT_COUNT          (2U)

typedef struct
{
    uint32_t elapsed_ticks;
    uint8_t repeat_count;
} turn_arrow_state_t;

static const char g_left_turn_arrow[TURN_ARROW_HEIGHT][TURN_ARROW_WIDTH + 1U] =
{
    "...M........",
    "..MM........",
    ".MOMMMMMMMMM",
    "MOOOOOOOOOOM",
    ".MOMMMMMMMMM",
    "..MM........",
    "...M........"
};

static const char g_right_turn_arrow[TURN_ARROW_HEIGHT][TURN_ARROW_WIDTH + 1U] =
{
    "........M...",
    "........MM..",
    "MMMMMMMMMOM.",
    "MOOOOOOOOOOM",
    "MMMMMMMMMOM.",
    "........MM..",
    "........M..."
};
static void LED_Turn_Draw_Arrow(
    const char arrow[TURN_ARROW_HEIGHT][TURN_ARROW_WIDTH + 1U],
    int16_t origin_x)
{
    const int16_t origin_y =
        (int16_t)((LED_MAP_ROW - TURN_ARROW_HEIGHT) / 2U);

    for (uint8_t row = 0U; row < TURN_ARROW_HEIGHT; ++row)
    {
        for (uint8_t col = 0U; col < TURN_ARROW_WIDTH; ++col)
        {
            int16_t x = origin_x + (int16_t)col;
            int16_t y = origin_y + (int16_t)row;
            char pixel = arrow[row][col];

            if (((pixel != 'M') && (pixel != 'O')) ||
                (x < 0) || (x >= (int16_t)LED_MAP_COL) ||
                (y < 0) || (y >= (int16_t)LED_MAP_ROW))
            {
                continue;
            }

            uint16_t map_idx =
                (uint16_t)(y * (int16_t)LED_MAP_COL + x);

            uint16_t red_percent = gn_led_brightness_percent;
            uint16_t green_percent = 0U;
            uint16_t blue_percent = 0U;

            if (pixel == 'M')
            {
                /* 마젠타 테두리: 빨강 + 파랑 */
                blue_percent = gn_led_brightness_percent;
            }
            else
            {
                /* 주황색 내부: 빨강 + 초록 38% */
                green_percent = (uint16_t)(
                    ((uint32_t)gn_led_brightness_percent * 38U + 50U)
                    / 100U);
            }

            gt_led_ld_buffer[map_idx].ld_r = LD_DATA(red_percent);
            gt_led_ld_buffer[map_idx].ld_g = LD_DATA(green_percent);
            gt_led_ld_buffer[map_idx].ld_b = LD_DATA(blue_percent);
        }
    }
}

static void LED_Update_Buffer_Turn(
    uint32_t* p_vsync_tick,
    turn_arrow_state_t* state,
    bool enter_from_right,
    const char arrow[TURN_ARROW_HEIGHT][TURN_ARROW_WIDTH + 1U])
{
    const int16_t left_final_x =
        (int16_t)((LED_MAP_COL - TURN_ARROW_WIDTH) / 2U);
    const int16_t right_final_x =
        (int16_t)(LED_MAP_COL - TURN_ARROW_WIDTH - (uint16_t)left_final_x);

    const int16_t start_x = enter_from_right
        ? (int16_t)(LED_MAP_COL - 1U)
        : -(int16_t)(TURN_ARROW_WIDTH - 1U);
    const int16_t final_x = enter_from_right
        ? left_final_x
        : right_final_x;

    const uint16_t entry_steps = enter_from_right
        ? (uint16_t)(start_x - final_x + 1)
        : (uint16_t)(final_x - start_x + 1);
    const uint32_t entry_ticks =
        (uint32_t)entry_steps * TURN_ARROW_MOVE_TICKS;
    const uint32_t blink_ticks =
        2U * TURN_ARROW_BLINK_COUNT * TURN_ARROW_BLINK_INTERVAL_TICKS;
    const uint32_t sequence_ticks = entry_ticks + blink_ticks;

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    if (state->elapsed_ticks >= sequence_ticks)
    {
        ++state->repeat_count;
        state->elapsed_ticks = 0U;

        if (state->repeat_count >= TURN_ARROW_REPEAT_COUNT)
        {
            state->repeat_count = 0U;
            if (gt_led_pattern == LED_PATTERN_P19)
            {
                gt_led_pattern = LED_PATTERN_P20;
            }
            else if (gt_led_pattern == LED_PATTERN_P20)
            {
                gt_led_pattern = LED_PATTERN_P21;
            }
            *p_vsync_tick = 0U;
            return;
        }
    }

    if (state->elapsed_ticks < entry_ticks)
    {
        uint16_t step =
            (uint16_t)(state->elapsed_ticks / TURN_ARROW_MOVE_TICKS);

        int16_t x = enter_from_right
            ? (int16_t)(start_x - (int16_t)step)
            : (int16_t)(start_x + (int16_t)step);

        LED_Turn_Draw_Arrow(arrow, x);
    }
    else
    {
        uint32_t blink_tick = state->elapsed_ticks - entry_ticks;
        uint32_t blink_phase =
            blink_tick / TURN_ARROW_BLINK_INTERVAL_TICKS;

        /* 각 깜빡임은 0.5초 켜짐, 0.5초 꺼짐입니다. */
        if ((blink_phase % 2U) == 0U)
        {
            LED_Turn_Draw_Arrow(arrow, final_x);
        }
    }

    ++state->elapsed_ticks;
}


static void LED_Update_Buffer_Left_Turn(uint32_t* p_vsync_tick)
{
    static turn_arrow_state_t state = {0U, 0U};

    LED_Update_Buffer_Turn(
        p_vsync_tick,
        &state,
        true,
        g_left_turn_arrow);
}

static void LED_Update_Buffer_Right_Turn(uint32_t* p_vsync_tick)
{
    static turn_arrow_state_t state = {0U, 0U};

    LED_Update_Buffer_Turn(
        p_vsync_tick,
        &state,
        false,
        g_right_turn_arrow);
}

static void LED_Update_Buffer_GT_Banner(uint32_t* p_vsync_tick)
{
    enum
    {
        FONT_WIDTH = 5U,
        FONT_HEIGHT = 7U,
        FONT_SPACING = 1U,
        BANNER_TEXT_LENGTH = 19U
    };

    static const char text[] = "GLOBAL TECHNOLOGIES";

    /* 각 문자의 7개 행: 유효한 비트는 하위 5비트입니다.
     * 왼쪽 픽셀부터 bit 4, 3, 2, 1, 0 순서입니다.
     */
    static const uint8_t font[26][FONT_HEIGHT] =
    {
        {14U, 17U, 17U, 31U, 17U, 17U, 17U}, /* A */
        {30U, 17U, 17U, 30U, 17U, 17U, 30U}, /* B */
        {14U, 17U, 16U, 16U, 16U, 17U, 14U}, /* C */
        {30U, 17U, 17U, 17U, 17U, 17U, 30U}, /* D */
        {31U, 16U, 16U, 30U, 16U, 16U, 31U}, /* E */
        {31U, 16U, 16U, 30U, 16U, 16U, 16U}, /* F */
        {14U, 17U, 16U, 23U, 17U, 17U, 14U}, /* G */
        {17U, 17U, 17U, 31U, 17U, 17U, 17U}, /* H */
        {31U,  4U,  4U,  4U,  4U,  4U, 31U}, /* I */
        { 7U,  2U,  2U,  2U, 18U, 18U, 12U}, /* J */
        {17U, 18U, 20U, 24U, 20U, 18U, 17U}, /* K */
        {16U, 16U, 16U, 16U, 16U, 16U, 31U}, /* L */
        {17U, 27U, 21U, 21U, 17U, 17U, 17U}, /* M */
        {17U, 25U, 25U, 21U, 19U, 19U, 17U}, /* N */
        {14U, 17U, 17U, 17U, 17U, 17U, 14U}, /* O */
        {30U, 17U, 17U, 30U, 16U, 16U, 16U}, /* P */
        {14U, 17U, 17U, 17U, 21U, 18U, 13U}, /* Q */
        {30U, 17U, 17U, 30U, 20U, 18U, 17U}, /* R */
        {15U, 16U, 16U, 14U,  1U,  1U, 30U}, /* S */
        {31U,  4U,  4U,  4U,  4U,  4U,  4U}, /* T */
        {17U, 17U, 17U, 17U, 17U, 17U, 14U}, /* U */
        {17U, 17U, 17U, 17U, 17U, 10U,  4U}, /* V */
        {17U, 17U, 17U, 21U, 21U, 21U, 10U}, /* W */
        {17U, 17U, 10U,  4U, 10U, 17U, 17U}, /* X */
        {17U, 17U, 10U,  4U,  4U,  4U,  4U}, /* Y */
        {31U,  1U,  2U,  4U,  8U, 16U, 31U}  /* Z */
    };

    const int32_t scroll_columns =
        (int32_t)(*p_vsync_tick / BANNER_SCROLL_TICKS_PER_COLUMN);
    const int32_t text_width =
        (int32_t)(BANNER_TEXT_LENGTH * (FONT_WIDTH + FONT_SPACING)
                  - FONT_SPACING);
    const int32_t first_x = (int32_t)LED_MAP_COL - scroll_columns;
    const uint8_t first_y =
        (uint8_t)((LED_MAP_ROW - FONT_HEIGHT) / 2U);

    for (uint16_t idx = 0U; idx < LED_MAP_SIZE; ++idx)
    {
        gt_led_ld_buffer[idx].ld_r = 0U;
        gt_led_ld_buffer[idx].ld_g = 0U;
        gt_led_ld_buffer[idx].ld_b = 0U;
    }

    for (uint8_t char_idx = 0U; char_idx < BANNER_TEXT_LENGTH; ++char_idx)
    {
        char ch = text[char_idx];
        if (ch == ' ')
        {
            continue;
        }

        uint8_t letter_idx = (uint8_t)(ch - 'A');
        int32_t char_x = first_x
            + (int32_t)char_idx * (FONT_WIDTH + FONT_SPACING);

        for (uint8_t row = 0U; row < FONT_HEIGHT; ++row)
        {
            for (uint8_t col = 0U; col < FONT_WIDTH; ++col)
            {
                int32_t x = char_x + col;

                if ((x < 0) || (x >= (int32_t)LED_MAP_COL) ||
                    ((font[letter_idx][row] & (1U << (FONT_WIDTH - 1U - col))) == 0U))
                {
                    continue;
                }

                uint16_t map_idx =
                    (uint16_t)((first_y + row) * LED_MAP_COL + (uint16_t)x);

                if (char_idx < 6U) /* GLOBAL: cyan */
                {
                    gt_led_ld_buffer[map_idx].ld_r = 0U;
                    gt_led_ld_buffer[map_idx].ld_g = LD_DATA((uint16_t)(((uint16_t)gn_led_brightness_percent * 70U) / 100U));
                    gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
                }
                else /* TECHNOLOGIES: white */
                {
                    uint16_t level = LD_DATA(gn_led_brightness_percent);
                    gt_led_ld_buffer[map_idx].ld_r = level;
                    gt_led_ld_buffer[map_idx].ld_g = level;
                    gt_led_ld_buffer[map_idx].ld_b = level;
                }
            }
        }
    }

    if (scroll_columns >= ((int32_t)LED_MAP_COL + text_width))
    {
        gt_led_pattern = LED_PATTERN_P0;
        *p_vsync_tick = 0U;
    }
}

static void LED_Update_Buffer_By_Pattern(uint32_t* p_vsync_tick)
{
    switch (gt_led_pattern)
    {
        case LED_PATTERN_P0 : // Red
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = 0U;
                gt_led_ld_buffer[map_idx].ld_b = 0U;
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P1;
            }
            break;
        case LED_PATTERN_P1 : // Green
            for (uint8_t map_idx = 0 ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0U;
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = 0U;
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P2;
            }
            break;
        case LED_PATTERN_P2 : // Blue
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0U;
                gt_led_ld_buffer[map_idx].ld_g = 0U;
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P3;
            }
            break;
        case LED_PATTERN_P3 : // Magenta
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = 0U;
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P4;
            }
            break;
        case LED_PATTERN_P4 : // Yellow
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = 0U;
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P5;
            }
            break;
        case LED_PATTERN_P5 : // Cyan
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = 0U;
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent);
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent);
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P6;
            }
            break;
        case LED_PATTERN_P6 : // White
            for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
            {
                gt_led_ld_buffer[map_idx].ld_r = LD_DATA(gn_led_brightness_percent) * 0.32f;
                gt_led_ld_buffer[map_idx].ld_g = LD_DATA(gn_led_brightness_percent) * 0.80f;
                gt_led_ld_buffer[map_idx].ld_b = LD_DATA(gn_led_brightness_percent) * 1.00f;
            }
            if ((*p_vsync_tick % PATTERN_CHANGE_COLOR_COUNT) == 0U)
            {
                *p_vsync_tick = 0U;
                gt_led_pattern = LED_PATTERN_P7;
            }
            break;
        case LED_PATTERN_P7 : // Rainbow direction to right
            LED_Update_Buffer_Rainbow_Left_to_Right(p_vsync_tick);
            break;
        case LED_PATTERN_P8 : // Rainbow direction to up to down
            LED_Update_Buffer_Rainbow_Up_to_Down(p_vsync_tick);
            break;
        case LED_PATTERN_P9 : // Pixel Rainbow Flow1
            LED_Update_Buffer_Pixel_Rainbow_Flow1(p_vsync_tick);
            break;
        case LED_PATTERN_P10 : // Pixel Rainbow Flow2
            LED_Update_Buffer_Pixel_Rainbow_Flow2(p_vsync_tick);
            break;
        case LED_PATTERN_P11 : // Pixel Rainbow Flow2
            LED_Update_Buffer_Blue_Cyan_Wave(p_vsync_tick);
            break;
        case LED_PATTERN_P12 : // Column Color Shift
            LED_Update_Buffer_Column_Color_Shift(p_vsync_tick);
            break;
        case LED_PATTERN_P13 : // Rain + Tail
            LED_Update_Buffer_Rain_and_Tail(p_vsync_tick);
            break;
        case LED_PATTERN_P14 : // Meteor
            LED_Update_Buffer_Meteor(p_vsync_tick);
            break;
        case LED_PATTERN_P15 : // Fireworks
            LED_Update_Buffer_Fireworks(p_vsync_tick);
            break;
        case LED_PATTERN_P16 : // Ball bounce
            LED_Update_Buffer_Ball_Bounce(p_vsync_tick);
            break;
        case LED_PATTERN_P17 : // Sparkle
            LED_Update_Buffer_Sparkle(p_vsync_tick);
            break;
        case LED_PATTERN_P18 : // Pixel Art
            LED_Update_Buffer_Pixel_Art(p_vsync_tick);
            break;
        case LED_PATTERN_P19 : // Left turn
            LED_Update_Buffer_Left_Turn(p_vsync_tick);
            break;
        case LED_PATTERN_P20 : // Right turn
            LED_Update_Buffer_Right_Turn(p_vsync_tick);
            break;
        case LED_PATTERN_P21 : // GT Banner
            LED_Update_Buffer_GT_Banner(p_vsync_tick);
            break;
        default :
            Print(LOG_ERROR, "\r\n Invalid LED Buffer Update By Pattern\r\n");
            Error_Handler();
            break;
    }

    if (gt_operate_state == OPERATE_STATE_ONLY_B)
    {
        for (uint8_t map_idx = 0U ; map_idx < LED_MAP_SIZE ; ++map_idx)
        {
            const uint16_t ld_r = gt_led_ld_buffer[map_idx].ld_r;
            const uint16_t ld_g = gt_led_ld_buffer[map_idx].ld_g;
            const uint16_t ld_b = gt_led_ld_buffer[map_idx].ld_b;

            const uint32_t gray =
                (299UL * ld_r + 587UL * ld_g + 114UL * ld_b) / 1000UL;

            uint32_t max = ld_r;

            if (ld_g > max)
            {
                max = ld_g;
            }

            if (ld_b > max)
            {
                max = ld_b;
            }

            const uint32_t blue =
                (gray * GRAY_BLEND_RATIO +
                max * (100UL - GRAY_BLEND_RATIO) + 50UL) / 100UL;

            gt_led_ld_buffer[map_idx].ld_r = 0U;
            gt_led_ld_buffer[map_idx].ld_g = 0U;
            gt_led_ld_buffer[map_idx].ld_b = (uint16_t)blue;
        }
    }
}

void LED_Update_Buffer(uint32_t* p_vsync_tick)
{
    LED_Update_Buffer_By_Pattern(p_vsync_tick);
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

    Serialize_Tx_Start(pwm_length);
    while (gb_pwm_dma_tx_flag) {}

    uint32_t n_response = 0U;

    if (true == Serialize_Rx_Start(XDIC_READ_RECV_BITS))
    {
        Print(LOG_ERROR, "Rx Timeout!!! [addr - 0x%02X]\r\n", in_addr);
    }
    else
    {
        float f_frequency = Decode_Input_Response(&n_response, XDIC_READ_RECV_BITS);
        Print(LOG_DEBUG, "Received DATA(0x%02X):[%1.3fMHz, CODE : 0x%01X, ID : 0x%02X, DATA - 0x%03X, FULL - 0x%06X]\r\n", in_addr, f_frequency,
            ((n_response >> 17U) & SERIAL_DECODE_MASK_CODE), ((n_response >> 12U) & SERIAL_DECODE_MASK_ID), ((n_response >> 0U) & SERIAL_DECODE_MASK_DATA), n_response);
    }

    us_delay(XDIC_READ_DELAY);

    return (uint16_t)(n_response & SERIAL_DECODE_MASK_DATA);
}

void MCU_IF_Write_LD(void)
{
    uint16_t pwm_length = 0U;

    gn_serialize_tx_buffer[pwm_length++] = 0U;

    for (uint8_t i = 0U ; i < LED_MAP_SIZE ; ++i)
    {
        uint8_t led_map_index = gt_led_id_map[i];
        uint16_t LD_data_R = (uint16_t)(gt_led_ld_buffer[led_map_index].ld_r * LED_BRT_COMP_RED);
        uint16_t LD_data_G = (uint16_t)(gt_led_ld_buffer[led_map_index].ld_g * LED_BRT_COMP_GREEN);
        uint16_t LD_data_B = (uint16_t)(gt_led_ld_buffer[led_map_index].ld_b * LED_BRT_COMP_BLUE);

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