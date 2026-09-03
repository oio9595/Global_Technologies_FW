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
/* 3. Project internal / System-related headers */
#include "id804_metadata.h"
#include "drv_id804_i2c.h"
#include "drv_id804_manchester.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool id804_RESET(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_RESET, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

bool id804_INITBIDIR(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_INITBIDIR, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

bool id804_CLRERROR(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_CLRERROR, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

bool id804_GOSLEEP(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_GOSLEEP, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

bool id804_GOACTIVE(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_GOACTIVE, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

bool id804_GODEEPSLEEP(void)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_GODEEPSLEEP, 0);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

uint16_t id804_get_r01_STATUS1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_STATUS1, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_STATUS1);
    }
    return regVal;
}

uint16_t id804_get_r02_STATUS2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_STATUS2, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_STATUS2);
    }
    return regVal;
}

uint16_t id804_get_r03_TEMP(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP);
    }
    return regVal;
}

uint32_t id804_get_r04_TEMPST(void)
{
    uint32_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMPST, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: TEMPST register cannot be read via I2C
    }
    return regVal;
}

uint16_t id804_get_r05_VEXT_TM(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_VEXT_TM, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_VEXT_TM);
    }
    return regVal;
}

uint16_t id804_get_r06_SETUP1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_SETUP1, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_SETUP1);
    }
    return regVal;
}

bool id804_set_r06_SETUP1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_SETUP1, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_SETUP1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r07_SETUP2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_SETUP2, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_SETUP2);
    }
    return regVal;
}

bool id804_set_r07_SETUP2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_SETUP2, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_SETUP2, value);
        return true;
    }
    return false;
}

uint32_t id804_get_MCAST(void)
{
    uint32_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_MCAST, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be read via I2C
    }
    return regVal;
}

bool id804_set_MCAST(uint32_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_MCAST, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: MCAST register cannot be written via I2C
        return false;
    }
    return false;
}

uint16_t id804_get_r08_MCAST1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: MCAST1 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_MCAST_1);
    }
    return regVal;
}

bool id804_set_r08_MCAST1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: MCAST1 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_MCAST_1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r09_MCAST2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: MCAST2 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_MCAST_2);
    }
    return regVal;
}

bool id804_set_r09_MCAST2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: MCAST2 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_MCAST_2, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r0A_TEMPTH(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMPTH, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMPTH);
    }
    return regVal;
}

bool id804_set_r0A_TEMPTH(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMPTH, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMPTH, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r0B_TEMPHYS(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMPHYS, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMPHYS);
    }
    return regVal;
}

bool id804_set_r0B_TEMPHYS(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMPHYS, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMPHYS, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r0C_CAL_PWM_RED(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_PWM_RED_VAL, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_CAL_PWM_RED);
    }
    return regVal;
}

uint16_t id804_get_r0D_CAL_PWM_GREEN(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_PWM_GREEN_VAL, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_CAL_PWM_GREEN);
    }
    return regVal;
}

uint16_t id804_get_r0E_CAL_PWM_BLUE(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_PWM_BLUE_VAL, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_CAL_PWM_BLUE);
    }
    return regVal;
}

uint16_t id804_get_r0F_CURR_MAX_LVL(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_CURR_MAX_LVL, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_CURR_MAX_LVL);
    }
    return regVal;
}

bool id804_set_r0F_CURR_MAX_LVL(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_CURR_MAX_LVL, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_CURR_MAX_LVL, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r10_TEMP_LUT_TC1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC1, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC1);
    }
    return regVal;
}

bool id804_set_r10_TEMP_LUT_TC1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC1, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r11_TEMP_LUT_TC2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC2, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC2);
    }
    return regVal;
}

bool id804_set_r11_TEMP_LUT_TC2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC2, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC2, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r12_TEMP_LUT_TC3(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC3, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC3);
    }
    return regVal;
}

bool id804_set_r12_TEMP_LUT_TC3(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC3, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC3, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r13_TEMP_LUT_TC4(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC4, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC4);
    }
    return regVal;
}

bool id804_set_r13_TEMP_LUT_TC4(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC4, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC4, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r14_TEMP_LUT_TC5(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC5, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC5);
    }
    return regVal;
}

bool id804_set_r14_TEMP_LUT_TC5(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC5, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC5, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r15_TEMP_LUT_TC6(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC6, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC6);
    }
    return regVal;
}

bool id804_set_r15_TEMP_LUT_TC6(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC6, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC6, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r16_TEMP_LUT_TC7(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC7, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC7);
    }
    return regVal;
}

bool id804_set_r16_TEMP_LUT_TC7(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC7, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC7, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r17_TEMP_LUT_TC8(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC8, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC8);
    }
    return regVal;
}

bool id804_set_r17_TEMP_LUT_TC8(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC8, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC8, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r18_TEMP_LUT_TC9(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC9, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC9);
    }
    return regVal;
}

bool id804_set_r18_TEMP_LUT_TC9(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC9, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC9, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r19_TEMP_LUT_TC10(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TEMP_LUT_TC10, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TEMP_LUT_TC10);
    }
    return regVal;
}

bool id804_set_r19_TEMP_LUT_TC10(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TEMP_LUT_TC10, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TEMP_LUT_TC10, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r1A_TIMEOUT(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_TIMEOUT, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TIMEOUT);
    }
    return regVal;
}

bool id804_set_r1A_TIMEOUT(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_TIMEOUT, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TIMEOUT, value);
        return true;
    }
    return false;
}

uint32_t id804_get_RGB(void)
{
    uint32_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        regVal = id804_receive_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_READ_RGB, 0U);
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: RGB register cannot be read via I2C
    }
    return regVal;
}

bool id804_set_RGB(uint32_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        id804_transmit_cmd_via_spi(0U /*ID804_DEV_ADDR*/, ID804_CMD_SET_RGB, value);
        return true;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        // Error: RGB register cannot be written via I2C
        return false;
    }
    return false;
}

uint16_t id804_get_r20_RGB1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: RGB1 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_RGB_1);
    }
    return regVal;
}

