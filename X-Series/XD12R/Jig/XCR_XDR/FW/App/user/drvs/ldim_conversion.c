
#include "drv_xdr12.h"
#include "drv_xcr24.h"

#include "comm_debugging.h"

#include "crc.h"

#include "ldim_conversion.h"

#define COLOR_NUM       (3U)    /* R/G/B */

#define XCR_LDIM_BURST_SIZE     (1U + (XCR_CH_SIZE * COLOR_NUM * XD_LINE_LENGTH) + 1U)  /* HDR + Payload + crc16 */ 
#define XDR_LDIM_BURST_SIZE     (XCR_LDIM_BURST_SIZE)

#define LDIM_BLK_SIZE           (100U)
const static uint8_t gn_LED_Map[LDIM_BLK_SIZE][2U] = /* Front View : { xcr_ch, xdr_daised_ch } */
{
    {  0, 100 },
};
/* RED : (xdr_daised_ch * 3) + 0  */
/* GREEN : (xdr_daised_ch * 3) + 1  */
/* BLUE : (xdr_daised_ch * 3) + 2  */

typedef union _xcr_ld_transfer_
{
    uint16_t buffer[XCR_LDIM_BURST_SIZE];
    struct
    {
        _cmd_t    cmd;
        uint16_t  data16[XD_LINE_LENGTH][COLOR_NUM][XCR_CH_SIZE];
        uint16_t  crc16;
    };
}xcr_ld_transfer_t;

typedef union _xdr_ld_transfer_
{
    uint16_t buffer[(XDR_DAISY_LENGTH * XDR_CH_LENGTH * COLOR_NUM)];
    struct
    {
        uint16_t        data16[XDR_DAISY_LENGTH][XDR_CH_LENGTH][COLOR_NUM];
    };
}xdr_ld_transfer_t;

static xcr_ld_transfer_t gt_xcr_ld_transfer_table;
static xdr_ld_transfer_t gt_xdr_ld_transfer_table;

void ldim_set_ld_color(uint16_t red, uint16_t green, uint16_t blue)
{
    gt_xcr_ld_transfer_table.cmd.bit.code = CMD_CODE3;
    gt_xcr_ld_transfer_table.cmd.bit.addr = 0U;
    gt_xcr_ld_transfer_table.cmd.bit.size = XD_LINE_LENGTH;

    for(uint16_t line = 0U ; line < XD_LINE_LENGTH ; ++line)
    {
        for(uint16_t xcr_ch = 0U ; xcr_ch < XCR_CH_SIZE ; ++xcr_ch)
        {
            gt_xcr_ld_transfer_table.data16[line][0U][xcr_ch] = red;
            gt_xcr_ld_transfer_table.data16[line][1U][xcr_ch] = green;
            gt_xcr_ld_transfer_table.data16[line][2U][xcr_ch] = blue;
        }
    }

    gt_xcr_ld_transfer_table.crc16 = Calculate_CRC16_CCITT_False(gt_xcr_ld_transfer_table.buffer, (XCR_LDIM_BURST_SIZE - 1U));

    for(uint16_t xdr_daisy = 0U ; xdr_daisy < XDR_DAISY_LENGTH ; ++xdr_daisy)
    {
        for(uint16_t xdr_ch = 0U ; xdr_ch < XDR_CH_LENGTH ; ++xdr_ch)
        {
            gt_xdr_ld_transfer_table.data16[xdr_daisy][xdr_ch][0U] = red;
            gt_xdr_ld_transfer_table.data16[xdr_daisy][xdr_ch][1U] = green;
            gt_xdr_ld_transfer_table.data16[xdr_daisy][xdr_ch][2U] = blue;
        }
    }
}

uint16_t* ldim_get_xcr_ld_transfer_buffer(void)
{
    return gt_xcr_ld_transfer_table.buffer;
}

uint16_t ldim_get_xcr_ld_transfer_size(void)
{
    return XCR_LDIM_BURST_SIZE;
}

uint16_t* ldim_get_xdr_ld_transfer_buffer(void)
{
    return gt_xdr_ld_transfer_table.buffer;
}

uint16_t ldim_get_xdr_ld_transfer_size(void)
{
    return (uint16_t)(XDR_DAISY_LENGTH * XDR_CH_LENGTH * COLOR_NUM);
}

