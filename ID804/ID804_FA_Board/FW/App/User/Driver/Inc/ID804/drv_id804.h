/* USER CODE BEGIN Header */
/*
    * File:   drv_id804.h
    * Author: GT
    *
    * Created on 2026. 09. 01.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_ID804_H__
#define __DRV_ID804_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "id804_metadata.h"
#include "drv_gpio.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ID804_DEV_ADDR_DUMMY    (0xFFFFU)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum tag_ID804_COMM_MODE
{
    ID804_COMM_MODE_NONE = 0U,
    ID804_COMM_MODE_ME,
    ID804_COMM_MODE_I2C,
    ID804_COMM_MODE_MAX,
} id804_comm_mode_t;

typedef uint32_t (*id804_get_addr_func)(uint16_t);
typedef bool (*id804_set_addr_func)(uint16_t, uint32_t);

typedef uint32_t (*id804_get_addr_otp_func)(void);
typedef bool (*id804_set_addr_otp_func)(uint32_t);

typedef uint32_t (*id804_get_cmd_func)(uint16_t);
typedef bool (*id804_set_cmd_func)(uint16_t, uint32_t);
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern bool id804_boot_i2c(void);
extern bool id804_boot_me(void);

extern bool id804_boot_lvds(id804_comm_direction_t direction);
extern bool id804_boot_can(id804_comm_direction_t direction);

extern void id804_set_comm_mode(id804_comm_mode_t mode);
extern id804_comm_mode_t id804_get_comm_mode(void);

extern id804_comm_result_t id804_read(uint16_t dev_addr, uint8_t target, uint32_t* p_data);
extern id804_comm_result_t id804_write(uint16_t dev_addr, uint8_t target, uint32_t data);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ID804_H__ */
