/** @file config.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* TRIM CONFIG  **********************************/
#define INIT_ADC_PER_REG_OSC        ( 1)
#define INIT_ADC_PER_REG_VREF       ( 1)
#define INIT_ADC_PER_REG_OFS        (10)
#define INIT_ADC_PER_REG_GAIN       (18)
/********************************************************/

/********************************************************/
#define APB1_TIM_FREQ               (90.0f) //MHz
#define APB2_TIM_FREQ               (APB1_TIM_FREQ * 2U) //MHz

#define CONST_MHz_TO_Hz             (1000000.0f)
#define CONST_mV_TO_V               (1000.0f)

#define VSYNC                       (120.0f)
#define XDIC_MCLK                   (50000000.0f)

#if 0
    #define SVSYNC_PHASE_RED        (1U)
    #define SVSYNC_PHASE_GREEN      (2U)
    #define SVSYNC_PHASE_BLUE       (0U)
    #define SVSYNC_GATING_TIME_US   (20U)
    #define SVSYNC_CYCLE            (3U)
    #define SVSYNC_SIZE             (32U)
    #define SVSYNC_TOTAL_CYCLE      (SVSYNC_CYCLE * SVSYNC_SIZE)
#else
    #define SVSYNC_PHASE_GREEN      (1U)
    #define SVSYNC_PHASE_BLUE       (0U)
    #define SVSYNC_GATING_TIME_US   (21U)
    #define SVSYNC_CYCLE            (2U)
    #define SVSYNC_SIZE             (32U)
    #define SVSYNC_TOTAL_CYCLE      (SVSYNC_CYCLE * SVSYNC_SIZE)
#endif

#define XDIC_DAISY_SIZE             (1U)
#define XDIC_CH_SIZE                (12U)

#define XDIC_SERIAL_CLK_CNT_LOW     (16U)
#define XDIC_SERIAL_CLK_CNT_HIGH    (34U)

#define XC24R_MCLK_INTERNAL          (0x00U)
#define XC24R_MCLK_EXTERNAL          (0x01U)
#define XC24R_MCLK_MODE              XC24R_MCLK_EXTERNAL

#if (XC24R_MCLK_MODE == XC24R_MCLK_INTERNAL)
    #define XC_SERIAL_CLK_CNT_LOW       (8U)
    #define XC_SERIAL_CLK_CNT_HIGH      (17U)
#else
    #define XC_SERIAL_CLK_CNT_LOW       (6U)
    #define XC_SERIAL_CLK_CNT_HIGH      (10U)
#endif

#define SERIAL_CMD_SIZE             (4U)
#define SERIAL_ID_SIZE              (5U)
#define SERIAL_ADDR_SIZE            (6U)
#define SERIAL_FAULT_DATA_SIZE      (4U)
#define SERIAL_DATA_SIZE            (12U)
#define SERIAL_ODD_SIZE             (14U)
#define SERIAL_EVEN_SIZE            (28U)
#define SERIAL_LD_SIZE              (SERIAL_ODD_SIZE + SERIAL_EVEN_SIZE)

#define XDIC_WRITE_BITS             ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE + SERIAL_DATA_SIZE) * XDIC_DAISY_SIZE))
#define XDIC_READ_BITS              ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE) * XDIC_DAISY_SIZE))
#define XDIC_READ_RECV_BITS         ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_ID_SIZE + SERIAL_DATA_SIZE) * XDIC_DAISY_SIZE))
#define XDIC_LD_TRANS_BITS          ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_LD_SIZE * ((uint32_t)(XDIC_CH_SIZE / 2.0f))) * XDIC_DAISY_SIZE))
#define XDIC_FAULT_READ_BITS        ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))
#define XDIC_FAULT_RECV_BITS        ((uint32_t)(SERIAL_CMD_SIZE + SERIAL_FAULT_DATA_SIZE))
#define XDIC_SYNCGEN_BITS           ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))
#define XDIC_IDGEN_BITS             ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))

#define XDIC_SERIALIZER_TIME        ((((CONST_MHz_TO_Hz / XDIC_MCLK) * (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))) * 2.0f)
#define XDIC_RESET_DELAY            (100U)
#define XDIC_WRITE_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_WRITE_BITS) + 0.5f))
#define XDIC_READ_DELAY             ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_READ_BITS) + 0.5f))
#define XDIC_READ_RECV_DELAY        ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_READ_RECV_BITS) + 0.5f))
#define XDIC_LD_TRANS_DELAY         ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_LD_TRANS_BITS) + 0.5f))
#define XDIC_FAULT_READ_DELAY       ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_FAULT_READ_BITS) + 0.5f))
#define XDIC_FAULT_RECV_DELAY       ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_FAULT_RECV_BITS) + 0.5f))
#define XDIC_SYNCGEN_DELAY          ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_SYNCGEN_BITS) + 0.5f))
#define XDIC_IDGEN_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_IDGEN_BITS) + 0.5f))

#define ANSI_FONT_NONE              "\033[0m"
#define ANSI_FONT_RED               "\033[31m"
#define ANSI_FONT_GREEN             "\033[32m"
#define ANSI_FONT_YELLOW            "\033[33m"
#define ANSI_FONT_BLUE              "\033[34m"
#define ANSI_FONT_MAGENTA           "\033[35m"

typedef struct tag_XDIC_REG_INFO_T
{
    uint8_t address;
    const char* name;
    void* reg_ptr;
} _reg_map_t;

#ifdef __cplusplus
}
#endif

#endif /* ~__CONFIG_H__ */

/*** end of file ***/