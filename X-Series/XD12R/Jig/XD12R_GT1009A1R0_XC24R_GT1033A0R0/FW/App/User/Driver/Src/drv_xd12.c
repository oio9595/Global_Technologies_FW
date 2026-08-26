#include <stdio.h>
#include <string.h>

#include "main.h"
#include "drv_timer.h"
#include "drv_xd12.h"
#include "drv_xc24.h"
#include "comm_debugging.h"
#include "drv_gpio.h"
#include "drv_ads124s08.h"
#include "ldim_conversion.h"

#define SERIALIZE_LOG_DUMP_DISABLE    (0U)
#define SERIALIZE_LOG_DUMP_ENABLE     (1U)
#define SERIALIZE_LOG_DUMP            (SERIALIZE_LOG_DUMP_DISABLE)

/* XD/IC602 serializer protocol */
#define CMD_CODE_WRITE          (0x0DU)  /* 0b1101 */
#define CMD_CODE_READ           (0x0EU)  /* 0b1110 */
#define CMD_CODE_LDIM           (0x0FU)  /* 0b1111 */
#define CMD_CODE_RD_FAULT       (0x0AU)  /* 0b1010 */
#define CMD_CODE_SYNCGEN        (0x09U)  /* 0b1001 */
#define CMD_CODE_IDGEN          (0x08U)  /* 0b1000 */

#define PWM_OUT_BIT0            (((TIM1_PERIOD + 1U) * 1) / 3U)
#define PWM_OUT_BIT1            (((TIM1_PERIOD + 1U) * 2) / 3U)

#define PWM_OUT_HEADER_SIZE     (0U)
#define PWM_OUT_TAIL_SIZE       (1U)
#define PWM_OUT_DUMMY_SIZE      (PWM_OUT_HEADER_SIZE + PWM_OUT_TAIL_SIZE)

#define XD_MAX_CURRENT_REF     (0x01FU)
#define XD_MAX_LDO_DIG         (0x00FU)
#define XD_MAX_LDO_DAC         (0x01FU)
#define XD_MAX_LDO_FLL         (0x00FU)
#define XD_MAX_OSC             (0x01FU)
#define XD_MAX_CH_GAIN         (0x07FU)
#define XD_MAX_CH_OFS          (0x1FFU)

#define XD_BIT_IDGEN           (4U)
#define XD_BIT_SYNCGEN         (4U)
#define XD_HDR_BIT             (4U)
#define XD_ADDR_BIT            (6U)
#define XD_ID_BIT              (5U)
#define XD_DATA_BIT            (12U)
#define XD_FAULT_BIT           (4U)
#define XD_LD_TRANSFER         (XD_HDR_BIT + (XD_LD_DATA_BIT * XD_LD_SIZE))
#define XD_CMD_WRITE           (XD_HDR_BIT + XD_ADDR_BIT + XD_DATA_BIT)
#define XD_CMD_READ            (XD_HDR_BIT + XD_ADDR_BIT)
#define XD_CMD_READOUT         (XD_HDR_BIT + XD_ID_BIT + XD_DATA_BIT)
#define XD_CMD_FAULT           (XD_HDR_BIT)
#define XD_CMD_FAULT_READOUT   (XD_HDR_BIT + XD_FAULT_BIT)

#define CMD_DELAY_DUMMY         (10U) /* 10us */
#define CMD_DELAY_REG_WR        ((XD_DAISY_LENGTH * XD_CMD_WRITE) + CMD_DELAY_DUMMY)           /* CMD_DELAY_DUMMY * 1 */
#define CMD_DELAY_LD            ((XD_DAISY_LENGTH * XD_LD_TRANSFER) + CMD_DELAY_DUMMY)         /* CMD_DELAY_DUMMY * 1 */
#define CMD_DELAY_SYNCGEN       ((XD_DAISY_LENGTH * XD_BIT_SYNCGEN) + CMD_DELAY_DUMMY)         /* CMD_DELAY_DUMMY * 1 */
#define CMD_DELAY_IDGEN         ((XD_DAISY_LENGTH * XD_BIT_IDGEN) + CMD_DELAY_DUMMY)           /* CMD_DELAY_DUMMY * 1 */
#define CMD_DELAY_FAULT_READ    ((XD_DAISY_LENGTH * XD_CMD_READOUT) + (CMD_DELAY_DUMMY * 4U))  /* CMD_DELAY_DUMMY * 4 */
#define CMD_DELAY_REG_RD        ((XD_DAISY_LENGTH * XD_CMD_READ) + (CMD_DELAY_DUMMY * 8U))     /* CMD_DELAY_DUMMY * 8 */

#define XD_FUNCTION_DIS        (0U)
#define XD_FUNCTION_EN         (1U)

#define XD_LD_MODE_NORMAL      (0U)
#define XD_LD_MODE_X8          (1U)

#define XD_LD_DIR_HEAD         (0U)
#define XD_LD_DIR_TAIL         (1U)

#define XD_PWM_RES_12BIT       (0U)
#define XD_PWM_RES_14BIT       (1U)

#define XD_FPWM_DIV_1          (0x17U)
#define XD_FPWM_DIV_2          (0x0CU)
#define XD_FPWM_DIV_3          (0x07U)

#define XD_CH_LD_TYPE_NTS_1ST  (0U) /* non-time-sharing data 1LD 1st */
#define XD_CH_LD_TYPE_NTS_2ND  (1U) /* non-time-sharing data 1LD 2nd */
#define XD_CH_LD_TYPE_TS_1ST   (2U) /* time-sharing data 2LD 1st */
#define XD_CH_LD_TYPE_TS_2ND   (3U) /* time-sharing data 2LD 2nd */

#define XD_MAX_CURR_VREF_1     (0xFFFU)
#define XD_MAX_CURR_VREF_2     (0xFFFU)
#define XD_MAX_CURR_VREF_3     (0xFFFU)
#define XD_MAX_CURR_VREF_4     (0x000U)
#define XD_MAX_CURR_VREF_5     (0x000U)

#define XD_FB_LVL_1            (FB_LEVEL_4)
#define XD_FB_LVL_2            (FB_LEVEL_4)
#define XD_FB_LVL_3            (FB_LEVEL_4)
#define XD_FB_LVL_4            (FB_LEVEL_4)
#define XD_FB_LVL_5            (FB_LEVEL_4)

#define XD_SHORT_LVL_1         (SHORT_LEVEL_36)
#define XD_SHORT_LVL_2         (SHORT_LEVEL_36)
#define XD_SHORT_LVL_3         (SHORT_LEVEL_36)
#define XD_SHORT_LVL_4         (SHORT_LEVEL_36)
#define XD_SHORT_LVL_5         (SHORT_LEVEL_36)

#define XD_MAX_CURR_LVL_1      (CURR_LEVEL_24)
#define XD_MAX_CURR_LVL_2      (CURR_LEVEL_24)
#define XD_MAX_CURR_LVL_3      (CURR_LEVEL_24)
#define XD_MAX_CURR_LVL_4      (CURR_LEVEL_24)
#define XD_MAX_CURR_LVL_5      (CURR_LEVEL_24)

#define XD_FLL_GAIN            (1U)
#define XD_FLL_ERROR_RANGE     (0U)

#define XD_SERIAL_LATENCY      (0x80U) /* default */

#define XD_V_MASK              (0x72U)
#define XD_SV_MASK             (0x89U)

#define XD_RST_COUNT           (0x00U)
#define XD_TIMEOUT             (0x3FFU)

#define XD_FLL_COUNT           (0x3413FU)
#define XD_FLL_RANGE           (0x3U)

#define XD_WR_PROTECT_1        (0xAAAU) /* Writable: FPWM_DIV_1/2/3, MAX_CURR_VREF_1/2/3/4/5 */
#define XD_WR_PROTECT_2        (0xCCCU) /* Writable: MAX_CURR_VREF_1/2/3/4/5 */
#define XD_WR_PROTECT_3        (0x555U) /* Writable: ALL */

#define XD_SPRD_SPD_128_CLK   (0U) /* '000' : 128 clock */
#define XD_SPRD_SPD_64_CLK    (1U) /* '001' : 64 clock */
#define XD_SPRD_SPD_32_CLK    (2U) /* '010' : 32 clock */
#define XD_SPRD_SPD_16_CLK    (3U) /* '011' : 16 clock */
#define XD_SPRD_SPD_8_CLK     (4U) /* '100' : 8 clock */
#define XD_SPRD_SPD_4_CLK     (5U) /* '101' : 4 clock */
#define XD_SPRD_SPD_2_CLK     (6U) /* '110' : 2 clock */
#define XD_SPRD_SPD_1_CLK     (7U) /* '111' : 1 clock */

#define XD_SPRD_GAIN_DIV_8    (0U)  /* '000' : calculated value/8 */
#define XD_SPRD_GAIN_DIV_4    (1U)  /* '001' : calculated value/4 */
#define XD_SPRD_GAIN_DIV_2    (2U)  /* '010' : calculated value/2 */
#define XD_SPRD_GAIN_1X       (3U)  /* '011' : calculated value */
#define XD_SPRD_GAIN_MUL_2    (4U)  /* '100' : calculated value*2 */
#define XD_SPRD_GAIN_MUL_4    (5U)  /* '101' : calculated value*4 */
#define XD_SPRD_GAIN_MUL_8    (6U)  /* '110' : calculated value*8 */
#define XD_SPRD_GAIN_MUL_16   (7U)  /* '111' : calculated value*16 */

typedef union tag_SERDES_WRITE_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t data       : 12;
        uint32_t addr       :  6;
        uint32_t cmd_code   :  4;
        uint32_t            : 10;   /* reserved */
    }bit;
}_v_serdes_write_command_t;

typedef union tag_SERDES_READ_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t addr       :  6;
        uint32_t cmd_code   :  4;
        uint32_t            : 22;   /* reserved */
    }bit;
}_v_serdes_read_command_t;

