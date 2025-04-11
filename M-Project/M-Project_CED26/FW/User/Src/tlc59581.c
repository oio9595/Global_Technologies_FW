#include "config.h"

#define TIM13_CHANGE_PATTERN_0_5s  22499
#define TIM13_CHANGE_PATTERN_0_1s  4499
#define TIM13_CHANGE_PATTERN_DIV   10
#define TIM13_CHANGE_PATTERN_DIFF   (uint16_t)((TIM13_CHANGE_PATTERN_0_5s - TIM13_CHANGE_PATTERN_0_1s) / TIM13_CHANGE_PATTERN_DIV)

static _tlc_fc1_t_ gt_tlc_fc1;
static _tlc_fc2_t_ gt_tlc_fc2;

static _tlc_gs_memory_t_ gt_tlc_gs_bank_A[LED_BUFF_LINE_NUM][LED_BUFF_CH_NUM] = {{0, }, };
static _tlc_gs_memory_t_ gt_tlc_gs_bank_B[LED_BUFF_LINE_NUM][LED_BUFF_CH_NUM] = {{0, }, };
static _tlc_gs_memory_t_ (*gp_tlc_gs_parsing_bank)[LED_BUFF_CH_NUM] = NULL;
static _tlc_gs_memory_t_ (*gp_tlc_gs_set_bank)[LED_BUFF_CH_NUM] = NULL;
static _tlc_gs_bank_address_t_ gt_tlc_gs_bank_address = ADDRESS_BANK_A;

/* tim1's peripheral registers must be written by half-words or words */
/* refer to page 493 in user manual */
uint32_t gn_gclk_pwm_table[GCLK_TABLE_LENGTH] = {0, };

static uint16_t gn_tlc_blk_num;

static uint8_t gn_tlc_parsing_line_cnt;
static uint8_t gn_tlc_parsing_ch_cnt;

static uint8_t gn_lat_hi_point[CMD_MAX] =
{
    LAT_HI_POINT_WRTGS,
    LAT_HI_POINT_VSYNC,
    LAT_HI_POINT_WRTFC,
    LAT_HI_POINT_READSID,
    LAT_HI_POINT_READFC1,
    LAT_HI_POINT_READFC2,
    LAT_HI_POINT_FCWRTEN,
};

static uint8_t gn_lat_lo_point[CMD_MAX] =
{
    LAT_LO_POINT_WRTGS,
    LAT_LO_POINT_VSYNC,
    LAT_LO_POINT_WRTFC,
    LAT_LO_POINT_READSID,
    LAT_LO_POINT_READFC1,
    LAT_LO_POINT_READFC2,
    LAT_LO_POINT_FCWRTEN,
};

typedef enum
{
    PAT_SPEED_INC = 0,
    PAT_SPEED_DEC,
}pat_speed_t;

static pat_speed_t gs_pat_speed;

void tlc59581_init()
{
    uint8_t tim1_period = 0;

    tlc_fc1_reg_init();
    tlc_fc2_reg_init();

    gt_tlc_gs_bank_address = ADDRESS_BANK_A;
    gp_tlc_gs_set_bank = gt_tlc_gs_bank_A;

    uint16_t dimming_data = get_dimming_value();
    for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
    {
        for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
        {
            gt_tlc_gs_bank_A[line][ch].u.OUT_Rn = dimming_data;
            gt_tlc_gs_bank_A[line][ch].u.OUT_Gn = dimming_data;
            gt_tlc_gs_bank_A[line][ch].u.OUT_Bn = dimming_data;

            gt_tlc_gs_bank_B[line][ch].u.OUT_Rn = dimming_data;
            gt_tlc_gs_bank_B[line][ch].u.OUT_Gn = dimming_data;
            gt_tlc_gs_bank_B[line][ch].u.OUT_Bn = dimming_data;
        }
    }
    tlc_parsing_gs_buff();

    tim1_period = LL_TIM_GetAutoReload(TIM1);

    /* GEN GCLK PWM DUTY TABLE */
    for (uint16_t i = 0 ; i < GCLK_TABLE_LENGTH ; i++)
    {
        if (i < DUMMY_LENGTH)
        {
            gn_gclk_pwm_table[i] = 0;
        }
        else if (i < GCLK_TABLE_LENGTH - DUMMY_LENGTH)
        {
            gn_gclk_pwm_table[i] = ((tim1_period + 1) / 2 - 1);
        }
        else
        {
            gn_gclk_pwm_table[i] = 0;
        }
    }
}

