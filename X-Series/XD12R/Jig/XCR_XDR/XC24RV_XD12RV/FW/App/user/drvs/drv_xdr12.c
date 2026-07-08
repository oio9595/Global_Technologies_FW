#include <string.h>

#include "main.h"
#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "comm_debugging.h"
#include "drv_gpio.h"
#include "ads124s08.h"
#include "ldim_conversion.h"

/* XDR/IC602 serializer protocol */
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

#define XDR_MAX_CURRENT_REF     (0x01FU)
#define XDR_MAX_LDO_DIG         (0x00FU)
#define XDR_MAX_LDO_DAC         (0x01FU)
#define XDR_MAX_LDO_FLL         (0x00FU)
#define XDR_MAX_OSC             (0x01FU)
#define XDR_MAX_CH_GAIN         (0x07FU)
#define XDR_MAX_CH_OFS          (0x1FFU)

#define XDR_BIT_IDGEN           (4U)
#define XDR_BIT_SYNCGEN         (4U)
#define XDR_HDR_BIT             (4U)
#define XDR_ADDR_BIT            (6U)
#define XDR_ID_BIT              (5U)
#define XDR_DATA_BIT            (12U)
#define XDR_LD_TRANSFER         (XDR_HDR_BIT + (XDR_LD_DATA_BIT * XDR_LD_SIZE))
#define XDR_CMD_WRITE           (XDR_HDR_BIT + XDR_ADDR_BIT + XDR_DATA_BIT)
#define XDR_CMD_READ            (XDR_HDR_BIT + XDR_ADDR_BIT)
#define XDR_CMD_READOUT         (XDR_HDR_BIT + XDR_ID_BIT + XDR_DATA_BIT)

#define CMD_DELAY_REG_WR        (XDR_CMD_WRITE + 10U) /* 10us */
#define CMD_DELAY_LD            (XDR_LD_TRANSFER + 10U) /* 10us */
#define CMD_DELAY_SYNCGEN       (XDR_BIT_SYNCGEN + 10U) /* 10us */
#define CMD_DELAY_IDGEN         (XDR_BIT_IDGEN + 10U) /* 10us */
#define CMD_DELAY_FAULT_READ    (XDR_CMD_READOUT + 40U) /* 40us */
#define CMD_DELAY_REG_RD        (XDR_CMD_READ + 80U) /* 80us */

#define XDR_FUNCTION_DIS        (0U)
#define XDR_FUNCTION_EN         (1U)

#define XDR_LD_MODE_NORMAL      (0U)
#define XDR_LD_MODE_X8          (1U)

#define XDR_LD_DIR_HEAD         (0U)
#define XDR_LD_DIR_TAIL         (1U)

#define XDR_PWM_RES_12BIT       (0U)
#define XDR_PWM_RES_14BIT       (1U)

#define XDR_SYNC_MODE_CMD       (0U)
#define XDR_SYNC_MODE_SVI       (1U)

#define XDR_FPWM_DIV_1          (0U)
#define XDR_FPWM_DIV_2          (1U)
#define XDR_FPWM_DIV_3          (2U)

#define XDR_CH_LD_TYPE_NTS_1ST  (0U) /* non-time-sharing data 1LD 1st */
#define XDR_CH_LD_TYPE_NTS_2ND  (1U) /* non-time-sharing data 1LD 2nd */
#define XDR_CH_LD_TYPE_TS_1ST   (2U) /* time-sharing data 2LD 1st */
#define XDR_CH_LD_TYPE_TS_2ND   (3U) /* time-sharing data 2LD 2nd */

#define XDR_MAX_CURR_VREF_1     (0xFFFU)
#define XDR_MAX_CURR_VREF_2     (0xFFFU)
#define XDR_MAX_CURR_VREF_3     (0xFFFU)
#define XDR_MAX_CURR_VREF_4     (0xFFFU)
#define XDR_MAX_CURR_VREF_5     (0xFFFU)

#define XDR_FB_LVL_1            (FB_LEVEL_4)
#define XDR_FB_LVL_2            (FB_LEVEL_4)
#define XDR_FB_LVL_3            (FB_LEVEL_4)
#define XDR_FB_LVL_4            (FB_LEVEL_4)
#define XDR_FB_LVL_5            (FB_LEVEL_4)

#define XDR_SHORT_LVL_1         (SHORT_LEVEL_36)
#define XDR_SHORT_LVL_2         (SHORT_LEVEL_36)
#define XDR_SHORT_LVL_3         (SHORT_LEVEL_36)
#define XDR_SHORT_LVL_4         (SHORT_LEVEL_36)
#define XDR_SHORT_LVL_5         (SHORT_LEVEL_36)

#define XDR_MAX_CURR_LVL_1      (CURR_LEVEL_16)
#define XDR_MAX_CURR_LVL_2      (CURR_LEVEL_16)
#define XDR_MAX_CURR_LVL_3      (CURR_LEVEL_16)
#define XDR_MAX_CURR_LVL_4      (CURR_LEVEL_16)
#define XDR_MAX_CURR_LVL_5      (CURR_LEVEL_16)

#define XDR_SERIAL_LATENCY      (0xC8U) /* default */

#define XDR_V_MASK              (0x00U) // ~~us
#define XDR_SV_MASK             (0x00U) // ~~us

#define XDR_RST_COUNT           (0x00U)
#define XDR_TIMEOUT             (0x00U)

#define XDR_FLL_COUNT           (0x00U)
#define XDR_FLL_RANGE           (0x00U)

#define XDR_WR_PROTECT_1        (0xAAAU) /* Writable: FPWM_DIV_1/2/3, MAX_CURR_VREF_1/2/3/4/5 */
#define XDR_WR_PROTECT_2        (0xCCCU) /* Writable: MAX_CURR_VREF_1/2/3/4/5 */
#define XDR_WR_PROTECT_3        (0x555U) /* Writable: ALL */

#define XDR_SPRD_SPD_128_CLK   (0U) /* '000' : 128 clock */
#define XDR_SPRD_SPD_64_CLK    (1U) /* '001' : 64 clock */
#define XDR_SPRD_SPD_32_CLK    (2U) /* '010' : 32 clock */
#define XDR_SPRD_SPD_16_CLK    (3U) /* '011' : 16 clock */
#define XDR_SPRD_SPD_8_CLK     (4U) /* '100' : 8 clock */
#define XDR_SPRD_SPD_4_CLK     (5U) /* '101' : 4 clock */
#define XDR_SPRD_SPD_2_CLK     (6U) /* '110' : 2 clock */
#define XDR_SPRD_SPD_1_CLK     (7U) /* '111' : 1 clock */

