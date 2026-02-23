
#ifndef SAL_DRIVER_H_
#define SAL_DRIVER_H_

#include "main.h"

/*! @brief Transfer type
 * Implements : sal_TransferType_Class
 */
typedef enum
{
	SAL_USING_DMA   = 0x00U,
	SAL_USING_INTERRUPTS,
}sal_TransferType;

/*! @brief Interface selection
 * Implements : sal_InterfaceType_Class
 */
typedef enum
{
    SALLED_INTERFACE_FLEXIO = 0x00U,
    SALLED_INTERFACE_LPSPI  = 0x01U,
    SALLED_INTERFACE_GPIO   = 0x02U,
}sal_InterfaceType;

/*! @brief Event type
 * Implements : sal_CommEventType_Class
 */
typedef enum
{
    TRANSMISSION_COMPLETE   = 0x01U,
    RECEPTION_COMPLETE      = 0x02U,
    RECEPTION_TIMEOUT       = 0x03U,
    CRC_ERROR               = 0x04U,
    CHAINLENGTH_ERROR       = 0x05U,
}sal_CommEventType;

/*! @brief Pin config for a LED strip
 * Implements : sal_PinConfigType_Class
 */
typedef struct
{
	uint8_t stripNr;
    uint8_t dataPin;
    uint8_t clockPin;
}sal_PinConfigType;

/*! @brief Timeout configuration for a LED strip
 * Implements : sal_TimeoutType_Class
 */
typedef struct
{
	//const timing_instance_t *timingPalInstance;
	uint32_t          timeoutDuration;
	uint8_t           timerChannel;
}sal_TimeoutType;

/*! @brief Configuration structure
 * Implements : sal_ConfigType_Class
 */
typedef struct
{
    uint8_t             nrOfStrips;
    void                (*callback)(sal_CommEventType event, uint8_t strip);
    sal_InterfaceType   interfaceType;
    sal_PinConfigType   *pinConfig;
    sal_TimeoutType     *timeoutConfig;
    sal_TransferType    transferType;
    uint8_t             dmaChanTx;
    uint8_t             dmaChanRx;
    uint8_t             *dmaParallelChan;
}sal_ConfigType;

/*! @brief Return type
 * Implements : sal_ReturnType_Class
 */
typedef enum
{
    SALLED_ERROR    = 0U,  /*!< Function returns with failure */
    SALLED_OK       = 1U,  /*!< Function returns successful */
	SALLED_BUSY     = 2U,  /*!< Function is busy*/
}sal_ReturnType;

/*! @brief Initialization parameters
 * Implements : sal_InitType_Class
 */
typedef struct
{
    uint16_t firstLedAdr;                      /*!< address of the first LED in the chain */
    bool crcEnable;                            /*!< 0:disable, 1:enable CRC generation at frames */
    bool tempCmpEnable;                        /*!< 0:disable, 1:enable temperature compensation for red LED */
	bool phaseShift;                           /*!< 0:activate the 3 PWM channels phase shift
	                                            *   1:deactivate the 3 PWM channels phase shift */
}sal_InitType;

/*! @brief Return data
 * Implements : sal_ReadDataResultType_Class
 */
typedef struct
{
    uint16_t chainLength;                     /*!< 0:4095 Amount of Smart LED drivers of the chain. */
    uint32_t *retData;                        /*!< Pointer to an array with status values of each Smart LED driver of the chain. */
}sal_ReadDataResultType;

/*! @brief Block command member. These are used to create a list of commands to be sent
 * Implements : sal_SendCmdBlockType_Class
 */
typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint16_t addr;
	uint8_t cmd;
	uint8_t padding[10U];
}sal_SendCmdBlockType;

/*! @brief Parallel command member. These are used to create a list of commands to be sent
 * Implements : sal_CmdType_Class
 */
typedef struct
{
    uint8_t stripNr;
    uint8_t cmdID;
    uint16_t addr;
    uint32_t param;
    void* paramExtension;
    uint8_t padding[4U];
} sal_CmdType;

/*! @brief Parallel command member. These are used to create a list of commands to be sent
 * Implements : sal_SendCmdParallelType_Class
 */
typedef struct
{
    uint8_t stripNr;
    uint16_t nrOfCmds;
    sal_CmdType* commands;
} sal_SendCmdParallelType;

#define DEV_ADDR_BROADCAST      0x000
#define DEV_ADDR_UNICAST_MIN    0x001
#define DEV_ADDR_UNICAST_MAX    0xFEE   /* 4078,  1 ~ 4078 */
#define DEV_ADDR_MULTICAST_MIN  0xFEF
#define DEV_ADDR_MULTICAST_MAX  0xFFE

