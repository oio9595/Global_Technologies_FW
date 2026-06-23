#include <string.h>

#include "drv_xdr12.h"
#include "drv_xcr24.h"
#include "comm_debugging.h"

/* XDR/IC602 serializer protocol */
#define CMD_CODE_WRITE      (0x5U)  /* 0b101 */
#define CMD_CODE_READ       (0x6U)  /* 0b110 */
#define CMD_CODE_LDIM       (0x7U)  /* 0b111 */
#define CMD_CODE_RD_FAULT   (0x2U)  /* 0b010 */
#define CMD_CODE_SYNCGEN    (0x1U)  /* 0b001 */
#define CMD_CODE_IDGEN      (0x0U)  /* 0b000 */

typedef union tag_SERDES_WRITE_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t data       : 12;
        uint32_t addr       :  6;
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 10;   /* reserved */
    }bit;
}_v_serdes_write_command_t;

typedef union tag_SERDES_READ_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t addr       :  6;
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
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
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 11;   /* reserved */
    }bit;
}_v_serdes_readout_command_t;

typedef union tag_SERDES_LD_TRANSFER_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t ld         : 16;   /* LD * N ea, ch(ld)_size, 12bit for ld_width=0, 14bit ld_width=1 */
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 12;   /* reserved */
    }bit;
}_v_serdes_ld_transfer_command_t;

typedef union tag_SERDES_FAULT_READ_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
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
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 24;   /* reserved */
    }bit;
}_v_serdes_fault_readout_command_t;

typedef union tag_SERDES_SYNC_GEN_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 28;   /* reserved */
    }bit;
}_v_serdes_sync_gen_command_t;

typedef union tag_SERDES_ID_GEN_CMD
{
    uint32_t ALL;
    struct
    {
        uint32_t cmd_code   :  3;
        uint32_t start      :  1;
        uint32_t            : 28;   /* reserved */
    }bit;
}_v_serdes_id_gen_command_t;

#define XDR_BIT_SYNCGEN         (4U)
#define XD_HDR_BIT              (4U)
#define XD_ADDR_BIT             (6U)
#define XD_ID_BIT               (5U)
#define XD_DATA_BIT             (12U)
#define XDR_CMD_WRITE           (XD_HDR_BIT + XD_ADDR_BIT + XD_DATA_BIT)
#define XDR_CMD_READ            (XD_HDR_BIT + XD_ADDR_BIT)
#define XDR_CMD_READOUT         (XD_HDR_BIT + XD_ID_BIT + XD_DATA_BIT)

#define XD_LD_DATA_BIT          (16U)
#define XDR_LD_TRANSFER         (XD_HDR_BIT + (XD_LD_DATA_BIT * XDR_CH_LENGTH))

#define PWM_OUT_BIT0            (((TIM1_PERIOD + 1U) * 1) / 3U)
#define PWM_OUT_BIT1            (((TIM1_PERIOD + 1U) * 2) / 3U)

#define PWM_OUT_HEADER_SIZE     (0U)
#define PWM_OUT_TAIL_SIZE       (1U)
#define PWM_OUT_DUMMY_SIZE      (PWM_OUT_HEADER_SIZE + PWM_OUT_TAIL_SIZE)

#define XDR_CONTROLLED_MCU      (0U)
#define XDR_CONTROLLED_XCR      (1U)
#define XDR_CONTROL_TYPE        (XDR_CONTROLLED_MCU)

#if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    static _xdr12_regs_t gt_xdr12_set_regs;
    static _xdr12_regs_t gt_xdr12_get_regs[XDR_DAISY_LENGTH];

    static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_set_regs;
    static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_get_regs[XDR_DAISY_LENGTH];

    static _xdr12_mirror_regs_t gt_xdr12_mirror_set_regs;
    static _xdr12_mirror_regs_t gt_xdr12_mirror_get_regs[XDR_DAISY_LENGTH];
#else
    static _xdr12_regs_t gt_xdr12_set_regs;
    static _xdr12_regs_t gt_xdr12_get_regs[XCR_CH_SIZE][XDR_DAISY_LENGTH];

    static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_set_regs;
    static _xdr12_otp_ctrl_regs_t gt_xdr12_otp_ctrl_get_regs[XCR_CH_SIZE][XDR_DAISY_LENGTH];

    static _xdr12_mirror_regs_t gt_xdr12_mirror_set_regs;
    static _xdr12_mirror_regs_t gt_xdr12_mirror_get_regs[XCR_CH_SIZE][XDR_DAISY_LENGTH];
#endif

static uint16_t gn_pwm_out_xd_syncgen[(XDR_DAISY_LENGTH * XDR_BIT_SYNCGEN) + PWM_OUT_DUMMY_SIZE];
static uint16_t gn_pwm_out_xd_write[(XDR_DAISY_LENGTH * XDR_CMD_WRITE) + PWM_OUT_DUMMY_SIZE];
static uint16_t gn_pwm_out_xd_ld_transfer[(XDR_DAISY_LENGTH * XDR_LD_TRANSFER) + PWM_OUT_DUMMY_SIZE];

static uint16_t gn_pwm_in_xd_response_freq[(XDR_DAISY_LENGTH * XDR_CMD_READOUT) + 1U];
static uint16_t gn_pwm_in_xd_response_duty[(XDR_DAISY_LENGTH * XDR_CMD_READOUT) + 1U];

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

