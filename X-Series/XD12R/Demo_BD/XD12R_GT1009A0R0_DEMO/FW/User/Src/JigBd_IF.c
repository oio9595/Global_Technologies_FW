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
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint16_t gn_serialize_tx_buffer[4000] = {0, };
static uint16_t gn_serialize_rx_risingBuffer[400] = {0, };
static uint16_t gn_serialize_rx_fallingBuffer[400] = {0, };

volatile uint16_t gn_xd_rx_timeout;
volatile bool gb_xd_timeout_event;
volatile bool gb_pwm_dma_tx_flag;
volatile bool gb_pwm_is_rx_flag;

static uint8_t gn_xdic_dimming_channel = 0;
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
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

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)(&(TIM2->CCR2)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gn_serialize_rx_risingBuffer);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)(&(TIM2->CCR1)));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_serialize_rx_fallingBuffer);
}

static uint16_t Get_Nth_Bit(uint64_t x, int n)
{
    return (x >> (n - 1)) & 1ULL;
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

    Serialize_Tx_Done();
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

    Serialize_Tx_Done();
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

void MCU_IF_Write_LD(uint16_t* p_in_LD_data)
{
    uint16_t pwm_length = 0;
    uint64_t LD_data_Even = p_in_LD_data[0];
    uint64_t LD_data_Odd = ((uint64_t)p_in_LD_data[1] << 14) | ((uint64_t)p_in_LD_data[2] << 0);

    gn_serialize_tx_buffer[pwm_length++] = 0;

    uint32_t pwm_period = LL_TIM_GetAutoReload(TIM1);
    uint16_t bit_0 = (uint16_t)(((pwm_period + 1) * BIT_0_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);
    uint16_t bit_1 = (uint16_t)(((pwm_period + 1) * BIT_1_RATIO / BIT_RATIO_SUM) - 1 + 0.5f);

    for (uint8_t i = 0 ; i < XDIC_DAISY_SIZE ; ++i)
    {
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;
        gn_serialize_tx_buffer[pwm_length++] = bit_1;

        for (uint8_t j = 0 ; j < XDIC_CH_SIZE ; ++j)
        {
            bool write_data = (gn_xdic_dimming_channel == 0) || ((gn_xdic_dimming_channel - 1) == j);
            if (j % 2 == 0) // Even
            {
                for (uint8_t k = 0 ; k < SERIAL_EVEN_SIZE ; ++k)
                {
                    if (write_data)
                    {
                        gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint64_t)LD_data_Even, SERIAL_EVEN_SIZE - k)) ? bit_1 : bit_0);
                    }
                    else
                    {
                        gn_serialize_tx_buffer[pwm_length++] = bit_0;
                    }
                }
            }
            else // Odd
            {
                for (uint8_t k = 0 ; k < SERIAL_ODD_SIZE ; ++k)
                {
                    if (write_data)
                    {
                        gn_serialize_tx_buffer[pwm_length++] = ((Get_Nth_Bit((uint64_t)LD_data_Odd, SERIAL_ODD_SIZE - k)) ? bit_1 : bit_0);
                    }
                    else
                    {
                        gn_serialize_tx_buffer[pwm_length++] = bit_0;
                    }
                }
            }
        }
    }

    gn_serialize_tx_buffer[pwm_length++] = 0;

    gb_pwm_is_rx_flag = false;
    Serialize_Tx_Start(pwm_length);

    while (gb_pwm_dma_tx_flag) {}

    Serialize_Tx_Done();
    //us_delay(XDIC_LD_TRANS_DELAY);
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

    Serialize_Tx_Done();

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

    Serialize_Tx_Done();
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

    Serialize_Tx_Done();
    us_delay(XDIC_SYNCGEN_DELAY);
}
/* USER CODE END */

/*** end of file ***/