void tlc_fc1_reg_init()
{
    gt_tlc_fc1.u.LODVTH          =  1;
    gt_tlc_fc1.u.SEL_TD0         =  1;
    gt_tlc_fc1.u.LOD_REMOVAL_EN  =  1;

    gt_tlc_fc1.u.CCB             =  25;
    gt_tlc_fc1.u.CCG             =  25;
    gt_tlc_fc1.u.CCR             =  25;    /* 100.5uA */

    gt_tlc_fc1.u.BC              =  0;
    gt_tlc_fc1.u.CMD_FC1         =  9;    /* 0b1001, refer to datasheet page.14 */
    tlc_send_daisy_reg_write_command(CMD_WRTFC, gt_tlc_fc1.value, gt_tlc_fc1.value);
}

void tlc_fc2_reg_init()
{
    gt_tlc_fc2.u.RSV_HIGH        =  1;
    gt_tlc_fc2.u.MAX_LINE        =  DIMMING_LINE_NUM - 1; /* Max Line num - 1 */
    gt_tlc_fc2.u.CMD_FC2         =  6;    /* 0b0110, refer to datasheet page.14 */
    tlc_send_daisy_reg_write_command(CMD_WRTFC, gt_tlc_fc2.value, gt_tlc_fc2.value);
}

void tlc_set_gs_buff()
{
    gdim_pattern_t dimming_pattern = get_pattern_value();
    uint16_t dimming_data = get_dimming_value();

    /* SET DIMMING BUFFER */
    if (ADDRESS_BANK_A == tlc_get_gs_bank_address())
    {
        gp_tlc_gs_set_bank = gt_tlc_gs_bank_B;
    }
    else
    {
        gp_tlc_gs_set_bank = gt_tlc_gs_bank_A;
    }

    switch (dimming_pattern)
    {
        case PATTERN_0 :
            /* separate dimming buffer TLC DEV_1, DEV_2 */
            if (gn_tlc_blk_num == 0)
            {
                for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
                {
                    for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                    {
                        gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                        gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                        gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                    }
                }
            }
            else
            {
                uint8_t temp_line = (gn_tlc_blk_num - 1) % 26;
                uint8_t temp_ch = (gn_tlc_blk_num - 1) / 26;
                for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
                {
                    for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                    {
                        if (line == temp_line && ch == temp_ch)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                    }
                }
            }
        break;

        case PATTERN_1 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                }
            }
        break;

        case PATTERN_2 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                }
            }
        break;

        case PATTERN_3 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                    gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                }
            }
        break;

        case PATTERN_4 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    /* R G B W*/
                    if (line < 13)
                    {
                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                    }
                    else
                    {

                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                    }
                }
            }
        break;

        case PATTERN_5 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    /* G B W R */
                    if (line < 13)
                    {
                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                    }
                    else
                    {

                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                    }
                }
            }
        break;

        case PATTERN_6 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    /* B W R G */
                    if (line < 13)
                    {
                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                    }
                    else
                    {

                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                    }
                }
            }
        break;

        case PATTERN_7 :
            for (uint8_t line = 0 ; line < LED_BUFF_LINE_NUM ; ++line)
            {
                for (uint8_t ch = 0 ; ch < LED_BUFF_CH_NUM ; ++ch)
                {
                    /* W R G B */
                    if (line < 13)
                    {
                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = 0;
                        }
                    }
                    else
                    {
                        if (ch < 13)
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = 0;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                        else
                        {
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Rn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Gn = dimming_data;
                            gp_tlc_gs_set_bank[line][ch].u.OUT_Bn = dimming_data;
                        }
                    }
                }
            }
        break;
    }
}

void tlc_parsing_gs_buff()
{
    if (ADDRESS_BANK_A == tlc_get_gs_bank_address())
    {
        gp_tlc_gs_parsing_bank = gt_tlc_gs_bank_A;
    }
    else
    {
        gp_tlc_gs_parsing_bank = gt_tlc_gs_bank_B;
    }

    tlc_send_daisy_reg_write_command(CMD_WRTGS, gp_tlc_gs_parsing_bank[gn_tlc_parsing_line_cnt][gn_tlc_parsing_ch_cnt].value, gp_tlc_gs_parsing_bank[gn_tlc_parsing_line_cnt][gn_tlc_parsing_ch_cnt + 16].value);
    ++gn_tlc_parsing_ch_cnt;

    if (gn_tlc_parsing_ch_cnt >= DIMMING_CH_NUM)
    {
        gn_tlc_parsing_ch_cnt = 0;
        ++gn_tlc_parsing_line_cnt;
    }

    if (gn_tlc_parsing_line_cnt >= DIMMING_LINE_NUM)
    {
        gn_tlc_parsing_line_cnt = 0;
        tim14_IT_stop();
    }
}

