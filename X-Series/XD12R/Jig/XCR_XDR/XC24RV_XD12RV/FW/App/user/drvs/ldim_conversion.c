#include "crc.h"
#include "drv_xcr24.h"
#include "ldim_conversion.h"
#include "comm_debugging.h"

#define XDR_LDIM_BURST_SIZE     (XCR_CH_SIZE * XDR_DAISY_LENGTH * XDR_LD_SIZE)
#define XCR_LDIM_BURST_SIZE     (1U + XDR_LDIM_BURST_SIZE + 1U)  /* HDR + Payload + crc16 */

uint8_t gn_block_map[LDIM_BLK_SIZE][3]; /* { xc_ch, xd_daisy, ld_order_max } */

typedef union tag_LD_BUFFER
{
    uint16_t buffer[(XCR_CH_SIZE * XDR_DAISY_LENGTH * XDR_LD_SIZE)];
    struct
    {
        uint16_t data16[XCR_CH_SIZE][XDR_DAISY_LENGTH][XDR_LD_SIZE];
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
    uint8_t xc, daisy, block = 0U;
    uint32_t map_idx = 0U;
    uint8_t current_ld_order = 0U;

    for (xc = 0U; xc < XCR_CH_SIZE; xc++)
    {
        for (daisy = 0U; daisy < XDR_DAISY_LENGTH; daisy++)
        {
            for (block = 0U; block < BLOCK_PER_XDR; block++)
            {
                if (map_idx >= LDIM_BLK_SIZE)
                {
                    return;
                }

                gn_block_map[map_idx][0] = xc + 1U;
                gn_block_map[map_idx][1] = daisy + 1U;

                current_ld_order += LED_PER_BLOCK;
                gn_block_map[map_idx][2] = current_ld_order;

                map_idx++;
            }
        }
    }
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
    const uint8_t xc_ch_max = gn_block_map[block][0];
    const uint8_t xd_daisy_max = gn_block_map[block][1];
    const uint8_t ld_order_max = gn_block_map[block][2];

    const uint16_t color_map[COLOR_ORDER_MAX] =
    {
        [COLOR_RED]   = red,
        [COLOR_GREEN] = green,
        [COLOR_BLUE]  = blue
    };

    for (uint8_t xc_ch = 0U; xc_ch < xc_ch_max; ++xc_ch)
    {
        for (uint8_t xd_daisy = 0U; xd_daisy < xd_daisy_max; ++xd_daisy)
        {
            uint16_t* p_ld_buffer = gt_xcr_ld_transfer_table.ld_buffer.data16[xc_ch][xd_daisy];
            for (uint8_t ld_order = (ld_order_max - LED_PER_BLOCK); ld_order < ld_order_max; ++ld_order)
            {
                uint8_t color_index = ld_order % COLOR_ORDER_MAX;
                p_ld_buffer[ld_order] = color_map[color_index];
            }
        }
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