bool id804_set_r20_RGB1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: RGB1 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_RGB_1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_r21_RGB2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: RGB2 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_RGB_2);
    }
    return regVal;
}

bool id804_set_r21_RGB2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: RGB2 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_RGB_2, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE0_TRIM_CONTROL(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: TRIM_CONTROL register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_TRIM_CONTROL);
    }
    return regVal;
}

bool id804_set_rE0_TRIM_CONTROL(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: TRIM_CONTROL register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_TRIM_CONTROL, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE1_OTP_CONTROL1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL1 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP_CONTROL1);
    }
    return regVal;
}

bool id804_set_rE1_OTP_CONTROL1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL1 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP_CONTROL1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE2_OTP_CONTROL2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL2 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP_CONTROL2);
    }
    return regVal;
}

bool id804_set_rE2_OTP_CONTROL2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL2 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP_CONTROL2, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE3_OTP_CONTROL3(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL3 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP_CONTROL3);
    }
    return regVal;
}

bool id804_set_rE3_OTP_CONTROL3(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP_CONTROL3 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP_CONTROL3, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE4_OTP1_MIRROR1(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR1 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR1);
    }
    return regVal;
}

bool id804_set_rE4_OTP1_MIRROR1(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR1 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR1, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE5_OTP1_MIRROR2(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR2 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR2);
    }
    return regVal;
}

bool id804_set_rE5_OTP1_MIRROR2(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR2 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR2, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE6_OTP1_MIRROR3(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR3 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR3);
    }
    return regVal;
}

bool id804_set_rE6_OTP1_MIRROR3(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR3 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR3, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE7_OTP1_MIRROR4(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR4 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR4);
    }
    return regVal;
}

bool id804_set_rE7_OTP1_MIRROR4(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR4 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR4, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE8_OTP1_MIRROR5(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR5 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR5);
    }
    return regVal;
}

bool id804_set_rE8_OTP1_MIRROR5(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR5 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR5, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rE9_OTP1_MIRROR6(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR6 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR6);
    }
    return regVal;
}

bool id804_set_rE9_OTP1_MIRROR6(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR6 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR6, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rEA_OTP1_MIRROR7(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR7 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR7);
    }
    return regVal;
}

bool id804_set_rEA_OTP1_MIRROR7(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR7 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR7, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rEB_OTP1_MIRROR8(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR8 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR8);
    }
    return regVal;
}

bool id804_set_rEB_OTP1_MIRROR8(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR8 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR8, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rEC_OTP1_MIRROR9(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR9 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR9);
    }
    return regVal;
}

bool id804_set_rEC_OTP1_MIRROR9(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR9 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR9, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rED_OTP1_MIRROR10(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR10 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR10);
    }
    return regVal;
}

bool id804_set_rED_OTP1_MIRROR10(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR10 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR10, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rEE_OTP1_MIRROR11(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR11 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR11);
    }
    return regVal;
}

bool id804_set_rEE_OTP1_MIRROR11(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR11 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR11, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rEF_OTP1_MIRROR12(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR12 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR12);
    }
    return regVal;
}

bool id804_set_rEF_OTP1_MIRROR12(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR12 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR12, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF0_OTP1_MIRROR13(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR13 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP1_MIRROR13);
    }
    return regVal;
}

bool id804_set_rF0_OTP1_MIRROR13(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP1_MIRROR13 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP1_MIRROR13, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF1_OTP2_MIRROR14(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR14 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR14);
    }
    return regVal;
}

bool id804_set_rF1_OTP2_MIRROR14(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR14 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR14, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF2_OTP2_MIRROR15(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR15 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR15);
    }
    return regVal;
}

bool id804_set_rF2_OTP2_MIRROR15(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR15 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR15, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF3_OTP2_MIRROR16(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR16 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR16);
    }
    return regVal;
}

bool id804_set_rF3_OTP2_MIRROR16(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR16 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR16, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF4_OTP2_MIRROR17(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR17 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR17);
    }
    return regVal;
}

bool id804_set_rF4_OTP2_MIRROR17(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR17 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR17, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF5_OTP2_MIRROR18(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR18 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR18);
    }
    return regVal;
}

bool id804_set_rF5_OTP2_MIRROR18(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR18 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR18, value);
        return true;
    }
    return false;
}

uint16_t id804_get_rF6_OTP2_MIRROR19(void)
{
    uint16_t regVal = 0U;
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR19 register cannot be read via ME communication mode
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        regVal = id804_read_data_via_i2c(ID804_ADDR_OTP2_MIRROR19);
    }
    return regVal;
}

bool id804_set_rF6_OTP2_MIRROR19(uint16_t value)
{
    if (ID804_COMM_MODE_ME == gt_id804_comm_mode)
    {
        // Error: OTP2_MIRROR19 register cannot be written via ME communication mode
        return false;
    }
    else if (ID804_COMM_MODE_I2C == gt_id804_comm_mode)
    {
        id804_write_data_via_i2c(ID804_ADDR_OTP2_MIRROR19, value);
        return true;
    }
    return false;
}

uint16_t id804_get_ic_state(void)
{
    uint16_t regVal = id804_get_r01_STATUS1();
    return (uint16_t)((regVal & ID804_STATUS1_IC_STATE_MSK) >> ID804_STATUS1_IC_STATE_POS);
}

uint16_t id804_get_sio2(void)
{
    uint16_t regVal = id804_get_r01_STATUS1();
    return (uint16_t)((regVal & ID804_STATUS1_SIO2_MSK) >> ID804_STATUS1_SIO2_POS);
}

uint16_t id804_get_sio1(void)
{
    uint16_t regVal = id804_get_r01_STATUS1();
    return (uint16_t)((regVal & ID804_STATUS1_SIO1_MSK) >> ID804_STATUS1_SIO1_POS);
}

uint16_t id804_get_otpcrc(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_OTPCRC_MSK) >> ID804_STATUS2_OTPCRC_POS);
}

uint16_t id804_get_t_out(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_T_OUT_MSK) >> ID804_STATUS2_T_OUT_POS);
}

uint16_t id804_get_open_r(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_OPEN_R_MSK) >> ID804_STATUS2_OPEN_R_POS);
}

