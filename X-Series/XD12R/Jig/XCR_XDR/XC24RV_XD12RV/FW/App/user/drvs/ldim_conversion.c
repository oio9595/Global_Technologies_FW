#include "crc.h"
#include "drv_xcr24.h"
#include "ldim_conversion.h"
#include "comm_debugging.h"

#define XDR_LDIM_BURST_SIZE     (XCR_CH_SIZE * XDR_DAISY_LENGTH * XDR_LD_SIZE)
#define XCR_LDIM_BURST_SIZE     (1U + XDR_LDIM_BURST_SIZE + 1U)  /* HDR + Payload + crc16 */

const uint8_t gn_block_map[LDIM_BLK_SIZE][3] = /* { xc_ch, xd_daisy, ld_order_max } */
{
    { 1, 1,  3 }, /* ld_order  1 ~  3 */
    { 1, 1,  6 }, /* ld_order  4 ~  6 */
    { 1, 1,  9 }, /* ld_order  7 ~  9 */
    { 1, 1, 12 }, /* ld_order 10 ~ 12 */
    { 1, 1, 15 }, /* ld_order 13 ~ 15 */
    { 1, 1, 18 }, /* ld_order 16 ~ 18 */
};

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
uint16_t gn_led_color_table[COLOR_ORDER_MAX] = { 0U };

uint16_t* ldim_get_led_color_buffer(void)
{
    return gn_led_color_table;
}

void ldim_set_led_color_buffer(uint16_t red, uint16_t green, uint16_t blue)
{
    gn_led_color_table[COLOR_RED] = red;
    gn_led_color_table[COLOR_GREEN] = green;
    gn_led_color_table[COLOR_BLUE] = blue;
}

void ldim_set_ldim_rgb(uint16_t block, uint16_t red, uint16_t green, uint16_t blue)
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

    for (uint8_t xc_ch = 0U ; xc_ch < xc_ch_max ; ++xc_ch)
    {
        for (uint8_t xd_daisy = 0U ; xd_daisy < xd_daisy_max ; ++xd_daisy)
        {
            uint16_t* p_ld_buffer = gt_xcr_ld_transfer_table.ld_buffer.data16[xc_ch][xd_daisy];
            for (uint8_t ld_order = 0U ; ld_order < ld_order_max ; ++ld_order)
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
    return XCR_LDIM_BURST_SIZE;
    //return XCR_LDIM_BURST_SIZE - 1U; // except CRC
}

uint16_t* ldim_get_xdr_ld_transfer_buffer(void)
{
    return gt_xcr_ld_transfer_table.ld_buffer.buffer;
}

uint16_t ldim_get_xdr_ld_transfer_size(void)
{
    return XDR_LDIM_BURST_SIZE;
}