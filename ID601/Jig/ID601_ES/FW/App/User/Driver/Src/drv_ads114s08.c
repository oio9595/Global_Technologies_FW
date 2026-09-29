/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_ads114s08.c
    * @brief          : ADS114S08 driver implementation
    ******************************************************************************
    * @attention
    *
    * Copyright (c) 2026 Global Technologies.
    * All rights reserved.
    *
    ******************************************************************************
    */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. Direct pairing header (Corresponding header for this source file) */

/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */
#include "main.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "drv_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum tag_ADS_DEVICE
{
    ADS_DEV_1 = 0U,
    ADS_DEV_2,
    ADS_DEV_MAX,
} ads_device_t;

typedef enum tag_ADS114_SPS
{
    ADS_SPS_2_5 = 0U,
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
} ads114_sps_t;

typedef enum tag_ADS114_ADDR
{
    REG_ADDR_ID         = 0x00U, // 0x00
    REG_ADDR_STATUS     = 0x01U, // 0x01
    REG_ADDR_INPMUX     = 0x02U, // 0x02
    REG_ADDR_PGA        = 0x03U, // 0x03
    REG_ADDR_DATARATE   = 0x04U, // 0x04
    REG_ADDR_REF        = 0x05U, // 0x05
    REG_ADDR_IDACMAG    = 0x06U, // 0x06
    REG_ADDR_IDACMUX    = 0x07U, // 0x07
    REG_ADDR_VBIAS      = 0x08U, // 0x08
    REG_ADDR_SYS        = 0x09U, // 0x09
    REG_ADDR_RESERVED1  = 0x0AU, // 0x0A
    REG_ADDR_OFCAL0     = 0x0BU, // 0x0B
    REG_ADDR_OFCAL1     = 0x0CU, // 0x0C
    REG_ADDR_RESERVED2  = 0x0DU, // 0x0D
    REG_ADDR_FSCAL0     = 0x0EU, // 0x0E
    REG_ADDR_FSCAL1     = 0x0FU, // 0x0F
    REG_ADDR_GPIODAT    = 0x10U, // 0x10
    REG_ADDR_GPIOCON    = 0x11U, // 0x11
    REG_ADDR_MAX        = 0x12U  // 0x12
} ads114_addr_t;

typedef struct tag_ADS114S08_ID
{
    uint8_t dev_id   : 3;
    uint8_t reserved : 5;
} ads114s08_id_t;

typedef struct tag_ADS114S08_STATUS
{
    uint8_t fl_ref_l0   : 1;
    uint8_t fl_ref_l1   : 1;
    uint8_t fl_n_railn  : 1;
    uint8_t fl_n_railp  : 1;
    uint8_t fl_p_railn  : 1;
    uint8_t fl_p_railp  : 1;
    uint8_t rdy         : 1;
    uint8_t fl_por      : 1;
} ads114s08_status_t;

typedef struct tag_ADS114S08_INPMUX
{
    uint8_t muxn : 4;
    uint8_t muxp : 4;
} ads114s08_inpmux_t;

typedef struct tag_ADS114S08_PGA
{
    uint8_t gain   : 3;
    uint8_t pga_en : 2;
    uint8_t delay  : 3;
} ads114s08_pga_t;

typedef struct tag_ADS114S08_DATARATE
{
    uint8_t dr     : 4;
    uint8_t filter : 1;
    uint8_t mode   : 1;
    uint8_t clk    : 1;
    uint8_t g_chop : 1;
} ads114s08_datarate_t;

typedef struct tag_ADS114S08_REF
{
    uint8_t refcon   : 2;
    uint8_t refsel   : 2;
    uint8_t refn_buf : 1;
    uint8_t refp_buf : 1;
    uint8_t fl_ref_en : 2;
} ads114s08_ref_t;

typedef struct tag_ADS114S08_IDACMAG
{
    uint8_t imag       : 4;
    uint8_t zero1      : 1;
    uint8_t zero2      : 1;
    uint8_t psw        : 1;
    uint8_t fl_rail_en : 1;
} ads114s08_idacmag_t;

typedef struct tag_ADS114S08_IDACMUX
{
    uint8_t i1mux : 4;
    uint8_t i2mux : 4;
} ads114s08_idacmux_t;

typedef struct tag_ADS114S08_VBIAS
{
    uint8_t vb_ain0  : 1;
    uint8_t vb_ain1  : 1;
    uint8_t vb_ain2  : 1;
    uint8_t vb_ain3  : 1;
    uint8_t vb_ain4  : 1;
    uint8_t vb_ain5  : 1;
    uint8_t vb_ainc  : 1;
    uint8_t vb_level : 1;
} ads114s08_vbias_t;