void tlc_send_daisy_fc_write_enable_command()
{
    for (uint8_t loop_cnt = 0 ; loop_cnt < TLC_DATA_LENGTH * TLC_DAISY_SIZE ; )
    {
        TLC_DATA_LO();
        if ((loop_cnt + 1) == LAT_HI_POINT_FCWRTEN)
        {
            TLC_LAT_HI();
        }

        /* MAKE SCLK PULSE */
        TLC_SCLK_HI();
        delay_100ns(1);
        TLC_SCLK_LO();
        ++loop_cnt;

        if (loop_cnt == LAT_LO_POINT_FCWRTEN)
        {
            TLC_LAT_LO();
        }
    }
}

/* The device connected to the MCU is number 1 */
void tlc_send_daisy_reg_write_command(cmd_type_t cmd_type, uint64_t dev1_value, uint64_t dev2_value)
{
    uint8_t lat_hi_point = gn_lat_hi_point[cmd_type] + TLC_DATA_LENGTH;
    uint8_t lat_lo_point = gn_lat_lo_point[cmd_type] + TLC_DATA_LENGTH;

    uint8_t tcl_data_table[TLC_DATA_LENGTH * TLC_DAISY_SIZE] = {0, };

    if (cmd_type == CMD_WRTFC)
    {
        /* daisy FC Write Enable */
        tlc_send_daisy_fc_write_enable_command();
    }

    for (uint8_t loop_cnt = 0 ; loop_cnt < TLC_DATA_LENGTH * TLC_DAISY_SIZE ; ++loop_cnt)
    {
        /* SET DATA & LAT */
        if (loop_cnt < TLC_DATA_LENGTH)
        {
            /* dev2_value */
            if ((dev2_value >> ((TLC_DATA_LENGTH - 1) - loop_cnt)) & 1)
            {
                tcl_data_table[loop_cnt] = 1;
            }
            else
            {
                tcl_data_table[loop_cnt] = 0;
            }
        }
        else
        {
            /* dev1_value */
            if ((dev1_value >> ((TLC_DATA_LENGTH  - 1) - (loop_cnt - TLC_DATA_LENGTH))) & 1)
            {
                tcl_data_table[loop_cnt] = 1;
            }
            else
            {
                tcl_data_table[loop_cnt] = 0;
            }
        }
    }

    for (uint8_t loop_cnt = 0 ; loop_cnt < TLC_DATA_LENGTH * TLC_DAISY_SIZE ; )
    {
        if (tcl_data_table[loop_cnt])
        {
            TLC_DATA_HI();
        }
        else
        {
            TLC_DATA_LO();
        }

        if ((loop_cnt + 1) == lat_hi_point)
        {
            TLC_LAT_HI();
        }

        /* MAKE SCLK PULSE */
        TLC_SCLK_HI();
        delay_100ns(1);
        TLC_SCLK_LO();
        ++loop_cnt;

        if (loop_cnt == lat_lo_point)
        {
            TLC_LAT_LO();
        }
    }
}

_tlc_gs_bank_address_t_ tlc_get_gs_bank_address()
{
    return gt_tlc_gs_bank_address;
}

void tlc_set_gs_bank_address(_tlc_gs_bank_address_t_ buff_address)
{
    gt_tlc_gs_bank_address = buff_address;
}

_tlc_gs_memory_t_ (*tlc_get_parsing_bank_address(void))[LED_BUFF_CH_NUM]
{
    return gp_tlc_gs_parsing_bank;
}

uint16_t tlc_get_blk_num()
{
    return gn_tlc_blk_num;
}

void tlc_set_blk_num(uint16_t n_blk_num)
{
    gn_tlc_blk_num = n_blk_num;
}


void tlc_change_pattern(void)
{
    gdim_pattern_t dimming_pattern = get_pattern_value();

    if (dimming_pattern < (PATTERN_MAX - 1))
    {
        ++dimming_pattern;
    }
    else
    {
        dimming_pattern = PATTERN_0;
        uint16_t tim_period = LL_TIM_GetAutoReload(TIM13);

        if (gs_pat_speed == PAT_SPEED_INC)
        {
            if (tim_period > TIM13_CHANGE_PATTERN_0_1s)
            {
                tim_period -= TIM13_CHANGE_PATTERN_DIFF;
            }
            LL_TIM_SetAutoReload(TIM13, (uint32_t)tim_period);
            if (tim_period <= TIM13_CHANGE_PATTERN_0_1s)
            {
                gs_pat_speed = PAT_SPEED_DEC;
                //print("pat_speed_change -> dec!!!\r\n");
            }
        }

        else
        {
            if (tim_period < TIM13_CHANGE_PATTERN_0_5s)
            {
                tim_period += TIM13_CHANGE_PATTERN_DIFF;
            }
            LL_TIM_SetAutoReload(TIM13, (uint32_t)tim_period);
            if (tim_period >= TIM13_CHANGE_PATTERN_0_5s)
            {
                gs_pat_speed = PAT_SPEED_INC;
                //print("pat_speed_change -> inc!!!\r\n");
            }
        }
    }

    set_pattern_value(dimming_pattern);
}
