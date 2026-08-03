#include "crc.h"

#include "ldim_conversion.h"
#include "comm_debugging.h"

#define XDR_LDIM_BURST_SIZE     (XDR_DAISY_LENGTH * XDR_LD_SIZE * XCR_CH_SIZE)
#define XCR_LDIM_BURST_SIZE     (1U + XDR_LDIM_BURST_SIZE + 1U)  /* HDR + Payload + crc16 */

#define LD_VAL 16383U

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
        ld_buffer_t ld_buffer;
    };
} xcr_ld_transfer_t;

static xcr_ld_transfer_t gt_xcr_ld_transfer_table;
static block_color_t gt_block_color_buffer[LDIM_BLK_SIZE];

void ldim_block_map_init(void)
{
    for (uint16_t blk = 0U; blk < XCR_LDIM_BURST_SIZE; ++blk)
    {
        gt_xcr_ld_transfer_table.ld_buffer.buffer[blk] = LD_VAL;
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