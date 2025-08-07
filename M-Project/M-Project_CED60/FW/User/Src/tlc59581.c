#ifndef __TLC59581_C__
#define __TLC59581_C__

#include "main.h"
#include "tlc59581.h"
#include "comm_debug.h"

#define CED_PARALLEL_SIZE           (3)

#define CMD_WRTFC                   (0x01)
#define CMD_FCWRTEN                 (0x02)
#define CMD_VSYNC                   (0x11)
#define CMD_WRTGS                   (0x21) // transmit FPGA's frame buffer to TLC59581
#define CMD_WRTFB                   (0x28) // custom CMD, fill FPGA's frame buffer

#define CMD_READSID                 (0x04)
#define CMD_READFC1                 (0x05)
#define CMD_READFC2                 (0x06)

#define TLC_CMD_SIZE                (1)
#define TLC_PAYLOAD                 (3)

#define TLC_WRTFC_SIZE              (TLC_CMD_SIZE + TLC_PAYLOAD)
#define TLC_FCWRTEN_SIZE            (TLC_CMD_SIZE + TLC_PAYLOAD)
#define TLC_VSYNC_SIZE              (TLC_CMD_SIZE + TLC_PAYLOAD)
#define TLC_WRTGS_SIZE              (TLC_CMD_SIZE + TLC_PAYLOAD)
#define TLC_WRTFB_SIZE              (TLC_CMD_SIZE + (TLC_LINE_SIZE * TLC_VIRTUAL_OUTPUT_SIZE * TLC_CH_SIZE))

#define TLC_VIRTUAL_OUTPUT_SIZE     (64)
#define TLC_PHYSICAL_OUTPUT_SIZE    (60)
#define TLC_LINE_SIZE               (30)
#define TLC_CH_SIZE                 (3)

#define CH_BLUE                     (0)
#define CH_GREEN                    (1)
#define CH_RED                      (2)

#define GRAY_SCALE_MAX              (0xFFFF)

typedef union _tag_tlc59581_reg_fc1_
{
    uint64_t value;
    uint16_t u16_val[4];
    struct _tag_fc1_table_
    {
        uint64_t LODVTH             : 2;
        uint64_t SEL_TD0            : 2;
        uint64_t LOD_REMOVAL_EN     : 1;
        uint64_t                    : 3;
        uint64_t PREC_EN            : 1;
        uint64_t PREC_MODE3         : 1;
        uint64_t                    : 3;
        uint64_t C_U_E              : 1;
        uint64_t CCB                : 9;    /* color brightness control, OUT_B0 ~ OUT_B15 */
        uint64_t CCG                : 9;    /* color brightness control, OUT_G0 ~ OUT_G15 */
        uint64_t CCR                : 9;    /* color brightness control, OUT_R0 ~ OUT_R15 */
        uint64_t BC                 : 3;    /* global brightness control */
        uint64_t CMD_FC1            : 4;    /* 0b1001 */
        uint64_t                    :64;    /* reserved */
    }u;
}_tlc59581_fc1_t_;

/* App Note, Page 16 */
typedef union _tag_tlc59518_reg_fc2_
{
    uint64_t value;
    uint16_t u16_val[4];
    struct _tag_fc2_table_
    {
        uint64_t MAX_LINE           : 5;
        uint64_t PSAVE_ENA          : 1;
        uint64_t SEL_GCLK_EDGE      : 1;
        uint64_t SEL_PCHG           : 1;
        uint64_t                    : 3;
        uint64_t EMI_REDUCE_B       : 1;
        uint64_t EMI_REDUCE_G       : 1;
        uint64_t EMI_REDUCE_R       : 1;
        uint64_t SEL_PWM            : 1;
        uint64_t _1st_LINE_ENH      : 2;
        uint64_t                    : 2;
        uint64_t RSV_HIGH           : 1;
        uint64_t LGSE1_B            : 4;
        uint64_t LGSE1_G            : 4;
        uint64_t LGSE1_R            : 4;
        uint64_t INTERFERENCE_B     : 2;
        uint64_t REVERSE_V_B        : 2;
        uint64_t INTERFERENCE_G     : 2;
        uint64_t REVERSE_V_G        : 2;
        uint64_t INTERFERENCE_R     : 2;
        uint64_t REVERSE_V_R        : 2;
        uint64_t CMD_FC2            : 4;    /* 0b0110 */
        uint64_t                    :64;    /* reserved */
    }u;
}_tlc59581_fc2_t_;

