/* USER CODE BEGIN Header */
/*
    * File:   drv_mcp23017.h
    * Author: GT
    *
    * Created on 2026. 09. 23.
    */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_MCP23017_H__
#define __DRV_MCP23017_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stdbool.h>
/* 2. Project internal / System-related headers */
#include "main.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
    MCP_GPA0 = 0x00U,
    MCP_GPA1,
    MCP_GPA2,
    MCP_GPA3,
    MCP_GPA4,
    MCP_GPA5,
    MCP_GPA6,
    MCP_GPA7,

    MCP_GPB0 = 0x08U,
    MCP_GPB1,
    MCP_GPB2,
    MCP_GPB3,
    MCP_GPB4,
    MCP_GPB5,
    MCP_GPB6,
    MCP_GPB7,

    MCP_GPIO_MAX = 0x10U,
} MCP_GPIO;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
extern HAL_StatusTypeDef mcp23017_write(uint8_t reg, uint8_t data);
extern HAL_StatusTypeDef mcp23017_read(uint8_t reg, uint8_t* p_buff);

extern bool mcp23017_init(void);

extern bool mcp23017_set_gpio_state(uint8_t pin, bool state);
extern bool mcp23017_get_gpio_state(uint8_t pin, bool* p_state);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_MCP23017_H__ */
