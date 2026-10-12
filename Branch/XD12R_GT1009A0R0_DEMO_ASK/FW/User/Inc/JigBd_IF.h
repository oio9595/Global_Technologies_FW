/** @file JigBd_IF.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __JIG_BD_IF_H__
#define __JIG_BD_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define XD_TIMEOUT_MS       (2)
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_LED_COLOR_TYPE_T
{
    LED_COLOR_ALL = 0U,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_MAGENTA,
    LED_COLOR_YELLOW,
    LED_COLOR_CYAN,
    LED_COLOR_MAX,
} led_color_type_t;

typedef enum tag_LED_PATTERN_T
{
    LED_PATTERN_P0 = 0U, // Red
    LED_PATTERN_P1, // Green
    LED_PATTERN_P2, // Blue
    LED_PATTERN_P3, // Magenta
    LED_PATTERN_P4, // Yellow
    LED_PATTERN_P5, // Cyan
    LED_PATTERN_P6, // White
    LED_PATTERN_P7, // Rainbow direction to left to right
    LED_PATTERN_P8, // Rainbow direction to up to down
    LED_PATTERN_P9, // Pixel Rainbow Flow1
    LED_PATTERN_P10, // Pixel Rainbow Flow2
    LED_PATTERN_P11, // Blue/Cyan Wave
    LED_PATTERN_P12, // Column Color Shift
    LED_PATTERN_P13, // Rain + Tail
    LED_PATTERN_P14, // Meteor
    LED_PATTERN_P15, // Fireworks
    LED_PATTERN_P16, // Ball bounce
    LED_PATTERN_P17, // Sparkle
    LED_PATTERN_P18, // Pixel Art
    LED_PATTERN_P19, // Left turn
    LED_PATTERN_P20, // Right turn
    LED_PATTERN_P21, // GT Banner
    LED_PATTERN_MAX,
} led_pattern_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern volatile bool gb_pwm_dma_tx_flag;
extern volatile bool gb_pwm_is_rx_flag;
extern volatile uint16_t gn_xd_rx_timeout;
extern volatile bool gb_xd_timeout_event;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
extern void LED_Update_Buffer(uint32_t* p_vsync_tick);

extern void LED_Set_Operation_State(uint8_t operation_state);
extern void LED_Set_Pattern(uint8_t pattern);

extern void us_delay(uint32_t n_delay);
extern void JigBD_IF_Link_DMA_With_Buffer(void);

extern void MCU_IF_Write_XDIC(uint8_t in_addr, uint16_t in_data);
extern uint16_t MCU_IF_Read_XDIC(uint8_t in_addr);
extern void MCU_IF_Write_LD(void);
extern void MCU_IF_IdGen_Command();
#if 0
extern uint16_t MCU_IF_Fault_Read_Command(void);
extern void MCU_IF_SyncGen_Command();
#endif

/* USER CODE END */

#ifdef __cplusplus
}
#endif

#endif /* ~__JIG_BD_IF_H__ */

/*** end of file ***/