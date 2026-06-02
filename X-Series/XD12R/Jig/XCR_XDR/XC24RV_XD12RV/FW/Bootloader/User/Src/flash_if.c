/*!
 * @file        flash_if.c
 *
 * @brief       Include IAP flash operation
 *
 * @version     V1.0.2
 *
 * @date        2023-03-01
 *
 * @attention
 *
 *  Copyright (C) 2020-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Includes */
#include "flash_if.h"

/** @addtogroup Examples
  @{
  */

/** @addtogroup IAP_BootLoader
  @{
  */

/** @defgroup IAP_BootLoader_Functions Functions
  @{
  */

/*!
 * @brief       Erase of all user flash area
 *
 * @retval      SUCCESS: user flash area successfully erased
 *              ERROR: error occurred
 */

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes */


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0U;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_6;
    }
    else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7) */
    {
        sector = FLASH_SECTOR_7;
    }

    return sector;
}
#if 0
/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
static uint32_t GetSectorSize(uint32_t Sector)
{
    uint32_t sectorsize = 0x00U;

    if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
    {
        sectorsize = 16U * 1024U;
    }
    else if(Sector == FLASH_SECTOR_4)
    {
        sectorsize = 64U * 1024U;
    }
    else
    {
        sectorsize = 128U * 1024U;
    }  

    return sectorsize;
}
#endif

uint32_t FLASH_IAP_Erase(int32_t len)
{
    if(len <= 0 || len > APP_IMAGE_SIZE)
    {
        return ERROR;
    }

    uint32_t result = 0U;
    uint32_t FirstSector = 0U;
    uint32_t LastSector = 0U;
    uint32_t NbOfSectors = 0U;
    uint32_t SECTORError = 0U;
    
    FLASH_EraseInitTypeDef EraseInitStruct;

    /* Get the 1st & last sector to erase */
    FirstSector = GetSector(APP_IMAGE_BEGIN);
    LastSector = GetSector(APP_IMAGE_BEGIN + len - 1U);
    if(LastSector >= FirstSector)
    {
        /* Get the number of sector to erase from 1st sector*/
        NbOfSectors = (LastSector - FirstSector) + 1U;
        /* Fill EraseInit structure*/
        EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.Banks         = FLASH_BANK_1;
        EraseInitStruct.Sector        = FirstSector;
        EraseInitStruct.NbSectors     = NbOfSectors;
        EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;

        __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
        __HAL_FLASH_DATA_CACHE_DISABLE();

        HAL_FLASH_Unlock();

        if(HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
        {
            result = (ERROR);
        }
        else
        {
            result = (SUCCESS);
        }

        HAL_FLASH_Lock();

        __HAL_FLASH_INSTRUCTION_CACHE_RESET();
        __HAL_FLASH_DATA_CACHE_RESET();
        __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
        __HAL_FLASH_DATA_CACHE_ENABLE();
    }
    else
    {
        result = (ERROR); 
    }
        
    return result;
}

/*!
 * @brief       Write the appoint data buffer in flash
 *
 * @param       address: start address for writing data buffer
 *
 * @param       data: pointer on data buffer
 *
 * @param       length: length of data buffer (unit is 32-bit word)
 *
 * @retval      SUCCESS: data successfully written to Flash memory
 *              ERROR: Error occurred while writing data in Flash memory
 *
 * @note        buffer data are 32-bit aligned
 */
uint32_t FLASH_IAP_Write(__IO uint32_t* address, uint32_t addressEnd, uint32_t* data, uint16_t length)
{
    uint32_t result = SUCCESS;

    HAL_FLASH_Unlock();

    for(uint32_t i = 0U ; i < length; ++i)
    {
        if(*address <= (addressEnd - 4U))
        {
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *address, *(data + i)) == HAL_OK)
            {
                *address += 4U;
            }
            else
            {
                result = ERROR;
                break;
            }
        }
        else
        {
            result = ERROR;
            break;
        }
    }

    HAL_FLASH_Lock();

    return result;
}

/**@} end of group IAP_BootLoader_Functions */
/**@} end of group IAP_BootLoader */
/**@} end of group Examples */