uint16_t id804_get_open_g(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_OPEN_G_MSK) >> ID804_STATUS2_OPEN_G_POS);
}

uint16_t id804_get_open_b(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_OPEN_B_MSK) >> ID804_STATUS2_OPEN_B_POS);
}

uint16_t id804_get_short_r(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_SHORT_R_MSK) >> ID804_STATUS2_SHORT_R_POS);
}

uint16_t id804_get_short_g(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_SHORT_G_MSK) >> ID804_STATUS2_SHORT_G_POS);
}

uint16_t id804_get_short_b(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_SHORT_B_MSK) >> ID804_STATUS2_SHORT_B_POS);
}

uint16_t id804_get_ot_flt(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_OT_FLT_MSK) >> ID804_STATUS2_OT_FLT_POS);
}

uint16_t id804_get_uv_flt(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_UV_FLT_MSK) >> ID804_STATUS2_UV_FLT_POS);
}

uint16_t id804_get_crc_flt(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_CRC_FLT_MSK) >> ID804_STATUS2_CRC_FLT_POS);
}

uint16_t id804_get_com_flt(void)
{
    uint16_t regVal = id804_get_r02_STATUS2();
    return (uint16_t)((regVal & ID804_STATUS2_COM_FLT_MSK) >> ID804_STATUS2_COM_FLT_POS);
}

uint16_t id804_get_temperature(void)
{
    uint16_t regVal = id804_get_r03_TEMP();
    return (regVal & ID804_TEMP_TEMPERATURE_MSK) >> ID804_TEMP_TEMPERATURE_POS;
}

uint16_t id804_get_vext_tm(void)
{
    uint16_t regVal = id804_get_r05_VEXT_TM();
    return (regVal & ID804_VEXT_TM_VEXT_TM_MSK) >> ID804_VEXT_TM_VEXT_TM_POS;
}

uint16_t id804_get_tc_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_TC_E_MSK) >> ID804_SETUP1_TC_E_POS);
}

bool id804_set_tc_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_TC_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_TC_E_POS) & ID804_SETUP1_TC_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_os_flt_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_OS_FLT_E_MSK) >> ID804_SETUP1_OS_FLT_E_POS);
}

bool id804_set_os_flt_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_OS_FLT_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_OS_FLT_E_POS) & ID804_SETUP1_OS_FLT_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_ot_flt_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_OT_FLT_E_MSK) >> ID804_SETUP1_OT_FLT_E_POS);
}

bool id804_set_ot_flt_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_OT_FLT_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_OT_FLT_E_POS) & ID804_SETUP1_OT_FLT_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_uv_flt_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_UV_FLT_E_MSK) >> ID804_SETUP1_UV_FLT_E_POS);
}

bool id804_set_uv_flt_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_UV_FLT_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_UV_FLT_E_POS) & ID804_SETUP1_UV_FLT_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_com_flt_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_COM_FLT_E_MSK) >> ID804_SETUP1_COM_FLT_E_POS);
}

bool id804_set_com_flt_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_COM_FLT_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_COM_FLT_E_POS) & ID804_SETUP1_COM_FLT_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_crc_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_CRC_E_MSK) >> ID804_SETUP1_CRC_E_POS);
}

bool id804_set_crc_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_CRC_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_CRC_E_POS) & ID804_SETUP1_CRC_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_ph_shift_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_PH_SHIFT_E_MSK) >> ID804_SETUP1_PH_SHIFT_E_POS);
}

bool id804_set_ph_shift_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_PH_SHIFT_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_PH_SHIFT_E_POS) & ID804_SETUP1_PH_SHIFT_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_lg_e(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_LG_E_MSK) >> ID804_SETUP1_LG_E_POS);
}

bool id804_set_lg_e(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_LG_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_LG_E_POS) & ID804_SETUP1_LG_E_MSK;

    return id804_set_r06_SETUP1(regVal);
}


uint16_t id804_get_f_fpwm_div(void)
{
    uint16_t regVal = id804_get_r06_SETUP1();
    return (uint16_t)((regVal & ID804_SETUP1_F_PWM_DIV_MSK) >> ID804_SETUP1_F_PWM_DIV_POS);
}

bool id804_set_f_fpwm_div(uint16_t value)
{
    uint16_t regVal = id804_get_r06_SETUP1();

    regVal &= ~ID804_SETUP1_F_PWM_DIV_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP1_F_PWM_DIV_POS) & ID804_SETUP1_F_PWM_DIV_MSK;

    return id804_set_r06_SETUP1(regVal);
}

uint16_t id804_get_clk_inv_e(void)
{
    uint16_t regVal = id804_get_r07_SETUP2();
    return (uint16_t)((regVal & ID804_SETUP2_CLK_INV_E_MSK) >> ID804_SETUP2_CLK_INV_E_POS);
}

bool id804_set_clk_inv_e(uint16_t value)
{
    uint16_t regVal = id804_get_r07_SETUP2();

    regVal &= ~ID804_SETUP2_CLK_INV_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP2_CLK_INV_E_POS) & ID804_SETUP2_CLK_INV_E_MSK;

    return id804_set_r07_SETUP2(regVal);
}

uint16_t id804_get_vext_mon_e(void)
{
    uint16_t regVal = id804_get_r07_SETUP2();
    return (uint16_t)((regVal & ID804_SETUP2_VEXT_MON_E_MSK) >> ID804_SETUP2_VEXT_MON_E_POS);
}

bool id804_set_vext_mon_e(uint16_t value)
{
    uint16_t regVal = id804_get_r07_SETUP2();

    regVal &= ~ID804_SETUP2_VEXT_MON_E_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP2_VEXT_MON_E_POS) & ID804_SETUP2_VEXT_MON_E_MSK;

    return id804_set_r07_SETUP2(regVal);
}

uint16_t id804_get_event_cyc(void)
{
    uint16_t regVal = id804_get_r07_SETUP2();
    return (uint16_t)((regVal & ID804_SETUP2_EVENT_CYC_MSK) >> ID804_SETUP2_EVENT_CYC_POS);
}

