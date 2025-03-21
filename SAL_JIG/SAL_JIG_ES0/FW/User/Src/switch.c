/** @file switch.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __SWITCH_C__

#include "switch.h"
#include <main.h>

#ifdef __SWITCH_C__

extern I2C_HandleTypeDef hi2c1;

void decode_mode_set(decode_mode_t n_mode)
{
    switch (n_mode)
    {
    case DECODE_ADC_GAIN_TRIM :
        EN_SEL_1_LO();
        EN_SEL_2_LO();
        EN_SEL_3_LO();
        EN_SEL_4_LO();
        break;
    case DECODE_LTC_DRIVER_A :
        EN_SEL_1_HI();
        EN_SEL_2_LO();
        EN_SEL_3_LO();
        EN_SEL_4_LO();
        break;
    case DECODE_TEST_V_TRIM :
        EN_SEL_1_LO();
        EN_SEL_2_HI();
        EN_SEL_3_LO();
        EN_SEL_4_LO();
        break;
    case DECODE_SIO1_LVDS_TX :
        EN_SEL_1_HI();
        EN_SEL_2_HI();
        EN_SEL_3_LO();
        EN_SEL_4_LO();
        break;
    case DECODE_SIO2_LVDS_TX :
        EN_SEL_1_LO();
        EN_SEL_2_LO();
        EN_SEL_3_HI();
        EN_SEL_4_LO();
        break;
    case DECODE_SIO1_LVDS_RX :
        EN_SEL_1_HI();
        EN_SEL_2_LO();
        EN_SEL_3_HI();
        EN_SEL_4_LO();
        break;
    case DECODE_SIO2_LVDS_RX :
        EN_SEL_1_LO();
        EN_SEL_2_HI();
        EN_SEL_3_HI();
        EN_SEL_4_LO();
        break;
    case DECODE_LTC_DRIVER_B :
        EN_SEL_1_HI();
        EN_SEL_2_HI();
        EN_SEL_3_HI();
        EN_SEL_4_LO();
        break;
    default :
        EN_SEL_1_HI();
        EN_SEL_2_HI();
        EN_SEL_3_HI();
        EN_SEL_4_HI();
    }
    LL_mDelay(10);
}

void sal_make_mcu_mode(void)
{
    HAL_I2C_DeInit(&hi2c1);

    // SIOxP : PU, SIOxN : PD
    I2C_SIO1P_PU_LO(); // disconnect i2c
    I2C_SIO1N_PU_LO(); // disconnect i2c
    //I2C_SIO1N_PD_LO(); // disconnect i2c

    I2C_SIO2P_PU_LO(); // disconnect i2c
    I2C_SIO2N_PU_LO(); // disconnect i2c
    //I2C_SIO2N_PD_LO(); // disconnect i2c

    decode_mode_set(DECODE_NONE);

    SPI1_SIO1P_HI(); // SIO1P PU
    SPI1_SIO1N_HI(); // SIO1P PD

    SPI1_SIO2P_LO(); // SIO2P PU
    SPI1_SIO2N_LO(); // SIO2P PD

    SPI2_SIO1P_LO(); // disconnect spi2
    SPI2_SIO1N_LO(); // disconnect spi2

    SPI2_SIO2P_LO(); // disconnect spi2
    SPI2_SIO2N_LO(); // disconnect spi2

    SAL_MODE_LO();
    // ??SAL_MODE2_LO();

    LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
    LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
    LL_mDelay(10);
}

void sal_make_i2c_mode(void)
{
    MX_I2C1_Init();

    // SIOxP : PU, SIOxN : PU
    SPI1_SIO1P_LO(); // disconnect spi1
    SPI1_SIO1N_LO(); // disconnect spi1

    SPI1_SIO2P_LO(); // disconnect spi1
    SPI1_SIO2N_LO(); // disconnect spi1

    SPI2_SIO1P_LO(); // disconnect spi2
    SPI2_SIO1N_LO(); // disconnect spi2

    SPI2_SIO2P_LO(); // disconnect spi2
    SPI2_SIO2N_LO(); // disconnect spi2

    decode_mode_set(DECODE_NONE);

    I2C_SIO1P_PU_HI(); // SIO1P PU
    I2C_SIO1N_PU_HI(); // SIO1N PU
    // I2C_SIO1N_PD_LO(); // disconnect SIO1N PD

    // I2C_SIO2P_PU_HI(); // SIO2P PU
    // I2C_SIO2N_PU_HI(); // SIO2N PU
    // I2C_SIO2N_PD_LO(); // disconnect SIO2N PD

    SAL_MODE_HI();

    LL_mDelay(10);
}

void sal_vled_en(power_state_t pwr)
{
    if (pwr == PWR_ON)
    {
        // vled enable
        SAL_VLED_EN_LO();
        print(LOG_LV_DEBUG, "VLED ON\r\n");
    }
    else
    {
        // vled disable
        SAL_VLED_EN_HI();
        print(LOG_LV_DEBUG, "VLED OFF\r\n");
    }
}

void sal_vcc_en(power_state_t pwr)
{
    if (pwr == PWR_ON)
    {
        // vcc enable
        SAL_VCC_EN_LO();
        print(LOG_LV_DEBUG, "VCC ON\r\n");
    }
    else
    {
        // vcc disable
        SAL_VCC_EN_HI();
        print(LOG_LV_DEBUG, "VCC OFF\r\n");
    }
}

void sal_vcc_level_set(_sal_vcc_level_t level)
{
    if (level == SAL_VCC_5V0)
    {
        // vcc set 5V0, default 5V
        SAL_VCC_5V5_LO();
        print(LOG_LV_DEBUG, "VCC 5V\r\n");
    }
    else
    {
        // vcc set 5V5
        SAL_VCC_5V5_HI();
        print(LOG_LV_DEBUG, "VCC 5.5V\r\n");
    }
}

#endif
/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/
