/* USER CODE BEGIN Header */
/*
    * File:   drv_id601.h
    * Author: GT
    *
    * Created on 2026. 09. 22.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID601_H__
#define __DRV_ID601_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "id601_metadata.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint8_t (*id601_crc_func_t)(uint32_t, uint16_t);

typedef enum tag_ID601_REGISTER_BANK
{
    ID601_REGISTER_BANK_GENERAL = 0U,
    ID601_REGISTER_BANK_MIRROR,
    ID601_REGISTER_BANK_COUNT,
} id601_register_bank_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void drv_id601_init(void);

extern bool id601_write_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, const uint16_t new_value, id601_crc_func_t p_crc_func);
extern bool id601_read_register(const uint16_t bank, const uint16_t device_select, const uint16_t addr, id601_crc_func_t p_crc_func);
extern bool id601_ld_transfer(uint16_t* p_buff, id601_crc_func_t p_crc_func);
extern bool id601_fault_read(id601_crc_func_t p_crc_func);
extern bool id601_sync_gen(id601_crc_func_t p_crc_func);
extern bool id601_id_gen(id601_crc_func_t p_crc_func);

extern bool id601_init_analog_test_mode(void);

extern void id601_trim_init_iref1(void);
extern void id601_trim_init_iref2(void);
extern void id601_trim_init_ldo_det(void);
extern void id601_trim_init_ldo_dig(void);
extern void id601_trim_init_ldo_dac(void);
extern void id601_trim_init_ldo_fll(void);
extern void id601_trim_init_ofs_temp(void);
extern void id601_trim_init_osc_rctl(void);
extern void id601_trim_init_gain(void);
extern void id601_trim_init_offset(void);

extern void id601_trim_init_gain_vref1(void);
extern void id601_trim_init_gain_vref2(void);
extern void id601_trim_init_offset_vref1(void);
extern void id601_trim_init_offset_vref2(void);

extern void id601_trim_init_ofs_temp_o1(void);
extern void id601_trim_init_ofs_temp_o2(void);
extern void id601_trim_init_current_chx(uint16_t ch);

extern void id601_trim_write_iref1(uint16_t val);
extern void id601_trim_write_iref2(uint16_t val);
extern void id601_trim_write_ldo_det(uint16_t val);
extern void id601_trim_write_ldo_dig(uint16_t val);
extern void id601_trim_write_ldo_dac(uint16_t val);
extern void id601_trim_write_ldo_fll(uint16_t val);
extern void id601_trim_write_ofs_temp(uint16_t val);
extern void id601_trim_write_osc_rctl(uint16_t val);
extern void id601_trim_write_gain(uint16_t val);
extern void id601_trim_write_offset(uint16_t val);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID601_H__ */