static void xdr12_trim_enable_write_protect(bool protect_en);

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

    BUFFER_OE_HI();

    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, length);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, length);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
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

    const uint32_t avg_freq = sum_freq / count;
    const uint16_t logic_1_min = (uint16_t)(avg_freq >> 1U);
    const uint16_t logic_1_max = (uint16_t)((avg_freq * 84U) / 100U);

    uint16_t id = 0;
    uint16_t header[XDR_DAISY_LENGTH] = {0, };

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
        header[id] = n_header;

        /* 4bit */
        n_data  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
        n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);

        gt_xd_fault_readout_command.bit.bit_thermal = 0U;
        gt_xd_fault_readout_command.bit.bit_short = 0U;
        gt_xd_fault_readout_command.bit.bit_open = 0U;
        gt_xd_fault_readout_command.bit.bit_fb = 0U;

        pdata[id++] = n_data;
        comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Fault Recv Packet\r\n\tCMD - 0x%01X, FAULT - 0x%01X", n_header, n_data);
    }
    else        /* read receive format : '1110' + id[4:0] + data[11:0] */
    {
        uint16_t duty_idx = 0;
        uint16_t n_id = 0U;
        uint16_t xdr12_id[XDR_DAISY_LENGTH] = {0, };

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
            header[id] = n_header;

            /* 5bit */
            n_id  = (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 4U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 3U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 2U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 1U);
            n_id |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << 0U);
            xdr12_id[id] = n_id;

            /* 12bit */
            for(int16_t bit = 11 ; bit >= 0 ; --bit)
            {
                n_data |= (DECODE_BIT(*p_src++, logic_1_min, logic_1_max) << (uint16_t)bit);
            }

            gt_xd_readout_command[id].bit.id = n_id;
            gt_xd_readout_command[id].bit.data = n_data;

            pdata[id++] = n_data;
            duty_idx += XDR_CMD_READOUT;
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Fault Recv Packet\r\n\tCMD - 0x%01X, ID - 0x%02X, DATA - 0x%01X", n_header, n_id, n_data);
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
#if (XD_MODEL_TYPE == XD_TYPE_XDR12R)
        case XD12R_RESET_ID:
            _r1->reg._r00.bit.lkg_e = 0U;
            _r1->reg._r00.bit.e_rst = 0U;
            _r1->reg._r00.bit.vs_rst = 0U;
            _r1->reg._r00.bit.rst = 0U;
            break;
        case XD12R_LD_CONTROL:
            _r1->reg._r01.bit.ld_mode = 0U;
            _r1->reg._r01.bit.ld_dir = 0U;
            _r1->reg._r01.bit.ld_res = 0U;
            _r1->reg._r01.bit.syncmode = 0U;
            _r1->reg._r01.bit.delay_mode_en = 0U;
            _r1->reg._r01.bit.sv_no = XDR_SV_NO;
            break;
        case XD12R_LD_SIZE:
            _r1->reg._r02.bit.ld_size = 0U;
            break;
        case XD12R_PWMCLK_DIV1_2:
            _r1->reg._r03.bit.pwmclk_div1 = 0U;
            _r1->reg._r03.bit.pwmclk_div2 = 0U;
            break;
        case XD12R_PWMCLK_DIV2_3:
            _r1->reg._r04.bit.pwmclk_div2 = 0U;
            _r1->reg._r04.bit.pwmclk_div3 = 0U;
            break;
        case XD12R_CHANNEL_ENABLE:
            _r1->reg._r05.bit.ch1_en = 0U;
            _r1->reg._r05.bit.ch2_en = 0U;
            _r1->reg._r05.bit.ch3_en = 0U;
            _r1->reg._r05.bit.ch4_en = 0U;
            _r1->reg._r05.bit.ch5_en = 0U;
            _r1->reg._r05.bit.ch6_en = 0U;
            _r1->reg._r05.bit.ch7_en = 0U;
            _r1->reg._r05.bit.ch8_en = 0U;
            _r1->reg._r05.bit.ch9_en = 0U;
            _r1->reg._r05.bit.ch10_en = 0U;
            _r1->reg._r05.bit.ch11_en = 0U;
            _r1->reg._r05.bit.ch12_en = 0U;
            break;
        case XD12R_FAULT_CONTROL:
            _r1->reg._r06.bit.o_off_e = 0U;
            _r1->reg._r06.bit.s_off_e = 0U;
            _r1->reg._r06.bit.t_off_e = 0U;
            _r1->reg._r06.bit.o_det_e = 0U;
            _r1->reg._r06.bit.s_det_e = 0U;
            _r1->reg._r06.bit.o_fb_e = 0U;
            _r1->reg._r06.bit.fb_mode = 0U;
            _r1->reg._r06.bit.auto_fault_fb_no = 0U;
            break;
        case XD12R_CHx_LD_TYPE0:
            _r1->reg._r07.bit.ch7_ld_type = 0U;
            _r1->reg._r07.bit.ch8_ld_type = 0U;
            _r1->reg._r07.bit.ch9_ld_type = 0U;
            _r1->reg._r07.bit.ch10_ld_type = 0U;
            _r1->reg._r07.bit.ch11_ld_type = 0U;
            _r1->reg._r07.bit.ch12_ld_type = 0U;
            break;
        case XD12R_CHx_LD_TYPE1:
            _r1->reg._r08.bit.ch1_ld_type = 0U;
            _r1->reg._r08.bit.ch2_ld_type = 0U;
            _r1->reg._r08.bit.ch3_ld_type = 0U;
            _r1->reg._r08.bit.ch4_ld_type = 0U;
            _r1->reg._r08.bit.ch5_ld_type = 0U;
            _r1->reg._r08.bit.ch6_ld_type = 0U;
            break;
        //case XD12R_FAULT_STATUS0:
        case XD12R_MAX_CURR_VREF1:
            _r1->reg._r0A.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF2:
            _r1->reg._r0B.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF3:
            _r1->reg._r0C.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF4:
            _r1->reg._r0D.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF5:
            _r1->reg._r0E.bit.max_curr_vref = 0U;
            break;
        case XD12R_DELAY_CH1_2:
            _r1->reg._r10.bit.delay_ch1 = 0U;   /* CH1 */
            _r1->reg._r10.bit.delay_ch2 = 0U;   /* CH2 */
            break;
        case XD12R_DELAY_CH3_4:
            _r1->reg._r11.bit.delay_ch3 = 0U;   /* CH3 */
            _r1->reg._r11.bit.delay_ch4 = 0U;   /* CH4 */
            break;
        case XD12R_DELAY_CH5_6:
            _r1->reg._r12.bit.delay_ch5 = 0U;   /* CH5 */
            _r1->reg._r12.bit.delay_ch6 = 0U;   /* CH6 */
            break;
        case XD12R_DELAY_CH7_8:
            _r1->reg._r13.bit.delay_ch7 = 0U;   /* CH7 */
            _r1->reg._r13.bit.delay_ch8 = 0U;   /* CH8 */
            break;
        case XD12R_DELAY_CH9_10:
            _r1->reg._r14.bit.delay_ch9 = 0U;   /* CH9 */
            _r1->reg._r14.bit.delay_ch10 = 0U;   /* CH10 */
            break;
        case XD12R_DELAY_CH11_12:
            _r1->reg._r15.bit.delay_ch11 = 0U;   /* CH11 */
            _r1->reg._r15.bit.delay_ch12 = 0U;   /* CH12 */
            break;
        case XD12R_FB_LEVEL:
            _r1->reg._r16.bit.fb1_level = 0U;
            _r1->reg._r16.bit.fb2_level = 0U;
            _r1->reg._r16.bit.fb3_level = 0U;
            _r1->reg._r16.bit.fb4_level = 0U;
            break;
        case XD12R_FB_SHORT_LEVEL:
            _r1->reg._r17.bit.fb5_level = 0U;
            _r1->reg._r17.bit.short1_level = 0U;
            _r1->reg._r17.bit.short2_level = 0U;
            _r1->reg._r17.bit.short3_level = 0U;
            break;
        case XD12R_SHORT_LEVEL:
            _r1->reg._r18.bit.short4_level = 0U;
            _r1->reg._r18.bit.short5_level = 0U;
            break;
        case XD12R_MAX_CURRENT_LEVEL1:
            _r1->reg._r19.bit.max_curr1_level = 0U;
            _r1->reg._r19.bit.max_curr2_level = 0U;
            _r1->reg._r19.bit.max_curr3_level = 0U;
            break;
        case XD12R_MAX_CURRENT_LEVEL2:
            _r1->reg._r1A.bit.max_curr4_level = 0U;
            _r1->reg._r1A.bit.max_curr5_level = 0U;
            break;
        case XD12R_PARITY_RD_EN:
            _r1->reg._r1B.bit.reg_rd_en = 0U;
            _r1->reg._r1B.bit.parity_e = 0U;
            break;
        case XD12R_SERIAL_CLK_GEN:
            _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
            _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
            break;
        case XD12R_SERIAL_LATENCY:
            _r1->reg._r1D.bit.serial_latency = 128U;
            break;
        case XD12R_V_MASK:
            _r1->reg._r1E.bit.v_mask = 0U;
            break;
        case XD12R_SV_MASK:
            _r1->reg._r1F.bit.sv_mask = 0U;
            _r1->reg._r1F.bit.sv_mask_en = 0U;
            break;
        case XD12R_RSTCNT:
            _r1->reg._r20.bit.rstcnt = 0U;
            break;
        case XD12R_TIMEOUT:
            _r1->reg._r21.bit.timeout = 1023U;
            break;
        case XD12R_FLLCNT1:
            _r1->reg._r22.bit.fllcnt = 0U;
            break;
        case XD12R_FLLCNT2:
            _r1->reg._r23.bit.fllcnt = 50U;
            _r1->reg._r23.bit.fll_range = 2U;
            _r1->reg._r23.bit.fll_en = 1U;
            break;
        case XD12R_WR_PROTECT:
            _r1->reg._r24.bit.wr_protect = 0x555U;
            break;
        case XD12R_NF_CONTROL:
            _r1->reg._r25.bit.DGRJT_EN1 = 0U;
            _r1->reg._r25.bit.DGRJT_EN2 = 0U;
            _r1->reg._r25.bit.BBKN_EN = 0U;
            _r1->reg._r25.bit.SGRJT_EN1 = 0U;
            _r1->reg._r25.bit.SGRJT_EN2 = 0U;
            _r1->reg._r25.bit.O_EMI_REJ_EN = 0U;
            _r1->reg._r25.bit.BBKN_TH = 0U;
            break;
        case XD12R_CHOP_EN:
            _r1->reg._r26.bit.CHOP_BGR_EN = 1U;
            _r1->reg._r26.bit.CHOP_DAC_EN = 1U;
            _r1->reg._r26.bit.CHOP_OSC_EN = 1U;
            _r1->reg._r26.bit.CHOP_OSCLDO_EN = 1U;
            _r1->reg._r26.bit.CHOP_DRV_EN = 1U;
            _r1->reg._r26.bit.CHOP_EN = 1U;
            break;
        case XD12R_TEMP:
            _r1->reg._r27.bit.flt_gain = 1U;
            _r1->reg._r27.bit.o_slew = 2U;
            _r1->reg._r27.bit.flt_ctl = 2U;
            _r1->reg._r27.bit.dac_rng = 0U;
            _r1->reg._r27.bit.ov_swap_en = 0U;
            _r1->reg._r27.bit.ofs_temp = 8U;
            break;
        case XD12R_OSC_FLL_MAN1:
            _r1->reg._r28.bit.osc_fll_man = 0U;
            break;
        case XD12R_OSC_FLL_MAN2:
            _r1->reg._r29.bit.osc_fll_man = 8U;
            _r1->reg._r29.bit.osc_fll_err_range = 0U;
            _r1->reg._r29.bit.osc_man_en = 1U;
            break;
        case XD12R_OSC_SPREAD:
            _r1->reg._r2A.bit.SPRD_GAIN = 0U;
            _r1->reg._r2A.bit.SPRD_SPD = 0U;
            _r1->reg._r2A.bit.SPRD_EN = 0U;
            break;
        case XD12R_CLOCK_GATE_EN:
            _r1->reg._r2B.bit.DC_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR1_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR2_MCLK_EN = 1U;
            _r1->reg._r2B.bit.OTP_MCLK_EN = 1U;
            break;