bool id804_set_event_cyc(uint16_t value)
{
    uint16_t regVal = id804_get_r07_SETUP2();

    regVal &= ~ID804_SETUP2_EVENT_CYC_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP2_EVENT_CYC_POS) & ID804_SETUP2_EVENT_CYC_MSK;

    return id804_set_r07_SETUP2(regVal);
}

uint16_t id804_get_uv_lvl(void)
{
    uint16_t regVal = id804_get_r07_SETUP2();
    return (uint16_t)((regVal & ID804_SETUP2_UV_LVL_MSK) >> ID804_SETUP2_UV_LVL_POS);
}

bool id804_set_uv_lvl(uint16_t value)
{
    uint16_t regVal = id804_get_r07_SETUP2();

    regVal &= ~ID804_SETUP2_UV_LVL_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP2_UV_LVL_POS) & ID804_SETUP2_UV_LVL_MSK;

    return id804_set_r07_SETUP2(regVal);
}

uint16_t id804_get_sh_lvl(void)
{
    uint16_t regVal = id804_get_r07_SETUP2();
    return (uint16_t)((regVal & ID804_SETUP2_SH_LVL_MSK) >> ID804_SETUP2_SH_LVL_POS);
}

bool id804_set_sh_lvl(uint16_t value)
{
    uint16_t regVal = id804_get_r07_SETUP2();

    regVal &= ~ID804_SETUP2_SH_LVL_MSK;
    regVal |= ((uint16_t)value << ID804_SETUP2_SH_LVL_POS) & ID804_SETUP2_SH_LVL_MSK;

    return id804_set_r07_SETUP2(regVal);
}

uint16_t id804_get_tempth(void)
{
    uint16_t regVal = id804_get_r0A_TEMPTH();
    return (uint16_t)((regVal & ID804_TEMPTH_THRESHOLD_MSK) >> ID804_TEMPTH_THRESHOLD_POS);
}

bool id804_set_tempth(uint16_t value)
{
    uint16_t regVal = id804_get_r0A_TEMPTH();

    regVal &= ~ID804_TEMPTH_THRESHOLD_MSK;
    regVal |= ((uint16_t)value << ID804_TEMPTH_THRESHOLD_POS) & ID804_TEMPTH_THRESHOLD_MSK;

    return id804_set_r0A_TEMPTH(regVal);
}

uint16_t id804_get_temphys(void)
{
    uint16_t regVal = id804_get_r0B_TEMPHYS();
    return (uint16_t)((regVal & ID804_TEMPHYS_HYSTERESIS_MSK) >> ID804_TEMPHYS_HYSTERESIS_POS);
}

bool id804_set_temphys(uint16_t value)
{
    uint16_t regVal = id804_get_r0B_TEMPHYS();

    regVal &= ~ID804_TEMPHYS_HYSTERESIS_MSK;
    regVal |= ((uint16_t)value << ID804_TEMPHYS_HYSTERESIS_POS) & ID804_TEMPHYS_HYSTERESIS_MSK;

    return id804_set_r0B_TEMPHYS(regVal);
}

uint16_t id804_get_pwm_red_val(void)
{
    uint16_t regVal = id804_get_r0C_CAL_PWM_RED();
    return (uint16_t)((regVal & ID804_CAL_PWM_VAL_MSK) >> ID804_CAL_PWM_VAL_POS);
}

uint16_t id804_get_pwm_green_val(void)
{
    uint16_t regVal = id804_get_r0D_CAL_PWM_GREEN();
    return (uint16_t)((regVal & ID804_CAL_PWM_VAL_MSK) >> ID804_CAL_PWM_VAL_POS);
}

uint16_t id804_get_pwm_blue_val(void)
{
    uint16_t regVal = id804_get_r0E_CAL_PWM_BLUE();
    return (uint16_t)((regVal & ID804_CAL_PWM_VAL_MSK) >> ID804_CAL_PWM_VAL_POS);
}

uint16_t id804_get_r_curr_max_lvl(void)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();
    return (uint16_t)((regVal & ID804_CURR_MAX_R_MSK) >> ID804_CURR_MAX_R_POS);
}

bool id804_set_r_curr_max_lvl(uint16_t value)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();

    regVal &= ~ID804_CURR_MAX_R_MSK;
    regVal |= ((uint16_t)value << ID804_CURR_MAX_R_POS) & ID804_CURR_MAX_R_MSK;

    return id804_set_r0F_CURR_MAX_LVL(regVal);
}

uint16_t id804_get_g_curr_max_lvl(void)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();
    return (uint16_t)((regVal & ID804_CURR_MAX_G_MSK) >> ID804_CURR_MAX_G_POS);
}

bool id804_set_g_curr_max_lvl(uint16_t value)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();

    regVal &= ~ID804_CURR_MAX_G_MSK;
    regVal |= ((uint16_t)value << ID804_CURR_MAX_G_POS) & ID804_CURR_MAX_G_MSK;

    return id804_set_r0F_CURR_MAX_LVL(regVal);
}

uint16_t id804_get_b_curr_max_lvl(void)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();
    return (uint16_t)((regVal & ID804_CURR_MAX_B_MSK) >> ID804_CURR_MAX_B_POS);
}

bool id804_set_b_curr_max_lvl(uint16_t value)
{
    uint16_t regVal = id804_get_r0F_CURR_MAX_LVL();

    regVal &= ~ID804_CURR_MAX_B_MSK;
    regVal |= ((uint16_t)value << ID804_CURR_MAX_B_POS) & ID804_CURR_MAX_B_MSK;

    return id804_set_r0F_CURR_MAX_LVL(regVal);
}

