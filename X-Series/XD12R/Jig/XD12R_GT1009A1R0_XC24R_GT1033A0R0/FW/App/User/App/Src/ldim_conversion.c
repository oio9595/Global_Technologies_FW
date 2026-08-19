#include "crc.h"
#include "drv_xc24.h"
#include "ldim_conversion.h"
#include "comm_debugging.h"

#define XD_LDIM_BURST_SIZE     (XD_DAISY_LENGTH * XD_LD_SIZE * XC_CH_SIZE)
#define XC_LDIM_BURST_SIZE     (1U + XD_LDIM_BURST_SIZE + 1U)  /* HDR + Payload + crc16 */

typedef enum tag_BLOCK_TBL
{
    BLK_TBL_XD_DAISY = 0U,
    BLK_TBL_LD_ORDER,
    BLK_TBL_XC_CH,
    BLK_TBL_MAX
} block_table_t;

typedef union tag_LD_BUFFER
{
    uint16_t buffer[(XD_DAISY_LENGTH * XD_LD_SIZE * XC_CH_SIZE)];
    struct
    {
        uint16_t data16[XD_DAISY_LENGTH][XD_LD_SIZE][XC_CH_SIZE];
    };
} ld_buffer_t;

typedef union tag_XC_LD_TRANSFER
{
    uint16_t buffer[XC_LDIM_BURST_SIZE];
    struct
    {
        _cmd_t      cmd;
        ld_buffer_t ld_buffer;
        uint16_t    crc16;
    };
} xc_ld_transfer_t;

static uint8_t gn_block_map[LDIM_BLK_SIZE][BLK_TBL_MAX]; /* { xd_daisy, ld_order_max, xc_ch } */

static xc_ld_transfer_t gt_xc_ld_transfer_table;
static block_color_t gt_block_color_buffer[LDIM_BLK_SIZE];

void ldim_block_map_init(void)
{
    for (uint16_t blk = 0U; blk < LDIM_BLK_SIZE; ++blk)
    {
        const uint8_t blk_size_per_xc_ch = (XD_DAISY_LENGTH * (BLOCK_PER_XD));
        const uint8_t blk_size_per_xd = BLOCK_PER_XD;

        gn_block_map[blk][BLK_TBL_XD_DAISY] = (blk % blk_size_per_xc_ch) / blk_size_per_xd; /* xd_daisy */
        gn_block_map[blk][BLK_TBL_LD_ORDER] = ((blk % blk_size_per_xd) + 1U) * COLOR_ORDER_MAX; /* ld_order_max */
        gn_block_map[blk][BLK_TBL_XC_CH] = blk / blk_size_per_xc_ch; /* xc_ch */
    }
}

void ldim_block_map_print(void)
{
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n====== gn_block_map Render Result ======\r\ngn_block_map = \r\n{");
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n\t      { xd_daisy, ld_order, xc_ch }");
    for (uint32_t i = 0U; i < LDIM_BLK_SIZE; i++)
    {
        uint8_t xd_daisy = gn_block_map[i][BLK_TBL_XD_DAISY];
        uint8_t ld_order_max = gn_block_map[i][BLK_TBL_LD_ORDER];
        uint8_t xc_ch = gn_block_map[i][BLK_TBL_XC_CH];
        uint8_t ld_order_min = (ld_order_max - LED_PER_BLOCK) + 1U;
        comm_UART_Printf(LOG_LV_DEBUG, "\r\n\tLED [%2u] { %2u, %2u, %2u }, /* ld_order %2u ~ %2u */", i, xd_daisy, ld_order_max, xc_ch, ld_order_min, ld_order_max);
    }
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n};\r\n========================================");
}

block_color_t* ldim_get_block_color_buffer(void)
{
    return gt_block_color_buffer;
}

void ldim_set_block_color_buffer(uint16_t index ,uint16_t red, uint16_t green, uint16_t blue)
{
    if (index == LDIM_BLK_INDEX_ALL)
    {
        for (uint8_t i = 0U; i < LDIM_BLK_SIZE; ++i)
        {
            gt_block_color_buffer[i].r = red;
            gt_block_color_buffer[i].g = green;
            gt_block_color_buffer[i].b = blue;
        }
    }
    else
    {
        gt_block_color_buffer[index - 1].r = red;
        gt_block_color_buffer[index - 1].g = green;
        gt_block_color_buffer[index - 1].b = blue;
    }
}

void ldim_conversion_block_to_ldim(uint16_t block, uint16_t red, uint16_t green, uint16_t blue)
{
    const uint8_t xd_daisy = gn_block_map[block][BLK_TBL_XD_DAISY];
    const uint8_t ld_order_max = gn_block_map[block][BLK_TBL_LD_ORDER];
    const uint8_t xc_ch = gn_block_map[block][BLK_TBL_XC_CH];

    const uint16_t color_map[COLOR_ORDER_MAX] =
    {
        [COLOR_RED]   = red,
        [COLOR_GREEN] = green,
        [COLOR_BLUE]  = blue
    };

    uint16_t (*p_ld_buffer)[XC_CH_SIZE] = gt_xc_ld_transfer_table.ld_buffer.data16[xd_daisy];
    for (uint8_t ld_order = (ld_order_max - LED_PER_BLOCK); ld_order < ld_order_max; ++ld_order)
    {
        uint8_t color_index = ld_order % COLOR_ORDER_MAX;
        p_ld_buffer[ld_order][xc_ch] = color_map[color_index];
    }

    if((LDIM_BLK_SIZE - 1U) == block)
    {
        gt_xc_ld_transfer_table.cmd.bit.code = CMD_CODE3;
        gt_xc_ld_transfer_table.cmd.bit.addr = 0U;
        gt_xc_ld_transfer_table.cmd.bit.size = XD_LDIM_BURST_SIZE;
        gt_xc_ld_transfer_table.crc16 = Calculate_CRC16_CCITT_False(gt_xc_ld_transfer_table.buffer, (XC_LDIM_BURST_SIZE - 1U));
    }
}

uint16_t* ldim_get_xc_ld_transfer_buffer(void)
{
    return gt_xc_ld_transfer_table.buffer;
}

uint16_t ldim_get_xc_ld_transfer_size(void)
{
    //return XC_LDIM_BURST_SIZE;
    return (XC_LDIM_BURST_SIZE - 1U); // except CRC
}

uint16_t* ldim_get_xd_ld_transfer_buffer(void)
{
    return gt_xc_ld_transfer_table.ld_buffer.buffer;
}

uint16_t ldim_get_xd_ld_transfer_size(void)
{
    return XD_LDIM_BURST_SIZE;
}