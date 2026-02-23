
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
#define CONST_MHz_TO_Hz             (1000000.0f)
#define CONST_mV_TO_V               (1000.0f)

#define VSYNC                       (120.0f)
#define XD_MCLK                     (39319200.0f)

#define XD_DAISY_SIZE               (2)
#define XD_CH_SIZE                  (12)
#define XC_CH_ENABLE_SIZE           (20)
#define TOTAL_BLOCK_SIZE            (XD_CH_SIZE * XD_DAISY_SIZE * XC_CH_ENABLE_SIZE)

#define XC_SERIAL_CLK_CNT_LOW       (8)
#define XC_SERIAL_CLK_CNT_HIGH      (17)

#define XD_SERIAL_CLK_CNT_LOW       (13)
#define XD_SERIAL_CLK_CNT_HIGH      (26)

#define SERIAL_CMD_SIZE             (4)
#define SERIAL_ID_SIZE              (5)
#define SERIAL_ADDR_SIZE            (6)
#define SERIAL_FAULT_DATA_SIZE      (4)
#define SERIAL_DATA_SIZE            (12)
#define SERIAL_LD_SIZE              (16)

#define XDIC_WRITE_BITS             (SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE + SERIAL_DATA_SIZE)
#define XDIC_READ_BITS              (SERIAL_CMD_SIZE + SERIAL_ADDR_SIZE)
#define XDIC_READ_RECV_BITS         (SERIAL_CMD_SIZE + SERIAL_ID_SIZE + SERIAL_DATA_SIZE)
#define XDIC_LD_TRANS_BITS          (SERIAL_CMD_SIZE + SERIAL_LD_SIZE * XD_CH_SIZE)
#define XDIC_FAULT_READ_BITS        (SERIAL_CMD_SIZE)
#define XDIC_FAULT_RECV_BITS        (SERIAL_CMD_SIZE + SERIAL_FAULT_DATA_SIZE)
#define XDIC_SYNCGEN_BITS           (SERIAL_CMD_SIZE)
#define XDIC_IDGEN_BITS             (SERIAL_CMD_SIZE)

#define XDIC_SERIALIZER_TIME        (((CONST_MHz_TO_Hz / XD_MCLK) * (XD_SERIAL_CLK_CNT_HIGH + XD_SERIAL_CLK_CNT_LOW)))
#define XDIC_RESET_DELAY            (100)
#define XDIC_WRITE_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_WRITE_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_READ_DELAY             ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_READ_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_READ_RECV_DELAY        ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_READ_RECV_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_LD_TRANS_DELAY         ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_LD_TRANS_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_FAULT_READ_DELAY       ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_FAULT_READ_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_FAULT_RECV_DELAY       ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_FAULT_RECV_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_SYNCGEN_DELAY          ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_SYNCGEN_BITS * XD_DAISY_SIZE) + 0.5f))
#define XDIC_IDGEN_DELAY            ((uint32_t)((XDIC_SERIALIZER_TIME * XDIC_IDGEN_BITS * XD_DAISY_SIZE) + 0.5f))
/********************************************************/

#define BUILD_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
              + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define BUILD_MONTH (__DATE__ [2] == 'n' ? 0 \
                    : __DATE__ [2] == 'b' ? 1 \
                    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
                    : __DATE__ [2] == 'y' ? 4 \
                    : __DATE__ [2] == 'n' ? 5 \
                    : __DATE__ [2] == 'l' ? 6 \
                    : __DATE__ [2] == 'g' ? 7 \
                    : __DATE__ [2] == 'p' ? 8 \
                    : __DATE__ [2] == 't' ? 9 \
                    : __DATE__ [2] == 'v' ? 10 : 11)

#define BUILD_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))

#define VERSION_MAJOR       0x0000
#define VERSION_MINOR       0x0001
#define VERSION_REVISION    0x0001

#define ANSI_FONT_NONE      "\033[0m"
#define ANSI_FONT_RED       "\033[31m"
#define ANSI_FONT_GREEN     "\033[32m"
#define ANSI_FONT_YELLOW    "\033[33m"
#define ANSI_FONT_BLUE      "\033[34m"
#define ANSI_FONT_MAGENTA   "\033[35m"

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