uint16_t id804_get_temp_lut_tc1(void)
{
    uint16_t regVal = id804_get_r10_TEMP_LUT_TC1();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc1(uint16_t value)
{
    uint16_t regVal = id804_get_r10_TEMP_LUT_TC1();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r10_TEMP_LUT_TC1(regVal);
}

uint16_t id804_get_temp_lut_tc2(void)
{
    uint16_t regVal = id804_get_r11_TEMP_LUT_TC2();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc2(uint16_t value)
{
    uint16_t regVal = id804_get_r11_TEMP_LUT_TC2();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r11_TEMP_LUT_TC2(regVal);
}

uint16_t id804_get_temp_lut_tc3(void)
{
    uint16_t regVal = id804_get_r12_TEMP_LUT_TC3();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc3(uint16_t value)
{
    uint16_t regVal = id804_get_r12_TEMP_LUT_TC3();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r12_TEMP_LUT_TC3(regVal);
}

uint16_t id804_get_temp_lut_tc4(void)
{
    uint16_t regVal = id804_get_r13_TEMP_LUT_TC4();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc4(uint16_t value)
{
    uint16_t regVal = id804_get_r13_TEMP_LUT_TC4();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r13_TEMP_LUT_TC4(regVal);
}

uint16_t id804_get_temp_lut_tc5(void)
{
    uint16_t regVal = id804_get_r14_TEMP_LUT_TC5();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc5(uint16_t value)
{
    uint16_t regVal = id804_get_r14_TEMP_LUT_TC5();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r14_TEMP_LUT_TC5(regVal);
}

uint16_t id804_get_temp_lut_tc6(void)
{
    uint16_t regVal = id804_get_r15_TEMP_LUT_TC6();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc6(uint16_t value)
{
    uint16_t regVal = id804_get_r15_TEMP_LUT_TC6();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r15_TEMP_LUT_TC6(regVal);
}

uint16_t id804_get_temp_lut_tc7(void)
{
    uint16_t regVal = id804_get_r16_TEMP_LUT_TC7();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc7(uint16_t value)
{
    uint16_t regVal = id804_get_r16_TEMP_LUT_TC7();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r16_TEMP_LUT_TC7(regVal);
}

uint16_t id804_get_temp_lut_tc8(void)
{
    uint16_t regVal = id804_get_r17_TEMP_LUT_TC8();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc8(uint16_t value)
{
    uint16_t regVal = id804_get_r17_TEMP_LUT_TC8();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r17_TEMP_LUT_TC8(regVal);
}

uint16_t id804_get_temp_lut_tc9(void)
{
    uint16_t regVal = id804_get_r18_TEMP_LUT_TC9();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc9(uint16_t value)
{
    uint16_t regVal = id804_get_r18_TEMP_LUT_TC9();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r18_TEMP_LUT_TC9(regVal);
}

uint16_t id804_get_temp_lut_tc10(void)
{
    uint16_t regVal = id804_get_r19_TEMP_LUT_TC10();
    return (uint16_t)((regVal & ID804_TEMP_LUT_TC_MSK) >> ID804_TEMP_LUT_TC_POS);
}

bool id804_set_temp_lut_tc10(uint16_t value)
{
    uint16_t regVal = id804_get_r19_TEMP_LUT_TC10();

    regVal &= ~ID804_TEMP_LUT_TC_MSK;
    regVal |= ((uint16_t)value << ID804_TEMP_LUT_TC_POS) & ID804_TEMP_LUT_TC_MSK;

    return id804_set_r19_TEMP_LUT_TC10(regVal);
}

uint16_t id804_get_timeout(void)
{
    uint16_t regVal = id804_get_r1A_TIMEOUT();
    return (regVal & ID804_TIMEOUT_VAL_MSK) >> ID804_TIMEOUT_VAL_POS;
}

bool id804_set_timeout(uint16_t value)
{
    uint16_t regVal = id804_get_r1A_TIMEOUT();

    regVal &= ~ID804_TIMEOUT_VAL_MSK;
    regVal |= ((uint16_t)value << ID804_TIMEOUT_VAL_POS) & ID804_TIMEOUT_VAL_MSK;

    return id804_set_r1A_TIMEOUT(regVal);
}

uint16_t id804_get_test_e(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_TEST_E_MSK) >> ID804_TRIM_TEST_E_POS);
}

bool id804_set_test_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_TEST_E_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_TEST_E_POS) & ID804_TRIM_TEST_E_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

bool id804_get_cko_e(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_CKO_E_MSK) >> ID804_TRIM_CKO_E_POS);
}

bool id804_set_cko_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_CKO_E_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_CKO_E_POS) & ID804_TRIM_CKO_E_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_adc_clk_pol(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_ADC_CLK_POL_MSK) >> ID804_TRIM_ADC_CLK_POL_POS);
}

bool id804_set_adc_clk_pol(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_ADC_CLK_POL_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_ADC_CLK_POL_POS) & ID804_TRIM_ADC_CLK_POL_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_ana_mod_sel(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_ANA_MOD_SEL_MSK) >> ID804_TRIM_ANA_MOD_SEL_POS);
}

bool id804_set_ana_mod_sel(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_ANA_MOD_SEL_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_ANA_MOD_SEL_POS) & ID804_TRIM_ANA_MOD_SEL_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_t_ana_sel(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_T_ANA_SEL_MSK) >> ID804_TRIM_T_ANA_SEL_POS);
}

bool id804_set_t_ana_sel(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_T_ANA_SEL_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_T_ANA_SEL_POS) & ID804_TRIM_T_ANA_SEL_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_pwm_max_r_e(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_PWM_MAX_R_E_MSK) >> ID804_TRIM_PWM_MAX_R_E_POS);
}

bool id804_set_pwm_max_r_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_PWM_MAX_R_E_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_PWM_MAX_R_E_POS) & ID804_TRIM_PWM_MAX_R_E_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_pwm_max_g_e(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_PWM_MAX_G_E_MSK) >> ID804_TRIM_PWM_MAX_G_E_POS);
}

bool id804_set_pwm_max_g_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_PWM_MAX_G_E_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_PWM_MAX_G_E_POS) & ID804_TRIM_PWM_MAX_G_E_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_pwm_max_b_e(void)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();
    return (uint16_t)((regVal & ID804_TRIM_PWM_MAX_B_E_MSK) >> ID804_TRIM_PWM_MAX_B_E_POS);
}

bool id804_set_pwm_max_b_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE0_TRIM_CONTROL();

    regVal &= ~ID804_TRIM_PWM_MAX_B_E_MSK;
    regVal |= ((uint16_t)value << ID804_TRIM_PWM_MAX_B_E_POS) & ID804_TRIM_PWM_MAX_B_E_MSK;

    return id804_set_rE0_TRIM_CONTROL(regVal);
}

