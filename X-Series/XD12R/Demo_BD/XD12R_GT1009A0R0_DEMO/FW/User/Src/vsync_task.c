/** @file vsync_task.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
 #define __VSYNC_TASK_C__


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "JigBd_IF.h"
#include "xdic.h"
#include "config.h"
#include "uart.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM3_APB_FREQ_Hz        (APB1_TIM_FREQ * 1000000U)
#define TIM3_APB_FREQ_MHz       (APB1_TIM_FREQ)
#define TIM3_PRESCALER          (0U)
#define TIM3_FREQUENCY_Hz       (TIM3_APB_FREQ_Hz / (TIM3_PRESCALER + 1U))
#define TIM3_CALC_PERIOD(freq)  ((uint32_t)((TIM3_FREQUENCY_Hz / freq) - 1U))
#define TIM3_CALC_COMPARE(us)   ((uint32_t)((TIM3_APB_FREQ_MHz * us) - 1U))

#define SVSYNC_VSYNC_FREQ       (50000U)
#define SVSYNC_VSYNC_ON_us      (10U) // 10us

#define SVSYNC_GREEN_FREQ       (7680U)
#define SVSYNC_BLUE_FREQ        (7680U)
#define SVSYNC_DIMMING_ON_us    (20U) // 20us
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool gb_xdic_vsync_flag;
static uint16_t gn_svsync_count;

static bool gb_xdic_write_flag;
static uint8_t gn_xdic_write_addr;
static uint16_t gn_xdic_write_data;

static bool gb_xdic_read_flag;
static uint8_t gn_xdic_read_addr;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
static inline void Vled_R_ON(void)
{
    VLED_R_EN_HI();
}

static inline void Vled_R_Off(void)
{
    VLED_R_EN_LO();
}

static inline void Vled_G_ON(void)
{
    VLED_EN_HI();
    VLED_G_SW_HI();
    VLED_G_OFF_HI();
}

static inline void Vled_G_Off(void)
{
    VLED_EN_LO();
    VLED_G_SW_LO();
    VLED_G_OFF_LO();
}

static inline void Vled_B_ON(void)
{
    VLED_EN_HI();
    VLED_B_SW_HI();
    VLED_B_OFF_HI();
}

static inline void Vled_B_Off(void)
{
    VLED_EN_LO();
    VLED_B_SW_LO();
    VLED_B_OFF_LO();
}

static void Svsync_Timer_Start(void)
{
    gn_svsync_count = 0;
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);

    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_ClearFlag_UPDATE(TIM3);

    uint32_t period = TIM3_CALC_PERIOD(SVSYNC_VSYNC_FREQ); // 50% of 1-sub frame
    LL_TIM_SetAutoReload(TIM3, period);

    uint32_t compare = TIM3_CALC_COMPARE(SVSYNC_VSYNC_ON_us); // 10us
    LL_TIM_OC_SetCompareCH1(TIM3, compare);

    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM3);
}

void Svsync_Update_Handler(void)
{
    ++gn_svsync_count;
    uint8_t phase = (gn_svsync_count % SVSYNC_CYCLE);

    if (phase == SVSYNC_PHASE_GREEN) // For Green, change frequency
    {
        Vled_B_Off();
        Vled_G_ON();
        uint32_t period = TIM3_CALC_PERIOD(SVSYNC_GREEN_FREQ); // 50% of 1-sub frame
        LL_TIM_SetAutoReload(TIM3, period);
    }
    else if (phase == SVSYNC_PHASE_BLUE) // For Blue, change frequency
    {
        Vled_G_Off();
        Vled_B_ON();
        uint32_t period = TIM3_CALC_PERIOD(SVSYNC_BLUE_FREQ); // 50% of 1-sub frame
        LL_TIM_SetAutoReload(TIM3, period);
        if (gn_svsync_count == SVSYNC_TOTAL_CYCLE)
        {
            us_delay(SVSYNC_GATING_TIME_US + 1U);
            LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);
            LL_TIM_DisableCounter(TIM3);
            LL_TIM_SetCounter(TIM3, 0U);
        }
    }

    uint32_t compare = TIM3_CALC_COMPARE(SVSYNC_DIMMING_ON_us); // 20us
    LL_TIM_OC_SetCompareCH1(TIM3, compare);
}

void Vsync_Timer_Start(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM8);
    LL_TIM_EnableIT_UPDATE(TIM8);
    LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM8);
    Vled_R_ON();
}

void Vsync_Timer_Stop(void)
{
    LL_TIM_DisableCounter(TIM8);
    LL_TIM_SetCounter(TIM8, 0);
    LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableIT_UPDATE(TIM8);
}

void Vsync_Update_Handler(void)
{
    Svsync_Timer_Start();
    gb_xdic_vsync_flag = true;
}

void XDIC_Set_Write_Target_Reg(uint8_t addr, uint16_t data)
{
    gn_xdic_write_addr = addr;
    gn_xdic_write_data = data;
    gb_xdic_write_flag = true;
}

void XDIC_Set_Read_Target_Reg(uint8_t addr)
{
    gn_xdic_read_addr = addr;
    gb_xdic_read_flag = true;
}

void XDIC_Vsync_Task(void)
{
    if (gb_xdic_vsync_flag)
    {
        MCU_IF_Write_LD();
        if (gb_xdic_write_flag)
        {
            XDIC_Write_General_Reg(gn_xdic_write_addr, gn_xdic_write_data);
            gb_xdic_write_flag = false;
        }
        if (gb_xdic_read_flag)
        {
            uint16_t ret = XDIC_Read_General_Reg(gn_xdic_read_addr);
            Print(LOG_INFO, "XDIC Read --> [ 0x%02X - 0x%04X] \r\n", gn_xdic_read_addr, ret);
            gb_xdic_read_flag = false;
        }
        LED_Update_Buffer();
        gb_xdic_vsync_flag = false;
    }
}
/* USER CODE END */
/*** end of file ***/