#elif (XD_MODEL_TYPE == XD_TYPE_XDR12D)

        case XD12R_RESET_ID:
            _r1->reg._r00.bit.rsi_e = 0U;
            _r1->reg._r00.bit.lkg_e = 0U;
            _r1->reg._r00.bit.e_rst = 0U;
            _r1->reg._r00.bit.vs_rst = 0U;
            _r1->reg._r00.bit.rst = 0U;
            break;
        case XD12R_LD_CONTROL:
            _r1->reg._r01.bit.ld_mode = 0U;
            _r1->reg._r01.bit.ld_dir = 0U;
            _r1->reg._r01.bit.ld_res = 0U;
            _r1->reg._r01.bit.ov_t_e = 0;
            _r1->reg._r01.bit.pwmclk_div = 0;
            break;
        case XD12R_LD_SIZE:
            _r1->reg._r02.bit.ld_size = 0U;
            break;
        case XD12R_CHANNEL_ENABLE:
            _r1->reg._r05.bit.ch1_en = 0U;
            _r1->reg._r05.bit.ch2_en = 0U;
            _r1->reg._r05.bit.ch3_en = 0U;
            _r1->reg._r05.bit.ch4_en = 0U;
            _r1->reg._r05.bit.ch5_en = 0U;
            _r1->reg._r05.bit.ch6_en = 0U;
            _r1->reg._r05.bit.ch7_en = 0U;
            _r1->reg._r05.bit.ch8_en = 0U;
            _r1->reg._r05.bit.ch9_en = 0U;
            _r1->reg._r05.bit.ch10_en = 0U;
            _r1->reg._r05.bit.ch11_en = 0U;
            _r1->reg._r05.bit.ch12_en = 0U;
            break;
        case XD12R_FAULT_CONTROL:
            _r1->reg._r06.bit.o_off_e = 0U;
            _r1->reg._r06.bit.s_off_e = 0U;
            _r1->reg._r06.bit.t_off_e = 0U;
            _r1->reg._r06.bit.o_det_e = 0U;
            _r1->reg._r06.bit.s_det_e = 0U;
            _r1->reg._r06.bit.o_fb_e = 0U;
            _r1->reg._r06.bit.fb_mode = 0U;
            _r1->reg._r06.bit.o_slew = 0U;
            _r1->reg._r06.bit.parity_e = 0U;
            break;
        case XD12R_FB_LEVEL:
            _r1->reg._r07.bit.fb1_level = 0U;
            _r1->reg._r07.bit.fb2_level = 0U;
            _r1->reg._r07.bit.fb3_level = 0U;
            break;
        case XD12R_SHORT_LEVEL:
            _r1->reg._r08.bit.short1_level = 0U;
            _r1->reg._r08.bit.short2_level = 0U;
            _r1->reg._r08.bit.short3_level = 0U;
            break;

        //case XD12R_FAULT_STATUS0:
        case XD12R_MAX_CURR_LEVEL:
            _r1->reg._r0A.bit.max_curr1_level = 0U;
            _r1->reg._r0A.bit.max_curr2_level = 0U;
            _r1->reg._r0A.bit.max_curr3_level = 0U;
            break;
        case XD12R_MAX_CURR_VREF1:
            _r1->reg._r0B.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF2:
            _r1->reg._r0C.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF3:
            _r1->reg._r0D.bit.max_curr_vref = 0U;
            break;
        case XD12R_CHx_LD_TYPE1:
            _r1->reg._r0E.bit.ch7_ld_type = 0U;
            _r1->reg._r0E.bit.ch8_ld_type = 0U;
            _r1->reg._r0E.bit.ch9_ld_type = 0U;
            _r1->reg._r0E.bit.ch10_ld_type = 0U;
            _r1->reg._r0E.bit.ch11_ld_type = 0U;
            _r1->reg._r0E.bit.ch12_ld_type = 0U;
            break;
        case XD12R_CHx_LD_TYPE2:
            _r1->reg._r0F.bit.ch1_ld_type = 0U;
            _r1->reg._r0F.bit.ch2_ld_type = 0U;
            _r1->reg._r0F.bit.ch3_ld_type = 0U;
            _r1->reg._r0F.bit.ch4_ld_type = 0U;
            _r1->reg._r0F.bit.ch5_ld_type = 0U;
            _r1->reg._r0F.bit.ch6_ld_type = 0U;
            break;
        case XD12R_DELAY_CH1:
            _r1->reg._r10.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH2:
            _r1->reg._r11.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH3:
            _r1->reg._r12.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH4:
            _r1->reg._r13.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH5:
            _r1->reg._r14.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH6:
            _r1->reg._r15.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH7:
            _r1->reg._r16.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH8:
            _r1->reg._r17.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH9:
            _r1->reg._r18.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH10:
            _r1->reg._r19.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH11:
            _r1->reg._r1A.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH12:
            _r1->reg._r1B.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_SERIAL_CLK_GEN:
            _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
            _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
            break;
        case XD12R_SERIAL_LATENCY:
            _r1->reg._r1D.bit.serial_latency = 128U;
            break;

        case XD12R_RSTCNT:
            _r1->reg._r20.bit.rstcnt = 0U;
            break;
        case XD12R_TIMEOUT:
            _r1->reg._r21.bit.timeout = 1023U;
            break;
        case XD12R_FLLCNT1:
            _r1->reg._r22.bit.fllcnt = 0U;
            break;
        case XD12R_FLLCNT2:
            _r1->reg._r23.bit.fllcnt = 50U;
            _r1->reg._r23.bit.fll_range = 2U;
            _r1->reg._r23.bit.fll_en = 1U;
            break;
        case XD12R_WR_PROTECT:
            _r1->reg._r24.bit.wr_protect = 0x555U;
            break;
        case XD12R_NF_CONTROL:
            _r1->reg._r25.bit.DGRJT_EN1 = 0U;
            _r1->reg._r25.bit.DGRJT_EN2 = 0U;
            _r1->reg._r25.bit.BBKN_EN = 0U;
            _r1->reg._r25.bit.BBKN_TH = 0U;
            break;
        case XD12R_CHOP_EN:
            _r1->reg._r26.bit.CHOP_BGR_EN = 0U;
            _r1->reg._r26.bit.CHOP_DAC_EN = 0U;
            _r1->reg._r26.bit.CHOP_OSC_EN = 0U;
            _r1->reg._r26.bit.CHOP_OSCLDO_EN = 0U;
            _r1->reg._r26.bit.CHOP_DRV_EN = 0U;
            _r1->reg._r26.bit.CHOP_EN = 0U;
            break;
        case XD12R_TEMP:
            _r1->reg._r27.bit.flt_gain = 1U;
            _r1->reg._r27.bit.o_slew = 2U;
            _r1->reg._r27.bit.flt_ctl = 2U;
            _r1->reg._r27.bit.dac_rng = 0U;
            _r1->reg._r27.bit.ofs_temp = 8U;
            break;
        case XD12R_OSC_FLL_MAN1:
            _r1->reg._r28.bit.osc_fll_man = 0U;
            break;
        case XD12R_OSC_FLL_MAN2:
            _r1->reg._r29.bit.osc_fll_man = 8U;
            _r1->reg._r29.bit.osc_fll_err_range = 0U;
            _r1->reg._r29.bit.osc_man_en = 1U;
            break;
        case XD12R_OSC_SPREAD:
            _r1->reg._r2A.bit.SPRD_GAIN = 0U;
            _r1->reg._r2A.bit.SPRD_SPD = 0U;
            _r1->reg._r2A.bit.SPRD_EN = 0U;
            break;
        case XD12R_CLOCK_GATE_EN:
            _r1->reg._r2B.bit.DC_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR1_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR2_MCLK_EN = 1U;
            _r1->reg._r2B.bit.OTP_MCLK_EN = 1U;
            break;