typedef union tag_SERDES_READOUT_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t data       : 12;
        uint32_t id         :  5;
        uint32_t cmd_code   :  4;
        uint32_t            : 11;   /* reserved */
    }bit;
}_v_serdes_readout_command_t;

typedef union tag_SERDES_LD_TRANSFER_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t ld         : 16;   /* LD * N ea, ch(ld)_size, 12bit for ld_width=0, 14bit ld_width=1 */
        uint32_t cmd_code   :  4;
        uint32_t            : 12;   /* reserved */
    }bit;
}_v_serdes_ld_transfer_command_t;

typedef union tag_SERDES_FAULT_READ_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  4;
        uint32_t            : 28;   /* reserved */
    }bit;
}_v_serdes_fault_read_command_t;

typedef union tag_SERDES_FAULT_READOUT_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t bit_fb     :  1;
        uint32_t bit_open   :  1;
        uint32_t bit_short  :  1;
        uint32_t bit_thermal:  1;
        uint32_t cmd_code   :  4;
        uint32_t            : 24;   /* reserved */
    }bit;
}_v_serdes_fault_readout_command_t;

typedef union tag_SERDES_SYNC_GEN_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  4;
        uint32_t            : 28;   /* reserved */
    }bit;
}_v_serdes_sync_gen_command_t;

typedef union tag_SERDES_ID_GEN_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  4;
        uint32_t            : 28;   /* reserved */
    }bit;
}_v_serdes_id_gen_command_t;

static _xd12_regs_t gt_xd12_set_regs;
static _xd12_regs_t gt_xd12_get_regs[XD_DAISY_LENGTH];

static _xd12_otp_ctrl_regs_t gt_xd12_otp_ctrl_set_regs;
static _xd12_otp_ctrl_regs_t gt_xd12_otp_ctrl_get_regs[XD_DAISY_LENGTH];

static _xd12_mirror_regs_t gt_xd12_mirror_set_regs;
static _xd12_mirror_regs_t gt_xd12_mirror_get_regs[XD_DAISY_LENGTH];

#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    static uint16_t gn_pwm_out_xd_write[(XD_DAISY_LENGTH * XD_CMD_WRITE) + PWM_OUT_DUMMY_SIZE];
    static uint16_t gn_pwm_out_xd_ld_transfer[(XD_DAISY_LENGTH * XD_LD_TRANSFER) + PWM_OUT_DUMMY_SIZE];
#endif

static uint16_t gn_pwm_in_xd_response_freq[(XD_DAISY_LENGTH * XD_CMD_READOUT) + 2U];
static uint16_t gn_pwm_in_xd_response_duty[(XD_DAISY_LENGTH * XD_CMD_READOUT) + 2U];

volatile bool gb_xd_pwm_out_flag;
volatile bool gb_xd_pwm_in_flag;
volatile bool gb_xd_pwm_in_timeout;

#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    static _v_serdes_write_command_t gt_xd_write_command[XD_DAISY_LENGTH];
    static _v_serdes_read_command_t gt_xd_read_command[XD_DAISY_LENGTH];
    static _v_serdes_fault_read_command_t gt_xd_fault_read_command[XD_DAISY_LENGTH];
    static _v_serdes_sync_gen_command_t gt_xd_syncgen_command[XD_DAISY_LENGTH];
    static _v_serdes_id_gen_command_t gt_xd_idgen_command[XD_DAISY_LENGTH];
#endif

static bool gb_xd_do_efuse;

#if (SERIALIZE_LOG_DUMP == SERIALIZE_LOG_DUMP_ENABLE)
static void xd12_serialize_log_dump(uint16_t addr, uint16_t data)
{
    UART_PutChar('\r');
    UART_PutChar('\n');

    // "0x" 전송
    UART_PutChar('0');
    UART_PutChar('x');

    // 16진수 4자리 변환 및 전송 (상위 니블부터 하위 니블까지)
    for (int8_t shift = 12; shift >= 0; shift -= 4)
    {
        uint8_t nibble = (addr >> shift) & 0x0F;
        char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));

        UART_PutChar(hex_char);
    }

    UART_PutChar(',');

    // "0x" 전송
    UART_PutChar('0');
    UART_PutChar('x');

    // 16진수 4자리 변환 및 전송 (상위 니블부터 하위 니블까지)
    for (int8_t shift = 12; shift >= 0; shift -= 4)
    {
        uint8_t nibble = (data >> shift) & 0x0F;
        char hex_char = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));

        UART_PutChar(hex_char);
    }
}
#endif

#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
static void start_timeout_timer(uint16_t timeout_us)
{
    /* Set the Autoreload Register value */
    LL_TIM_SetAutoReload(TIM12, (uint32_t)timeout_us);
    /* Enable the Interrupt sources */
    LL_TIM_EnableIT_UPDATE(TIM12);
    /* Enable the TIM Counter */
    LL_TIM_EnableCounter(TIM12);
}

static void stop_timeout_timer(void)
{
    /* Disable the Interrupt sources */
    LL_TIM_DisableIT_UPDATE(TIM12);
    /* Disable the TIM Counter */
    LL_TIM_DisableCounter(TIM12);
    /* Set the Counter Register value */
    LL_TIM_SetCounter(TIM12, 0U);
    /* Set the Autoreload Register value */
    LL_TIM_SetAutoReload(TIM12, TIM12_PERIOD);
}

static bool xd12_pwm_in(uint16_t length, uint16_t timeout_us)
{
    gb_xd_pwm_in_flag = true;
    gb_xd_pwm_in_timeout = false;

    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_TIM_ClearFlag_CC1(TIM2);
    LL_TIM_ClearFlag_CC2(TIM2);

    BUFFER_OE_HI();

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, length);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, length);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM2);

    start_timeout_timer(timeout_us);

    while(true == gb_xd_pwm_in_flag)
    {
        __NOP();

        if(true == gb_xd_pwm_in_timeout)
        {
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
            gb_xd_pwm_in_flag = false;
            break;
        }
    }

    stop_timeout_timer();

    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, 0);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, 0);
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_ClearFlag_TC5(DMA1);

    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_DisableCounter(TIM2);

    BUFFER_OE_LO();

    return gb_xd_pwm_in_timeout;
}

static void xd12_pwm_out_done(void)
{
    while(gb_xd_pwm_out_flag)
    {
        __NOP();
    }
}

static void xd12_pwm_out(uint32_t mem_addr, uint32_t length)
{
    gb_xd_pwm_out_flag = true;
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, mem_addr);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, length);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
    LL_TIM_EnableCounter(TIM1);
}

static uint16_t xd12_make_pwm_out_stream(uint32_t data, uint16_t *p, uint16_t len)
{
    if((p == NULL) || (len == 0U) || (len > 32U))
    {
        return 0U;
    }

    uint32_t bit_data = (uint32_t)(data << (32U - len));
    uint16_t* p_dst = p;

    for(uint16_t i = 0; i < len; ++i)
    {
        *p_dst++ = (bit_data & 0x80000000UL) ? (uint16_t)PWM_OUT_BIT1 : (uint16_t)PWM_OUT_BIT0;
        bit_data <<= 1U;
    }

    return len;
}

static inline uint16_t DECODE_BIT(uint16_t duty, uint16_t min, uint16_t max)
{
    return (duty > min && duty < max) ? 1U : 0U;
}

static uint16_t xd12_decode_pwm_input_stream(uint16_t* pfreq, uint16_t* pduty, uint16_t* pdata, uint16_t len)
{
    if((pfreq == NULL) || (pduty == NULL) || (pdata == NULL) || (len == 0U))
    {
        return 0U;
    }

    uint32_t sum_freq = 0U;
    uint16_t count = 0;

    for (uint16_t i = 0U; i < len; ++i)
    {
        sum_freq += pfreq[i];
        ++count;
    }

    const uint32_t avg_freq = sum_freq / count; // average counter
    const uint16_t logic_1_min = (uint16_t)(avg_freq >> 1U);            // duty : 50%
    const uint16_t logic_1_max = (uint16_t)((avg_freq * 84U) / 100U);   // duty : 84%

    uint16_t id = 0;
    uint16_t n_header = 0U;
    uint16_t n_data = 0U;

    if(len == 8U)    /* fault receive format : '1010' + 'thermal_short_open_fb' */
    {
        const uint16_t* p_src = pduty;

        /* 4bit */
        n_header  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
        n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
        n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
        n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);

        /* 4bit */
        n_data  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);

        pdata[id++] = n_data;
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nXD Fault Recv Packet\r\n\tCMD - 0x%01X, FAULT - 0x%01X", n_header, n_data);
    }
    else        /* read receive format : '1110' + id[4:0] + data[11:0] */
    {
        uint16_t duty_idx = 0;
        uint16_t n_id = 0U;

        while(((uint32_t)duty_idx + XD_CMD_READOUT) <= len)
        {
            if(id >= XD_DAISY_LENGTH)
            {
                break;
            }

            const uint16_t* p_src = &pduty[duty_idx];

            n_data = 0U;

            /* 4bit */
            n_header  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
            n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
            n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
            n_header |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);

            /* 5bit */
            n_id  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 4U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);

            /* 12bit */
            for(int16_t bit = 11; bit >= 0; --bit)
            {
                n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << (uint16_t)bit);
            }

            pdata[id++] = n_data;
            duty_idx += XD_CMD_READOUT;
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXD Read Recv Packet\r\n\tCMD - 0x%01X, ID - 0x%02X, DATA - 0x%03X", n_header, n_id, n_data);
        }
    }

    return id;
}
#endif