typedef enum
{
    SAL_PWR_SLEEP   = 0x00U,
    SAL_PWR_ACTIVE,
    SAL_PWR_DEEPSLEEP,
    SAL_PWR_MAX
}sal_PowerMode_t;

typedef enum
{
    SAL_TEMP_LUT_TC1   = 0x00U,
    SAL_TEMP_LUT_TC2,
    SAL_TEMP_LUT_TC3,
    SAL_TEMP_LUT_TC4,
    SAL_TEMP_LUT_TC5,
    SAL_TEMP_LUT_TC6,
    SAL_TEMP_LUT_TC7,
    SAL_TEMP_LUT_TC8,
    SAL_TEMP_LUT_TC9,
    SAL_TEMP_LUT_TC10,
    SAL_TEMP_LUT_MAX
}sal_TempLutTC_t;

typedef enum
{
    SAL_MCAST_GRP_NONE = 0x00U,
    SAL_MCAST_GRP_01,
    SAL_MCAST_GRP_02,
    SAL_MCAST_GRP_03,
    SAL_MCAST_GRP_04,
    SAL_MCAST_GRP_05,
    SAL_MCAST_GRP_06,
    SAL_MCAST_GRP_07,
    SAL_MCAST_GRP_08,
    SAL_MCAST_GRP_09,
    SAL_MCAST_GRP_10,
    SAL_MCAST_GRP_11,
    SAL_MCAST_GRP_12,
    SAL_MCAST_GRP_13,
    SAL_MCAST_GRP_14,
    SAL_MCAST_GRP_15,
    SAL_MCAST_GRP_16,
    SAL_MCAST_GRP_MAX,
}sal_MCAST_Group_t;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
}sal_rgb_t;


void sal_tx_completed(uint8_t strip_num);
void sal_rx_completed(uint8_t strip_num);
uint32_t sal_rx_parser(uint8_t* in, uint8_t size, uint8_t strip_num);
sal_rgb_t* sal_get_rgb_data(uint8_t strip_num);
uint16_t sal_get_chain_length(void);
void sal_set_chain_length(uint8_t length);

/*!
 * @brief Initializes the SAL interface or interfaces
 *
 * This function initializes the SAL driver
 *
 * @param nrOfInterfaces          Number of hardware interfaces used for SAL communication (FlexIO / LPSPI)
 * @param configStruct            Pointer to the configuration structure
 * @return
 * 		- SALLED_OK: Interface configured successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters
 */
sal_ReturnType sal_init_interface(uint8_t nrOfInterfaces, const sal_ConfigType *configStruct);

/*!
 * @brief Initializes on LED strip
 *
 * This function initializes one LED strip for communication
 *
 * @param type                    Type of hardware interface to be used (FlexIO / LPSPI)
 * @param configStruct            Pointer to the configuration structure
 * @param strip_num               Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_init_strip(const sal_InitType* ChainInitPtr, sal_ReadDataResultType* ChainInitResultPtr, uint8_t strip_num);

/*!
  * @brief Define multicast groups - 0: do not join group, 1 ~ 16 : join group
 *
 * @param  Param                  0-16: Group id
 * @param  address:               0-4079: address of the target LED. 0 addresses all LEDs of the chain.
 * @param strip_num                 Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_set_mcast(sal_MCAST_Group_t grp, uint16_t address, uint8_t strip_num);

/*!
 * @brief Set a RGB value to the addressed LED
 *
 * This function sets the color of the addressed LED. address = 0 is a broadcast address targeting all LEDs in parallel.
 *
 * @param Red                     0-255: Sets the intensity of the red LED
 * @param Green                   0-255: Sets the intensity of the green LED
 * @param Blue                    0-255: Sets the intensity of the blue LED
 * @param address                 0-4079: address of the target LED. 0 addresses all LEDs of the chain.
 * @param strip_num               Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 */
sal_ReturnType sal_set_RGB(uint8_t red, uint8_t green, uint8_t blue, uint16_t address, uint8_t strip_num);