#elif (XD_MODEL_TYPE == XD_TYPE_IC601)
#endif

        default:
            continue;
        }

        xdr12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
}

static void xdr12_regs_trim_init_table(void)
{
    _xdr12_regs_t* _r1 = &gt_xdr12_set_regs;
    _xdr12_mirror_regs_t* _r2 = &gt_xdr12_mirror_set_regs;
    _xdr12_otp_ctrl_regs_t* _r3 = &gt_xdr12_otp_ctrl_set_regs;

    for(xd12_addr_t addr = XD12R_RESET_ID ; addr < XD12R_MAX ; ++addr)
    {
        switch(addr)
        {
#if (XD_MODEL_TYPE == XD_TYPE_XDR12R)
        case XD12R_RESET_ID:
            _r1->reg._r00.bit.lkg_e = 0U;
            _r1->reg._r00.bit.e_rst = 0U;
            _r1->reg._r00.bit.vs_rst = 0U;
            _r1->reg._r00.bit.rst = 0U;
            break;
        case XD12R_LD_CONTROL:
            _r1->reg._r01.bit.ld_mode = 0U;
            _r1->reg._r01.bit.ld_dir = 0U;
            _r1->reg._r01.bit.ld_res = 0U;
            _r1->reg._r01.bit.syncmode = 0U;
            _r1->reg._r01.bit.delay_mode_en = 0U;
            _r1->reg._r01.bit.sv_no = XDR_SV_NO;
            break;
        case XD12R_LD_SIZE:
            _r1->reg._r02.bit.ld_size = 0U;
            break;
        case XD12R_PWMCLK_DIV1_2:
            _r1->reg._r03.bit.pwmclk_div1 = 0U;
            _r1->reg._r03.bit.pwmclk_div2 = 0U;
            break;
        case XD12R_PWMCLK_DIV2_3:
            _r1->reg._r04.bit.pwmclk_div2 = 0U;
            _r1->reg._r04.bit.pwmclk_div3 = 0U;
            break;
        case XD12R_CHANNEL_ENABLE:
            _r1->reg._r05.bit.ch1_en = 0U;
            _r1->reg._r05.bit.ch2_en = 0U;
            _r1->reg._r05.bit.ch3_en = 0U;
            _r1->reg._r05.bit.ch4_en = 0U;
            _r1->reg._r05.bit.ch5_en = 0U;
            _r1->reg._r05.bit.ch6_en = 0U;
            _r1->reg._r05.bit.ch7_en = 0U;
            _r1->reg._r05.bit.ch8_en = 0U;
            _r1->reg._r05.bit.ch9_en = 0U;
            _r1->reg._r05.bit.ch10_en = 0U;
            _r1->reg._r05.bit.ch11_en = 0U;
            _r1->reg._r05.bit.ch12_en = 0U;
            break;
        case XD12R_FAULT_CONTROL:
            _r1->reg._r06.bit.o_off_e = 0U;
            _r1->reg._r06.bit.s_off_e = 0U;
            _r1->reg._r06.bit.t_off_e = 0U;
            _r1->reg._r06.bit.o_det_e = 0U;
            _r1->reg._r06.bit.s_det_e = 0U;
            _r1->reg._r06.bit.o_fb_e = 0U;
            _r1->reg._r06.bit.fb_mode = 0U;
            _r1->reg._r06.bit.auto_fault_fb_no = 0U;
            break;
        case XD12R_CHx_LD_TYPE0:
            _r1->reg._r07.bit.ch7_ld_type = 0U;
            _r1->reg._r07.bit.ch8_ld_type = 0U;
            _r1->reg._r07.bit.ch9_ld_type = 0U;
            _r1->reg._r07.bit.ch10_ld_type = 0U;
            _r1->reg._r07.bit.ch11_ld_type = 0U;
            _r1->reg._r07.bit.ch12_ld_type = 0U;
            break;
        case XD12R_CHx_LD_TYPE1:
            _r1->reg._r08.bit.ch1_ld_type = 0U;
            _r1->reg._r08.bit.ch2_ld_type = 0U;
            _r1->reg._r08.bit.ch3_ld_type = 0U;
            _r1->reg._r08.bit.ch4_ld_type = 0U;
            _r1->reg._r08.bit.ch5_ld_type = 0U;
            _r1->reg._r08.bit.ch6_ld_type = 0U;
            break;
        //case XD12R_FAULT_STATUS0:
        case XD12R_MAX_CURR_VREF1:
            _r1->reg._r0A.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF2:
            _r1->reg._r0B.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF3:
            _r1->reg._r0C.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF4:
            _r1->reg._r0D.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF5:
            _r1->reg._r0E.bit.max_curr_vref = 0U;
            break;
        case XD12R_DELAY_CH1_2:
            _r1->reg._r10.bit.delay_ch1 = 0U;   /* CH1 */
            _r1->reg._r10.bit.delay_ch2 = 0U;   /* CH2 */
            break;
        case XD12R_DELAY_CH3_4:
            _r1->reg._r11.bit.delay_ch3 = 0U;   /* CH3 */
            _r1->reg._r11.bit.delay_ch4 = 0U;   /* CH4 */
            break;
        case XD12R_DELAY_CH5_6:
            _r1->reg._r12.bit.delay_ch5 = 0U;   /* CH5 */
            _r1->reg._r12.bit.delay_ch6 = 0U;   /* CH6 */
            break;
        case XD12R_DELAY_CH7_8:
            _r1->reg._r13.bit.delay_ch7 = 0U;   /* CH7 */
            _r1->reg._r13.bit.delay_ch8 = 0U;   /* CH8 */
            break;
        case XD12R_DELAY_CH9_10:
            _r1->reg._r14.bit.delay_ch9 = 0U;   /* CH9 */
            _r1->reg._r14.bit.delay_ch10 = 0U;   /* CH10 */
            break;
        case XD12R_DELAY_CH11_12:
            _r1->reg._r15.bit.delay_ch11 = 0U;   /* CH11 */
            _r1->reg._r15.bit.delay_ch12 = 0U;   /* CH12 */
            break;
        case XD12R_FB_LEVEL:
            _r1->reg._r16.bit.fb1_level = 0U;
            _r1->reg._r16.bit.fb2_level = 0U;
            _r1->reg._r16.bit.fb3_level = 0U;
            _r1->reg._r16.bit.fb4_level = 0U;
            break;
        case XD12R_FB_SHORT_LEVEL:
            _r1->reg._r17.bit.fb5_level = 0U;
            _r1->reg._r17.bit.short1_level = 0U;
            _r1->reg._r17.bit.short2_level = 0U;
            _r1->reg._r17.bit.short3_level = 0U;
            break;
        case XD12R_SHORT_LEVEL:
            _r1->reg._r18.bit.short4_level = 0U;
            _r1->reg._r18.bit.short5_level = 0U;
            break;
        case XD12R_MAX_CURRENT_LEVEL1:
            _r1->reg._r19.bit.max_curr1_level = 0U;
            _r1->reg._r19.bit.max_curr2_level = 0U;
            _r1->reg._r19.bit.max_curr3_level = 0U;
            break;
        case XD12R_MAX_CURRENT_LEVEL2:
            _r1->reg._r1A.bit.max_curr4_level = 0U;
            _r1->reg._r1A.bit.max_curr5_level = 0U;
            break;
        case XD12R_PARITY_RD_EN:
            _r1->reg._r1B.bit.reg_rd_en = 0U;
            _r1->reg._r1B.bit.parity_e = 0U;
            break;
        case XD12R_SERIAL_CLK_GEN:
            _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
            _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
            break;
        case XD12R_SERIAL_LATENCY:
            _r1->reg._r1D.bit.serial_latency = 128U;
            break;
        case XD12R_V_MASK:
            _r1->reg._r1E.bit.v_mask = 0U;
            break;
        case XD12R_SV_MASK:
            _r1->reg._r1F.bit.sv_mask = 0U;
            _r1->reg._r1F.bit.sv_mask_en = 0U;
            break;
        case XD12R_RSTCNT:
            _r1->reg._r20.bit.rstcnt = 0U;
            break;
        case XD12R_TIMEOUT:
            _r1->reg._r21.bit.timeout = 1023U;
            break;
        case XD12R_FLLCNT1:
            _r1->reg._r22.bit.fllcnt = 0U;
            break;
        case XD12R_FLLCNT2:
            _r1->reg._r23.bit.fllcnt = 50U;
            _r1->reg._r23.bit.fll_range = 2U;
            _r1->reg._r23.bit.fll_en = 1U;
            break;
        case XD12R_WR_PROTECT:
            _r1->reg._r24.bit.wr_protect = 0x555U;
            break;
        case XD12R_NF_CONTROL:
            _r1->reg._r25.bit.DGRJT_EN1 = 0U;
            _r1->reg._r25.bit.DGRJT_EN2 = 0U;
            _r1->reg._r25.bit.BBKN_EN = 0U;
            _r1->reg._r25.bit.SGRJT_EN1 = 0U;
            _r1->reg._r25.bit.SGRJT_EN2 = 0U;
            _r1->reg._r25.bit.O_EMI_REJ_EN = 0U;
            _r1->reg._r25.bit.BBKN_TH = 0U;
            break;
        case XD12R_CHOP_EN:
            _r1->reg._r26.bit.CHOP_BGR_EN = 1U;
            _r1->reg._r26.bit.CHOP_DAC_EN = 1U;
            _r1->reg._r26.bit.CHOP_OSC_EN = 1U;
            _r1->reg._r26.bit.CHOP_OSCLDO_EN = 1U;
            _r1->reg._r26.bit.CHOP_DRV_EN = 1U;
            _r1->reg._r26.bit.CHOP_EN = 1U;
            break;
        case XD12R_TEMP:
            _r1->reg._r27.bit.flt_gain = 1U;
            _r1->reg._r27.bit.o_slew = 2U;
            _r1->reg._r27.bit.flt_ctl = 2U;
            _r1->reg._r27.bit.dac_rng = 0U;
            _r1->reg._r27.bit.ov_swap_en = 0U;
            _r1->reg._r27.bit.ofs_temp = 8U;
            break;
        case XD12R_OSC_FLL_MAN1:
            _r1->reg._r28.bit.osc_fll_man = 0U;
            break;
        case XD12R_OSC_FLL_MAN2:
            _r1->reg._r29.bit.osc_fll_man = 8U;
            _r1->reg._r29.bit.osc_fll_err_range = 0U;
            _r1->reg._r29.bit.osc_man_en = 1U;
            break;
        case XD12R_OSC_SPREAD:
            _r1->reg._r2A.bit.SPRD_GAIN = 0U;
            _r1->reg._r2A.bit.SPRD_SPD = 0U;
            _r1->reg._r2A.bit.SPRD_EN = 0U;
            break;
        case XD12R_CLOCK_GATE_EN:
            _r1->reg._r2B.bit.DC_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR1_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR2_MCLK_EN = 1U;
            _r1->reg._r2B.bit.OTP_MCLK_EN = 1U;
            break;

#elif (XD_MODEL_TYPE == XD_TYPE_XDR12D)

        case XD12R_RESET_ID:
            _r1->reg._r00.bit.rsi_e = 0U;
            _r1->reg._r00.bit.lkg_e = 0U;
            _r1->reg._r00.bit.e_rst = 0U;
            _r1->reg._r00.bit.vs_rst = 0U;
            _r1->reg._r00.bit.rst = 0U;
            break;
        case XD12R_LD_CONTROL:
            _r1->reg._r01.bit.ld_mode = 0U;
            _r1->reg._r01.bit.ld_dir = 0U;
            _r1->reg._r01.bit.ld_res = 0U;
            _r1->reg._r01.bit.ov_t_e = 0;
            _r1->reg._r01.bit.pwmclk_div = 0;
            break;
        case XD12R_LD_SIZE:
            _r1->reg._r02.bit.ld_size = 0U;
            break;
        case XD12R_CHANNEL_ENABLE:
            _r1->reg._r05.bit.ch1_en = 0U;
            _r1->reg._r05.bit.ch2_en = 0U;
            _r1->reg._r05.bit.ch3_en = 0U;
            _r1->reg._r05.bit.ch4_en = 0U;
            _r1->reg._r05.bit.ch5_en = 0U;
            _r1->reg._r05.bit.ch6_en = 0U;
            _r1->reg._r05.bit.ch7_en = 0U;
            _r1->reg._r05.bit.ch8_en = 0U;
            _r1->reg._r05.bit.ch9_en = 0U;
            _r1->reg._r05.bit.ch10_en = 0U;
            _r1->reg._r05.bit.ch11_en = 0U;
            _r1->reg._r05.bit.ch12_en = 0U;
            break;
        case XD12R_FAULT_CONTROL:
            _r1->reg._r06.bit.o_off_e = 0U;
            _r1->reg._r06.bit.s_off_e = 0U;
            _r1->reg._r06.bit.t_off_e = 0U;
            _r1->reg._r06.bit.o_det_e = 0U;
            _r1->reg._r06.bit.s_det_e = 0U;
            _r1->reg._r06.bit.o_fb_e = 0U;
            _r1->reg._r06.bit.fb_mode = 0U;
            _r1->reg._r06.bit.o_slew = 0U;
            _r1->reg._r06.bit.parity_e = 0U;
            break;
        case XD12R_FB_LEVEL:
            _r1->reg._r07.bit.fb1_level = 0U;
            _r1->reg._r07.bit.fb2_level = 0U;
            _r1->reg._r07.bit.fb3_level = 0U;
            break;
        case XD12R_SHORT_LEVEL:
            _r1->reg._r08.bit.short1_level = 0U;
            _r1->reg._r08.bit.short2_level = 0U;
            _r1->reg._r08.bit.short3_level = 0U;
            break;

        //case XD12R_FAULT_STATUS0:
        case XD12R_MAX_CURR_LEVEL:
            _r1->reg._r0A.bit.max_curr1_level = 0U;
            _r1->reg._r0A.bit.max_curr2_level = 0U;
            _r1->reg._r0A.bit.max_curr3_level = 0U;
            break;
        case XD12R_MAX_CURR_VREF1:
            _r1->reg._r0B.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF2:
            _r1->reg._r0C.bit.max_curr_vref = 0U;
            break;
        case XD12R_MAX_CURR_VREF3:
            _r1->reg._r0D.bit.max_curr_vref = 0U;
            break;
        case XD12R_CHx_LD_TYPE1:
            _r1->reg._r0E.bit.ch7_ld_type = 0U;
            _r1->reg._r0E.bit.ch8_ld_type = 0U;
            _r1->reg._r0E.bit.ch9_ld_type = 0U;
            _r1->reg._r0E.bit.ch10_ld_type = 0U;
            _r1->reg._r0E.bit.ch11_ld_type = 0U;
            _r1->reg._r0E.bit.ch12_ld_type = 0U;
            break;
        case XD12R_CHx_LD_TYPE2:
            _r1->reg._r0F.bit.ch1_ld_type = 0U;
            _r1->reg._r0F.bit.ch2_ld_type = 0U;
            _r1->reg._r0F.bit.ch3_ld_type = 0U;
            _r1->reg._r0F.bit.ch4_ld_type = 0U;
            _r1->reg._r0F.bit.ch5_ld_type = 0U;
            _r1->reg._r0F.bit.ch6_ld_type = 0U;
            break;
        case XD12R_DELAY_CH1:
            _r1->reg._r10.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH2:
            _r1->reg._r11.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH3:
            _r1->reg._r12.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH4:
            _r1->reg._r13.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH5:
            _r1->reg._r14.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH6:
            _r1->reg._r15.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH7:
            _r1->reg._r16.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH8:
            _r1->reg._r17.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH9:
            _r1->reg._r18.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH10:
            _r1->reg._r19.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH11:
            _r1->reg._r1A.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_DELAY_CH12:
            _r1->reg._r1B.bit.delay_chx = 0U;   /* CH1 */
            break;
        case XD12R_SERIAL_CLK_GEN:
            _r1->reg._r1C.bit.serial_clk_high = XDR_SERIAL_CLK_HIGH;
            _r1->reg._r1C.bit.serial_clk_low = XDR_SERIAL_CLK_LOW;
            break;
        case XD12R_SERIAL_LATENCY:
            _r1->reg._r1D.bit.serial_latency = 128U;
            break;

        case XD12R_RSTCNT:
            _r1->reg._r20.bit.rstcnt = 0U;
            break;
        case XD12R_TIMEOUT:
            _r1->reg._r21.bit.timeout = 1023U;
            break;
        case XD12R_FLLCNT1:
            _r1->reg._r22.bit.fllcnt = 0U;
            break;
        case XD12R_FLLCNT2:
            _r1->reg._r23.bit.fllcnt = 50U;
            _r1->reg._r23.bit.fll_range = 2U;
            _r1->reg._r23.bit.fll_en = 1U;
            break;
        case XD12R_WR_PROTECT:
            _r1->reg._r24.bit.wr_protect = 0x555U;
            break;
        case XD12R_NF_CONTROL:
            _r1->reg._r25.bit.DGRJT_EN1 = 0U;
            _r1->reg._r25.bit.DGRJT_EN2 = 0U;
            _r1->reg._r25.bit.BBKN_EN = 0U;
            _r1->reg._r25.bit.BBKN_TH = 0U;
            break;
        case XD12R_CHOP_EN:
            _r1->reg._r26.bit.CHOP_BGR_EN = 0U;
            _r1->reg._r26.bit.CHOP_DAC_EN = 0U;
            _r1->reg._r26.bit.CHOP_OSC_EN = 0U;
            _r1->reg._r26.bit.CHOP_OSCLDO_EN = 0U;
            _r1->reg._r26.bit.CHOP_DRV_EN = 0U;
            _r1->reg._r26.bit.CHOP_EN = 0U;
            break;
        case XD12R_TEMP:
            _r1->reg._r27.bit.flt_gain = 1U;
            _r1->reg._r27.bit.o_slew = 2U;
            _r1->reg._r27.bit.flt_ctl = 2U;
            _r1->reg._r27.bit.dac_rng = 0U;
            _r1->reg._r27.bit.ofs_temp = 8U;
            break;
        case XD12R_OSC_FLL_MAN1:
            _r1->reg._r28.bit.osc_fll_man = 0U;
            break;
        case XD12R_OSC_FLL_MAN2:
            _r1->reg._r29.bit.osc_fll_man = 8U;
            _r1->reg._r29.bit.osc_fll_err_range = 0U;
            _r1->reg._r29.bit.osc_man_en = 1U;
            break;
        case XD12R_OSC_SPREAD:
            _r1->reg._r2A.bit.SPRD_GAIN = 0U;
            _r1->reg._r2A.bit.SPRD_SPD = 0U;
            _r1->reg._r2A.bit.SPRD_EN = 0U;
            break;
        case XD12R_CLOCK_GATE_EN:
            _r1->reg._r2B.bit.DC_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR1_MCLK_EN = 1U;
            _r1->reg._r2B.bit.FR2_MCLK_EN = 1U;
            _r1->reg._r2B.bit.OTP_MCLK_EN = 1U;
            break;

#elif (XD_MODEL_TYPE == XD_TYPE_IC601)
#endif
        default:
            continue;
        }
        xdr12_write_by_type(addr, _r1->ALL[addr], XD12R_ADDR_TYPE_GENERAL);
    }
    xdr12_trim_enable_write_protect(false);
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
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_XCR)
    {
        _v_id_gen_command_t _r04 = { 0, };

        _r04.bit.enable = 1U;

        xcr24_set_xcr24_gr1_reg(XCR_ID_GEN_COMMAND, &_r04.ALL, 1U);
    }
    #else
    {
        uint16_t* out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xdr12_pwm_out_done();

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            len += xdr12_make_pwm_out_stream(gt_xd_idgen_command[daisy].ALL, &out[len], XDR_BIT_SYNCGEN);
        }
        out[len++] = 0U;

        xdr12_pwm_out((uint32_t)gn_pwm_out_xd_write, (len + PWM_OUT_HEADER_SIZE));
    }
    #endif
}

