/** @file ads124s08.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __ADS124S08_C__


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "uart.h"
#include "ads124s08.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USE_DISPLAY_DEVICE_REGS
#define ADS114S08_READ_OFS_COUNT    (128) /* must be power of 2 */
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_ADC_SPS_T
{
    ADS_SPS_2_5 = 0,
    ADS_SPS_5,
    ADS_SPS_10,
    ADS_SPS_16_6,
    ADS_SPS_20,
    ADS_SPS_50,
    ADS_SPS_60,
    ADS_SPS_100,
    ADS_SPS_200,
    ADS_SPS_400,
    ADS_SPS_800,
    ADS_SPS_1000,
    ADS_SPS_2000,
    ADS_SPS_4000,
    ADS_SPS_RSVD,
}ads_sps_t;
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static SPI_TypeDef* gp_SPI;
static ads114s08_regs_t gt_ads114s08_regs;

static volatile bool gb_ads114s08_drdy_done;

static uint64_t gn_ads114s08_adc_temp;
static uint16_t gn_adc_read_count;

volatile uint16_t gn_ads114s08_read_timeout;
/* USER CODE END PV */

#ifdef USE_DISPLAY_DEVICE_REGS
static void ADS114S08_Dump_Registers(void)
{
    Print(LOG_DEBUG, "======== ADS114S08 regs value ========\r\n");

    Print(LOG_DEBUG, "ID : 0x%02X\r\n", gt_ads114s08_regs.id.value);
    Print(LOG_DEBUG, "\tDEV ID : %s\r\n", (gt_ads114s08_regs.id.u.dev_id == 0x04 ? "ADS114S08" : (gt_ads114s08_regs.id.u.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));

    Print(LOG_DEBUG, "Device Status : 0x%02X\r\n", gt_ads114s08_regs.status.value);
    Print(LOG_DEBUG, "\tPOR : %u\r\n", gt_ads114s08_regs.status.u.fl_por);
    Print(LOG_DEBUG, "\tRDY : %u\r\n", gt_ads114s08_regs.status.u.rdy);

    Print(LOG_DEBUG, "Data Rate : 0x%02X\r\n", gt_ads114s08_regs.datarate.value);
    Print(LOG_DEBUG, "\tDR : %u\r\n", gt_ads114s08_regs.datarate.u.dr);

    Print(LOG_DEBUG, "SYS : 0x%02X\r\n", gt_ads114s08_regs.sys.value);
    Print(LOG_DEBUG, "\tSENDSTAT : %u\r\n", gt_ads114s08_regs.sys.u.sendstat);
    Print(LOG_DEBUG, "\tCRC : %u\r\n", gt_ads114s08_regs.sys.u.crc);

    Print(LOG_DEBUG, "======================================\r\n");
}
#endif

static inline uint8_t xferWord(SPI_TypeDef *SPIx, uint8_t tx)
{
    uint8_t rx = 0;

    if (LL_SPI_IsEnabled(SPIx) != 1)
    {
      /* Enable SPI peripheral */
        LL_SPI_Enable(SPIx);
    }

    /* Set up data for the next xmit */
    while(!LL_SPI_IsActiveFlag_TXE(SPIx)) {};
    LL_SPI_TransmitData8(SPIx, tx);
    /* Wait for data to appear */
    while(!LL_SPI_IsActiveFlag_RXNE(SPIx)) {};
    /* Grab that data*/
    rx = LL_SPI_ReceiveData8(SPIx);

    return rx;
}

static uint8_t ADS114S08_Read_Register(uint8_t reg_addr)
{
    uint8_t TxBuffer[3] = {0, };
    uint8_t RxBuffer[3] = {0, };

    TxBuffer[0] = (CMD_RREG | (reg_addr & 0x1F));

    ADS_CS_LO();

    for(uint8_t i = 0 ; i < 3 ; ++i)
    {
        RxBuffer[i] = xferWord(gp_SPI, TxBuffer[i]);
    }

    ADS_CS_HI();

    return RxBuffer[2];
}

static int32_t ADS114S08_Get_RData(void)
{
    uint8_t TxBuffer[3] = {0, };
    uint8_t RxBuffer[3] = {0, };
    int32_t iData = 0;

    TxBuffer[0] = CMD_RDATA;

    ADS_CS_LO();

    for (uint8_t i = 0 ; i < 3 ; ++i)
    {
        RxBuffer[i] = xferWord(gp_SPI, TxBuffer[i]);
    }

    iData = (RxBuffer[1] << 8 | RxBuffer[2]);

    ADS_CS_HI();
    return iData;
}

static void ADS114S08_Write_Register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t TxBuffer[3];

    TxBuffer[0] = (CMD_WREG | (reg_addr & 0x1F));
    TxBuffer[1] = 0x00;
    TxBuffer[2] = reg_data;

    ADS_CS_LO();

    for(uint8_t i = 0 ; i < 3 ; ++i)
    {
        xferWord(gp_SPI, TxBuffer[i]);
    }

    ADS_CS_HI();
}

