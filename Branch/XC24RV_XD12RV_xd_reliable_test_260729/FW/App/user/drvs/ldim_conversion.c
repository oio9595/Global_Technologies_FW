#include "crc.h"
#include "drv_xcr24.h"
#include "ldim_conversion.h"
#include "comm_debugging.h"

#define XDR_LDIM_BURST_SIZE     (XDR_DAISY_LENGTH * XDR_LD_SIZE * XCR_CH_SIZE)
#define XCR_LDIM_BURST_SIZE     (1U + XDR_LDIM_BURST_SIZE + 1U)  /* HDR + Payload + crc16 */

typedef enum tag_BLOCK_TBL
{
    BLK_TBL_XD_DAISY = 0U,
    BLK_TBL_LD_ORDER,
    BLK_TBL_XC_CH,
    BLK_TBL_MAX
} block_table_t;

uint8_t gn_block_map[LDIM_BLK_SIZE][BLK_TBL_MAX]; /* { xd_daisy, ld_order_max, xc_ch } */

typedef union tag_LD_BUFFER
{
    uint16_t buffer[(XDR_DAISY_LENGTH * XDR_LD_SIZE * XCR_CH_SIZE)];
    struct
    {
        uint16_t data16[XDR_DAISY_LENGTH][XDR_LD_SIZE][XCR_CH_SIZE];
    };
} ld_buffer_t;

typedef union tag_XCR_LD_TRANSFER
{
    uint16_t buffer[XCR_LDIM_BURST_SIZE];
    struct
    {
        _cmd_t      cmd;
        ld_buffer_t ld_buffer;
        uint16_t    crc16;
    };
} xcr_ld_transfer_t;

static xcr_ld_transfer_t gt_xcr_ld_transfer_table;
static block_color_t gt_block_color_buffer[LDIM_BLK_SIZE];

void ldim_block_map_init(void)
{
    for (uint16_t blk = 0U; blk < LDIM_BLK_SIZE; ++blk)
    {
        const uint8_t blk_size_per_xc_ch = (XDR_DAISY_LENGTH * (BLOCK_PER_XDR));
        const uint8_t blk_size_per_xd = BLOCK_PER_XDR;

        gn_block_map[blk][BLK_TBL_XD_DAISY] = (blk % blk_size_per_xc_ch) / blk_size_per_xd; /* xd_daisy */
        gn_block_map[blk][BLK_TBL_LD_ORDER] = ((blk % blk_size_per_xd) + 1U) * COLOR_ORDER_MAX; /* ld_order_max */
        gn_block_map[blk][BLK_TBL_XC_CH] = blk / blk_size_per_xc_ch; /* xc_ch */
    }
}

void ldim_block_map_print(void)
{
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n====== gn_block_map Render Result ======");
    comm_UART_Printf(LOG_LV_DEBUG, "\r\ngn_block_map = \r\n{");

    for (uint32_t i = 0; i < LDIM_BLK_SIZE; i++)
    {
        uint8_t xd_daisy = gn_block_map[i][BLK_TBL_XD_DAISY];
        uint8_t ld_order_max = gn_block_map[i][BLK_TBL_LD_ORDER];
        uint8_t xc_ch = gn_block_map[i][BLK_TBL_XC_CH];
        uint8_t ld_order_min = (ld_order_max - LED_PER_BLOCK) + 1;

        comm_UART_Printf(LOG_LV_DEBUG, "\r\nLED [%2u] { %2d, %2d, %2d }, /* ld_order %2d ~ %2d */", i, xd_daisy, ld_order_max, xc_ch, ld_order_min, ld_order_max);
    }

    comm_UART_Printf(LOG_LV_DEBUG, "\r\n};");
    comm_UART_Printf(LOG_LV_DEBUG, "\r\n========================================");
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
        gt_block_color_buffer[index - 1U].r = red;
        gt_block_color_buffer[index - 1U].g = green;
        gt_block_color_buffer[index - 1U].b = blue;
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

    uint16_t (*p_ld_buffer)[XCR_CH_SIZE] = gt_xcr_ld_transfer_table.ld_buffer.data16[xd_daisy];
    for (uint8_t ld_order = (ld_order_max - LED_PER_BLOCK); ld_order < ld_order_max; ++ld_order)
    {
        uint8_t color_index = ld_order % COLOR_ORDER_MAX;
        p_ld_buffer[ld_order][xc_ch] = color_map[color_index];
    }

    if((LDIM_BLK_SIZE - 1U) == block)
    {
        gt_xcr_ld_transfer_table.cmd.bit.code = CMD_CODE3;
        gt_xcr_ld_transfer_table.cmd.bit.addr = 0U;
        gt_xcr_ld_transfer_table.cmd.bit.size = XDR_LDIM_BURST_SIZE;
        gt_xcr_ld_transfer_table.crc16 = Calculate_CRC16_CCITT_False(gt_xcr_ld_transfer_table.buffer, (XCR_LDIM_BURST_SIZE - 1U));
    }
}

uint16_t* ldim_get_xcr_ld_transfer_buffer(void)
{
    return gt_xcr_ld_transfer_table.buffer;
}

uint16_t ldim_get_xcr_ld_transfer_size(void)
{
    //return XCR_LDIM_BURST_SIZE;
    return (XCR_LDIM_BURST_SIZE - 1U); // except CRC
}

uint16_t* ldim_get_xdr_ld_transfer_buffer(void)
{
    return gt_xcr_ld_transfer_table.ld_buffer.buffer;
}

uint16_t ldim_get_xdr_ld_transfer_size(void)
{
    return XDR_LDIM_BURST_SIZE;
}