/*!
 * @brief the software reset
 *
 * This function resets the internal state of the LEDs. sal_Init_Strip can be called after this command. sal_reset can also be called before
 * sal_Init_Strip to bring all the LEDs in the chain to a known state. When sent as a first command, sal_reset is always sent with CRC.
 *
 * @param address               0-4079: address of the target LED. 0 addresses all LEDs of the chain.
 * @param strip_num             Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Interface not initialized or
 * 						strip_num bigger than maximum supported number of strips.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_set_reset(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_initbidir(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_clear_error(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_power_mode(sal_PowerMode_t mode, uint16_t address, uint8_t strip_num);

/*!
 * @brief Configure the addressed Smart LED driver.
 *
 * Sets the 12 bit configuration value of the addressed Smart LED driver.
 *
 * @param Param                   0-4079: 12 bit configuration value
 * @param address                 0-4079: address of the target LED. 0 addresses all LEDs of the chain.
 * @param strip_num                 Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_set_setup1(uint16_t param, uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_setup2(uint16_t param, uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_temperature_threshold(int16_t temperature, uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_temperature_hysterisis(int16_t temperature, uint16_t address, uint8_t strip_num);
sal_ReturnType sal_set_current_max_level(uint16_t param, uint16_t address, uint8_t strip_num);

/*!
 * @brief Set temperature compensation look-up table entry for red LED
 *
 * Set temperature compensation look-up table entry for red LED of the addressed Smart LED driver.
 * LUT is automatically initialized with linearly increasing values (see: _TC_Base and _TC_Offset).
 * These setting can be manually overwritten with any values.
 *
 * @param LUT_Adr                 0-10: 4 bit look-up table entry (address)
 * @param LUT_Value               0-511: 9 bit look-up table value
 * @param address                 0-4079: address of the target LED. 0 addresses all LEDs of the chain.
 * @param strip_num                 Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_set_temperature_lut_tcx(sal_TempLutTC_t tc_num, uint16_t param, uint16_t address, uint8_t strip_num);

/*!
 * @brief Sets the timeout duration for a strip
 *
 * This function sets the timeout duration for a given strip
 *
 * @param duration                Timeout duration in microseconds
 * @param strip_num                 Number of the strip on which the timeout will be set
 * @return
 * 		- SALLED_OK: Timeout set successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters
 */
sal_ReturnType sal_set_timeout(uint32_t param, uint16_t address, uint8_t strip_num);

/*!
 * @brief Get status of all LEDs
 *
 * Reads the status of all LEDs in chain and stores the data in a structure of type sal_ReadDataResultType.
 *
 * @param ChainStatusPtr          Pointer to structure containing the status values of all LEDs in the chain.
 * @param strip_num               Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_get_status1(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_status2(uint16_t address, uint8_t strip_num);

/*!
 * @brief Get temperature of all LEDs
 *
 * Reads the temperature of all LEDs in chain and stores the data in a structure of type sal_ReadDataResultType.
 *
 * @param ChainTempPtr            Pointer to structure containing the temperature values of all LEDs in the chain.
 * @param strip_num                 Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_get_temperature(uint16_t address, uint8_t strip_num);

sal_ReturnType sal_get_temperature_st(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_vext_tm(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_setup1(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_setup2(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_mcast(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_rgb(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_temperature_threshold(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_temperature_hysterisis(uint16_t address, uint8_t strip_num);


/*!
 * @brief Read current PWM value for the red LED
 *
 * @param ChainStatusPtr           Pointer to structure containing the red PWM values of all LEDs in the chain.
 * @param strip_num                  Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_get_pwm_red(uint16_t address, uint8_t strip_num);

/*!
 * @brief Read current PWM value for the green LED
 *
 * @param ChainStatusPtr           Pointer to structure containing the green PWM values of all LEDs in the chain.
 * @param strip_num                  Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_get_pwm_green(uint16_t address, uint8_t strip_num);

/*!
 * @brief Read current PWM value for the blue LED
 *
 * @param ChainStatusPtr           Pointer to structure containing the blue PWM values of all LEDs in the chain.
 * @param strip_num                  Number of the strip on which the commands will be sent
 * @return
 * 		- SALLED_OK: Function returned successfully.
 * 		- SALLED_ERROR: Error due to invalid parameters or
 * 						Interface not initialized or
 * 						Strip not initialized or
 * 						strip_num bigger than maximum supported number of strips or
 * 						chainLength is 0.
 * 		- SALLED_BUSY: The previous command has not finished transmitting or receiving. Bus is busy.
 *
 */
sal_ReturnType sal_get_pwm_blue(uint16_t address, uint8_t strip_num);

sal_ReturnType sal_get_current_max_level(uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_temperature_lut_tcx(sal_TempLutTC_t tc_num, uint16_t address, uint8_t strip_num);
sal_ReturnType sal_get_timeout(uint16_t address, uint8_t strip_num);

EXTERN void display_command_list(void);

/*! @}*/
#endif /* SAL_DRIVER_H_ */