typedef struct tag_ADS114S08_SYS
{
    uint8_t sendstat : 1;
    uint8_t crc      : 1;
    uint8_t timeout  : 1;
    uint8_t cal_samp : 2;
    uint8_t sys_mon  : 3;
} ads114s08_sys_t;

typedef struct tag_ADS114S08_OFCAL0
{
    uint8_t ofc : 8;
} ads114s08_ofcal0_t;

typedef struct tag_ADS114S08_OFCAL1
{
    uint8_t ofc : 8;
} ads114s08_ofcal1_t;

typedef struct tag_ADS114S08_FSCAL0
{
    uint8_t fsc : 8;
} ads114s08_fscal0_t;

typedef struct tag_ADS114S08_FSCAL1
{
    uint8_t fsc : 8;
} ads114s08_fscal1_t;

typedef struct tag_ADS114S08_GPIODAT
{
    uint8_t dat : 4;
    uint8_t dir : 4;
} ads114s08_gpiodat_t;

typedef struct tag_ADS114S08_GPIOCON
{
    uint8_t con      : 4;
    uint8_t reserved : 4;
} ads114s08_gpiocon_t;

typedef struct tag_ADS114S08_RESERVED
{
    uint8_t rsvd : 8;
} ads114s08_reserved_t;

typedef struct tag_ADS114S08_REGS
{
    ads114s08_id_t       id;         /* 00h */
    ads114s08_status_t   status;     /* 01h */
    ads114s08_inpmux_t   inpmux;     /* 02h */
    ads114s08_pga_t      pga;        /* 03h */
    ads114s08_datarate_t datarate;   /* 04h */
    ads114s08_ref_t      ref;        /* 05h */
    ads114s08_idacmag_t  idacmag;    /* 06h */
    ads114s08_idacmux_t  idacmux;    /* 07h */
    ads114s08_vbias_t    vbias;      /* 08h */
    ads114s08_sys_t      sys;        /* 09h */
    ads114s08_reserved_t reserved1;  /* 0Ah */
    ads114s08_ofcal0_t   ofcal0;     /* 0Bh */
    ads114s08_ofcal1_t   ofcal1;     /* 0Ch */
    ads114s08_reserved_t reserved2;  /* 0Dh */
    ads114s08_fscal0_t   fscal0;     /* 0Eh */
    ads114s08_fscal1_t   fscal1;     /* 0Fh */
    ads114s08_gpiodat_t  gpiodat;    /* 10h */
    ads114s08_gpiocon_t  gpiocon;    /* 11h */
} ads114s08_regs_t;

typedef bool (*ads114_spi_cs_fn_t)(bool);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USE_DISPLAY_DEVICE_REGS
#define ADS114S08_SPI_BASE      (SPI2)

#define ADS114S08_READ_COUNT    (16U) /* must be power of 2 */

/* Commands */
#define CMD_NOP                 (0x00U)
#define CMD_WAKEUP              (0x02U) /* or 0x03 */
#define CMD_POWER_DOWN          (0x04U) /* or 0x05 */
#define CMD_RESET               (0x06U) /* or 0x07 */
#define CMD_START               (0x08U) /* or 0x09 */
#define CMD_STOP                (0x0AU) /* or 0x0B */
#define CMD_SYOCAL              (0x16U) /* System offset calibration */
#define CMD_SYGCAL              (0x17U) /* System gain calibration */
#define CMD_SFOCAL              (0x19U) /* Self offset calibration */
#define CMD_RDATA               (0x12U) /* or 0x13 */
#define CMD_RREG                (0x20U) /* Read nnnnn registers starting at address rrrrr */
#define CMD_WREG                (0x40U) /* Write nnnnn registers starting at address rrrrr */

#define LTC_R_RIN               (2200.0f)
#define LTC_R_ROUT              (3300.0f)
#define LTC_R_RS_HIGH           (13.1f)
#define LTC_R_RS_MID            (75.0f)
#define LTC_R_RS_LOW            (3900.0f)

#define ICC_XD_R                (22.0f)
#define ICC_XC_R                (4.7f)

#define ADC_VREF                (5000.0f) /* mV */
#define ADC_RES                 (32768U)
#define mVOLTAGE_PER_ADC        (ADC_VREF / (float)ADC_RES)
#define ADC_CONV_COEFF_HIGH     (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_HIGH * LTC_R_ROUT)))
#define ADC_CONV_COEFF_MID      (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_MID * LTC_R_ROUT)))
#define ADC_CONV_COEFF_LOW      (mVOLTAGE_PER_ADC * (LTC_R_RIN / (LTC_R_RS_LOW * LTC_R_ROUT)))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static ads114s08_regs_t gt_ads114s08_regs[ADS_DEV_MAX];
static volatile bool gb_ads114s08_conversion_done;

static int32_t gn_ads114s08_adc_sum;
static uint16_t gn_ads114s08_conversion_count;

static volatile uint16_t gn_ads114s08_conversion_timeout;