void xd12_make_readable(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->ALL = 0U;
    xd12_write_by_type(XD12R_OP_MODE + XD12R_OTP_CTRL_BASE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_parity_rd_en_t* _r1B = &gt_xd12_set_regs.reg._r1B;
    _r1B->bit.reg_rd_en = XD_FUNCTION_EN;
    _r1B->bit.parity_e = XD_FUNCTION_DIS;
    xd12_write_by_type(XD12R_PARITY_RD_EN, _r1B->ALL, XD12R_ADDR_TYPE_GENERAL);
}

static void xd12_regs_init_table(void)
{
    _xd12_regs_t* _r1 = &gt_xd12_set_regs;

    for(xd12_addr_t addr = XD12R_RESET_ID; addr < XD12R_MAX; ++addr)
    {
        switch(addr)
        {
            case XD12R_RESET_ID:
            {
                _r1->reg._r00.bit.lkg_e = XD_FUNCTION_EN;
                _r1->reg._r00.bit.e_rst = XD_FUNCTION_DIS;
                _r1->reg._r00.bit.vs_rst = XD_FUNCTION_DIS;
                _r1->reg._r00.bit.rst = XD_FUNCTION_DIS;
                break;
            }
            case XD12R_LD_CONTROL:
            {
                _r1->reg._r01.bit.ld_mode = XD_LD_MODE_NORMAL;
                _r1->reg._r01.bit.ld_dir = XD_LD_DIR_TAIL;
                _r1->reg._r01.bit.ld_res = XD_PWM_RES_14BIT;
                _r1->reg._r01.bit.syncmode = XD_SYNC_MODE;
                _r1->reg._r01.bit.delay_mode_en = XD_FUNCTION_EN;
                _r1->reg._r01.bit.sv_no = XD_SV_NO;
                break;
            }
            case XD12R_LD_SIZE:
            {
                _r1->reg._r02.bit.ld_size = XD_LD_SIZE;
                break;
            }
            case XD12R_PWMCLK_DIV1_2:
            {
                _r1->reg._r03.bit.pwmclk_div1 = XD_FPWM_DIV_1;
                _r1->reg._r03.bit.pwmclk_div2 = ((XD_FPWM_DIV_2 & 0x0FU) >> 0U);
                break;
            }
            case XD12R_PWMCLK_DIV2_3:
            {
                _r1->reg._r04.bit.pwmclk_div2 = ((XD_FPWM_DIV_2 & 0xF0U) >> 4U);
                _r1->reg._r04.bit.pwmclk_div3 = XD_FPWM_DIV_3;
                break;
            }
            case XD12R_CHANNEL_ENABLE:
            {
                _r1->reg._r05.bit.ch1_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch2_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch3_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch4_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch5_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch6_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch7_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch8_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch9_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch10_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch11_en = XD_FUNCTION_EN;
                _r1->reg._r05.bit.ch12_en = XD_FUNCTION_EN;
                break;
            }
            case XD12R_FAULT_CONTROL:
            {
                _r1->reg._r06.bit.o_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.s_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.t_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.o_det_e = XD_FUNCTION_EN;
                _r1->reg._r06.bit.s_det_e = XD_FUNCTION_EN;
                _r1->reg._r06.bit.o_fb_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.fb_mode = 0U;
                _r1->reg._r06.bit.auto_fault_fb_no = 2U;
                break;
            }
            case XD12R_CHx_LD_TYPE0:
            {
                _r1->reg._r07.bit.ch7_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch8_ld_type = XD_CH_LD_TYPE_TS_1ST;
                _r1->reg._r07.bit.ch9_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch10_ld_type = XD_CH_LD_TYPE_TS_1ST;
                _r1->reg._r07.bit.ch11_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch12_ld_type = XD_CH_LD_TYPE_TS_1ST;
                break;
            }
            case XD12R_CHx_LD_TYPE1:
            {
                _r1->reg._r08.bit.ch1_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch2_ld_type = XD_CH_LD_TYPE_TS_1ST;
                _r1->reg._r08.bit.ch3_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch4_ld_type = XD_CH_LD_TYPE_TS_1ST;
                _r1->reg._r08.bit.ch5_ld_type = XD_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch6_ld_type = XD_CH_LD_TYPE_TS_1ST;
                break;
            }
            case XD12R_MAX_CURR_VREF1:
            {
                _r1->reg._r0A.bit.max_curr_vref = XD_MAX_CURR_VREF_1;
                break;
            }
            case XD12R_MAX_CURR_VREF2:
            {
                _r1->reg._r0B.bit.max_curr_vref = XD_MAX_CURR_VREF_2;
                break;
            }
            case XD12R_MAX_CURR_VREF3:
            {
                _r1->reg._r0C.bit.max_curr_vref = XD_MAX_CURR_VREF_3;
                break;
            }
            case XD12R_MAX_CURR_VREF4:
            {
                _r1->reg._r0D.bit.max_curr_vref = XD_MAX_CURR_VREF_4;
                break;
            }
            case XD12R_MAX_CURR_VREF5:
            {
                _r1->reg._r0E.bit.max_curr_vref = XD_MAX_CURR_VREF_5;
                break;
            }
            case XD12R_DELAY_CH1_2:
            {
                _r1->reg._r10.bit.delay_ch1 = 0U;   /* CH1 */
                _r1->reg._r10.bit.delay_ch2 = 0U;   /* CH2 */
                break;
            }
            case XD12R_DELAY_CH3_4:
            {
                _r1->reg._r11.bit.delay_ch3 = 0U;   /* CH3 */
                _r1->reg._r11.bit.delay_ch4 = 0U;   /* CH4 */
                break;
            }
            case XD12R_DELAY_CH5_6:
            {
                _r1->reg._r12.bit.delay_ch5 = 0U;   /* CH5 */
                _r1->reg._r12.bit.delay_ch6 = 0U;   /* CH6 */
                break;
            }
            case XD12R_DELAY_CH7_8:
            {
                _r1->reg._r13.bit.delay_ch7 = 0U;   /* CH7 */
                _r1->reg._r13.bit.delay_ch8 = 0U;   /* CH8 */
                break;
            }
            case XD12R_DELAY_CH9_10:
            {
                _r1->reg._r14.bit.delay_ch9 = 0U;   /* CH9 */
                _r1->reg._r14.bit.delay_ch10 = 0U;   /* CH10 */
                break;
            }
            case XD12R_DELAY_CH11_12:
            {
                _r1->reg._r15.bit.delay_ch11 = 0U;   /* CH11 */
                _r1->reg._r15.bit.delay_ch12 = 0U;   /* CH12 */
                break;
            }
            case XD12R_FB_LEVEL:
            {
                _r1->reg._r16.bit.fb1_level = XD_FB_LVL_1;
                _r1->reg._r16.bit.fb2_level = XD_FB_LVL_2;
                _r1->reg._r16.bit.fb3_level = XD_FB_LVL_3;
                _r1->reg._r16.bit.fb4_level = XD_FB_LVL_4;
                break;
            }
            case XD12R_FB_SHORT_LEVEL:
            {
                _r1->reg._r17.bit.fb5_level = XD_FB_LVL_5;
                _r1->reg._r17.bit.short1_level = XD_SHORT_LVL_1;
                _r1->reg._r17.bit.short2_level = XD_SHORT_LVL_2;
                _r1->reg._r17.bit.short3_level = XD_SHORT_LVL_3;
                break;
            }
            case XD12R_SHORT_LEVEL:
            {
                _r1->reg._r18.bit.short4_level = XD_SHORT_LVL_4;
                _r1->reg._r18.bit.short5_level = XD_SHORT_LVL_5;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL1:
            {
                _r1->reg._r19.bit.max_curr1_level = XD_MAX_CURR_LVL_1;
                _r1->reg._r19.bit.max_curr2_level = XD_MAX_CURR_LVL_2;
                _r1->reg._r19.bit.max_curr3_level = XD_MAX_CURR_LVL_3;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL2:
            {
                _r1->reg._r1A.bit.max_curr4_level = XD_MAX_CURR_LVL_4;
                _r1->reg._r1A.bit.max_curr5_level = XD_MAX_CURR_LVL_5;
                break;
            }
            case XD12R_SERIAL_CLK_GEN:
            {
                _r1->reg._r1C.bit.serial_clk_high = XD_SERIAL_CLK_HIGH;
                _r1->reg._r1C.bit.serial_clk_low = XD_SERIAL_CLK_LOW;
                break;
            }
            case XD12R_SERIAL_LATENCY:
            {
                _r1->reg._r1D.bit.serial_latency = XD_SERIAL_LATENCY;
                break;
            }
            case XD12R_V_MASK:
            {
                _r1->reg._r1E.bit.v_mask = XD_V_MASK;
                break;
            }
            case XD12R_SV_MASK:
            {
                _r1->reg._r1F.bit.sv_mask = XD_SV_MASK;
                _r1->reg._r1F.bit.sv_mask_en = XD_FUNCTION_EN;
                break;
            }
            case XD12R_RSTCNT:
            {
                _r1->reg._r20.bit.rstcnt = XD_RST_COUNT;
                break;
            }
            case XD12R_TIMEOUT:
            {
                _r1->reg._r21.bit.timeout = XD_TIMEOUT;
                break;
            }
            case XD12R_FLLCNT1:
            {
                _r1->reg._r22.bit.fllcnt = ((XD_FLL_COUNT & 0x000FFFUL) >>  0U);
                break;
            }
            case XD12R_FLLCNT2:
            {
                _r1->reg._r23.bit.fllcnt = ((XD_FLL_COUNT & 0x1FF000UL) >> 12U);
                _r1->reg._r23.bit.fll_range = XD_FLL_RANGE;
                _r1->reg._r23.bit.fll_en = XD_FUNCTION_EN;
                break;
            }
            case XD12R_WR_PROTECT:
            {
                _r1->reg._r24.bit.wr_protect = XD_WR_PROTECT_3;
                break;
            }
            case XD12R_NF_CONTROL:
            {
                _r1->reg._r25.bit.DGRJT_EN1 = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.DGRJT_EN2 = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_EN = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN1 = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN2 = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.O_EMI_REJ_EN = XD_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_TH = XD_FUNCTION_DIS;
                break;
            }
            case XD12R_CHOP_EN:
            {
                _r1->reg._r26.bit.CHOP_BGR_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DAC_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSC_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSCLDO_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DRV_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_EN = XD_FUNCTION_EN;
                break;
            }
            case XD12R_TEMP:
            {
                _r1->reg._r27.bit.flt_gain = XD_FLL_GAIN;
                _r1->reg._r27.bit.o_slew = 2U;
                _r1->reg._r27.bit.flt_ctl = 2U;
                _r1->reg._r27.bit.dac_rng = 0U;
                _r1->reg._r27.bit.ov_swap_en = 0U;
                _r1->reg._r27.bit.ofs_temp = 8U;
                break;
            }
            case XD12R_OSC_FLL_MAN1:
            {
                _r1->reg._r28.bit.osc_fll_man = 0U;
                break;
            }
            case XD12R_OSC_FLL_MAN2:
            {
                _r1->reg._r29.bit.osc_fll_man = 8U;
                _r1->reg._r29.bit.osc_fll_err_range = XD_FLL_ERROR_RANGE;
                _r1->reg._r29.bit.osc_man_en = XD_FUNCTION_DIS;
                break;
            }
            case XD12R_OSC_SPREAD:
            {
                _r1->reg._r2A.bit.SPRD_GAIN = XD_SPRD_GAIN_DIV_8;
                _r1->reg._r2A.bit.SPRD_SPD = XD_SPRD_SPD_128_CLK;
                _r1->reg._r2A.bit.SPRD_EN = XD_FUNCTION_DIS;
                break;
            }
            case XD12R_CLOCK_GATE_EN:
            {
                _r1->reg._r2B.bit.DC_MCLK_EN = XD_FUNCTION_EN;
                _r1->reg._r2B.bit.FR1_MCLK_EN = XD_FUNCTION_EN;
                _r1->reg._r2B.bit.FR2_MCLK_EN = XD_FUNCTION_EN;
                _r1->reg._r2B.bit.OTP_MCLK_EN = XD_FUNCTION_EN;
                break;
            }
            default:
            {
                continue;
            }
        }
        xd12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
}

static void xd12_regs_trim_init_table(void)
{
    _xd12_regs_t* _r1 = &gt_xd12_set_regs;
    //_xd12_mirror_regs_t* _r2 = &gt_xd12_mirror_set_regs;
    _xd12_otp_ctrl_regs_t* _r3 = &gt_xd12_otp_ctrl_set_regs;

    for(xd12_addr_t addr = XD12R_RESET_ID; addr < XD12R_MAX; ++addr)
    {
        switch(addr)
        {
            case XD12R_RESET_ID:
            {
                _r1->reg._r00.bit.lkg_e = XD_FUNCTION_DIS;
                _r1->reg._r00.bit.e_rst = XD_FUNCTION_DIS;
                _r1->reg._r00.bit.vs_rst = XD_FUNCTION_DIS;
                _r1->reg._r00.bit.rst = XD_FUNCTION_DIS;
                break;
            }
            case XD12R_FAULT_CONTROL:
            {
                _r1->reg._r06.bit.o_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.s_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.t_off_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.o_det_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.s_det_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.o_fb_e = XD_FUNCTION_DIS;
                _r1->reg._r06.bit.fb_mode = 0U;
                _r1->reg._r06.bit.auto_fault_fb_no = 0U;
                break;
            }
            case XD12R_MAX_CURR_VREF1:
            {
                _r1->reg._r0A.bit.max_curr_vref = XD_MAX_CURR_VREF_1;
                break;
            }
            case XD12R_MAX_CURR_VREF2:
            {
                _r1->reg._r0B.bit.max_curr_vref = XD_MAX_CURR_VREF_2;
                break;
            }
            case XD12R_MAX_CURR_VREF3:
            {
                _r1->reg._r0C.bit.max_curr_vref = XD_MAX_CURR_VREF_3;
                break;
            }
            case XD12R_MAX_CURR_VREF4:
            {
                _r1->reg._r0D.bit.max_curr_vref = XD_MAX_CURR_VREF_4;
                break;
            }
            case XD12R_MAX_CURR_VREF5:
            {
                _r1->reg._r0E.bit.max_curr_vref = XD_MAX_CURR_VREF_5;
                break;
            }
            case XD12R_FB_LEVEL:
            {
                _r1->reg._r16.bit.fb1_level = XD_FB_LVL_1;
                _r1->reg._r16.bit.fb2_level = XD_FB_LVL_2;
                _r1->reg._r16.bit.fb3_level = XD_FB_LVL_3;
                _r1->reg._r16.bit.fb4_level = XD_FB_LVL_4;
                break;
            }
            case XD12R_FB_SHORT_LEVEL:
            {
                _r1->reg._r17.bit.fb5_level = XD_FB_LVL_5;
                _r1->reg._r17.bit.short1_level = XD_SHORT_LVL_1;
                _r1->reg._r17.bit.short2_level = XD_SHORT_LVL_2;
                _r1->reg._r17.bit.short3_level = XD_SHORT_LVL_3;
                break;
            }
            case XD12R_SHORT_LEVEL:
            {
                _r1->reg._r18.bit.short4_level = XD_SHORT_LVL_4;
                _r1->reg._r18.bit.short5_level = XD_SHORT_LVL_5;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL1:
            {
                _r1->reg._r19.bit.max_curr1_level = XD_MAX_CURR_LVL_1;
                _r1->reg._r19.bit.max_curr2_level = XD_MAX_CURR_LVL_2;
                _r1->reg._r19.bit.max_curr3_level = XD_MAX_CURR_LVL_3;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL2:
            {
                _r1->reg._r1A.bit.max_curr4_level = XD_MAX_CURR_LVL_4;
                _r1->reg._r1A.bit.max_curr5_level = XD_MAX_CURR_LVL_5;
                break;
            }
            case XD12R_SERIAL_CLK_GEN:
            {
                _r1->reg._r1C.bit.serial_clk_high = XD_SERIAL_CLK_HIGH;
                _r1->reg._r1C.bit.serial_clk_low = XD_SERIAL_CLK_LOW;
                break;
            }
            case XD12R_SERIAL_LATENCY:
            {
                _r1->reg._r1D.bit.serial_latency = XD_SERIAL_LATENCY;
                break;
            }
            case XD12R_RSTCNT:
            {
                _r1->reg._r20.bit.rstcnt = XD_RST_COUNT;
                break;
            }
            case XD12R_TIMEOUT:
            {
                _r1->reg._r21.bit.timeout = XD_TIMEOUT;
                break;
            }
            case XD12R_WR_PROTECT:
            {
                _r1->reg._r24.bit.wr_protect = XD_WR_PROTECT_3;
                break;
            }
            case XD12R_CHOP_EN:
            {
                _r1->reg._r26.bit.CHOP_BGR_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DAC_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSC_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSCLDO_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DRV_EN = XD_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_EN = XD_FUNCTION_EN;
                break;
            }
            default:
            {
                continue;
            }
        }
        xd12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }

    for(xd12_otp_ctrl_addr_t addr = XD12R_OTP_ACCESS1; addr < XD12R_OTP_MAX; ++addr)
    {
        switch(addr)
        {
            case XD12R_OTP_PROTECT:
            {
                _r3->reg._r3E.bit.protect_en = 0xA5AU;
                break;
            }
            default:
            {
                continue;
            }
        }
        xd12_write_by_type(addr + XD12R_OTP_CTRL_BASE, _r3->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
}

void xd12_reset(void)
{
    _xd12_regs_t* _r1 = &gt_xd12_set_regs;
    _xd12_otp_ctrl_regs_t* _r3 = &gt_xd12_otp_ctrl_set_regs;

    _r1->reg._r00.bit.lkg_e = 0U;
    _r1->reg._r00.bit.e_rst = 0U;
    _r1->reg._r00.bit.vs_rst = 0U;
    _r1->reg._r00.bit.rst = 1U;
    xd12_write_by_type(XD12R_RESET_ID, _r1->ALL[XD12R_RESET_ID], XD12R_ADDR_TYPE_GENERAL);

    us_delay(10U);
#if 0
    {
        _r1->reg._r2B.ALL = 0x000U;
        xd12_write_by_type(XD12R_CLOCK_GATE_EN, _r1->ALL[XD12R_CLOCK_GATE_EN], XD12R_ADDR_TYPE_GENERAL);

        _r1->reg._r2B.ALL = 0x017U;
        xd12_write_by_type(XD12R_CLOCK_GATE_EN, _r1->ALL[XD12R_CLOCK_GATE_EN], XD12R_ADDR_TYPE_GENERAL);
    }
#endif
    _r3->reg._r3F.ALL = 0x800U;
    xd12_write_by_type(XD12R_OP_MODE + XD12R_OTP_CTRL_BASE, _r3->ALL[XD12R_OP_MODE], XD12R_ADDR_TYPE_GENERAL);

    _r3->reg._r3F.ALL = 0x000U;
    xd12_write_by_type(XD12R_OP_MODE + XD12R_OTP_CTRL_BASE, _r3->ALL[XD12R_OP_MODE], XD12R_ADDR_TYPE_GENERAL);
}

void xd12_idgen(void)
{
    #if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xd12_pwm_out_done();

        for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
        {
            len += xd12_make_pwm_out_stream(gt_xd_idgen_command[daisy].ALL, &p_pwm_out[len], XD_BIT_IDGEN);
        }
        p_pwm_out[len++] = 0U;

        xd12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _v_id_gen_command_t _r04 = { 0, };

        _r04.bit.enable = 1U;

        xc24_write_grp1_reg(XC_ID_GEN_COMMAND, &_r04.ALL, 1U);
    }
    #endif
    us_delay(CMD_DELAY_IDGEN);
}

void xd12_syncgen(void)
{
    #if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xd12_pwm_out_done();

        for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
        {
            len += xd12_make_pwm_out_stream(gt_xd_syncgen_command[daisy].ALL, &p_pwm_out[len], XD_BIT_SYNCGEN);
        }
        p_pwm_out[len++] = 0U;

        xd12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _xc_group1_regs_t* const _r = (_xc_group1_regs_t*)xc24_get_xc24_set_gr1_regs();

        _r->reg._r07.bit.enable = 1U;

        xc24_write_grp1_reg(XC_SYNC_GEN_COMMAND, &_r->ALL[XC_SYNC_GEN_COMMAND], 1U);
    }
    #endif
    us_delay(CMD_DELAY_SYNCGEN);
}

static void xd12_dump_registers(void)
{
#if 0
    comm_UART_Printf(LOG_LV_INFO, "\r\nXD12 General Registers");
    for (xd12_addr_t addr = XD12R_RESET_ID; addr < XD12R_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%03X", addr, gt_xd12_get_regs[0].ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXD12 OTP Control Registers");
    for (xd12_otp_ctrl_addr_t addr = XD12R_OTP_ACCESS1; addr < XD12R_OTP_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%03X", (XD12R_OTP_CTRL_BASE + addr), gt_xd12_otp_ctrl_get_regs[0].ALL[addr]);
    }

    comm_UART_Printf(LOG_LV_INFO, "\r\nXD12 Mirror Registers");
    for (xd12_mirror_addr_t addr = XD12R_MIRROR1; addr < XD12R_MIRROR_MAX; ++addr)
    {
        comm_UART_Printf(LOG_LV_INFO, "\r\n\t\tADDR|0x%02X|DATA|0x%03X", addr, gt_xd12_mirror_get_regs[0].ALL[addr]);
    }
#else
    char line_buf[128] = { 0 };
    int len = 0;

    // 1. XD12 General Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n=== XD12 General Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    for (uint16_t addr = 0; addr < (uint16_t)XD12R_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, "  %03X ", gt_xd12_get_regs[0].ALL[addr]);
    }

    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 2. XD12 OTP Control Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XD12 OTP Control Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t offset = 0; offset < (uint16_t)XD12R_OTP_MAX; ++offset)
    {
        uint16_t real_addr = (uint16_t)XD12R_OTP_CTRL_BASE + offset;
        if ((offset % 16) == 0)
        {
            if (offset > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", real_addr & 0xF0U);
        }

        if (offset == 0 && (real_addr % 16) != 0)
        {
            for (uint8_t pad = 0; pad < (real_addr % 16); ++pad)
            {
                len += snprintf(&line_buf[len], sizeof(line_buf) - len, " ---- ");
            }
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, "  %03X ", gt_xd12_otp_ctrl_get_regs[0].ALL[offset]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    // 3. XD12 Mirror Registers
    comm_UART_Printf(LOG_LV_INFO, "\r\n\r\n=== XD12 Mirror Registers ===");
    comm_UART_Printf(LOG_LV_INFO, "\r\nADDR |  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F");
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    len = 0;
    for (uint16_t addr = 0; addr < (uint16_t)XD12R_MIRROR_MAX; ++addr)
    {
        if ((addr % 16) == 0)
        {
            if (addr > 0)
            {
                comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
            }
            len = snprintf(line_buf, sizeof(line_buf), "\r\n0x%02X |", addr);
        }
        len += snprintf(&line_buf[len], sizeof(line_buf) - len, "  %03X ", gt_xd12_mirror_get_regs[0].ALL[addr]);
    }
    if (len > 0)
    {
        comm_UART_Printf(LOG_LV_INFO, "%s", line_buf);
    }
    comm_UART_Printf(LOG_LV_INFO, "\r\n-----+--------------------------------------------------------------------------------------------------");

    comm_UART_Printf(LOG_LV_INFO, "\r\n");
#endif
}

static void xd12_memory_copy(void)
{
    gt_xd12_set_regs = gt_xd12_get_regs[0];
    gt_xd12_otp_ctrl_set_regs = gt_xd12_otp_ctrl_get_regs[0];
    gt_xd12_mirror_set_regs = gt_xd12_mirror_get_regs[0];
}

void xd12_read_all(void)
{
    for (xd12_addr_t addr = XD12R_RESET_ID; addr < XD12R_MAX; ++addr)
    {
        xd12_read_by_type(addr, XD12R_ADDR_TYPE_GENERAL);
    }

    for (xd12_mirror_addr_t addr = XD12R_MIRROR1; addr < XD12R_MIRROR_MAX; ++addr)
    {
        xd12_read_by_type(addr, XD12R_ADDR_TYPE_MIRROR);
    }

    for (xd12_otp_ctrl_addr_t addr = XD12R_OTP_ACCESS1; addr < XD12R_OTP_MAX; ++addr)
    {
        xd12_read_by_type(XD12R_OTP_CTRL_BASE + addr, XD12R_ADDR_TYPE_GENERAL);
    }
    xd12_dump_registers();
    xd12_memory_copy();
}

void xd12_init_param(void)
{
    #if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    {
        for(uint16_t num = 0U; num < XD_DAISY_LENGTH; ++num)
        {
            gt_xd_write_command[num].bit.cmd_code = CMD_CODE_WRITE;
            gt_xd_read_command[num].bit.cmd_code = CMD_CODE_READ;
            gt_xd_fault_read_command[num].bit.cmd_code = CMD_CODE_RD_FAULT;
            gt_xd_syncgen_command[num].bit.cmd_code = CMD_CODE_SYNCGEN;
            gt_xd_idgen_command[num].bit.cmd_code = CMD_CODE_IDGEN;
        }
    }
    #endif

    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_pwm_in_xd_response_duty);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gn_pwm_in_xd_response_freq);
}

void xd12_init(void)
{
#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
#elif (XD_CONTROL_TYPE == XD_CONTROLLED_XC24)
    gpio_set_xc_vdd_5v(VCC_ON_3V3);
    LL_mDelay(99U);
    xc24_init();
    gpio_set_vled_dcdc(VLED_ON);
#else
    #error "XD_CONTROL_TYPE is not defined"
#endif

    xd12_reset();
    xd12_idgen();
    xd12_make_readable();
    xd12_regs_init_table();
    xd12_read_all();
}

void xd12_trim_init(void)
{
#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
#elif (XD_CONTROL_TYPE == XD_CONTROLLED_XC24)
    gpio_set_xc_vdd_5v(VCC_ON_3V3);
    LL_mDelay(99U);
    xc24_init();
    gpio_set_vled_dcdc(VLED_ON);
#else
    #error "XD_CONTROL_TYPE is not defined"
#endif

    xd12_reset();
    xd12_idgen();
    xd12_make_readable();
    xd12_regs_trim_init_table();
    xd12_read_all();
}

void xd12_init_for_read(void)
{
#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
#elif (XD_CONTROL_TYPE == XD_CONTROLLED_XC24)
    gpio_set_xc_vdd_5v(VCC_ON_3V3);
    LL_mDelay(99U);
    xc24_init();
    gpio_set_vled_dcdc(VLED_ON);
#else
    #error "XD_CONTROL_TYPE is not defined"
#endif

    xd12_reset();
    xd12_idgen();
    xd12_make_readable();
}

static void xd12_write(uint16_t addr, uint16_t data)
{
#if (SERIALIZE_LOG_DUMP == SERIALIZE_LOG_DUMP_ENABLE)
    xd12_serialize_log_dump(addr, data);
#endif
    #if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xd12_pwm_out_done();

        for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
        {
            gt_xd_write_command[daisy].bit.addr = addr;
            gt_xd_write_command[daisy].bit.data = data;
            len += xd12_make_pwm_out_stream(gt_xd_write_command[daisy].ALL, &p_pwm_out[len], XD_CMD_WRITE);
        }
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nXD Write Packet\r\n\tADDR - 0x%02X, DATA - 0x%03X", gt_xd_write_command[0].bit.addr, gt_xd_write_command[0].bit.data);
        p_pwm_out[len++] = 0U;

        xd12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _v_global_write_command_t _r01 = { 0, };

        _r01.bit.enable = 1U;
        _r01.bit.addr = addr;

        xc24_write_grp1_reg(XC_GLOBAL_WRITE_DATA, &data, 1U);
        xc24_write_grp1_reg(XC_GLOBAL_WRITE_COMMAND, &_r01.ALL, 1U);
    }
    #endif
    us_delay(CMD_DELAY_REG_WR);
}

static uint16_t xd12_read(uint16_t addr, uint16_t* p_xd_buffer)
{
    #if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;
        const uint16_t pwm_in_length = XD_DAISY_LENGTH * (XD_HDR_BIT + XD_ID_BIT + XD_DATA_BIT);

        for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
        {
            gt_xd_read_command[daisy].bit.addr = addr;
            len += xd12_make_pwm_out_stream(gt_xd_read_command[daisy].ALL, &p_pwm_out[len], XD_CMD_READ);
        }
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nXD Read Packet\r\n\tIN_LENGTH [%u] CMD[0x%01X] ADDR[0x%02X]", pwm_in_length, gt_xd_read_command[0].bit.cmd_code, gt_xd_read_command[0].bit.addr);
        p_pwm_out[len++] = 0U;

        memset(gn_pwm_in_xd_response_freq, 0U, sizeof(gn_pwm_in_xd_response_freq));
        memset(gn_pwm_in_xd_response_duty, 0U, sizeof(gn_pwm_in_xd_response_duty));

        xd12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
        xd12_pwm_out_done();

        if(false == xd12_pwm_in(pwm_in_length, 150U))
        {
            uint8_t max_id = xd12_decode_pwm_input_stream(gn_pwm_in_xd_response_freq, gn_pwm_in_xd_response_duty, p_xd_buffer, pwm_in_length);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nFunction[%s] timeout!!", __func__);
        }
    }
    #else // to do
    {
        uint16_t buffer[XD_DAISY_LENGTH] = { 0, };

        const uint16_t daisy_size = XD_DAISY_LENGTH;
        const uint16_t ch_seg_max = MODEL_XC24_SEG;

        for(uint16_t ch_seg = 0U; ch_seg < ch_seg_max; ++ch_seg)
        {
            uint16_t offset = 0U;
            uint16_t remaining = daisy_size;

            xc24_read_local(ch_seg, addr);

            /* clear read buffer */
            memset(buffer, 0U, sizeof(buffer));

            while(remaining > 0U)
            {
                uint16_t chunk = (remaining > XC_SPI_RW_LEN) ? XC_SPI_RW_LEN : remaining;

                xc24_get_local_rw_data(XC_PORT1_LOCAL_RW_DATA1 + offset, &buffer[offset], chunk);
                offset += chunk;
                remaining -= chunk;
            }

            /* iteration - IC num */
            for(uint16_t xd_daisy = 0U; xd_daisy < daisy_size; ++xd_daisy)
            {
                p_xd_buffer[xd_daisy] = buffer[xd_daisy];
            }
        }
    }
    #endif
    us_delay(CMD_DELAY_REG_RD);

    return 0U;
}

static void xd12_change_addr_type(xd12r_addr_type_t addr_type)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    xd12r_addr_type_t current_type = (xd12r_addr_type_t)(_r3F->bit.addr_ext);

    if (current_type != addr_type)
    {
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange XD ADDR_TYPE TO %s", (addr_type == XD12R_ADDR_TYPE_GENERAL)?("GENERAL"):("MIRROR"));
        _r3F->bit.addr_ext = addr_type;
        xd12_write(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, gt_xd12_otp_ctrl_set_regs.reg._r3F.ALL);
    }
}

void xd12_write_by_type(uint16_t addr, uint16_t data, xd12r_addr_type_t addr_type)
{
    if (addr_type == XD12R_ADDR_TYPE_GENERAL)
    {
        if (addr < XD12R_OTP_CTRL_BASE) // general register
        {
            if (addr >= XD12R_MAX)
            {
                FATAL_INVALID_INPUT(addr);
                return;
            }
        }
        else if (addr < (XD12R_OTP_CTRL_BASE + XD12R_OTP_MAX)) // otp control register
        {
            if (addr >= (XD12R_OTP_CTRL_BASE + XD12R_OTP_MAX))
            {
                FATAL_INVALID_INPUT(addr);
                return;
            }
        }
    }
    else // mirror register
    {
        if (addr >= XD12R_MIRROR_MAX)
        {
            FATAL_INVALID_INPUT(addr);
            return;
        }
    }

    xd12_change_addr_type(addr_type);
    xd12_write(addr, data);

    for (uint8_t id = 0U; id < XD_DAISY_LENGTH; ++id)
    {
        if (addr_type == XD12R_ADDR_TYPE_GENERAL)
        {
            if (addr < XD12R_OTP_CTRL_BASE)
            {
                gt_xd12_set_regs.ALL[addr] = data;
            }
            else
            {
                gt_xd12_otp_ctrl_set_regs.ALL[addr - XD12R_OTP_CTRL_BASE] = data;
            }
        }
        else
        {
            gt_xd12_mirror_set_regs.ALL[addr] = data;
        }
    }
}

uint16_t xd12_read_by_type(uint16_t addr, xd12r_addr_type_t addr_type)
{
    uint16_t xd_buffer[XD_DAISY_LENGTH] = { 0U };

    if (addr_type == XD12R_ADDR_TYPE_GENERAL)
    {
        if (addr < XD12R_OTP_CTRL_BASE) // general register
        {
            if (addr >= XD12R_MAX)
            {
                FATAL_INVALID_INPUT(addr);
                return 0U;
            }
        }
        else if (addr < (XD12R_OTP_CTRL_BASE + XD12R_OTP_MAX)) // otp control register
        {
            if (addr >= (XD12R_OTP_CTRL_BASE + XD12R_OTP_MAX))
            {
                FATAL_INVALID_INPUT(addr);
                return 0U;
            }
        }
    }
    else // mirror register
    {
        if (addr >= XD12R_MIRROR_MAX)
        {
            FATAL_INVALID_INPUT(addr);
            return 0U;
        }
    }

    xd12_change_addr_type(addr_type);
    xd12_read(addr, xd_buffer);

    for (uint8_t id = 0U; id < XD_DAISY_LENGTH; ++id)
    {
        if (addr_type == XD12R_ADDR_TYPE_GENERAL)
        {
            if (addr < XD12R_OTP_CTRL_BASE)
            {
                gt_xd12_get_regs[id].ALL[addr] = xd_buffer[id];
            }
            else
            {
                gt_xd12_otp_ctrl_get_regs[id].ALL[addr - XD12R_OTP_CTRL_BASE] = xd_buffer[id];
            }
        }
        else
        {
            gt_xd12_mirror_get_regs[id].ALL[addr] = xd_buffer[id];
        }
    }
    return xd_buffer[0];
}

void xd12_ld_transfer(void)
{
#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    uint16_t* p_ld_buffer = ldim_get_xd_ld_transfer_buffer();
    uint16_t* const p_pwm_out = gn_pwm_out_xd_ld_transfer + PWM_OUT_HEADER_SIZE;
    uint16_t len = 0;

    for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
    {
        /* header : 1111 */
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;

        for(uint8_t ld_idx = 0U; ld_idx < XD_LD_SIZE; ++ld_idx)
        {
            len += xd12_make_pwm_out_stream(*p_ld_buffer++, &p_pwm_out[len], XD_LD_DATA_BIT);
        }
    }
    p_pwm_out[len++] = 0U;

    xd12_pwm_out((uint32_t)p_pwm_out, (uint32_t)len);
    us_delay(CMD_DELAY_LD);
#elif (XD_CONTROL_TYPE == XD_CONTROLLED_XC24)
    uint16_t* p = ldim_get_xc_ld_transfer_buffer();
    uint16_t len = ldim_get_xc_ld_transfer_size();
    xc24_set_ld_transfer(p, len);
    while (true == gb_xc_ld_transfer_spi_dma_flag)
    {

    }
#else
    #error "XD_CONTROL_TYPE is not defined"
#endif
}

void xd12_fault_readout(void)
{
    static uint16_t prev_fault_data = 0x0FU;
    uint16_t fault_data = 0x0FU;
#if (XD_CONTROL_TYPE == XD_CONTROLLED_MCU)
    uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
    uint16_t len = 0U;
    const uint16_t pwm_in_length = XD_DAISY_LENGTH * (XD_HDR_BIT);

    for(uint16_t daisy = 0U; daisy < XD_DAISY_LENGTH; ++daisy)
    {
        len += xd12_make_pwm_out_stream(gt_xd_fault_read_command[daisy].ALL, &p_pwm_out[len], XD_CMD_FAULT);
    }
    comm_UART_Printf(LOG_LV_DEBUG, "\r\nXD Fault Packet\r\n\tIN_LENGTH [%u] CMD[0x%01X]", pwm_in_length, gt_xd_fault_read_command[0].bit.cmd_code);
    p_pwm_out[len++] = 0U;

    memset(gn_pwm_in_xd_response_freq, 0U, sizeof(gn_pwm_in_xd_response_freq));
    memset(gn_pwm_in_xd_response_duty, 0U, sizeof(gn_pwm_in_xd_response_duty));

    xd12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    xd12_pwm_out_done();

    if(false == xd12_pwm_in(pwm_in_length, 150U))
    {
        uint8_t max_id = xd12_decode_pwm_input_stream(gn_pwm_in_xd_response_freq, gn_pwm_in_xd_response_duty, &fault_data, pwm_in_length);
    }
    else
    {
        comm_UART_Printf(LOG_LV_ERROR, "\r\nFunction[%s] timeout!!", __func__);
    }
#elif (XD_CONTROL_TYPE == XD_CONTROLLED_XC24)
    fault_data = (uint8_t)(xc24_read_grp1_reg(XC_GLOBAL_FAULT_READ_DATA1, 1U) & 0x000FU);
#else
    #error "XD_CONTROL_TYPE is not defined"
#endif
    if (prev_fault_data != fault_data)
    {
        const uint8_t fault_fault = ((fault_data & 0x08U) >> 3U);
        const uint8_t fault_fb3 = ((fault_data & 0x04U) >> 2U);
        const uint8_t fault_fb2 = ((fault_data & 0x02U) >> 1U);
        const uint8_t fault_fb1 = ((fault_data & 0x01U) >> 0U);
        comm_UART_Printf(LOG_LV_INFO, "\r\n Fault Detect> [FAULT: %u, FB3: %u, FB2: %u, FB1: %u]\n\rJIG> \0", fault_fault, fault_fb3, fault_fb2, fault_fb1);
        prev_fault_data = fault_data;
    }
    us_delay(CMD_DELAY_FAULT_READ);
}

void xd12_set_max_curr_vref(xd12r_setting_grp_t set_grp, uint16_t vref)
{
    if ((set_grp >= XD12R_SETTING_GRP_MAX) || (vref > 4095U))
    {
        FATAL_INVALID_INPUT(set_grp);
        FATAL_INVALID_INPUT(vref);
        return;
    }

    const uint16_t vref_addr[5] = { XD12R_MAX_CURR_VREF1, XD12R_MAX_CURR_VREF2, XD12R_MAX_CURR_VREF3, XD12R_MAX_CURR_VREF4, XD12R_MAX_CURR_VREF5 };
    _v_xd12_max_curr_vref_t* p_vref = &gt_xd12_set_regs.reg._r0A;

    if (set_grp == XD12R_SETTING_GRP_ALL)
    {
        for (uint8_t i = 0U; i < 5U; ++i)
        {
            (p_vref + i)->bit.max_curr_vref = vref;
            xd12_write_by_type(vref_addr[i], (p_vref + i)->ALL, XD12R_ADDR_TYPE_GENERAL);
        }
    }
    else if ((set_grp >= XD12R_SETTING_GRP_1) && (set_grp <= XD12R_SETTING_GRP_5))
    {
        uint8_t idx = (uint8_t)set_grp - (uint8_t)XD12R_SETTING_GRP_1;
        (p_vref + idx)->bit.max_curr_vref = vref;
        xd12_write_by_type(vref_addr[idx], (p_vref + idx)->ALL, XD12R_ADDR_TYPE_GENERAL);
    }
    else
    {
        FATAL_INVALID_INPUT(set_grp);
    }
}

void xd12_set_max_curr_lvl(xd12r_setting_grp_t set_grp, max_curr_level_t curr_lvl)
{
    if ((set_grp >= XD12R_SETTING_GRP_MAX) || (curr_lvl > CURR_LEVEL_MAX))
    {
        FATAL_INVALID_INPUT(set_grp);
        FATAL_INVALID_INPUT(curr_lvl);
        return;
    }
    uint16_t lvl_val = (uint16_t)curr_lvl & 0x0FU;

    if (set_grp == XD12R_SETTING_GRP_ALL)
    {
        gt_xd12_set_regs.reg._r19.bit.max_curr1_level = lvl_val;
        gt_xd12_set_regs.reg._r19.bit.max_curr2_level = lvl_val;
        gt_xd12_set_regs.reg._r19.bit.max_curr3_level = lvl_val;
        xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, gt_xd12_set_regs.reg._r19.ALL, XD12R_ADDR_TYPE_GENERAL);

        gt_xd12_set_regs.reg._r1A.bit.max_curr4_level = lvl_val;
        gt_xd12_set_regs.reg._r1A.bit.max_curr5_level = lvl_val;
        xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, gt_xd12_set_regs.reg._r1A.ALL, XD12R_ADDR_TYPE_GENERAL);
    }
    else
    {
        switch (set_grp)
        {
            case XD12R_SETTING_GRP_1:
            {
                gt_xd12_set_regs.reg._r19.bit.max_curr1_level = lvl_val;
                xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, gt_xd12_set_regs.reg._r19.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_2:
            {
                gt_xd12_set_regs.reg._r19.bit.max_curr2_level = lvl_val;
                xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, gt_xd12_set_regs.reg._r19.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_3:
            {
                gt_xd12_set_regs.reg._r19.bit.max_curr3_level = lvl_val;
                xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, gt_xd12_set_regs.reg._r19.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_4:
            {
                gt_xd12_set_regs.reg._r1A.bit.max_curr4_level = lvl_val;
                xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, gt_xd12_set_regs.reg._r1A.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_5:
            {
                gt_xd12_set_regs.reg._r1A.bit.max_curr5_level = lvl_val;
                xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, gt_xd12_set_regs.reg._r1A.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(set_grp);
                break;
            }
        }
    }
}

void xd12_set_fb_lvl(xd12r_setting_grp_t set_grp, fb_level_t fb_lvl)
{
    if ((set_grp >= XD12R_SETTING_GRP_MAX) || (fb_lvl > FB_LEVEL_MAX))
    {
        FATAL_INVALID_INPUT(set_grp);
        FATAL_INVALID_INPUT(fb_lvl);
        return;
    }
    uint16_t lvl_val = (uint16_t)fb_lvl & 0x07U;

    if (set_grp == XD12R_SETTING_GRP_ALL)
    {
        gt_xd12_set_regs.reg._r16.bit.fb1_level = lvl_val;
        gt_xd12_set_regs.reg._r16.bit.fb2_level = lvl_val;
        gt_xd12_set_regs.reg._r16.bit.fb3_level = lvl_val;
        gt_xd12_set_regs.reg._r16.bit.fb4_level = lvl_val;
        xd12_write_by_type(XD12R_FB_LEVEL, gt_xd12_set_regs.reg._r16.ALL, XD12R_ADDR_TYPE_GENERAL);

        gt_xd12_set_regs.reg._r17.bit.fb5_level = lvl_val;
        xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);
    }
    else
    {
        switch (set_grp)
        {
            case XD12R_SETTING_GRP_1:
            {
                gt_xd12_set_regs.reg._r16.bit.fb1_level = lvl_val;
                xd12_write_by_type(XD12R_FB_LEVEL, gt_xd12_set_regs.reg._r16.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_2:
            {
                gt_xd12_set_regs.reg._r16.bit.fb2_level = lvl_val;
                xd12_write_by_type(XD12R_FB_LEVEL, gt_xd12_set_regs.reg._r16.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_3:
            {
                gt_xd12_set_regs.reg._r16.bit.fb3_level = lvl_val;
                xd12_write_by_type(XD12R_FB_LEVEL, gt_xd12_set_regs.reg._r16.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_4:
            {
                gt_xd12_set_regs.reg._r16.bit.fb4_level = lvl_val;
                xd12_write_by_type(XD12R_FB_LEVEL, gt_xd12_set_regs.reg._r16.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_5:
            {
                gt_xd12_set_regs.reg._r17.bit.fb5_level = lvl_val;
                xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(set_grp);
                break;
            }
        }
    }
}

void xd12_set_short_lvl(xd12r_setting_grp_t set_grp, short_level_t short_lvl)
{
    if ((set_grp >= XD12R_SETTING_GRP_MAX) || (short_lvl > SHORT_LEVEL_MAX))
    {
        FATAL_INVALID_INPUT(set_grp);
        FATAL_INVALID_INPUT(short_lvl);
        return;
    }
    uint16_t lvl_val = (uint16_t)short_lvl & 0x07U;

    if (set_grp == XD12R_SETTING_GRP_ALL)
    {
        gt_xd12_set_regs.reg._r17.bit.short1_level = lvl_val;
        gt_xd12_set_regs.reg._r17.bit.short2_level = lvl_val;
        gt_xd12_set_regs.reg._r17.bit.short3_level = lvl_val;
        xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);

        gt_xd12_set_regs.reg._r18.bit.short4_level = lvl_val;
        gt_xd12_set_regs.reg._r18.bit.short5_level = lvl_val;
        xd12_write_by_type(XD12R_SHORT_LEVEL, gt_xd12_set_regs.reg._r18.ALL, XD12R_ADDR_TYPE_GENERAL);
    }
    else
    {
        switch (set_grp)
        {
            case XD12R_SETTING_GRP_1:
            {
                gt_xd12_set_regs.reg._r17.bit.short1_level = lvl_val;
                xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_2:
            {
                gt_xd12_set_regs.reg._r17.bit.short2_level = lvl_val;
                xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_3:
            {
                gt_xd12_set_regs.reg._r17.bit.short3_level = lvl_val;
                xd12_write_by_type(XD12R_FB_SHORT_LEVEL, gt_xd12_set_regs.reg._r17.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_4:
            {
                gt_xd12_set_regs.reg._r18.bit.short4_level = lvl_val;
                xd12_write_by_type(XD12R_SHORT_LEVEL, gt_xd12_set_regs.reg._r18.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            case XD12R_SETTING_GRP_5:
            {
                gt_xd12_set_regs.reg._r18.bit.short5_level = lvl_val;
                xd12_write_by_type(XD12R_SHORT_LEVEL, gt_xd12_set_regs.reg._r18.ALL, XD12R_ADDR_TYPE_GENERAL);
                break;
            }
            default:
            {
                FATAL_INVALID_INPUT(set_grp);
                break;
            }
        }
    }
}

void xd12_trim_set_efuse_enable(bool en)
{
    gb_xd_do_efuse = en;
}

bool xd12_trim_get_efuse_enable(void)
{
    return gb_xd_do_efuse;
}

void xd12_trim_init_current_ref(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_ldo_dig(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 2U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_ldo_dac(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_ldo_fll(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_osc(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    const uint16_t osc_fll_man = 0x8000U; /* 16b' 1000_0000_0000_0000*/

    _v_xd12_osc_fll_man1_t* _r28 = &gt_xd12_set_regs.reg._r28;
    _r28->bit.osc_fll_man = ((osc_fll_man & 0x0FFFU) >>  0U);
    xd12_write_by_type(XD12R_OSC_FLL_MAN1, _r28->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_fll_man = ((osc_fll_man & 0xF000U) >> 12U);
    _r29->bit.osc_man_en = 1U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_ch_gain(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level1_t* _r19 = &gt_xd12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_24;
    _r19->bit.max_curr2_level = CURR_LEVEL_24;
    _r19->bit.max_curr3_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level2_t* _r1A = &gt_xd12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_24;
    _r1A->bit.max_curr5_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_init_ch_ofs(void)
{
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level1_t* _r19 = &gt_xd12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_24;
    _r19->bit.max_curr2_level = CURR_LEVEL_24;
    _r19->bit.max_curr3_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level2_t* _r1A = &gt_xd12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_24;
    _r1A->bit.max_curr5_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_set_channel_enable(uint8_t chx)
{
    _v_xd12_channel_enable_t* _r05 = &gt_xd12_set_regs.reg._r05;
    _r05->ALL = (uint16_t)(1U << chx);
    xd12_write_by_type(XD12R_CHANNEL_ENABLE, _r05->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_set_max_curr_vref(uint16_t vref)
{
    for (uint8_t i = 0U; i < 5; ++i)
    {
        uint16_t* p_vref = &gt_xd12_set_regs.ALL[XD12R_MAX_CURR_VREF1 + i];
        *p_vref = vref;
        xd12_write_by_type(XD12R_MAX_CURR_VREF1 + i, *p_vref, XD12R_ADDR_TYPE_GENERAL);
    }
}

void xd12_trim_set_max_curr_lvl(uint16_t curr_lvl)
{
    _v_xd12_max_current_level1_t* _r19 = &gt_xd12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = curr_lvl;
    _r19->bit.max_curr2_level = curr_lvl;
    _r19->bit.max_curr3_level = curr_lvl;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level2_t* _r1A = &gt_xd12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = curr_lvl;
    _r1A->bit.max_curr5_level = curr_lvl;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

bool xd12_trim_set_current_ref(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XD_MAX_CURRENT_REF)
    {
        _v_xd12_mirror2_t* _r01 = &gt_xd12_mirror_set_regs.reg._r01;
        _r01->bit.iref_ctl = reg_val;
        xd12_write_by_type(XD12R_MIRROR2, _r01->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_ldo_dig(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XD_MAX_LDO_DIG)
    {
        _v_xd12_mirror4_t* _r03 = &gt_xd12_mirror_set_regs.reg._r03;
        _r03->bit.ldo_ctl = reg_val;
        xd12_write_by_type(XD12R_MIRROR4, _r03->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_ldo_dac(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XD_MAX_LDO_DAC)
    {
        _v_xd12_mirror3_t* _r02 = &gt_xd12_mirror_set_regs.reg._r02;
        _r02->bit.ldo_dac_ctl = reg_val;
        xd12_write_by_type(XD12R_MIRROR3, _r02->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_ldo_fll(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XD_MAX_LDO_FLL)
    {
        _v_xd12_mirror4_t* _r03 = &gt_xd12_mirror_set_regs.reg._r03;
        _r03->bit.ldo_osc_ctl = reg_val;
        xd12_write_by_type(XD12R_MIRROR4, _r03->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_osc(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XD_MAX_OSC)
    {
        _v_xd12_mirror3_t* _r02 = &gt_xd12_mirror_set_regs.reg._r02;
        _r02->bit.osc_rctl = reg_val;
        xd12_write_by_type(XD12R_MIRROR3, _r02->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_ch_gain(uint16_t reg_val, uint8_t chx)
{
    bool ret = false;
    uint16_t* p_base_address = &gt_xd12_mirror_set_regs.ALL[XD12R_MIRROR_GAIN_CH01];
    uint16_t addr_offset = (uint16_t)(chx);
    if (reg_val <= XD_MAX_CH_GAIN)
    {
        *(p_base_address + addr_offset) = reg_val;
        xd12_write_by_type(XD12R_MIRROR_GAIN_CH01 + addr_offset, *(p_base_address + addr_offset), XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xd12_trim_set_ch_ofs(uint16_t reg_val, uint8_t chx)
{
    bool ret = false;
    uint16_t* p_base_address = &gt_xd12_mirror_set_regs.ALL[XD12R_MIRROR_OFS_CH01];
    uint16_t addr_offset = (uint16_t)(chx);
    if (reg_val <= XD_MAX_CH_OFS)
    {
        *(p_base_address + addr_offset) = reg_val;
        xd12_write_by_type(XD12R_MIRROR_OFS_CH01 + addr_offset, *(p_base_address + addr_offset), XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

void xd12_trim_init_efuse(void)
{
    _v_xd12_otp_access1_t* _r3A = &gt_xd12_otp_ctrl_set_regs.reg._r3A;
    _r3A->bit.otp_pg_acc_cycle = 0x000U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_ACCESS1, _r3A->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_otp_access2_t* _r3B = &gt_xd12_otp_ctrl_set_regs.reg._r3B;
    _r3B->bit.otp_pg_acc_cycle = 0x3FFU;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_ACCESS2, _r3B->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_otp_write_t* _r3C = &gt_xd12_otp_ctrl_set_regs.reg._r3C;
    _r3C->bit.otp_wsel = 4U;
    _r3C->bit.otp_rd = 0U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_WRITE, _r3C->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_start_efuse(void)
{
    _v_xd12_otp_rd_prog_t* _r3D = &gt_xd12_otp_ctrl_set_regs.reg._r3D;
    _r3D->bit.otp_pg_s = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_RD_PROG, _r3D->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_trim_save_mirror_register(void)
{
    for (xd12_mirror_addr_t mirror_addr = XD12R_MIRROR1; mirror_addr < XD12R_MIRROR_MAX; ++mirror_addr)
    {
        xd12_read_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
    }
}

uint32_t xd12_trim_verify_mirror_dump(void)
{
    uint32_t ret = 0U;
    for (xd12_mirror_addr_t mirror_addr = XD12R_MIRROR1; mirror_addr < XD12R_MIRROR_MAX; ++mirror_addr)
    {
        uint16_t saved_reg = gt_xd12_mirror_set_regs.ALL[mirror_addr];
        uint16_t read_reg = xd12_read_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
        if (saved_reg != read_reg)
        {
            ret |= (1UL << mirror_addr);
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✕]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_RED, ANSI_FONT_NONE, mirror_addr, saved_reg, read_reg);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✔]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_GREEN, ANSI_FONT_NONE, mirror_addr, saved_reg, read_reg);
        }
    }
    return ret;
}

void xd12_test_init_icc_stby(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
}

void xd12_test_init_icc_actv(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
    /* set proper xd12 register */
    xd12_trim_init();
}

void xd12_test_init_current_ref(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_ldo_dig(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 2U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_ldo_dac(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_ldo_fll(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_osc(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    const uint16_t osc_fll_man = 0x8000U; /* 16b' 1000_0000_0000_0000*/

    _v_xd12_osc_fll_man1_t* _r28 = &gt_xd12_set_regs.reg._r28;
    _r28->bit.osc_fll_man = ((osc_fll_man & 0x0FFFU) >>  0U);
    xd12_write_by_type(XD12R_OSC_FLL_MAN1, _r28->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_fll_man = ((osc_fll_man & 0xF000U) >> 12U);
    _r29->bit.osc_man_en = 1U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_fll_40M(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / (vsync * 2.0f) + 0.5f); /* round up */

    _v_xd12_fllcnt1_t* _r22 = &gt_xd12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xd12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_fllcnt2_t* _r23 = &gt_xd12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    _r23->bit.fll_range = 3U;
    _r23->bit.fll_en = 1U;
    xd12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_fll_50M(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 50000000.0f; /* 50MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / (vsync * 2.0f) + 0.5f); /* round up */

    _v_xd12_fllcnt1_t* _r22 = &gt_xd12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xd12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_fllcnt2_t* _r23 = &gt_xd12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    _r23->bit.fll_range = 3U;
    _r23->bit.fll_en = 1U;
    xd12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_fll_60M(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 60000000.0f; /* 60MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / (vsync * 2.0f) + 0.5f); /* round up */

    _v_xd12_fllcnt1_t* _r22 = &gt_xd12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xd12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_fllcnt2_t* _r23 = &gt_xd12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    _r23->bit.fll_range = 3U;
    _r23->bit.fll_en = 1U;
    xd12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_iout_3P(void)
{
    // turn on proper power if needed like VLED
    gpio_set_vled_9v(VLED_ON);
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);

    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level1_t* _r19 = &gt_xd12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_24;
    _r19->bit.max_curr2_level = CURR_LEVEL_24;
    _r19->bit.max_curr3_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level2_t* _r1A = &gt_xd12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_24;
    _r1A->bit.max_curr5_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_test_init_max_sweep(void)
{
    // turn on proper power if needed like VLED
    gpio_set_vled_9v(VLED_ON);
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);

    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    xd12_trim_set_max_curr_vref(300U);
}

void xd12_test_start_icc_stby(void)
{
    ADS114S08_Set_Start(true);
}
void xd12_test_start_icc_actv(void)
{
    ADS114S08_Set_Start(true);
}
void xd12_test_start_current_ref(void)
{
    mcu_peripheral_adc_start();
}
void xd12_test_start_ldo_dig(void)
{
    mcu_peripheral_adc_start();
}
void xd12_test_start_ldo_dac(void)
{
    mcu_peripheral_adc_start();
}
void xd12_test_start_ldo_fll(void)
{
    mcu_peripheral_adc_start();
}
void xd12_test_start_osc(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xd12_test_start_fll_40M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xd12_test_start_fll_50M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xd12_test_start_fll_60M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xd12_test_start_iout_3P(void)
{
    ADS114S08_Set_Start(true);
}
void xd12_test_start_max_sweep(void)
{
    ADS114S08_Set_Start(true);
}

void xd12_aging_init_icc_test(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
    /* set proper xd12 register */
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_current_ref(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_ldo_dig(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 2U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_ldo_dac(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_ldo_fll(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_osc(void)
{
    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.mclk64_o = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    const uint16_t osc_fll_man = 0x8000U; /* 16b' 1000_0000_0000_0000*/

    _v_xd12_osc_fll_man1_t* _r28 = &gt_xd12_set_regs.reg._r28;
    _r28->bit.osc_fll_man = ((osc_fll_man & 0x0FFFU) >>  0U);
    xd12_write_by_type(XD12R_OSC_FLL_MAN1, _r28->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_osc_fll_man2_t* _r29 = &gt_xd12_set_regs.reg._r29;
    _r29->bit.osc_fll_man = ((osc_fll_man & 0xF000U) >> 12U);
    _r29->bit.osc_man_en = 1U;
    xd12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_init_iout(void)
{
    // turn on proper power if needed like VLED
    gpio_set_vled_9v(VLED_ON);
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);

    // set proper xd12 register
    _v_xd12_op_mode_t* _r3F = &gt_xd12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xd12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level1_t* _r19 = &gt_xd12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_24;
    _r19->bit.max_curr2_level = CURR_LEVEL_24;
    _r19->bit.max_curr3_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xd12_max_current_level2_t* _r1A = &gt_xd12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_24;
    _r1A->bit.max_curr5_level = CURR_LEVEL_24;
    xd12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xd12_aging_start_icc_test(void)
{
    ADS114S08_Set_Start(true);
}
void xd12_aging_start_current_ref(void)
{
    mcu_peripheral_adc_start();
}
void xd12_aging_start_ldo_dig(void)
{
    mcu_peripheral_adc_start();
}
void xd12_aging_start_ldo_dac(void)
{
    mcu_peripheral_adc_start();
}
void xd12_aging_start_ldo_fll(void)
{
    mcu_peripheral_adc_start();
}
void xd12_aging_start_osc(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xd12_aging_start_iout(void)
{
    ADS114S08_Set_Start(true);
}

void xd12_test(void)
{
}