#define XDR_SPRD_GAIN_DIV_8    (0U)  /* '000' : calculated value/8 */
#define XDR_SPRD_GAIN_DIV_4    (1U)  /* '001' : calculated value/4 */
#define XDR_SPRD_GAIN_DIV_2    (2U)  /* '010' : calculated value/2 */
#define XDR_SPRD_GAIN_1X       (3U)  /* '011' : calculated value */
#define XDR_SPRD_GAIN_MUL_2    (4U)  /* '100' : calculated value*2 */
#define XDR_SPRD_GAIN_MUL_4    (5U)  /* '101' : calculated value*4 */
#define XDR_SPRD_GAIN_MUL_8    (6U)  /* '110' : calculated value*8 */
#define XDR_SPRD_GAIN_MUL_16   (7U)  /* '111' : calculated value*16 */

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

static _xdr12_regs_t gt_xdr12_set_regs;
static _xdr12_regs_t gt_xdr12_get_regs[XDR_DAISY_LENGTH];

static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_set_regs;
static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_get_regs[XDR_DAISY_LENGTH];

static _xdr12_mirror_regs_t gt_xdr12_mirror_set_regs;
static _xdr12_mirror_regs_t gt_xdr12_mirror_get_regs[XDR_DAISY_LENGTH];

static uint16_t gn_pwm_out_xd_write[(XDR_DAISY_LENGTH * XDR_CMD_WRITE) + PWM_OUT_DUMMY_SIZE];
static uint16_t gn_pwm_out_xd_ld_transfer[(XDR_DAISY_LENGTH * XDR_LD_TRANSFER) + PWM_OUT_DUMMY_SIZE];

static uint16_t gn_pwm_in_xd_response_freq[(XDR_DAISY_LENGTH * XDR_CMD_READOUT) + 2U];
static uint16_t gn_pwm_in_xd_response_duty[(XDR_DAISY_LENGTH * XDR_CMD_READOUT) + 2U];

volatile bool gb_xd_pwm_out_flag;
volatile bool gb_xd_pwm_in_flag;
volatile bool gb_xd_pwm_in_timeout;

static _v_serdes_write_command_t gt_xd_write_command[XDR_DAISY_LENGTH];
static _v_serdes_read_command_t gt_xd_read_command[XDR_DAISY_LENGTH];
static _v_serdes_readout_command_t gt_xd_readout_command[XDR_DAISY_LENGTH];
static _v_serdes_ld_transfer_command_t gt_xd_ld_transfer_command[XDR_DAISY_LENGTH];
static _v_serdes_fault_read_command_t gt_xd_fault_read_command[XDR_DAISY_LENGTH];
static _v_serdes_fault_readout_command_t gt_xd_fault_readout_command;
static _v_serdes_sync_gen_command_t gt_xd_syncgen_command[XDR_DAISY_LENGTH];
static _v_serdes_id_gen_command_t gt_xd_idgen_command[XDR_DAISY_LENGTH];

static bool gb_xdr_do_efuse;

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

static bool xdr12_pwm_in(uint16_t length, uint16_t timeout_us)
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

static void xdr12_pwm_out_done(void)
{
    while(gb_xd_pwm_out_flag)
    {
        __NOP();
    }
}

static void xdr12_pwm_out(uint32_t mem_addr, uint32_t length)
{
    gb_xd_pwm_out_flag = true;
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_1, mem_addr);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_1, length);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_1);
    LL_TIM_EnableCounter(TIM1);
}

static uint16_t xdr12_make_pwm_out_stream(uint32_t data, uint16_t *p, uint16_t len)
{
    if((p == NULL) || (len == 0U) || (len > 32U))
    {
        return 0U;
    }

    uint32_t bit_data = (uint32_t)(data << (32U - len));
    uint16_t* p_dst = p;

    for(uint16_t i = 0 ; i < len ; ++i)
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

static uint16_t xdr12_decode_pwm_input_stream(uint16_t* pfreq, uint16_t* pduty, uint16_t* pdata, uint16_t len)
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

        gt_xd_fault_readout_command.bit.cmd_code = n_header;
        gt_xd_fault_readout_command.bit.bit_thermal = ((n_data >> 3U) & 0x01U);
        gt_xd_fault_readout_command.bit.bit_short = ((n_data >> 2U) & 0x01U);
        gt_xd_fault_readout_command.bit.bit_open = ((n_data >> 1U) & 0x01U);
        gt_xd_fault_readout_command.bit.bit_fb = ((n_data >> 0U) & 0x01U);

        pdata[id++] = n_data;
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Fault Recv Packet\r\n\tCMD - 0x%01X, FAULT - 0x%01X", n_header, n_data);
    }
    else        /* read receive format : '1110' + id[4:0] + data[11:0] */
    {
        uint16_t duty_idx = 0;
        uint16_t n_id = 0U;

        while(((uint32_t)duty_idx + XDR_CMD_READOUT) <= len)
        {
            if(id >= XDR_DAISY_LENGTH)
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
            for(int16_t bit = 11 ; bit >= 0 ; --bit)
            {
                n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << (uint16_t)bit);
            }

            gt_xd_readout_command[id].bit.cmd_code = n_header;
            gt_xd_readout_command[id].bit.id = n_id;
            gt_xd_readout_command[id].bit.data = n_data;

            pdata[id++] = n_data;
            duty_idx += XDR_CMD_READOUT;
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Read Recv Packet\r\n\tCMD - 0x%01X, ID - 0x%02X, DATA - 0x%03X", n_header, n_id, n_data);
        }
    }

    return id;
}