static const ads114_spi_cs_fn_t gp_ads114s08_cs_transition[2] = { drv_gpio_ads114s08_dev1_cs, drv_gpio_ads114s08_dev2_cs };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef USE_DISPLAY_DEVICE_REGS
static void ads114s08_dump_registers(ads_device_t dev)
{
    if (dev >= ADS_DEV_MAX)
    {
        return;
    }
    drv_uart_printf("\r\n======== ADS114S08 regs value ========");

    drv_uart_printf("\r\nID : 0x%X", *(uint8_t*)(&gt_ads114s08_regs[dev].id));
    drv_uart_printf("\r\n\tDEV ID : %s", (gt_ads114s08_regs[dev].id.dev_id == 0x04 ? "ADS114S08" : (gt_ads114s08_regs[dev].id.dev_id == 0x05 ? "ADS114S06" : "UNKNOWN")));

    drv_uart_printf("\r\nDevice Status : 0x%X", *(uint8_t*)(&gt_ads114s08_regs[dev].status));
    drv_uart_printf("\r\n\tPOR : %u", gt_ads114s08_regs[dev].status.fl_por);
    drv_uart_printf("\r\n\tRDY : %u", gt_ads114s08_regs[dev].status.rdy);

    drv_uart_printf("\r\nData Rate : 0x%X", *(uint8_t*)(&gt_ads114s08_regs[dev].datarate));
    drv_uart_printf("\r\n\tDR : %u", gt_ads114s08_regs[dev].datarate.dr);

    drv_uart_printf("\r\nSYS : 0x%X", *(uint8_t*)(&gt_ads114s08_regs[dev].sys));
    drv_uart_printf("\r\n\tSENDSTAT : %u", gt_ads114s08_regs[dev].sys.sendstat);
    drv_uart_printf("\r\n\tCRC : %u", gt_ads114s08_regs[dev].sys.crc);

    drv_uart_printf("\r\n======================================");
}
#endif

static uint8_t ads114s08_read_register(ads_device_t dev, uint8_t reg_addr)
{
    if (dev >= ADS_DEV_MAX)
    {
        return 0U;
    }

    uint8_t TxBuffer[3] = { 0U };
    uint8_t RxBuffer[3] = { 0U };

    TxBuffer[0] = (CMD_RREG | (reg_addr & 0x1FU));

    gp_ads114s08_cs_transition[dev](false);
    if (false == drv_spi_receive_dma_8bit(ADS114S08_SPI_BASE, TxBuffer, RxBuffer, 3U, 20U))
    {
        gp_ads114s08_cs_transition[dev](true);
        return 0U;
    }

    return RxBuffer[2];
}

static bool ads114s08_write_register(ads_device_t dev, uint8_t reg_addr, uint8_t reg_data)
{
    if (dev >= ADS_DEV_MAX)
    {
        return false;
    }

    uint8_t TxBuffer[3] = { 0U };

    TxBuffer[0] = (CMD_WREG | (reg_addr & 0x1FU));
    TxBuffer[1] = 0x00U;
    TxBuffer[2] = reg_data;

    gp_ads114s08_cs_transition[dev](false);
    if (false == drv_spi_transmit_dma_8bit(ADS114S08_SPI_BASE, TxBuffer, 3U, 20U))
    {
        gp_ads114s08_cs_transition[dev](true);
        return false;
    }
    return true;
}

static int16_t ads114s08_read_conversion(ads_device_t dev)
{
    if (dev >= ADS_DEV_MAX)
    {
        return 0;
    }

    uint8_t TxBuffer[3] = { 0U };
    uint8_t RxBuffer[3] = { 0U };

    TxBuffer[0] = CMD_RDATA;

    gp_ads114s08_cs_transition[dev](false);
    if (false == drv_spi_receive_dma_8bit(ADS114S08_SPI_BASE, TxBuffer, RxBuffer, 3U, 20U))
    {
        gp_ads114s08_cs_transition[dev](true);
        return 0;
    }

    return (int16_t)(((uint16_t)RxBuffer[1] << 8U) | ((uint16_t)RxBuffer[2] << 0U));
}

static void ads114s08_send_command(ads_device_t dev, uint8_t cmd_code)
{
    if (dev >= ADS_DEV_MAX)
    {
        return;
    }
    uint8_t TxBuffer[1] = { 0U };
    TxBuffer[0] = cmd_code;

    gp_ads114s08_cs_transition[dev](false);
    if (false == drv_spi_transmit_dma_8bit(ADS114S08_SPI_BASE, TxBuffer, 1U, 20U))
    {
        gp_ads114s08_cs_transition[dev](true);
        return;
    }
}