static _tlc59581_fc1_t_ gt_tlc59581_fc1;
static _tlc59581_fc2_t_ gt_tlc59581_fc2;

typedef struct _tag_rgb_format_
{
    uint16_t out_b;
    uint16_t out_g;
    uint16_t out_r;
}_tlc59581_rgb_format_t_;

typedef struct _tag_gray_scale_format_
{
    uint16_t command;
    _tlc59581_rgb_format_t_ payload[TLC_VIRTUAL_OUTPUT_SIZE][TLC_LINE_SIZE];
}_tlc59581_gray_scale_format_t_;

static _tlc59581_gray_scale_format_t_ gt_tlc59581_gray_scale_buffer;
static _tlc59581_pattern_t_ gt_tlc59581_pattern;
static uint16_t gn_gray_scale_value;

static uint16_t gn_tlc_fc1_buffer[4] = {0, };
static uint16_t gn_tlc_fc2_buffer[4] = {0, };

static const uint16_t const_cmd_vsync[4] = { CMD_VSYNC, 0, 0, 0 };
static const uint16_t const_cmd_write_gray_scale[4] = { CMD_WRTGS, 0, 0, 0 };
static const uint16_t const_cmd_fc_enable[4] = { CMD_FCWRTEN, 0, 0, 0 };

volatile bool gb_vsync_flag = false;
volatile bool gb_gray_scale_parsing_flag = false;
volatile bool gb_spi_tx_flag = false;

#define PATTERN_INCREASE_TABLE_SIZE (133)
static uint16_t gn_gs_increase_table[PATTERN_INCREASE_TABLE_SIZE] =
{
    0,780,1560,2340,3120,3900,4680,5460,6240,7020,7800,8580,9360,10140,10920,11700,12480,13260,14040,14820,15600,16380,17160,17940,18720,19500,20280,21060,21840,22620,23400,24180,24960,25740,26520,27300,28080,28860,29640,30420,31200,31980,32760,33540,34320,35100,35880,36660,37440,38220,39000,39780,40560,41340,42120,42900,43680,44460,45240,46020,46800,47580,48360,49140,49920,50700,51480,52260,53040,53820,54600,55380,56160,56940,57720,58500,59280,60060,60840,61620,62400,63180,63960,64740,65520,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,0,
};

#define PATTERN_DECREASE_TABLE_SIZE (133)
static uint16_t gn_gs_decrease_table[PATTERN_DECREASE_TABLE_SIZE] =
{
    65535,64755,63975,63195,62415,61635,60855,60075,59295,58515,57735,56955,56175,55395,54615,53835,53055,52275,51495,50715,49935,49155,48375,47595,46815,46035,45255,44475,43695,42915,42135,41355,40575,39795,39015,38235,37455,36675,35895,35115,34335,33555,32775,31995,31215,30435,29655,28875,28095,27315,26535,25755,24975,24195,23415,22635,21855,21075,20295,19515,18735,17955,17175,16395,15615,14835,14055,13275,12495,11715,10935,10155,9375,8595,7815,7035,6255,5475,4695,3915,3135,2355,1575,795,15,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,
};
static uint16_t gn_gs_inc_dec_table_index;

static void spi_write(uint16_t* p_data, uint16_t length)
{
    while(gb_spi_tx_flag) {}

    gb_spi_tx_flag = true;

    while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {}

    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);

    SPI1_CS_LO();

    if (!LL_SPI_IsEnabled(SPI1))
    {
        LL_SPI_Enable(SPI1);
    }

    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_3, (uint32_t)p_data);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, length);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
}

uint16_t tlc59581_get_gray_scale(void)
{
    return gn_gray_scale_value;
}

void tlc59581_set_gray_scale(uint32_t gs_value)
{
    if (gs_value < 65536)
    {
        gn_gray_scale_value = gs_value;
        print("OK!!\r\n");
    }
    else
    {
        print("Invalid Input [%u]!! [0 ~ %u]\r\n", gs_value, GRAY_SCALE_MAX);
    }
}

