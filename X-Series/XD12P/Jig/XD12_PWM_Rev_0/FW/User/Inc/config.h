
/** @file config.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__


/* TEST CONFIG  ***************************************/
//#define _DBG_ALL // Release Debug Print

#define DBG_TEST //Developing
//#define DBG_TRIM_ALGORITHM
/********************************************************/


/* TARGET CHIP CONFIG  **********************************/
#define TARGET_CHIP_NAME "XD12"
/********************************************************/


/* TRIMMING CONFIG  **********************************/
#define INIT_ADC_PER_REG_OSC        (1)
#define INIT_ADC_PER_REG_VREF       (1)
#define INIT_ADC_PER_REG_ICTL_L     (6)
#define INIT_ADC_PER_REG_ICTL_H     (18)
/********************************************************/


/********************************************************/
#define XD_DAISY_SIZE           (1)
#define XD_CH_SIZE              (12)

#define XC_SERIAL_CLK_CNT_LOW   (6)
#define XC_SERIAL_CLK_CNT_HIGH  (10)

#define XD_SERIAL_CLK_CNT_LOW   (13)
#define XD_SERIAL_CLK_CNT_HIGH  (26)

#define XD12_FREQ_ERR_RATE      (2.0)
#define XD12_FOSC_MIN           (39019200UL)
#define XD12_FOSC_TYP           (39319200UL)
#define XD12_FOSC_MAX           (39619200UL)

#define XD12_DELAY              (((1000000.0 / XD12_FOSC_MIN) * (XD_SERIAL_CLK_CNT_HIGH + XD_SERIAL_CLK_CNT_LOW)))
#define XD12_WRITE_BITS         (22)
#define XD12_READ_BITS          (10)
#define XD12_READ_RECV_BITS     (21)
#define XD12_IDGEN_BITS         (4)

#define XD12_WRITE_DELAY        ((uint32_t)((XD12_DELAY * XD12_WRITE_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_IDGEN_DELAY        ((uint32_t)((XD12_DELAY * XD12_IDGEN_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_DELAY_DELAY        ((uint32_t)((XD12_DELAY * XD12_WRITE_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_READ_DELAY         ((uint32_t)((XD12_DELAY * XD12_READ_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
#define XD12_READ_RECV_DELAY    ((uint32_t)((XD12_DELAY * XD12_READ_RECV_BITS * XD_DAISY_SIZE) * XD12_FREQ_ERR_RATE + 0.5f))
/********************************************************/

#endif /* ~__CONFIG_H__ */

#include "main.h"
#include "types.h"
#include "common.h"
#include "trimming.h"
#include "JigBd_IF.h"
#include "JigBd_latch.h"
#include "ADS124S08.h"
#include "xd12.h"
#include "xc24.h"
#include "dimming_test.h"
#include "MCP4251.h"

/*** end of file ***/
