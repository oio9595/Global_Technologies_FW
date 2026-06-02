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
    LED_COLOR_ALL = 0,
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
    LED_PATTERN_P0 = 0, // Checkerboard pattern 1
    LED_PATTERN_P1,     // Checkerboard pattern 2
    LED_PATTERN_P2,
    LED_PATTERN_P3,
    LED_PATTERN_P4,
    LED_PATTERN_P5,
    LED_PATTERN_P6,
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
extern void LED_Select_Color(uint8_t color);
extern void LED_Select_Brightness(uint8_t brightness);
extern void LED_Select_Brightness_Up(void);
extern void LED_Select_Brightness_Down(void);
extern void LED_Select_Pattern(uint8_t pattern);
extern void LED_Select_Pixel(uint8_t pixel);
extern void LED_Update_Buffer(void);

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