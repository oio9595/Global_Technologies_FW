#include "crc.h"
#include "drv_xcr24.h"
#include "ldim_conversion.h"

#define COLOR_NUM       (3U)    /* R/G/B */

#define XCR_LDIM_BURST_SIZE     (1U + (XCR_CH_SIZE * COLOR_NUM * XD_LINE_LENGTH) + 1U)  /* HDR + Payload + crc16 */
#define XDR_LDIM_BURST_SIZE     (XCR_LDIM_BURST_SIZE)

const static uint8_t gn_Block_Map[LDIM_BLK_SIZE][2U] = /* Front View : { xcr_ch, xdr_daised_ch } */
{
    {  0,   0 },{  1,   0 },{  2,   0 },{  3,   0 },{  4,   0 },{  5,   0 },{  6,   0 },{  7,   0 },{  8,   0 },{  9,   0 },{ 10,   0 },{ 11,   0 },{ 12,   0 },{ 13,   0 },{ 14,   0 },{ 15,   0 },{ 16,   0 },{ 17,   0 },{ 18,   0 },{ 19,   0 },{ 20,   0 },{ 21,   0 },{ 22,   0 },{ 23,   0 },
    {  0,   1 },{  1,   1 },{  2,   1 },{  3,   1 },{  4,   1 },{  5,   1 },{  6,   1 },{  7,   1 },{  8,   1 },{  9,   1 },{ 10,   1 },{ 11,   1 },{ 12,   1 },{ 13,   1 },{ 14,   1 },{ 15,   1 },{ 16,   1 },{ 17,   1 },{ 18,   1 },{ 19,   1 },{ 20,   1 },{ 21,   1 },{ 22,   1 },{ 23,   1 },
    {  0,   2 },{  1,   2 },{  2,   2 },{  3,   2 },{  4,   2 },{  5,   2 },{  6,   2 },{  7,   2 },{  8,   2 },{  9,   2 },{ 10,   2 },{ 11,   2 },{ 12,   2 },{ 13,   2 },{ 14,   2 },{ 15,   2 },{ 16,   2 },{ 17,   2 },{ 18,   2 },{ 19,   2 },{ 20,   2 },{ 21,   2 },{ 22,   2 },{ 23,   2 },
    {  0,   3 },{  1,   3 },{  2,   3 },{  3,   3 },{  4,   3 },{  5,   3 },{  6,   3 },{  7,   3 },{  8,   3 },{  9,   3 },{ 10,   3 },{ 11,   3 },{ 12,   3 },{ 13,   3 },{ 14,   3 },{ 15,   3 },{ 16,   3 },{ 17,   3 },{ 18,   3 },{ 19,   3 },{ 20,   3 },{ 21,   3 },{ 22,   3 },{ 23,   3 },
    {  0,   4 },{  1,   4 },{  2,   4 },{  3,   4 },{  4,   4 },{  5,   4 },{  6,   4 },{  7,   4 },{  8,   4 },{  9,   4 },{ 10,   4 },{ 11,   4 },{ 12,   4 },{ 13,   4 },{ 14,   4 },{ 15,   4 },{ 16,   4 },{ 17,   4 },{ 18,   4 },{ 19,   4 },{ 20,   4 },{ 21,   4 },{ 22,   4 },{ 23,   4 },
    {  0,   5 },{  1,   5 },{  2,   5 },{  3,   5 },{  4,   5 },{  5,   5 },{  6,   5 },{  7,   5 },{  8,   5 },{  9,   5 },{ 10,   5 },{ 11,   5 },{ 12,   5 },{ 13,   5 },{ 14,   5 },{ 15,   5 },{ 16,   5 },{ 17,   5 },{ 18,   5 },{ 19,   5 },{ 20,   5 },{ 21,   5 },{ 22,   5 },{ 23,   5 },
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
        uint16_t data16[XDR_DAISY_LENGTH][XDR_CH_LENGTH][COLOR_NUM];
    };
}xdr_ld_transfer_t;

static xcr_ld_transfer_t gt_xcr_ld_transfer_table;
static xdr_ld_transfer_t gt_xdr_ld_transfer_table;

void ldim_set_ldim_rgb(uint16_t block, uint16_t red, uint16_t green, uint16_t blue)
{
    uint8_t xcr_ch = gn_Block_Map[block][0U];
    uint8_t xdr_ch = gn_Block_Map[block][1U];

    gt_xcr_ld_transfer_table.data16[xdr_ch][0U][xcr_ch] = red;
    gt_xcr_ld_transfer_table.data16[xdr_ch][1U][xcr_ch] = green;
    gt_xcr_ld_transfer_table.data16[xdr_ch][2U][xcr_ch] = blue;

    if((LDIM_BLK_SIZE -1U) == block)
    {
        gt_xcr_ld_transfer_table.cmd.bit.code = CMD_CODE3;
        gt_xcr_ld_transfer_table.cmd.bit.addr = 0U;
        gt_xcr_ld_transfer_table.cmd.bit.size = (XD_LINE_LENGTH * COLOR_NUM);

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