uint16_t id804_get_otp_pg2(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_OTP_PG2_MSK) >> ID804_OTP_CTRL1_OTP_PG2_POS);
}

bool id804_set_otp_pg2(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_OTP_PG2_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_OTP_PG2_POS) & ID804_OTP_CTRL1_OTP_PG2_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_otp_pg1(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_OTP_PG1_MSK) >> ID804_OTP_CTRL1_OTP_PG1_POS);
}

bool id804_set_otp_pg1(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_OTP_PG1_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_OTP_PG1_POS) & ID804_OTP_CTRL1_OTP_PG1_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_t_ana_e(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_T_ANA_E_MSK) >> ID804_OTP_CTRL1_T_ANA_E_POS);
}

bool id804_set_t_ana_e(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_T_ANA_E_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_T_ANA_E_POS) & ID804_OTP_CTRL1_T_ANA_E_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_otp_rd(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_OTP_RD_MSK) >> ID804_OTP_CTRL1_OTP_RD_POS);
}

bool id804_set_otp_rd(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_OTP_RD_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_OTP_RD_POS) & ID804_OTP_CTRL1_OTP_RD_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_otp_wsel(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_OTP_WSEL_MSK) >> ID804_OTP_CTRL1_OTP_WSEL_POS);
}

bool id804_set_otp_wsel(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_OTP_WSEL_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_OTP_WSEL_POS) & ID804_OTP_CTRL1_OTP_WSEL_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_tmux_sel(void)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();
    return (uint16_t)((regVal & ID804_OTP_CTRL1_TMUX_SEL_MSK) >> ID804_OTP_CTRL1_TMUX_SEL_POS);
}

bool id804_set_tmux_sel(uint16_t value)
{
    uint16_t regVal = id804_get_rE1_OTP_CONTROL1();

    regVal &= ~ID804_OTP_CTRL1_TMUX_SEL_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL1_TMUX_SEL_POS) & ID804_OTP_CTRL1_TMUX_SEL_MSK;

    return id804_set_rE1_OTP_CONTROL1(regVal);
}

uint16_t id804_get_otp_pg_acc_cycle(void)
{
    uint16_t regVal = id804_get_rE2_OTP_CONTROL2();
    return (uint16_t)((regVal & ID804_OTP_CTRL2_ACC_CYCLE_MSK) >> ID804_OTP_CTRL2_ACC_CYCLE_POS);
}

bool id804_set_otp_pg_acc_cycle(uint16_t value)
{
    uint16_t regVal = id804_get_rE2_OTP_CONTROL2();

    regVal &= ~ID804_OTP_CTRL2_ACC_CYCLE_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL2_ACC_CYCLE_POS) & ID804_OTP_CTRL2_ACC_CYCLE_MSK;

    return id804_set_rE2_OTP_CONTROL2(regVal);
}

uint16_t id804_get_protect(void)
{
    uint16_t regVal = id804_get_rE3_OTP_CONTROL3();
    return (uint16_t)((regVal & ID804_OTP_CTRL3_PROTECT_MSK) >> ID804_OTP_CTRL3_PROTECT_POS);
}

bool id804_set_protect(uint16_t value)
{
    uint16_t regVal = id804_get_rE3_OTP_CONTROL3();

    regVal &= ~ID804_OTP_CTRL3_PROTECT_MSK;
    regVal |= ((uint16_t)value << ID804_OTP_CTRL3_PROTECT_POS) & ID804_OTP_CTRL3_PROTECT_MSK;

    return id804_set_rE3_OTP_CONTROL3(regVal);
}

uint16_t id804_get_sio1_tr_tx_bias(void)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();
    return (uint16_t)((regVal & ID804_MIRROR1_SIO1_TR_TX_BIAS_MSK) >> ID804_MIRROR1_SIO1_TR_TX_BIAS_POS);
}

bool id804_set_sio1_tr_tx_bias(uint16_t value)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();

    regVal &= ~ID804_MIRROR1_SIO1_TR_TX_BIAS_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR1_SIO1_TR_TX_BIAS_POS) & ID804_MIRROR1_SIO1_TR_TX_BIAS_MSK;

    return id804_set_rE4_OTP1_MIRROR1(regVal);
}

uint16_t id804_get_sio2_tr_tx_bias(void)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();
    return (uint16_t)((regVal & ID804_MIRROR1_SIO2_TR_TX_BIAS_MSK) >> ID804_MIRROR1_SIO2_TR_TX_BIAS_POS);
}

bool id804_set_sio2_tr_tx_bias(uint16_t value)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();

    regVal &= ~ID804_MIRROR1_SIO2_TR_TX_BIAS_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR1_SIO2_TR_TX_BIAS_POS) & ID804_MIRROR1_SIO2_TR_TX_BIAS_MSK;

    return id804_set_rE4_OTP1_MIRROR1(regVal);
}

uint16_t id804_get_v_trim_bgr(void)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();
    return (uint16_t)((regVal & ID804_MIRROR1_V_TRIM_BGR_MSK) >> ID804_MIRROR1_V_TRIM_BGR_POS);
}

bool id804_set_v_trim_bgr(uint16_t value)
{
    uint16_t regVal = id804_get_rE4_OTP1_MIRROR1();

    regVal &= ~ID804_MIRROR1_V_TRIM_BGR_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR1_V_TRIM_BGR_POS) & ID804_MIRROR1_V_TRIM_BGR_MSK;

    return id804_set_rE4_OTP1_MIRROR1(regVal);
}

uint16_t id804_get_sio1_tr_res(void)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();
    return (uint16_t)((regVal & ID804_MIRROR2_SIO1_TR_RES_MSK) >> ID804_MIRROR2_SIO1_TR_RES_POS);
}

bool id804_set_sio1_tr_res(uint16_t value)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();

    regVal &= ~ID804_MIRROR2_SIO1_TR_RES_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR2_SIO1_TR_RES_POS) & ID804_MIRROR2_SIO1_TR_RES_MSK;

    return id804_set_rE5_OTP1_MIRROR2(regVal);
}

