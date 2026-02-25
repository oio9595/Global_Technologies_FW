
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

/********************************************************/
#define VERSION_MAJOR       (0)
#define VERSION_MINOR       (0)
#define VERSION_PATCH       (1) // At every new build, increase this value
/********************************************************/
#define APB1_TIM_FREQ               (90.0f) //MHz
#define APB2_TIM_FREQ               (APB1_TIM_FREQ * 2) //MHz

#define CONST_MHz_TO_Hz             (1000000.0f)
#define CONST_mV_TO_V               (1000.0f)

#define VSYNC                       (120.0f)
#define XDIC_MCLK                   (50000000.0f)

#define SVSYNC_PHASE_GREEN          (1U)
#define SVSYNC_PHASE_BLUE           (0U)
#define SVSYNC_GATING_TIME_US       (21U)
#define SVSYNC_CYCLE                (2U)
#define SVSYNC_SIZE                 (32U)
#define SVSYNC_TOTAL_CYCLE          (SVSYNC_CYCLE * SVSYNC_SIZE)

#define XDIC_DAISY_SIZE             (24)
//#define XDIC_DAISY_SIZE             (1)
#define XDIC_CH_SIZE                (12)

#define XDIC_SERIAL_CLK_CNT_LOW     (16)
#define XDIC_SERIAL_CLK_CNT_HIGH    (34)

#define SERIAL_CMD_SIZE             (4)
#define SERIAL_ID_SIZE              (5)
#define SERIAL_ADDR_SIZE            (6)
#define SERIAL_FAULT_DATA_SIZE      (4)
#define SERIAL_DATA_SIZE            (12)
#define SERIAL_EVEN_SIZE            (14)
#define SERIAL_ODD_SIZE             (28)
#define SERIAL_LD_SIZE              (SERIAL_EVEN_SIZE + SERIAL_ODD_SIZE)

#define XDIC_WRITE_BITS             ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE + SERIAL_DATA_SIZE) * XDIC_DAISY_SIZE))
#define XDIC_READ_BITS              ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE) * XDIC_DAISY_SIZE))
#define XDIC_READ_RECV_BITS         ((uint32_t)(SERIAL_CMD_SIZE + SERIAL_ID_SIZE + SERIAL_DATA_SIZE))
#define XDIC_LD_TRANS_BITS          ((uint32_t)((SERIAL_CMD_SIZE + SERIAL_LD_SIZE * ((uint32_t)(XDIC_CH_SIZE / 2.0f))) * XDIC_DAISY_SIZE))
#define XDIC_FAULT_READ_BITS        ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))
#define XDIC_FAULT_RECV_BITS        ((uint32_t)(SERIAL_CMD_SIZE + SERIAL_FAULT_DATA_SIZE))
#define XDIC_SYNCGEN_BITS           ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))
#define XDIC_IDGEN_BITS             ((uint32_t)(SERIAL_CMD_SIZE * XDIC_DAISY_SIZE))

#define XDIC_SERIALIZER_TIME        ((((CONST_MHz_TO_Hz / XDIC_MCLK) * (XDIC_SERIAL_CLK_CNT_HIGH + XDIC_SERIAL_CLK_CNT_LOW))) * 2.0f)
#define XDIC_RESET_DELAY            (100)
#define XDIC_WRITE_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_WRITE_BITS) + 0.5f))
#define XDIC_READ_DELAY             ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_READ_BITS) + 0.5f))
#define XDIC_LD_TRANS_DELAY         ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_LD_TRANS_BITS) + 0.5f))
#define XDIC_FAULT_READ_DELAY       ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_FAULT_READ_BITS) + 0.5f))
#define XDIC_SYNCGEN_DELAY          ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_SYNCGEN_BITS) + 0.5f))
#define XDIC_IDGEN_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_IDGEN_BITS) + 0.5f))

/********************************************************/
#define ANSI_FONT_NONE      "\033[0m"
#define ANSI_FONT_RED       "\033[31m"
#define ANSI_FONT_GREEN     "\033[32m"
#define ANSI_FONT_YELLOW    "\033[33m"
#define ANSI_FONT_BLUE      "\033[34m"
#define ANSI_FONT_MAGENTA   "\033[35m"
/********************************************************/
#ifdef __cplusplus
}
#endif

#endif /* ~__CONFIG_H__ */
/*** end of file ***/