void xdr12_init_param(void)
{
    for(uint16_t num = 0U ; num < XDR_DAISY_LENGTH ; ++num)
    {
        gt_xd_write_command[num].bit.start = 1U;
        gt_xd_write_command[num].bit.cmd_code = CMD_CODE_WRITE;

        gt_xd_read_command[num].bit.start = 1U;
        gt_xd_read_command[num].bit.cmd_code = CMD_CODE_READ;

        gt_xd_readout_command[num].bit.start = 1U;
        gt_xd_readout_command[num].bit.cmd_code = CMD_CODE_READ;

        gt_xd_ld_transfer_command[num].bit.start = 1U;
        gt_xd_ld_transfer_command[num].bit.cmd_code = CMD_CODE_LDIM;

        gt_xd_fault_read_command[num].bit.start = 1U;
        gt_xd_fault_read_command[num].bit.cmd_code = CMD_CODE_RD_FAULT;

        gt_xd_syncgen_command[num].bit.start = 1U;
        gt_xd_syncgen_command[num].bit.cmd_code = CMD_CODE_SYNCGEN;

        gt_xd_idgen_command[num].bit.start = 1U;
        gt_xd_idgen_command[num].bit.cmd_code = CMD_CODE_IDGEN;
    }

    gt_xd_fault_readout_command.bit.start = 1U;
    gt_xd_fault_readout_command.bit.cmd_code = CMD_CODE_RD_FAULT;

    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_5, (uint32_t)gn_pwm_in_xd_response_duty);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)gn_pwm_in_xd_response_freq);
}