_tlc59581_pattern_t_ tlc59581_get_pattern(void)
{
    return gt_tlc59581_pattern;
}

void tlc59581_set_pattern(_tlc59581_pattern_t_ pattern)
{
    if (pattern < PATTERN_MAX)
    {
        gt_tlc59581_pattern = pattern;
        gn_gs_inc_dec_table_index = 0;
        print("OK!!\r\n");
    }
    else
    {
        print("Invalid Input [%u]!! [0 ~ %u]\r\n", pattern, (PATTERN_MAX - 1));
    }
}

static void tlc59581_generate_pattern(void)
{
    if (gb_gray_scale_parsing_flag)
    {
        switch (gt_tlc59581_pattern)
        {
            case PATTERN_NONE :
            case PATTERN_INCREASE :
            case PATTERN_DECREASE :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = gn_gray_scale_value;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = gn_gray_scale_value;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = gn_gray_scale_value;
                    }
                }
                break;

            case PATTERN_RED :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = gn_gray_scale_value;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = 0;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = 0;
                    }
                }
                break;
            case PATTERN_GREEN :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = 0;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = gn_gray_scale_value;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = 0;
                    }
                }
                break;
            case PATTERN_BLUE :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = 0;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = 0;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = gn_gray_scale_value;
                    }
                }
                break;
            case PATTERN_HORIZONTAL :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    bool is_hor_even = (output % 2) ? false : true;
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = (is_hor_even ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = (is_hor_even ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = (is_hor_even ? gn_gray_scale_value : 0);
                    }
                }
                break;
            case PATTERN_VERTICAL :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        bool is_ver_odd = (line % 2) ? false : true;
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = (is_ver_odd ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = (is_ver_odd ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = (is_ver_odd ? gn_gray_scale_value : 0);
                    }
                }
                break;
            case PATTERN_CHECKERBOARD_1 :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        bool is_bright = ((output % 2) == (line % 2));
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = (is_bright ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = (is_bright ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = (is_bright ? gn_gray_scale_value : 0);
                    }
                }
                break;
            case PATTERN_CHECKERBOARD_2 :
                for (uint8_t output = 0 ; output < TLC_PHYSICAL_OUTPUT_SIZE ; ++output)
                {
                    for (uint8_t line = 0 ; line < TLC_LINE_SIZE ; ++line)
                    {
                        bool is_bright = ((output % 2) != (line % 2));
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_r = (is_bright ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_g = (is_bright ? gn_gray_scale_value : 0);
                        gt_tlc59581_gray_scale_buffer.payload[output][line].out_b = (is_bright ? gn_gray_scale_value : 0);
                    }
                }
                break;
        }
        gb_gray_scale_parsing_flag = false;
    }
}

void tlc59581_transmit_vsync(void)
{
    spi_write((uint16_t*)const_cmd_vsync, TLC_VSYNC_SIZE);
}

static inline void tlc59581_transmit_frame_buffer(void)
{
    gt_tlc59581_gray_scale_buffer.command = CMD_WRTFB;
    spi_write(&gt_tlc59581_gray_scale_buffer.command, TLC_WRTFB_SIZE);

    gb_gray_scale_parsing_flag = true;
}

static inline void tlc59581_transmit_write_gray_scale(void)
{
    spi_write((uint16_t*)const_cmd_write_gray_scale, TLC_WRTGS_SIZE);
}

static inline void tlc59581_transmit_fc_enable(void)
{
    spi_write((uint16_t*)const_cmd_fc_enable, TLC_FCWRTEN_SIZE);
}

