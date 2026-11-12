/* USER CODE BEGIN Header */
/*
    * File:   id601_dimming.h
    * Author: GT
    *
    * Created on 2026. 10. 27.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ID601_DIMMING_H__
#define __ID601_DIMMING_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdbool.h>
#include <stdint.h>
/* 2. Project internal / System-related headers */
#include "id601_metadata.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ID601_LD_SIZE_MAX_12BIT         (4095U)
#define ID601_LD_SIZE_MAX_14BIT         (16383U)

#define ID601_SV_NO_DEFAULT             (32U)
#define ID601_LD_TYPE_DEFAULT           (1U)
#define ID601_DELAY_CH_EN_DEFAULT       (0U)
#define ID601_SYNCMODE_DEFAULT          (0U)
#define ID601_PWM_RES_DEFAULT           (1U)
#define ID601_LD_DIR_DEFAULT            (1U)
#define ID601_LD_MODE_DEFAULT           (0U)

#define ID601_CHANNEL_SIZE              (12U)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern void id601_dimming_init(void);

extern void id601_config_ld_control(id601_ld_control_reg_t const* p_config);
extern id601_ld_control_reg_t const* id601_get_ld_control(void);

extern void id601_config_fault_read(bool enable);
extern bool id601_get_fault_read_enabled(void);

extern void id601_config_ld_transfer(bool enable);
extern bool id601_get_ld_transfer_enabled(void);

extern uint16_t id601_get_ld_size_max(void);

extern uint16_t* id601_get_ld_buffer(void);
extern void id601_set_ld_buffer(uint16_t daisy, uint16_t ch, uint16_t led, uint16_t value);

extern void id601_vsync_dimming_task(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __ID601_DIMMING_H__ */