void xdr12_init(void)
{
    xdr12_reset();
    xdr12_idgen();

    xdr12_regs_init_table();
}

void xdr12_trim_init(void)
{
    xdr12_reset();
    xdr12_idgen();

    xdr12_regs_trim_init_table();
}

static void xdr12_write(uint16_t addr, uint16_t data)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_XCR)
    {
        _v_global_write_command_t _r01 = { 0, };

        _r01.bit.enable = 1U;
        _r01.bit.addr = addr;

        xcr24_set_xcr24_gr1_reg(XCR_GLOBAL_WRITE_DATA, &data, 1U);
        xcr24_set_xcr24_gr1_reg(XCR_GLOBAL_WRITE_COMMAND, &_r01.ALL, 1U);
    }
    #else
    {
        uint16_t* out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;

        xdr12_pwm_out_done();

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            gt_xd_write_command[daisy].bit.addr = addr;
            gt_xd_write_command[daisy].bit.data = data;
            len += xdr12_make_pwm_out_stream(gt_xd_write_command[daisy].ALL, &out[len], XDR_CMD_WRITE);
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Write Packet\r\n\tADDR - 0x%02X, DATA - 0x%03X", gt_xd_write_command->bit.addr, gt_xd_write_command->bit.data);
        }
        out[len++] = 0U;

        xdr12_pwm_out((uint32_t)gn_pwm_out_xd_write, (len + PWM_OUT_HEADER_SIZE));
    }
    #endif
}