static void tlc59581_fc1_reg_init(void)
{
    gt_tlc59581_fc1.u.LODVTH          =  1;
    gt_tlc59581_fc1.u.SEL_TD0         =  1;
    gt_tlc59581_fc1.u.LOD_REMOVAL_EN  =  0;

    gt_tlc59581_fc1.u.CCB             =  511; // CED_PARALLEL_SIZE * 25; /* 100.5uA */
    gt_tlc59581_fc1.u.CCG             =  511; // CED_PARALLEL_SIZE * 25;
    gt_tlc59581_fc1.u.CCR             =  511; // CED_PARALLEL_SIZE * 25;

    gt_tlc59581_fc1.u.BC              =  7;
    gt_tlc59581_fc1.u.CMD_FC1         =  9;    /* 0b1001, refer to datasheet page.14 */

    gn_tlc_fc1_buffer[0] = CMD_WRTFC;
    gn_tlc_fc1_buffer[1] = ((gt_tlc59581_fc1.value >> 32) & 0xFFFF);
    gn_tlc_fc1_buffer[2] = ((gt_tlc59581_fc1.value >> 16) & 0xFFFF);
    gn_tlc_fc1_buffer[3] = ((gt_tlc59581_fc1.value >>  0) & 0xFFFF);

    print("0x%016llX --> 0x%04X, 0x%04X, 0x%04X, 0x%04X\r\n", gt_tlc59581_fc1.value, gn_tlc_fc1_buffer[0], gn_tlc_fc1_buffer[1], gn_tlc_fc1_buffer[2], gn_tlc_fc1_buffer[3]);

    tlc59581_transmit_fc_enable();
    spi_write((uint16_t*)gn_tlc_fc1_buffer, TLC_WRTFC_SIZE);
}

static void tlc59581_fc2_reg_init(void)
{
    gt_tlc59581_fc2.u.RSV_HIGH = 1;
    gt_tlc59581_fc2.u.MAX_LINE = (TLC_LINE_SIZE - 1); /* Max Line num - 1 */
    gt_tlc59581_fc2.u.CMD_FC2 = 6; /* 0b0110, refer to datasheet page.14 */

    gn_tlc_fc2_buffer[0] = CMD_WRTFC;
    gn_tlc_fc2_buffer[1] = ((gt_tlc59581_fc2.value >> 32) & 0xFFFF);
    gn_tlc_fc2_buffer[2] = ((gt_tlc59581_fc2.value >> 16) & 0xFFFF);
    gn_tlc_fc2_buffer[3] = ((gt_tlc59581_fc2.value >>  0) & 0xFFFF);

    print("0x%016llX --> 0x%04X, 0x%04X, 0x%04X, 0x%04X\r\n", gt_tlc59581_fc2.value, gn_tlc_fc2_buffer[0], gn_tlc_fc2_buffer[1], gn_tlc_fc2_buffer[2], gn_tlc_fc2_buffer[3]);

    tlc59581_transmit_fc_enable();
    spi_write((uint16_t*)gn_tlc_fc2_buffer, TLC_WRTFC_SIZE);
}

void tlc59581_init(void)
{
    tlc59581_fc1_reg_init();
    tlc59581_fc2_reg_init();

    gn_gray_scale_value = 30000;

    LL_TIM_EnableCounter(TIM1);
}

static void tlc59581_vsync_task(void)
{
    if (gb_vsync_flag)
    {
        LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
        // tlc59581_transmit_vsync();
        tlc59581_transmit_write_gray_scale();

        tlc59581_transmit_frame_buffer();

        if (gt_tlc59581_pattern == PATTERN_INCREASE)
        {
            gn_gray_scale_value = gn_gs_increase_table[gn_gs_inc_dec_table_index];
            ++gn_gs_inc_dec_table_index;
            if (gn_gs_inc_dec_table_index >= PATTERN_INCREASE_TABLE_SIZE)
            {
                gn_gs_inc_dec_table_index = 0;
            }
        }
        else if (gt_tlc59581_pattern == PATTERN_DECREASE)
        {
            gn_gray_scale_value = gn_gs_decrease_table[gn_gs_inc_dec_table_index];
            ++gn_gs_inc_dec_table_index;
            if (gn_gs_inc_dec_table_index >= PATTERN_DECREASE_TABLE_SIZE)
            {
                gn_gs_inc_dec_table_index = 0;
            }
        }

        gb_vsync_flag = false;
    }
}

void tlc59581_process(void)
{
    tlc59581_vsync_task();
    tlc59581_generate_pattern();
}

#endif /* end of __TLC59581_C__ */