void ads114s08_set_input_mux(ads_device_t dev, uint8_t input_p, uint8_t input_n)
{
    if (dev >= ADS_DEV_MAX)
    {
        return;
    }
    ads114s08_inpmux_t input_mux;

    input_mux.muxp = input_p;
    input_mux.muxn = input_n;

    uint8_t value = (input_mux.muxp << 4U) | (input_mux.muxn << 0U);

    ads114s08_write_register(dev, REG_ADDR_INPMUX, value);
}

static void ads114s08_reset(ads_device_t dev)
{
    if (dev >= ADS_DEV_MAX)
    {
        return;
    }
    ads114s08_send_command(dev, CMD_RESET);
}

void ads114s08_set_conversion_enable(ads_device_t dev, bool b_start)
{
    if (dev >= ADS_DEV_MAX)
    {
        return;
    }
    if(true == b_start)
    {
        gb_ads114s08_conversion_done = false;
        gn_ads114s08_adc_sum = 0;
        gn_ads114s08_conversion_count = ADS114S08_READ_COUNT;
        gn_ads114s08_conversion_timeout = 15U; // 2000SPS * 16 EA = 8ms
        ads114s08_send_command(dev, CMD_START);
    }
    else
    {
        ads114s08_send_command(dev, CMD_STOP);
    }
}

bool ads114s08_wait_conversion_complete(void)
{
    while (false == gb_ads114s08_conversion_done)
    {
        if (0U == gn_ads114s08_conversion_timeout)
        {
            drv_uart_printf("\r\nADS114S08 timeout!");
            return false;
        }
    }

    return true;
}

void ads114s08_timeout(void)
{
    if (gn_ads114s08_conversion_timeout)
    {
        --gn_ads114s08_conversion_timeout;
    }
}

void ads114s08_init(void)
{
    for (ads_device_t dev = ADS_DEV_1; dev < ADS_DEV_MAX; ++dev)
    {
        ads114s08_reset(dev);
        LL_mDelay(1U);

        for(uint8_t reg = REG_ADDR_ID; reg < REG_ADDR_MAX; ++reg)
        {
            *((uint8_t *)&(gt_ads114s08_regs[dev].id) + reg) = ads114s08_read_register(dev, reg);
            drv_uart_printf("\r\nreg[0x%02X] = 0x%02X", reg, *((uint8_t *)&(gt_ads114s08_regs[dev].id) + reg));
        }
        gt_ads114s08_regs[dev].status.fl_por = 0U; /* Clear POR flag */
        gt_ads114s08_regs[dev].status.rdy = 0U; /* Clear Device Ready Flag */
        ads114s08_write_register(dev, REG_ADDR_STATUS, *(uint8_t*)(&gt_ads114s08_regs[dev].status));

        gt_ads114s08_regs[dev].datarate.dr = ADS_SPS_2000; /* 2000 SPS */
        ads114s08_write_register(dev, REG_ADDR_DATARATE, *(uint8_t*)(&gt_ads114s08_regs[dev].datarate));

        gt_ads114s08_regs[dev].ref.refsel = 0U; /* Reference Input Select REFP0, REFN0 */
        gt_ads114s08_regs[dev].ref.refcon = 0U; /* Internal Reference Off */
        ads114s08_write_register(dev, REG_ADDR_REF, *(uint8_t*)(&gt_ads114s08_regs[dev].ref));

        gt_ads114s08_regs[dev].gpiocon.con = 0U; /* GPIO[x] Configured As Analog Input */
        ads114s08_write_register(dev, REG_ADDR_GPIOCON, *(uint8_t*)(&gt_ads114s08_regs[dev].gpiocon));

        #ifdef USE_DISPLAY_DEVICE_REGS
            ads114s08_dump_registers(dev);
        #endif
    }
}

void ads114s08_drdy1_irq_handler(void)
{
    int32_t temp = ads114s08_read_conversion(ADS_DEV_1);

    if (temp > 32767U)
    {
        temp = 0U;
    }

    if (gn_ads114s08_conversion_count > 0U)
    {
        gn_ads114s08_adc_sum += temp;
        --gn_ads114s08_conversion_count;
    }

    if (0U == gn_ads114s08_conversion_count)
    {
        gb_ads114s08_conversion_done = true;
        ads114s08_set_conversion_enable(ADS_DEV_1, false);    /* stop continuous conversion */
    }
}

void ads114s08_drdy2_irq_handler(void)
{
    int32_t temp = ads114s08_read_conversion(ADS_DEV_1);

    if (temp > 32767U)
    {
        temp = 0U;
    }

    if (gn_ads114s08_conversion_count > 0U)
    {
        gn_ads114s08_adc_sum += temp;
        --gn_ads114s08_conversion_count;
    }

    if (0U == gn_ads114s08_conversion_count)
    {
        gb_ads114s08_conversion_done = true;
        ads114s08_set_conversion_enable(ADS_DEV_1, false);    /* stop continuous conversion */
    }
}

/* USER CODE END 0 */
