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
#define MCP23017_I2C_ADDR   (0x21U << 1U)
#define I2C_Tx              (0U)
#define I2C_Rx              (1U)

#define MCP_IODIRA          (0x00U)
#define MCP_IODIRB          (0x01U)
#define MCP_IPOLA           (0x02U)
#define MCP_IPOLB           (0x03U)
#define MCP_GPINTENA        (0x04U)
#define MCP_GPINTENB        (0x05U)
#define MCP_DEFVALA         (0x06U)
#define MCP_DEFVALB         (0x07U)
#define MCP_INTCONA         (0x08U)
#define MCP_INTCONB         (0x09U)
#define MCP_IOCONA          (0x0AU)
#define MCP_IOCONB          (0x0BU)
#define MCP_GPPUA           (0x0CU)
#define MCP_GPPUB           (0x0DU)
#define MCP_INTFA           (0x0EU)
#define MCP_INTFB           (0x0FU)
#define MCP_INTCAPA         (0x10U)
#define MCP_INTCAPB         (0x11U)
#define MCP_GPIOA           (0x12U)
#define MCP_GPIOB           (0x13U)
#define MCP_OLATA           (0x14U)
#define MCP_OLATB           (0x15U)
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

extern void mcp23017_init(void);

extern bool mcp23017_set_gpio_state(uint8_t pin, bool state);
extern uint8_t mcp23017_get_gpio_state(uint8_t pin);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DRV_MCP23017_H__ */
