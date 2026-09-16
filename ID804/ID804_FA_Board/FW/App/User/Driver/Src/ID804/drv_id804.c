/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file           : drv_id804.c
    * @brief          : ID804 driver implementation
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
#include "drv_id804.h"
/* 2. C standard library headers (Alphabetical order) */
#include <stdint.h>
#include <stddef.h>
/* 3. Project internal / System-related headers */
#include "drv_id804_i2c.h"
#include "drv_id804_manchester.h"

#include "drv_gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct tag_ID804_COMM_OPS
{
    id804_comm_result_t (*read_func)(uint16_t dev_addr, uint8_t target, uint32_t* p_data);
    id804_comm_result_t (*write_func)(uint16_t dev_addr, uint8_t target, uint32_t data);
} id804_comm_ops_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static id804_comm_mode_t gt_id804_comm_mode;

static const id804_comm_ops_t gp_id804_comm_ops_i2c =
{
    .read_func = id804_read_via_i2c,
    .write_func = id804_write_via_i2c,
};

static const id804_comm_ops_t gp_id804_comm_ops_me =
{
    .read_func = id804_read_via_me,
    .write_func = id804_write_via_me,
};

static const id804_comm_ops_t* gp_id804_comm_ops = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Boot the ID804 device in I2C mode.
 * @return true if the boot process was successful, false otherwise.
 */
bool id804_boot_i2c(void)
{
    drv_gpio_id804_boot_i2c();
    id804_set_comm_mode(ID804_COMM_MODE_I2C);
    gp_id804_comm_ops = &gp_id804_comm_ops_i2c;
    return true;
}

/**
 * @brief Boot the ID804 device in Manchester encoding (ME) mode.
 * @return true if the boot process was successful, false otherwise.
 */
bool id804_boot_me(void)
{
    drv_gpio_id804_boot_mcu();
    id804_set_comm_mode(ID804_COMM_MODE_ME);
    gp_id804_comm_ops = &gp_id804_comm_ops_me;
    return true;
}
/**
 * @brief Boot the ID804 device in LVDS mode.
 * @return true if the boot process was successful, false otherwise.
 */

bool id804_boot_lvds(id804_comm_direction_t direction)
{
    drv_gpio_id804_boot_lvds(direction);
    id804_set_comm_mode(ID804_COMM_MODE_ME);
    gp_id804_comm_ops = &gp_id804_comm_ops_me;
    return true;
}
/**
 * @brief Boot the ID804 device in CAN mode.
 * @return true if the boot process was successful, false otherwise.
 */
bool id804_boot_can(id804_comm_direction_t direction)
{
    drv_gpio_id804_boot_can(direction);
    id804_set_comm_mode(ID804_COMM_MODE_ME);
    gp_id804_comm_ops = &gp_id804_comm_ops_me;
    return true;
}

/**
 * @brief Set the communication mode for the ID804 device.
 * @param mode The communication mode to set.
 */
void id804_set_comm_mode(id804_comm_mode_t mode)
{
    gt_id804_comm_mode = mode;
}

/**
 * @brief Get the current communication mode of the ID804 device.
 * @return The current communication mode.
 */
id804_comm_mode_t id804_get_comm_mode(void)
{
    return gt_id804_comm_mode;
}

/**
 * @brief Read data from the ID804 device using the current communication mode.
 * @param dev_addr The device address.
 * @param target The target register or command.
 * @param p_data Pointer to store the read data.
 * @return The result of the communication operation.
 */
id804_comm_result_t id804_read(uint16_t dev_addr, uint8_t target, uint32_t* p_data)
{
    return gp_id804_comm_ops->read_func(dev_addr, target, p_data);
}

/**
 * @brief Write data to the ID804 device using the current communication mode.
 * @param dev_addr The device address.
 * @param target The target register or command.
 * @param data The data to write.
 * @return The result of the communication operation.
 */
id804_comm_result_t id804_write(uint16_t dev_addr, uint8_t target, uint32_t data)
{
    return gp_id804_comm_ops->write_func(dev_addr, target, data);
}
/* USER CODE END 0 */