static uint16_t xdr12_read(uint16_t addr, uint16_t* p_xdr_buffer)
{
    #if (XDR_CONTROL_TYPE == XDR_CONTROLLED_XCR)
    {
        uint16_t buffer[MODEL_XDR_DAISY_SEG_SIZE] = { 0, };

        const uint16_t daisy_size = MODEL_XDR_DAISY_SEG_SIZE;
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

            uint16_t seg_base_idx = ch_seg * XCR_SEG_CH_SIZE;
            uint16_t buf_ptr = 0U;

            /* iteration - IC num */
            for(uint16_t ic = 0U; ic < XDR_DAISY_LENGTH; ++ic)
            {
                /* iteration - seg channel */
                for(uint16_t ch = 0U; ch < XCR_SEG_CH_SIZE; ++ch)
                {
                    uint16_t xc_ch = (seg_base_idx + ch);
                    if(xc_ch < XCR_CH_SIZE)
                    {
                        if(addr < XD12R_OTP_CTRL_BASE)
                        {
                            gt_xdr12_get_regs[xc_ch][ic].ALL[addr] = buffer[buf_ptr];
                        }
                        else
                        {
                            gt_xdr12_otp_ctrl_get_regs[xc_ch][ic].ALL[addr] = buffer[buf_ptr];
                        }
                    }
                    ++buf_ptr;
                }
            }
        }
    }
    #else
    {
        uint16_t* out = gn_pwm_out_xd_write + PWM_OUT_HEADER_SIZE;
        uint16_t len = 0U;
        uint16_t pwm_in_length = XDR_DAISY_LENGTH * (XD_HDR_BIT + XD_ID_BIT + XD_DATA_BIT);

        for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
        {
            gt_xd_read_command[daisy].bit.addr = addr;
            len += xdr12_make_pwm_out_stream(gt_xd_read_command[daisy].ALL, &out[len], XDR_CMD_READ);
            comm_UART_Printf(LOG_LV_DEBUG, "\r\nXDR Read Packet\r\n\tADDR - 0x%02X", gt_xd_write_command->bit.addr);
        }
        out[len++] = 0U;

        xdr12_pwm_out((uint32_t)gn_pwm_out_xd_write, (len + PWM_OUT_HEADER_SIZE));
        xdr12_pwm_out_done();

        if(false == xdr12_pwm_in(pwm_in_length, 100U))
        {
            uint8_t max_id = xdr12_decode_pwm_input_stream(gn_pwm_in_xd_response_freq, gn_pwm_in_xd_response_duty, p_xdr_buffer, pwm_in_length);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "%s timeout\r\n", __func__);
        }
    }
    #endif

    return 0U;
}