uint16_t id804_get_sio2_tr_res(void)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();
    return (uint16_t)((regVal & ID804_MIRROR2_SIO2_TR_RES_MSK) >> ID804_MIRROR2_SIO2_TR_RES_POS);
}

bool id804_set_sio2_tr_res(uint16_t value)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();

    regVal &= ~ID804_MIRROR2_SIO2_TR_RES_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR2_SIO2_TR_RES_POS) & ID804_MIRROR2_SIO2_TR_RES_MSK;

    return id804_set_rE5_OTP1_MIRROR2(regVal);
}

uint16_t id804_get_vldo_ctrl(void)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();
    return (uint16_t)((regVal & ID804_MIRROR2_VLDO_CTRL_MSK) >> ID804_MIRROR2_VLDO_CTRL_POS);
}

bool id804_set_vldo_ctrl(uint16_t value)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();

    regVal &= ~ID804_MIRROR2_VLDO_CTRL_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR2_VLDO_CTRL_POS) & ID804_MIRROR2_VLDO_CTRL_MSK;

    return id804_set_rE5_OTP1_MIRROR2(regVal);
}

uint16_t id804_get_i_trim_bgr(void)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();
    return (uint16_t)((regVal & ID804_MIRROR2_I_TRIM_BGR_MSK) >> ID804_MIRROR2_I_TRIM_BGR_POS);
}

bool id804_set_i_trim_bgr(uint16_t value)
{
    uint16_t regVal = id804_get_rE5_OTP1_MIRROR2();

    regVal &= ~ID804_MIRROR2_I_TRIM_BGR_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR2_I_TRIM_BGR_POS) & ID804_MIRROR2_I_TRIM_BGR_MSK;

    return id804_set_rE5_OTP1_MIRROR2(regVal);
}

uint16_t id804_get_temp_trim_bgr(void)
{
    uint16_t regVal = id804_get_rE6_OTP1_MIRROR3();
    return (uint16_t)((regVal & ID804_MIRROR3_TEMP_TRIM_BGR_MSK) >> ID804_MIRROR3_TEMP_TRIM_BGR_POS);
}

bool id804_set_temp_trim_bgr(uint16_t value)
{
    uint16_t regVal = id804_get_rE6_OTP1_MIRROR3();

    regVal &= ~ID804_MIRROR3_TEMP_TRIM_BGR_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR3_TEMP_TRIM_BGR_POS) & ID804_MIRROR3_TEMP_TRIM_BGR_MSK;

    return id804_set_rE6_OTP1_MIRROR3(regVal);
}

uint16_t id804_get_bgr_r1_trim(void)
{
    uint16_t regVal = id804_get_rE6_OTP1_MIRROR3();
    return (uint16_t)((regVal & ID804_MIRROR3_BGR_R1_TRIM_MSK) >> ID804_MIRROR3_BGR_R1_TRIM_POS);
}

bool id804_set_bgr_r1_trim(uint16_t value)
{
    uint16_t regVal = id804_get_rE6_OTP1_MIRROR3();

    regVal &= ~ID804_MIRROR3_BGR_R1_TRIM_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR3_BGR_R1_TRIM_POS) & ID804_MIRROR3_BGR_R1_TRIM_MSK;

    return id804_set_rE6_OTP1_MIRROR3(regVal);
}

uint16_t id804_get_osc_ctl(void)
{
    uint16_t regVal = id804_get_rE7_OTP1_MIRROR4();
    return (uint16_t)((regVal & ID804_MIRROR4_OSC_CTL_MSK) >> ID804_MIRROR4_OSC_CTL_POS);
}

bool id804_set_osc_ctl(uint16_t value)
{
    uint16_t regVal = id804_get_rE7_OTP1_MIRROR4();

    regVal &= ~ID804_MIRROR4_OSC_CTL_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR4_OSC_CTL_POS) & ID804_MIRROR4_OSC_CTL_MSK;

    return id804_set_rE7_OTP1_MIRROR4(regVal);
}

uint16_t id804_get_bgr_r2_trim(void)
{
    uint16_t regVal = id804_get_rE7_OTP1_MIRROR4();
    return (uint16_t)((regVal & ID804_MIRROR4_BGR_R2_TRIM_MSK) >> ID804_MIRROR4_BGR_R2_TRIM_POS);
}

bool id804_set_bgr_r2_trim(uint16_t value)
{
    uint16_t regVal = id804_get_rE7_OTP1_MIRROR4();

    regVal &= ~ID804_MIRROR4_BGR_R2_TRIM_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR4_BGR_R2_TRIM_POS) & ID804_MIRROR4_BGR_R2_TRIM_MSK;

    return id804_set_rE7_OTP1_MIRROR4(regVal);
}

uint16_t id804_get_adc_offset(void)
{
    uint16_t regVal = id804_get_rEA_OTP1_MIRROR7();
    return (uint16_t)((regVal & ID804_MIRROR7_ADC_OFFSET_MSK) >> ID804_MIRROR7_ADC_OFFSET_POS);
}

bool id804_set_adc_offset(uint16_t value)
{
    uint16_t regVal = id804_get_rEA_OTP1_MIRROR7();

    regVal &= ~ID804_MIRROR7_ADC_OFFSET_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR7_ADC_OFFSET_POS) & ID804_MIRROR7_ADC_OFFSET_MSK;

    return id804_set_rEA_OTP1_MIRROR7(regVal);
}

uint16_t id804_get_adc_gain(void)
{
    uint16_t regVal = id804_get_rEB_OTP1_MIRROR8();
    return (uint16_t)((regVal & ID804_MIRROR8_ADC_GAIN_MSK) >> ID804_MIRROR8_ADC_GAIN_POS);
}

bool id804_set_adc_gain(uint16_t value)
{
    uint16_t regVal = id804_get_rEB_OTP1_MIRROR8();

    regVal &= ~ID804_MIRROR8_ADC_GAIN_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR8_ADC_GAIN_POS) & ID804_MIRROR8_ADC_GAIN_MSK;

    return id804_set_rEB_OTP1_MIRROR8(regVal);
}