static void ADS114S08_Set_Input(uint8_t input_p, uint8_t input_n)
{
    ads114s08_inpmux_t t = {0,};

    t.u.muxp = input_p;
    t.u.muxn = input_n;

    ADS114S08_Write_Register(REG_ADDR_INPMUX, t.value);
}

void ADS114S08_Select_Input_CH(uint8_t input)
{
    switch(input)
    {
    case 0:
        ADS114S08_Set_Input(ADS_AIN0, ADS_AINCOM);
    break;
    case 1:
        ADS114S08_Set_Input(ADS_AIN1, ADS_AINCOM);
    break;
    case 2:
        ADS114S08_Set_Input(ADS_AIN2, ADS_AINCOM);
    break;
    case 3:
        ADS114S08_Set_Input(ADS_AIN3, ADS_AINCOM);
    break;
    case 4:
        ADS114S08_Set_Input(ADS_AIN4, ADS_AINCOM);
    break;
    case 5:
        ADS114S08_Set_Input(ADS_AIN5, ADS_AINCOM);
    break;
    case 6:
        ADS114S08_Set_Input(ADS_AIN6, ADS_AINCOM);
    break;
    case 7:
        ADS114S08_Set_Input(ADS_AIN7, ADS_AINCOM);
    break;
    case 8:
        ADS114S08_Set_Input(ADS_AIN8, ADS_AINCOM);
    break;
    case 9:
        ADS114S08_Set_Input(ADS_AIN9, ADS_AINCOM);
    break;
    case 10:
        ADS114S08_Set_Input(ADS_AIN10, ADS_AINCOM);
    break;
    case 11:
        ADS114S08_Set_Input(ADS_AIN11, ADS_AINCOM);
    break;
    default:
    break;
    }
}

static void ADS114S08_Set_CMD(uint8_t cmd_code)
{
    ADS_CS_LO();
    xferWord(gp_SPI, cmd_code);
    ADS_CS_HI();
}

static void ADS114S08_Reset(void)
{
    ADS114S08_Set_CMD(CMD_RESET);
}

void ADS114S08_Set_Start(uint8_t b_set)
{
    if (b_set)
    {
        gb_ads114s08_drdy_done = 0;
        gn_ads114s08_adc_temp = 0;
        gn_adc_read_count = ADS114S08_READ_COUNT;
        ADS114S08_Set_CMD(CMD_START);
    }
    else
    {
        ADS114S08_Set_CMD(CMD_STOP);
    }
}

void ADS114S08_Wait_Done(void)
{
    gn_ads114s08_read_timeout = 15; // 2000SPS * 16 EA = 8ms
    while(1)
    {
        if (gb_ads114s08_drdy_done)
        {
            break;
        }
        if (gn_ads114s08_read_timeout == 0)
        {
            Print(LOG_ERROR, "%s timeout\r\n", __func__);
            break;
        }
    }
}

void ADS114S08_Init(void)
{
    gp_SPI = SPI1;

    ADS114S08_Reset();
    LL_mDelay(1);

    for (uint8_t reg = REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
    {
        *(&gt_ads114s08_regs.id.value + reg) = ADS114S08_Read_Register(reg);
        Print(LOG_DEBUG, "reg[0x%02X] = 0x%02X\r\n", reg, *(&gt_ads114s08_regs.id.value + reg));
    }

    gt_ads114s08_regs.status.u.fl_por = 0x00; /* clear POR flag */
    gt_ads114s08_regs.status.u.rdy = 0x00; /* clear device ready flag */
    ADS114S08_Write_Register(REG_ADDR_STATUS, gt_ads114s08_regs.status.value);

    gt_ads114s08_regs.datarate.u.dr = ADS_SPS_2000; /* 2000 SPS */
    ADS114S08_Write_Register(REG_ADDR_DATARATE, gt_ads114s08_regs.datarate.value);

    for (uint8_t reg = REG_ADDR_ID ; reg < REG_ADDR_MAX ; ++reg)
    {
        *(&gt_ads114s08_regs.id.value + reg) = ADS114S08_Read_Register(reg);
        Print(LOG_DEBUG, "reg[0x%02X] = 0x%02X\r\n", reg, *(&gt_ads114s08_regs.id.value + reg));
    }

#ifdef USE_DISPLAY_DEVICE_REGS
    ADS114S08_Dump_Registers();
#endif
    Print(LOG_DEBUG, "\r\n %s Done\r\n", __func__);
}

void ADC_DRDY_INT_Handler(void)
{
    uint16_t temp = 0;
    temp = ADS114S08_Get_RData();

    if (temp > 32767)
    {
        temp = 0;
    }

    if (gn_adc_read_count)
    {
        gn_ads114s08_adc_temp += temp;
        --gn_adc_read_count;
    }

    if (gn_adc_read_count == 0)
    {
        gb_ads114s08_drdy_done = 1;
        ADS114S08_Set_Start(0);    /* stop continuous conversion */
    }
}

uint16_t ADS114S08_Get_ADC_Value(void)
{
    return (uint16_t)((float)gn_ads114s08_adc_temp / ADS114S08_READ_COUNT + 0.5f);
}
/*** end of file ***/