static void xdr12_regs_init_table(void)
{
    _xdr12_regs_t* _r1 = &gt_xdr12_set_regs;

    for(xd12_addr_t addr = XD12R_RESET_ID ; addr < XD12R_MAX ; ++addr)
    {
        switch(addr)
        {
            case XD12R_RESET_ID:
            {
                _r1->reg._r00.bit.lkg_e = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.e_rst = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.vs_rst = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.rst = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_LD_CONTROL:
            {
                _r1->reg._r01.bit.ld_mode = XDR_LD_MODE_NORMAL;
                _r1->reg._r01.bit.ld_dir = XDR_LD_DIR_HEAD;
                _r1->reg._r01.bit.ld_res = XDR_PWM_RES_14BIT;
                _r1->reg._r01.bit.syncmode = XDR_SYNC_MODE_SVI;
                _r1->reg._r01.bit.delay_mode_en = XDR_FUNCTION_DIS;
                _r1->reg._r01.bit.sv_no = XDR_SV_NO;
                break;
            }
            case XD12R_LD_SIZE:
            {
                _r1->reg._r02.bit.ld_size = XDR_LD_SIZE;
                break;
            }
            case XD12R_PWMCLK_DIV1_2:
            {
                _r1->reg._r03.bit.pwmclk_div1 = XDR_FPWM_DIV_1;
                _r1->reg._r03.bit.pwmclk_div2 = ((XDR_FPWM_DIV_2 & 0x0FU) >> 0U);
                break;
            }
            case XD12R_PWMCLK_DIV2_3:
            {
                _r1->reg._r04.bit.pwmclk_div2 = ((XDR_FPWM_DIV_2 & 0xF0U) >> 4U);
                _r1->reg._r04.bit.pwmclk_div3 = XDR_FPWM_DIV_3;
                break;
            }
            case XD12R_CHANNEL_ENABLE:
            {
                _r1->reg._r05.bit.ch1_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch2_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch3_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch4_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch5_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch6_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch7_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch8_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch9_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch10_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch11_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch12_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_FAULT_CONTROL:
            {
                _r1->reg._r06.bit.o_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.s_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.t_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.o_det_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.s_det_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.o_fb_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.fb_mode = 0U;
                _r1->reg._r06.bit.auto_fault_fb_no = 0U;
                break;
            }
            case XD12R_CHx_LD_TYPE0:
            {
                _r1->reg._r07.bit.ch7_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch8_ld_type = XDR_CH_LD_TYPE_NTS_2ND;
                _r1->reg._r07.bit.ch9_ld_type = XDR_CH_LD_TYPE_TS_1ST;
                _r1->reg._r07.bit.ch10_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch11_ld_type = XDR_CH_LD_TYPE_NTS_2ND;
                _r1->reg._r07.bit.ch12_ld_type = XDR_CH_LD_TYPE_TS_1ST;
                break;
            }
            case XD12R_CHx_LD_TYPE1:
            {
                _r1->reg._r08.bit.ch1_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch2_ld_type = XDR_CH_LD_TYPE_NTS_2ND;
                _r1->reg._r08.bit.ch3_ld_type = XDR_CH_LD_TYPE_TS_1ST;
                _r1->reg._r08.bit.ch4_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch5_ld_type = XDR_CH_LD_TYPE_NTS_2ND;
                _r1->reg._r08.bit.ch6_ld_type = XDR_CH_LD_TYPE_TS_1ST;
                break;
            }
            case XD12R_MAX_CURR_VREF1:
            {
                _r1->reg._r0A.bit.max_curr_vref = XDR_MAX_CURR_VREF_1;
                break;
            }
            case XD12R_MAX_CURR_VREF2:
            {
                _r1->reg._r0B.bit.max_curr_vref = XDR_MAX_CURR_VREF_2;
                break;
            }
            case XD12R_MAX_CURR_VREF3:
            {
                _r1->reg._r0C.bit.max_curr_vref = XDR_MAX_CURR_VREF_3;
                break;
            }
            case XD12R_MAX_CURR_VREF4:
            {
                _r1->reg._r0D.bit.max_curr_vref = XDR_MAX_CURR_VREF_4;
                break;
            }
            case XD12R_MAX_CURR_VREF5:
            {
                _r1->reg._r0E.bit.max_curr_vref = XDR_MAX_CURR_VREF_5;
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
                _r1->reg._r16.bit.fb1_level = XDR_FB_LVL_1;
                _r1->reg._r16.bit.fb2_level = XDR_FB_LVL_2;
                _r1->reg._r16.bit.fb3_level = XDR_FB_LVL_3;
                _r1->reg._r16.bit.fb4_level = XDR_FB_LVL_4;
                break;
            }
            case XD12R_FB_SHORT_LEVEL:
            {
                _r1->reg._r17.bit.fb5_level = XDR_FB_LVL_5;
                _r1->reg._r17.bit.short1_level = XDR_SHORT_LVL_1;
                _r1->reg._r17.bit.short2_level = XDR_SHORT_LVL_2;
                _r1->reg._r17.bit.short3_level = XDR_SHORT_LVL_3;
                break;
            }
            case XD12R_SHORT_LEVEL:
            {
                _r1->reg._r18.bit.short4_level = XDR_SHORT_LVL_4;
                _r1->reg._r18.bit.short5_level = XDR_SHORT_LVL_5;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL1:
            {
                _r1->reg._r19.bit.max_curr1_level = XDR_MAX_CURR_LVL_1;
                _r1->reg._r19.bit.max_curr2_level = XDR_MAX_CURR_LVL_2;
                _r1->reg._r19.bit.max_curr3_level = XDR_MAX_CURR_LVL_3;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL2:
            {
                _r1->reg._r1A.bit.max_curr4_level = XDR_MAX_CURR_LVL_4;
                _r1->reg._r1A.bit.max_curr5_level = XDR_MAX_CURR_LVL_5;
                break;
            }
            case XD12R_PARITY_RD_EN:
            {
                _r1->reg._r1B.bit.reg_rd_en = XDR_FUNCTION_EN;
                _r1->reg._r1B.bit.parity_e = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_SERIAL_CLK_GEN:
            {
                _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
                _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
                break;
            }
            case XD12R_SERIAL_LATENCY:
            {
                _r1->reg._r1D.bit.serial_latency = XDR_SERIAL_LATENCY;
                break;
            }
            case XD12R_V_MASK:
            {
                _r1->reg._r1E.bit.v_mask = XDR_V_MASK;
                break;
            }
            case XD12R_SV_MASK:
            {
                _r1->reg._r1F.bit.sv_mask = XDR_SV_MASK;
                _r1->reg._r1F.bit.sv_mask_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_RSTCNT:
            {
                _r1->reg._r20.bit.rstcnt = XDR_RST_COUNT;
                break;
            }
            case XD12R_TIMEOUT:
            {
                _r1->reg._r21.bit.timeout = XDR_TIMEOUT;
                break;
            }
            case XD12R_FLLCNT1:
            {
                _r1->reg._r22.bit.fllcnt = ((XDR_FLL_COUNT & 0x000FFFUL) >>  0U);
                break;
            }
            case XD12R_FLLCNT2:
            {
                _r1->reg._r23.bit.fllcnt = ((XDR_FLL_COUNT & 0x1FF000UL) >> 12U);
                _r1->reg._r23.bit.fll_range = XDR_FLL_RANGE;
                _r1->reg._r23.bit.fll_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_WR_PROTECT:
            {
                _r1->reg._r24.bit.wr_protect = XDR_WR_PROTECT_3;
                break;
            }
            case XD12R_NF_CONTROL:
            {
                _r1->reg._r25.bit.DGRJT_EN1 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.DGRJT_EN2 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_EN = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN1 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN2 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.O_EMI_REJ_EN = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_TH = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_CHOP_EN:
            {
                _r1->reg._r26.bit.CHOP_BGR_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DAC_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSC_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSCLDO_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DRV_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_EN = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_TEMP:
            {
                _r1->reg._r27.bit.flt_gain = 1U;
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
                _r1->reg._r29.bit.osc_fll_err_range = 0U;
                _r1->reg._r29.bit.osc_man_en = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_OSC_SPREAD:
            {
                _r1->reg._r2A.bit.SPRD_GAIN = XDR_SPRD_GAIN_MUL_2;
                _r1->reg._r2A.bit.SPRD_SPD = XDR_SPRD_SPD_32_CLK;
                _r1->reg._r2A.bit.SPRD_EN = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_CLOCK_GATE_EN:
            {
                _r1->reg._r2B.bit.DC_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.FR1_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.FR2_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.OTP_MCLK_EN = XDR_FUNCTION_EN;
                break;
            }
            default:
            {
                continue;
            }
        }
        xdr12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
}

static void xdr12_regs_trim_init_table(void)
{
    _xdr12_regs_t* _r1 = &gt_xdr12_set_regs;
    //_xdr12_mirror_regs_t* _r2 = &gt_xdr12_mirror_set_regs;
    _xdr12_otp_ctrl_regs_t* _r3 = &gt_xdr12_otp_ctrl_set_regs;

    for(xd12_addr_t addr = XD12R_RESET_ID ; addr < XD12R_MAX ; ++addr)
    {
        switch(addr)
        {
            case XD12R_RESET_ID:
            {
                _r1->reg._r00.bit.lkg_e = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.e_rst = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.vs_rst = XDR_FUNCTION_DIS;
                _r1->reg._r00.bit.rst = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_LD_CONTROL:
            {
                _r1->reg._r01.bit.ld_mode = XDR_LD_MODE_NORMAL;
                _r1->reg._r01.bit.ld_dir = XDR_LD_DIR_HEAD;
                _r1->reg._r01.bit.ld_res = XDR_PWM_RES_14BIT;
                _r1->reg._r01.bit.syncmode = XDR_SYNC_MODE_SVI;
                _r1->reg._r01.bit.delay_mode_en = XDR_FUNCTION_DIS;
                _r1->reg._r01.bit.sv_no = XDR_SV_NO;
                break;
            }
            case XD12R_LD_SIZE:
            {
                _r1->reg._r02.bit.ld_size = XDR_LD_SIZE;
                break;
            }
            case XD12R_PWMCLK_DIV1_2:
            {
                _r1->reg._r03.bit.pwmclk_div1 = XDR_FPWM_DIV_1;
                _r1->reg._r03.bit.pwmclk_div2 = ((XDR_FPWM_DIV_2 & 0x0FU) >> 0U);
                break;
            }
            case XD12R_PWMCLK_DIV2_3:
            {
                _r1->reg._r04.bit.pwmclk_div2 = ((XDR_FPWM_DIV_2 & 0xF0U) >> 4U);
                _r1->reg._r04.bit.pwmclk_div3 = XDR_FPWM_DIV_3;
                break;
            }
            case XD12R_CHANNEL_ENABLE:
            {
                _r1->reg._r05.bit.ch1_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch2_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch3_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch4_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch5_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch6_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch7_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch8_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch9_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch10_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch11_en = XDR_FUNCTION_EN;
                _r1->reg._r05.bit.ch12_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_FAULT_CONTROL:
            {
                _r1->reg._r06.bit.o_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.s_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.t_off_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.o_det_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.s_det_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.o_fb_e = XDR_FUNCTION_DIS;
                _r1->reg._r06.bit.fb_mode = 0U;
                _r1->reg._r06.bit.auto_fault_fb_no = 0U;
                break;
            }
            case XD12R_CHx_LD_TYPE0:
            {
                _r1->reg._r07.bit.ch7_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch8_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch9_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch10_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch11_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r07.bit.ch12_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                break;
            }
            case XD12R_CHx_LD_TYPE1:
            {
                _r1->reg._r08.bit.ch1_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch2_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch3_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch4_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch5_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                _r1->reg._r08.bit.ch6_ld_type = XDR_CH_LD_TYPE_NTS_1ST;
                break;
            }
            case XD12R_MAX_CURR_VREF1:
            {
                _r1->reg._r0A.bit.max_curr_vref = XDR_MAX_CURR_VREF_1;
                break;
            }
            case XD12R_MAX_CURR_VREF2:
            {
                _r1->reg._r0B.bit.max_curr_vref = XDR_MAX_CURR_VREF_2;
                break;
            }
            case XD12R_MAX_CURR_VREF3:
            {
                _r1->reg._r0C.bit.max_curr_vref = XDR_MAX_CURR_VREF_3;
                break;
            }
            case XD12R_MAX_CURR_VREF4:
            {
                _r1->reg._r0D.bit.max_curr_vref = XDR_MAX_CURR_VREF_4;
                break;
            }
            case XD12R_MAX_CURR_VREF5:
            {
                _r1->reg._r0E.bit.max_curr_vref = XDR_MAX_CURR_VREF_5;
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
                _r1->reg._r16.bit.fb1_level = XDR_FB_LVL_1;
                _r1->reg._r16.bit.fb2_level = XDR_FB_LVL_2;
                _r1->reg._r16.bit.fb3_level = XDR_FB_LVL_3;
                _r1->reg._r16.bit.fb4_level = XDR_FB_LVL_4;
                break;
            }
            case XD12R_FB_SHORT_LEVEL:
            {
                _r1->reg._r17.bit.fb5_level = XDR_FB_LVL_5;
                _r1->reg._r17.bit.short1_level = XDR_SHORT_LVL_1;
                _r1->reg._r17.bit.short2_level = XDR_SHORT_LVL_2;
                _r1->reg._r17.bit.short3_level = XDR_SHORT_LVL_3;
                break;
            }
            case XD12R_SHORT_LEVEL:
            {
                _r1->reg._r18.bit.short4_level = XDR_SHORT_LVL_4;
                _r1->reg._r18.bit.short5_level = XDR_SHORT_LVL_5;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL1:
            {
                _r1->reg._r19.bit.max_curr1_level = XDR_MAX_CURR_LVL_1;
                _r1->reg._r19.bit.max_curr2_level = XDR_MAX_CURR_LVL_2;
                _r1->reg._r19.bit.max_curr3_level = XDR_MAX_CURR_LVL_3;
                break;
            }
            case XD12R_MAX_CURRENT_LEVEL2:
            {
                _r1->reg._r1A.bit.max_curr4_level = XDR_MAX_CURR_LVL_4;
                _r1->reg._r1A.bit.max_curr5_level = XDR_MAX_CURR_LVL_5;
                break;
            }
            case XD12R_PARITY_RD_EN:
            {
                _r1->reg._r1B.bit.reg_rd_en = XDR_FUNCTION_EN;
                _r1->reg._r1B.bit.parity_e = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_SERIAL_CLK_GEN:
            {
                _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
                _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
                break;
            }
            case XD12R_SERIAL_LATENCY:
            {
                _r1->reg._r1D.bit.serial_latency = XDR_SERIAL_LATENCY;
                break;
            }
            case XD12R_V_MASK:
            {
                _r1->reg._r1E.bit.v_mask = XDR_V_MASK;
                break;
            }
            case XD12R_SV_MASK:
            {
                _r1->reg._r1F.bit.sv_mask = XDR_SV_MASK;
                _r1->reg._r1F.bit.sv_mask_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_RSTCNT:
            {
                _r1->reg._r20.bit.rstcnt = XDR_RST_COUNT;
                break;
            }
            case XD12R_TIMEOUT:
            {
                _r1->reg._r21.bit.timeout = XDR_TIMEOUT;
                break;
            }
            case XD12R_FLLCNT1:
            {
                _r1->reg._r22.bit.fllcnt = ((XDR_FLL_COUNT & 0x000FFFUL) >>  0U);
                break;
            }
            case XD12R_FLLCNT2:
            {
                _r1->reg._r23.bit.fllcnt = ((XDR_FLL_COUNT & 0x1FF000UL) >> 12U);
                _r1->reg._r23.bit.fll_range = XDR_FLL_RANGE;
                _r1->reg._r23.bit.fll_en = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_WR_PROTECT:
            {
                _r1->reg._r24.bit.wr_protect = XDR_WR_PROTECT_3;
                break;
            }
            case XD12R_NF_CONTROL:
            {
                _r1->reg._r25.bit.DGRJT_EN1 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.DGRJT_EN2 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_EN = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN1 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.SGRJT_EN2 = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.O_EMI_REJ_EN = XDR_FUNCTION_DIS;
                _r1->reg._r25.bit.BBKN_TH = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_CHOP_EN:
            {
                _r1->reg._r26.bit.CHOP_BGR_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DAC_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSC_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_OSCLDO_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_DRV_EN = XDR_FUNCTION_EN;
                _r1->reg._r26.bit.CHOP_EN = XDR_FUNCTION_EN;
                break;
            }
            case XD12R_TEMP:
            {
                _r1->reg._r27.bit.flt_gain = 1U;
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
                _r1->reg._r29.bit.osc_fll_err_range = 0U;
                _r1->reg._r29.bit.osc_man_en = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_OSC_SPREAD:
            {
                _r1->reg._r2A.bit.SPRD_GAIN = XDR_SPRD_GAIN_MUL_2;
                _r1->reg._r2A.bit.SPRD_SPD = XDR_SPRD_SPD_32_CLK;
                _r1->reg._r2A.bit.SPRD_EN = XDR_FUNCTION_DIS;
                break;
            }
            case XD12R_CLOCK_GATE_EN:
            {
                _r1->reg._r2B.bit.DC_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.FR1_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.FR2_MCLK_EN = XDR_FUNCTION_EN;
                _r1->reg._r2B.bit.OTP_MCLK_EN = XDR_FUNCTION_EN;
                break;
            }
            default:
            {
                continue;
            }
        }
        xdr12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }

    for(xd12_otp_ctrl_addr_t addr = XD12R_OTP_ACCESS1 ; addr < XD12R_OTP_MAX ; ++addr)
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
        xdr12_write_by_type(addr, _r3->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
}

void xdr12_reset(void)
{
    _v_xdr12_reset_id_t _r00 = { 0U };
    _r00.bit.lkg_e = 0U;
    _r00.bit.e_rst = 0U;
    _r00.bit.vs_rst = 0U;
    _r00.bit.rst = 1U;
    xdr12_write_by_type(XD12R_RESET_ID, _r00.ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_idgen(void)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xdr12_pwm_out_done();

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            len += xdr12_make_pwm_out_stream(gt_xd_idgen_command[daisy].ALL, &p_pwm_out[len], XDR_BIT_IDGEN);
        }
        p_pwm_out[len++] = 0U;

        xdr12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _v_id_gen_command_t _r04 = { 0, };

        _r04.bit.enable = 1U;

        xcr24_write_grp1_reg(XCR_ID_GEN_COMMAND, &_r04.ALL, 1U);
    }
    #endif
    us_delay(CMD_DELAY_IDGEN);
}

void xdr12_syncgen(void)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xdr12_pwm_out_done();

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            len += xdr12_make_pwm_out_stream(gt_xd_syncgen_command[daisy].ALL, &p_pwm_out[len], XDR_BIT_SYNCGEN);
        }
        p_pwm_out[len++] = 0U;

        xdr12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _v_id_gen_command_t _r04 = { 0, };

        _r04.bit.enable = 1U;

        xcr24_write_grp1_reg(XCR_ID_GEN_COMMAND, &_r04.ALL, 1U);
    }
    #endif
    us_delay(CMD_DELAY_SYNCGEN);
}

static void xdr12_memory_copy(void)
{
    gt_xdr12_set_regs = gt_xdr12_get_regs[0];
    gt_xdr12_otp_ctrl_set_regs = gt_xdr12_otp_ctrl_get_regs[0];
    gt_xdr12_mirror_set_regs = gt_xdr12_mirror_get_regs[0];
}

void xdr12_read_all(void)
{
    for (xd12_addr_t addr = XD12R_RESET_ID ; addr < XD12R_MAX ; ++addr)
    {
        xdr12_read_by_type(addr, XD12R_ADDR_TYPE_GENERAL);
    }

    for (xd12_otp_ctrl_addr_t addr = XD12R_OTP_ACCESS1 ; addr < XD12R_OTP_MAX ; ++addr)
    {
        xdr12_read_by_type(XD12R_OTP_CTRL_BASE + addr, XD12R_ADDR_TYPE_GENERAL);
    }

    for (xd12_mirror_addr_t addr = XD12R_MIRROR1 ; addr < XD12R_MIRROR_MAX ; ++addr)
    {
        xdr12_read_by_type(addr, XD12R_ADDR_TYPE_MIRROR);
    }

    xdr12_memory_copy();
}

void xdr12_init_param(void)
{
    for(uint16_t num = 0U ; num < XDR_DAISY_LENGTH ; ++num)
    {
        gt_xd_write_command[num].bit.cmd_code = CMD_CODE_WRITE;
        gt_xd_read_command[num].bit.cmd_code = CMD_CODE_READ;
        gt_xd_readout_command[num].bit.cmd_code = CMD_CODE_READ;
        gt_xd_ld_transfer_command[num].bit.cmd_code = CMD_CODE_LDIM;
        gt_xd_fault_read_command[num].bit.cmd_code = CMD_CODE_RD_FAULT;
        gt_xd_syncgen_command[num].bit.cmd_code = CMD_CODE_SYNCGEN;
        gt_xd_idgen_command[num].bit.cmd_code = CMD_CODE_IDGEN;
    }

    gt_xd_fault_readout_command.bit.cmd_code = CMD_CODE_RD_FAULT;

    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_pwm_in_xd_response_duty);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gn_pwm_in_xd_response_freq);
}

void xdr12_init(void)
{
    xdr12_reset();
    xdr12_idgen();
    xdr12_regs_init_table();
    xdr12_read_all();
}

void xdr12_trim_init(void)
{
    xdr12_reset();
    xdr12_idgen();
    xdr12_regs_trim_init_table();
    xdr12_read_all();
}

static void xdr12_write(uint16_t addr, uint16_t data)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xdr12_pwm_out_done();

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            gt_xd_write_command[daisy].bit.addr = addr;
            gt_xd_write_command[daisy].bit.data = data;
            len += xdr12_make_pwm_out_stream(gt_xd_write_command[daisy].ALL, &p_pwm_out[len], XDR_CMD_WRITE);
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Write Packet\r\n\tADDR - 0x%02X, DATA - 0x%03X", gt_xd_write_command->bit.addr, gt_xd_write_command->bit.data);
        }
        p_pwm_out[len++] = 0U;

        xdr12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
    }
    #else
    {
        _v_global_write_command_t _r01 = { 0, };

        _r01.bit.enable = 1U;
        _r01.bit.addr = addr;

        xcr24_write_grp1_reg(XCR_GLOBAL_WRITE_DATA, &data, 1U);
        xcr24_write_grp1_reg(XCR_GLOBAL_WRITE_COMMAND, &_r01.ALL, 1U);
    }
    #endif
    us_delay(CMD_DELAY_REG_WR);
}

static uint16_t xdr12_read(uint16_t addr, uint16_t* p_xdr_buffer)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    {
        uint16_t* const p_pwm_out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;
        const uint16_t pwm_in_length = XDR_DAISY_LENGTH * (XDR_HDR_BIT + XDR_ID_BIT + XDR_DATA_BIT);

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            gt_xd_read_command[daisy].bit.addr = addr;
            len += xdr12_make_pwm_out_stream(gt_xd_read_command[daisy].ALL, &p_pwm_out[len], XDR_CMD_READ);
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Read Packet\r\n\tIN_LENGTH [%u] CMD[0x%01X] ADDR[0x%02X]", pwm_in_length, gt_xd_read_command->bit.cmd_code, gt_xd_read_command->bit.addr);
        }
        p_pwm_out[len++] = 0U;

        memset(gn_pwm_in_xd_response_freq, 0U, sizeof(gn_pwm_in_xd_response_freq));
        memset(gn_pwm_in_xd_response_duty, 0U, sizeof(gn_pwm_in_xd_response_duty));

        xdr12_pwm_out((uint32_t)p_pwm_out, (len + PWM_OUT_HEADER_SIZE));
        xdr12_pwm_out_done();

        LL_GPIO_SetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);

        if(false == xdr12_pwm_in(pwm_in_length, 150U))
        {
            uint8_t max_id = xdr12_decode_pwm_input_stream(gn_pwm_in_xd_response_freq, gn_pwm_in_xd_response_duty, p_xdr_buffer, pwm_in_length);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\nFunction[%s] timeout!!", __func__);
        }
        LL_GPIO_ResetOutputPin(DEBUG_GPIO_Port, DEBUG_Pin);
    }
    #else // to do
    {
        uint16_t buffer[XDR_DAISY_LENGTH] = { 0, };

        const uint16_t daisy_size = XDR_DAISY_LENGTH;
        const uint16_t ch_seg_max = MODEL_XCR24_SEG;

        for(uint16_t ch_seg = 0U ; ch_seg < ch_seg_max ; ++ch_seg)
        {
            uint16_t offset = 0U;
            uint16_t remaining = daisy_size;

            xcr24_read_local(ch_seg, addr);

            /* clear read buffer */
            memset(buffer, 0U, sizeof(buffer));

            while(remaining > 0U)
            {
                uint16_t chunk = (remaining > XCR_SPI_RW_LEN) ? XCR_SPI_RW_LEN : remaining;

                xcr24_get_local_rw_data(XCR_PORT1_LOCAL_RW_DATA1 + offset, &buffer[offset], chunk);
                offset += chunk;
                remaining -= chunk;
            }

            /* iteration - IC num */
            for(uint16_t xd_daisy = 0U; xd_daisy < daisy_size; ++xd_daisy)
            {
                p_xdr_buffer[xd_daisy] = buffer[xd_daisy];
            }
        }
    }
    #endif
    us_delay(CMD_DELAY_REG_RD);

    return 0U;
}

static void xdr12_change_addr_type(xd12r_addr_type_t addr_type)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    xd12r_addr_type_t current_type = (xd12r_addr_type_t)(_r3F->bit.addr_ext);

    if (current_type != addr_type)
    {
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nChange XDR ADDR_TYPE TO %s", (addr_type == XD12R_ADDR_TYPE_GENERAL)?("GENERAL"):("MIRROR"));
        _r3F->bit.addr_ext = addr_type;
        xdr12_write(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, gt_xdr12_otp_ctrl_set_regs.reg._r3F.ALL);
    }
}

void xdr12_write_by_type(uint16_t addr, uint16_t data, xd12r_addr_type_t addr_type)
{
    xdr12_change_addr_type(addr_type);
    xdr12_write(addr, data);
}

uint16_t xdr12_read_by_type(uint16_t addr, xd12r_addr_type_t addr_type)
{
    uint16_t xdr_buffer[XDR_DAISY_LENGTH] = { 0U };
    xdr12_change_addr_type(addr_type);
    xdr12_read(addr, xdr_buffer);

    for (uint8_t id = 0U ; id < XDR_DAISY_LENGTH ; ++id)
    {
        if (addr_type == XD12R_ADDR_TYPE_GENERAL)
        {
            if (addr < XD12R_OTP_CTRL_BASE)
            {
                gt_xdr12_get_regs[id].ALL[addr] = xdr_buffer[id];
            }
            else
            {
                gt_xdr12_otp_ctrl_get_regs[id].ALL[addr] = xdr_buffer[id];
            }
        }
        else
        {
            gt_xdr12_mirror_get_regs[id].ALL[addr] = xdr_buffer[id];
        }
    }
    return xdr_buffer[0];
}

void xdr12_ld_transfer(void)
{
#if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    uint16_t* p_ld_buffer = ldim_get_xdr_ld_transfer_buffer();
    uint16_t* const p_pwm_out = gn_pwm_out_xd_ld_transfer + PWM_OUT_HEADER_SIZE;
    uint16_t len = 0;

    for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
    {
        /* header : 1111 */
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;
        p_pwm_out[len++] = PWM_OUT_BIT1;

        for(uint8_t ld_idx = 0U ; ld_idx < XDR_LD_SIZE ; ++ld_idx)
        {
            len += xdr12_make_pwm_out_stream(*p_ld_buffer++, &p_pwm_out[len], XDR_LD_DATA_BIT);
        }
    }
    p_pwm_out[len++] = 0U;

    xdr12_pwm_out((uint32_t)p_pwm_out, (uint32_t)len);
#elif (XDR_CONTROL_TYPE == XDR_CONTROLLED_XCR)
    uint16_t* p = ldim_get_xcr_ld_transfer_buffer();
    uint16_t len = ldim_get_xcr_ld_transfer_size();
    xcr24_set_ld_transfer(p, len);
#else
    #error "XDR_CONTROL_TYPE is not defined"
#endif
    us_delay(CMD_DELAY_LD);
}

void xdr12_trim_set_efuse_enable(bool en)
{
    gb_xdr_do_efuse = en;
}

bool xdr12_trim_get_efuse_enable(void)
{
    return gb_xdr_do_efuse;
}

void xdr12_trim_init_current_ref(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_init_ldo_dig(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 2U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_init_ldo_dac(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_init_ldo_fll(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_init_osc(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.mclk64_o = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    const uint16_t osc_fll_man = 0x8000U; /* 16b' 1000_0000_0000_0000*/

    _v_xdr12_osc_fll_man1_t* _r28 = &gt_xdr12_set_regs.reg._r28;
    _r28->bit.osc_fll_man = ((osc_fll_man & 0x0FFFU) >>  0U);
    xdr12_write_by_type(XD12R_OSC_FLL_MAN1, _r28->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_osc_fll_man2_t* _r29 = &gt_xdr12_set_regs.reg._r29;
    _r29->bit.osc_fll_man = ((osc_fll_man & 0xF000U) >> 12U);
    _r29->bit.osc_man_en = 1U;
    xdr12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_init_ch_gain(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

#if (XD_MODEL_TYPE == XD_TYPE_XDR12R)
    _v_xdr12_max_current_level1_t* _r19 = &gt_xdr12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_16;
    _r19->bit.max_curr2_level = CURR_LEVEL_16;
    _r19->bit.max_curr3_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_max_current_level2_t* _r1A = &gt_xdr12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_16;
    _r1A->bit.max_curr5_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
#elif (XD_MODEL_TYPE == XD_TYPE_XDR12D)
    _v_xdr12_max_current_level_t* _r0A = &gt_xdr12_set_regs.reg._r0A;
    _r0A->bit.max_curr1_level = CURR_LEVEL_16;
    _r0A->bit.max_curr2_level = CURR_LEVEL_16;
    _r0A->bit.max_curr3_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURR_LEVEL, _r0A->ALL, XD12R_ADDR_TYPE_GENERAL);
#elif (XD_MODEL_TYPE == XD_TYPE_IC601)
#endif
}

void xdr12_trim_init_ch_ofs(void)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

#if (XD_MODEL_TYPE == XD_TYPE_XDR12R)
    _v_xdr12_max_current_level1_t* _r19 = &gt_xdr12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_16;
    _r19->bit.max_curr2_level = CURR_LEVEL_16;
    _r19->bit.max_curr3_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_max_current_level2_t* _r1A = &gt_xdr12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_16;
    _r1A->bit.max_curr5_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
#elif (XD_MODEL_TYPE == XD_TYPE_XDR12D)
    _v_xdr12_max_current_level_t* _r0A = &gt_xdr12_set_regs.reg._r0A;
    _r0A->bit.max_curr1_level = CURR_LEVEL_16;
    _r0A->bit.max_curr2_level = CURR_LEVEL_16;
    _r0A->bit.max_curr3_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURR_LEVEL, _r0A->ALL, XD12R_ADDR_TYPE_GENERAL);
#elif (XD_MODEL_TYPE == XD_TYPE_IC601)
#endif
}

void xdr12_trim_set_channel_enable(XD_CH_t chx)
{
    _v_xdr12_channel_enable_t* _r05 = &gt_xdr12_set_regs.reg._r05;
    _r05->ALL = (uint16_t)(1U << chx);
    xdr12_write_by_type(XD12R_CHANNEL_ENABLE, _r05->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_set_max_curr_vref(uint16_t vref)
{
    for (uint8_t i = 0U ; i < 5 ; ++i)
    {
        uint16_t* p_vref = &gt_xdr12_set_regs.ALL[XD12R_MAX_CURR_VREF1 + i];
        *p_vref = vref;
        xdr12_write_by_type(XD12R_MAX_CURR_VREF1 + i, *p_vref, XD12R_ADDR_TYPE_GENERAL);
    }
}

void xdr12_trim_set_max_curr_lvl(uint16_t curr_lvl)
{
    _v_xdr12_max_current_level1_t* _r19 = &gt_xdr12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = curr_lvl;
    _r19->bit.max_curr2_level = curr_lvl;
    _r19->bit.max_curr3_level = curr_lvl;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_max_current_level2_t* _r1A = &gt_xdr12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = curr_lvl;
    _r1A->bit.max_curr5_level = curr_lvl;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

bool xdr12_trim_set_current_ref(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XDR_MAX_CURRENT_REF)
    {
        _v_xdr12_mirror2_t* _r01 = &gt_xdr12_mirror_set_regs.reg._r01;
        _r01->bit.iref_ctl = reg_val;
        xdr12_write_by_type(XD12R_MIRROR2, _r01->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_ldo_dig(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XDR_MAX_LDO_DIG)
    {
        _v_xdr12_mirror4_t* _r03 = &gt_xdr12_mirror_set_regs.reg._r03;
        _r03->bit.ldo_ctl = reg_val;
        xdr12_write_by_type(XD12R_MIRROR4, _r03->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_ldo_dac(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XDR_MAX_LDO_DAC)
    {
        _v_xdr12_mirror3_t* _r02 = &gt_xdr12_mirror_set_regs.reg._r02;
        _r02->bit.ldo_dac_ctl = reg_val;
        xdr12_write_by_type(XD12R_MIRROR3, _r02->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_ldo_fll(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XDR_MAX_LDO_FLL)
    {
        _v_xdr12_mirror4_t* _r03 = &gt_xdr12_mirror_set_regs.reg._r03;
        _r03->bit.ldo_osc_ctl = reg_val;
        xdr12_write_by_type(XD12R_MIRROR4, _r03->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_osc(uint16_t reg_val)
{
    bool ret = false;
    if (reg_val <= XDR_MAX_OSC)
    {
        _v_xdr12_mirror3_t* _r02 = &gt_xdr12_mirror_set_regs.reg._r02;
        _r02->bit.osc_rctl = reg_val;
        xdr12_write_by_type(XD12R_MIRROR3, _r02->ALL, XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_ch_gain(uint16_t reg_val, XD_CH_t chx)
{
    bool ret = false;
    uint16_t* p_base_address = &gt_xdr12_mirror_set_regs.ALL[XD12R_MIRROR_GAIN_CH01];
    uint16_t addr_offset = (uint16_t)(chx);
    if (reg_val <= XDR_MAX_CH_GAIN)
    {
        *(p_base_address + addr_offset) = reg_val;
        xdr12_write_by_type(XD12R_MIRROR_GAIN_CH01 + addr_offset, *(p_base_address + addr_offset), XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

bool xdr12_trim_set_ch_ofs(uint16_t reg_val, XD_CH_t chx)
{
    bool ret = false;
    uint16_t* p_base_address = &gt_xdr12_mirror_set_regs.ALL[XD12R_MIRROR_OFS_CH01];
    uint16_t addr_offset = (uint16_t)(chx);
    if (reg_val <= XDR_MAX_CH_OFS)
    {
        *(p_base_address + addr_offset) = reg_val;
        xdr12_write_by_type(XD12R_MIRROR_OFS_CH01 + addr_offset, *(p_base_address + addr_offset), XD12R_ADDR_TYPE_MIRROR);
        ret = true;
    }
    else
    {
        FATAL_INVALID_INPUT(reg_val);
    }

    return ret;
}

void xdr12_trim_init_efuse(void)
{
    _v_xdr12_otp_access1_t* _r3A = &gt_xdr12_otp_ctrl_set_regs.reg._r3A;
    _r3A->bit.otp_pg_acc_cycle = 0x000U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_ACCESS1, _r3A->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_otp_access2_t* _r3B = &gt_xdr12_otp_ctrl_set_regs.reg._r3B;
    _r3B->bit.otp_pg_acc_cycle = 0x3FFU;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_ACCESS2, _r3B->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_otp_write_t* _r3C = &gt_xdr12_otp_ctrl_set_regs.reg._r3C;
    _r3C->bit.otp_wsel = 4U;
    _r3C->bit.otp_rd = 0U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_WRITE, _r3C->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_start_efuse(void)
{
    _v_xdr12_otp_rd_prog_t* _r3D = &gt_xdr12_otp_ctrl_set_regs.reg._r3D;
    _r3D->bit.otp_pg_s = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_RD_PROG, _r3D->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_trim_save_mirror_register(void)
{
    for (xd12_mirror_addr_t mirror_addr = XD12R_MIRROR1 ; mirror_addr < XD12R_MIRROR_MAX ; ++mirror_addr)
    {
        xdr12_read_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
    }
}

uint32_t xdr12_trim_verify_mirror_dump(void)
{
    uint32_t ret = 0U;
    for (xd12_mirror_addr_t mirror_addr = XD12R_MIRROR1 ; mirror_addr < XD12R_MIRROR_MAX ; ++mirror_addr)
    {
        uint16_t saved_reg = gt_xdr12_mirror_set_regs.ALL[mirror_addr];
        uint16_t read_reg = xdr12_read_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
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

void xdr12_test_init_icc_stby(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
}

void xdr12_test_init_icc_actv(void)
{
    /* change adc ch_p, ch_n */
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P, ADS114S08_CH_XD_ICC_N);
    /* set proper xdr12 register */
    xdr12_trim_init();
}

void xdr12_test_init_current_ref(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 0U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_ldo_dig(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 2U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_ldo_dac(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_ldo_fll(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_osc(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.mclk64_o = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    const uint16_t osc_fll_man = 0x8000U; /* 16b' 1000_0000_0000_0000*/

    _v_xdr12_osc_fll_man1_t* _r28 = &gt_xdr12_set_regs.reg._r28;
    _r28->bit.osc_fll_man = ((osc_fll_man & 0x0FFFU) >>  0U);
    xdr12_write_by_type(XD12R_OSC_FLL_MAN1, _r28->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_osc_fll_man2_t* _r29 = &gt_xdr12_set_regs.reg._r29;
    _r29->bit.osc_fll_man = ((osc_fll_man & 0xF000U) >> 12U);
    _r29->bit.osc_man_en = 1U;
    xdr12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_fll_40M(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.mclk64_o = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_osc_fll_man2_t* _r29 = &gt_xdr12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xdr12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 40000000.0f; /* 40MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / vsync + 0.5f); /* round up */

    _v_xdr12_fllcnt1_t* _r22 = &gt_xdr12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xdr12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_fllcnt2_t* _r23 = &gt_xdr12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    xdr12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_fll_50M(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.mclk64_o = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_osc_fll_man2_t* _r29 = &gt_xdr12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xdr12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 50000000.0f; /* 50MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / vsync + 0.5f); /* round up */

    _v_xdr12_fllcnt1_t* _r22 = &gt_xdr12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xdr12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_fllcnt2_t* _r23 = &gt_xdr12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    xdr12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_fll_60M(void)
{
    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.mclk64_o = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_osc_fll_man2_t* _r29 = &gt_xdr12_set_regs.reg._r29;
    _r29->bit.osc_man_en = 0U;
    xdr12_write_by_type(XD12R_OSC_FLL_MAN2, _r29->ALL, XD12R_ADDR_TYPE_GENERAL);

    const float xd_mclk = 60000000.0f; /* 60MHz */
    const float vsync = 120.0f; /* 120Hz */
    const uint32_t fll_out = (uint32_t)(xd_mclk / vsync + 0.5f); /* round up */

    _v_xdr12_fllcnt1_t* _r22 = &gt_xdr12_set_regs.reg._r22;
    _r22->bit.fllcnt = (uint16_t)((fll_out & 0x000FFFU) >>  0U);
    xdr12_write_by_type(XD12R_FLLCNT1, _r22->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_fllcnt2_t* _r23 = &gt_xdr12_set_regs.reg._r23;
    _r23->bit.fllcnt = (uint16_t)((fll_out & 0x1FF000U) >> 12U);
    xdr12_write_by_type(XD12R_FLLCNT2, _r23->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_iout_3P(void)
{
    // turn on proper power if needed like VLED
    gpio_set_vled_9v(VLED_ON);
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
    gpio_set_current_gain(GAIN_HIGH);

    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_max_current_level1_t* _r19 = &gt_xdr12_set_regs.reg._r19;
    _r19->bit.max_curr1_level = CURR_LEVEL_16;
    _r19->bit.max_curr2_level = CURR_LEVEL_16;
    _r19->bit.max_curr3_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL1, _r19->ALL, XD12R_ADDR_TYPE_GENERAL);

    _v_xdr12_max_current_level2_t* _r1A = &gt_xdr12_set_regs.reg._r1A;
    _r1A->bit.max_curr4_level = CURR_LEVEL_16;
    _r1A->bit.max_curr5_level = CURR_LEVEL_16;
    xdr12_write_by_type(XD12R_MAX_CURRENT_LEVEL2, _r1A->ALL, XD12R_ADDR_TYPE_GENERAL);
}

void xdr12_test_init_max_sweep(void)
{
    // turn on proper power if needed like VLED
    gpio_set_vled_9v(VLED_ON);
    // change adc ch_p, ch_n
    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT, ADS_AINCOM);
    gpio_set_current_gain(GAIN_HIGH);

    // set proper xdr12 register
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    _r3F->bit.test_en = 1U;
    _r3F->bit.test_ana_en = 5U; /* ??? */
    _r3F->bit.sw_sel = 1U;
    _r3F->bit.mclk64_o = 0U;
    _r3F->bit.pwmout_full = 1U;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OP_MODE, _r3F->ALL, XD12R_ADDR_TYPE_GENERAL);

    xdr12_trim_set_max_curr_vref(300U);
}

void xdr12_test_start_icc_stby(void)
{
    ADS114S08_Set_Start(true);
}
void xdr12_test_start_icc_actv(void)
{
    ADS114S08_Set_Start(true);
}
void xdr12_test_start_current_ref(void)
{
    mcu_peripheral_adc_start();
}
void xdr12_test_start_ldo_dig(void)
{
    mcu_peripheral_adc_start();
}
void xdr12_test_start_ldo_dac(void)
{
    mcu_peripheral_adc_start();
}
void xdr12_test_start_ldo_fll(void)
{
    mcu_peripheral_adc_start();
}
void xdr12_test_start_osc(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xdr12_test_start_fll_40M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xdr12_test_start_fll_50M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xdr12_test_start_fll_60M(void)
{
    mcu_peripheral_tim_input_capture_start();
}
void xdr12_test_start_iout_3P(void)
{
    ADS114S08_Set_Start(true);
}
void xdr12_test_start_max_sweep(void)
{
    ADS114S08_Set_Start(true);
}