static void xdr12_change_addr_type(xd12r_addr_type_t addr_type)
{
    _v_xdr12_op_mode_t* _r3F = &gt_xdr12_otp_ctrl_set_regs.reg._r3F;
    xd12r_addr_type_t current_type = (xd12r_addr_type_t)(_r3F->bit.addr_ext);

    if (current_type != addr_type)
    {
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
#if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    uint16_t xdr_buffer[XDR_DAISY_LENGTH] = { 0U };
#else
    uint16_t xdr_buffer[XCR_CH_SIZE][XDR_DAISY_LENGTH] = { 0U };
#endif
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

uint16_t xdr12_get_by_type(uint16_t addr, xd12r_addr_type_t addr_type)
{
    uint16_t ret = 0U;
#if (XDR_CONTROL_TYPE == XDR_CONTROLLED_MCU)
    if (addr_type == XD12R_ADDR_TYPE_GENERAL)
    {
        if (addr < XD12R_OTP_CTRL_BASE)
        {
            ret = gt_xdr12_get_regs[0].ALL[addr];
        }
        else
        {
            ret = gt_xdr12_otp_ctrl_get_regs[0].ALL[addr];
        }
    }
    else
    {
        ret = gt_xdr12_mirror_get_regs[0].ALL[addr];
    }
#else
    if (addr_type == XD12R_ADDR_TYPE_GENERAL)
    {
        if (addr < XD12R_OTP_CTRL_BASE)
        {
            ret = gt_xdr12_get_regs[0][0].ALL[addr];
        }
        else
        {
            ret = gt_xdr12_otp_ctrl_get_regs[0][0].ALL[addr];
        }
    }
    else
    {
        ret = gt_xdr12_mirror_get_regs[0][0].ALL[addr];
    }
#endif

    return ret;
}

void xdr12_pwm_out_syncgen(void)
{
    xdr12_pwm_out((uint32_t)gn_pwm_out_xd_syncgen, (uint32_t)(sizeof(gn_pwm_out_xd_syncgen)/sizeof(gn_pwm_out_xd_syncgen[0U])));
}

void xdr12_ld_transfer(uint16_t* p, uint16_t line_size)
{
    if((p == NULL) || (line_size == 0U))
    {
        return;
    }

    uint16_t* const out = gn_pwm_out_xd_ld_transfer + PWM_OUT_HEADER_SIZE;
    uint16_t len = 0;
    const uint16_t* p_src = p;

    for(uint16_t daisy = 0U ; daisy < XDR_DAISY_LENGTH ; ++daisy)
    {
        if((len + XDR_LD_TRANSFER) > (XDR_DAISY_LENGTH * XDR_LD_TRANSFER + 1U))
        {
            break;
        }

        /* header : 1111 */
        out[len++] = PWM_OUT_BIT1;
        out[len++] = PWM_OUT_BIT1;
        out[len++] = PWM_OUT_BIT1;
        out[len++] = PWM_OUT_BIT1;

        for(uint8_t ch = 0U ; ch < XDR_CH_LENGTH ; ++ch)
        {
            const uint16_t written = xdr12_make_pwm_out_stream(*p_src++, &out[len], XD_LD_DATA_BIT);
            len += written;
        }
    }

    if((uint32_t)len < (XDR_DAISY_LENGTH * XDR_LD_TRANSFER + 1U))
    {
        out[len++] = 0U; /* PWM_DMA_TAIL_SIZE */
    }

    xdr12_pwm_out((uint32_t)gn_pwm_out_xd_ld_transfer, (uint32_t)len);
}

static void xdr12_trim_enable_write_protect(bool protect_en)
{
    _v_xdr12_otp_protect_t* _r3E = &gt_xdr12_otp_ctrl_set_regs.reg._r3E;
    _r3E->bit.protect_en = (protect_en == true) ? XD12R_REG_PROTECT_ENABLE : XD12R_REG_PROTECT_DISABLE;
    xdr12_write_by_type(XD12R_OTP_CTRL_BASE + XD12R_OTP_PROTECT, _r3E->ALL, XD12R_ADDR_TYPE_GENERAL);
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
    xdr12_write_by_type(XD12R_CHANNEL_ENABLE, _r05->ALL, XD12R_ADDR_TYPE_MIRROR);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid current reference value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid ldo digital value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid ldo dac value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid ldo fll value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid osc value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid ch gain value: %u", __func__, reg_val);
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
        comm_UART_Printf(LOG_LV_ERROR, "\r\n%s invalid ch offset value: %u", __func__, reg_val);
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
        uint16_t saved_reg = xdr12_get_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
        uint16_t mirror_reg = xdr12_read_by_type(mirror_addr, XD12R_ADDR_TYPE_MIRROR);
        if (saved_reg != mirror_reg)
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✕]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_RED, ANSI_FONT_NONE, mirror_addr, saved_reg, mirror_reg);
        }
        else
        {
            comm_UART_Printf(LOG_LV_ERROR, "\r\n\t%s[✔]%s ADDR [0x%02X] - [0x%03X - 0x%03X]", \
                ANSI_FONT_GREEN, ANSI_FONT_NONE, mirror_addr, saved_reg, mirror_reg);
        }
    }
}