uint16_t id804_get_itrim_r(void)
{
    uint16_t regVal = id804_get_rEC_OTP1_MIRROR9();
    return (uint16_t)((regVal & ID804_MIRROR_ITRIM_MSK) >> ID804_MIRROR_ITRIM_POS);
}

bool id804_set_itrim_r(uint16_t value)
{
    uint16_t regVal = id804_get_rEC_OTP1_MIRROR9();

    regVal &= ~ID804_MIRROR_ITRIM_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_ITRIM_POS) & ID804_MIRROR_ITRIM_MSK;

    return id804_set_rEC_OTP1_MIRROR9(regVal);
}

uint16_t id804_get_itrim_g(void)
{
    uint16_t regVal = id804_get_rED_OTP1_MIRROR10();
    return (uint16_t)((regVal & ID804_MIRROR_ITRIM_MSK) >> ID804_MIRROR_ITRIM_POS);
}

bool id804_set_itrim_g(uint16_t value)
{
    uint16_t regVal = id804_get_rED_OTP1_MIRROR10();

    regVal &= ~ID804_MIRROR_ITRIM_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_ITRIM_POS) & ID804_MIRROR_ITRIM_MSK;

    return id804_set_rED_OTP1_MIRROR10(regVal);
}

uint16_t id804_get_itrim_b(void)
{
    uint16_t regVal = id804_get_rEE_OTP1_MIRROR11();
    return (uint16_t)((regVal & ID804_MIRROR_ITRIM_MSK) >> ID804_MIRROR_ITRIM_POS);
}

bool id804_set_itrim_b(uint16_t value)
{
    uint16_t regVal = id804_get_rEE_OTP1_MIRROR11();

    regVal &= ~ID804_MIRROR_ITRIM_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_ITRIM_POS) & ID804_MIRROR_ITRIM_MSK;

    return id804_set_rEE_OTP1_MIRROR11(regVal);
}

uint16_t id804_get_tc_base(void)
{
    uint16_t regVal = id804_get_rEF_OTP1_MIRROR12();
    return (uint16_t)((regVal & ID804_MIRROR12_TC_BASE_MSK) >> ID804_MIRROR12_TC_BASE_POS);
}

bool id804_set_tc_base(uint16_t value)
{
    uint16_t regVal = id804_get_rEF_OTP1_MIRROR12();

    regVal &= ~ID804_MIRROR12_TC_BASE_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR12_TC_BASE_POS) & ID804_MIRROR12_TC_BASE_MSK;

    return id804_set_rEF_OTP1_MIRROR12(regVal);
}

uint16_t id804_get_tc_offset(void)
{
    uint16_t regVal = id804_get_rF0_OTP1_MIRROR13();
    return (uint16_t)((regVal & ID804_MIRROR13_TC_OFFSET_MSK) >> ID804_MIRROR13_TC_OFFSET_POS);
}

bool id804_set_tc_offset(uint16_t value)
{
    uint16_t regVal = id804_get_rF0_OTP1_MIRROR13();

    regVal &= ~ID804_MIRROR13_TC_OFFSET_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR13_TC_OFFSET_POS) & ID804_MIRROR13_TC_OFFSET_MSK;

    return id804_set_rF0_OTP1_MIRROR13(regVal);
}

uint16_t id804_get_pwm_max_r(void)
{
    uint16_t regVal = id804_get_rF1_OTP2_MIRROR14();
    return (uint16_t)((regVal & ID804_MIRROR_PWM_MAX_VAL_MSK) >> ID804_MIRROR_PWM_MAX_VAL_POS);
}

bool id804_set_pwm_max_r(uint16_t value)
{
    uint16_t regVal = id804_get_rF1_OTP2_MIRROR14();

    regVal &= ~ID804_MIRROR_PWM_MAX_VAL_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_PWM_MAX_VAL_POS) & ID804_MIRROR_PWM_MAX_VAL_MSK;

    return id804_set_rF1_OTP2_MIRROR14(regVal);
}

uint16_t id804_get_pwm_max_g(void)
{
    uint16_t regVal = id804_get_rF2_OTP2_MIRROR15();
    return (uint16_t)((regVal & ID804_MIRROR_PWM_MAX_VAL_MSK) >> ID804_MIRROR_PWM_MAX_VAL_POS);
}

bool id804_set_pwm_max_g(uint16_t value)
{
    uint16_t regVal = id804_get_rF2_OTP2_MIRROR15();

    regVal &= ~ID804_MIRROR_PWM_MAX_VAL_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_PWM_MAX_VAL_POS) & ID804_MIRROR_PWM_MAX_VAL_MSK;

    return id804_set_rF2_OTP2_MIRROR15(regVal);
}

uint16_t id804_get_pwm_max_b(void)
{
    uint16_t regVal = id804_get_rF3_OTP2_MIRROR16();
    return (uint16_t)((regVal & ID804_MIRROR_PWM_MAX_VAL_MSK) >> ID804_MIRROR_PWM_MAX_VAL_POS);
}

bool id804_set_pwm_max_b(uint16_t value)
{
    uint16_t regVal = id804_get_rF3_OTP2_MIRROR16();

    regVal &= ~ID804_MIRROR_PWM_MAX_VAL_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR_PWM_MAX_VAL_POS) & ID804_MIRROR_PWM_MAX_VAL_MSK;

    return id804_set_rF3_OTP2_MIRROR16(regVal);
}

uint16_t id804_get_otp_crc(void)
{
    uint16_t regVal = id804_get_rF4_OTP2_MIRROR17();
    return (uint16_t)((regVal & ID804_MIRROR17_OTP_CRC_MSK) >> ID804_MIRROR17_OTP_CRC_POS);
}

bool id804_set_otp_crc(uint16_t value)
{
    uint16_t regVal = id804_get_rF4_OTP2_MIRROR17();

    regVal &= ~ID804_MIRROR17_OTP_CRC_MSK;
    regVal |= ((uint16_t)value << ID804_MIRROR17_OTP_CRC_POS) & ID804_MIRROR17_OTP_CRC_MSK;

    return id804_set_rF4_OTP2_MIRROR17(regVal);
}
/* USER CODE END 0 */
