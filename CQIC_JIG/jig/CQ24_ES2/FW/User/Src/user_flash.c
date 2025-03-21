/** @file user_flash.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#include "main.h"
#include "user_flash.h"

adc_offset_t gt_offset_data;
int16_t gn_offset_cur_pos = -1;
int16_t gn_offset_next_pos = -1;

/* Clear flags */
#if 0
static void FLASH_If_Init(void)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    HAL_FLASH_Lock();
}
#endif
/* Erase flash memory */
uint32_t FLASH_If_Erase(void)
{
    uint32_t SectorError;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    pEraseInit.Sector = FLASH_SECTOR_1;
    pEraseInit.NbSectors = 1;
    status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);

    HAL_FLASH_Lock();

    if (status != HAL_OK)
    {
        return FLASHIF_ERASEKO;
    }

    return FLASHIF_OK;
}

/* Write flash memory */
uint32_t FLASH_If_Write(int16_t n_offset_save_pos, adc_offset_t *p_data)
{
    uint32_t destination = OFFSET_DATA_START + (n_offset_save_pos * sizeof(adc_offset_t));

    HAL_FLASH_Unlock();

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, (uint32_t)(p_data->n_offset_id)) == HAL_OK)
    {
        /* Validate the written value */
        if (*(uint32_t*)destination != (uint32_t)(p_data->n_offset_id))
        {
            HAL_FLASH_Lock();
            return (FLASHIF_WRITINGCTRL_ERROR);
        }

        /* Increase WORD length */
        destination += 4;
    }
    else
    {
        HAL_FLASH_Lock();
        return (FLASHIF_WRITING_ERROR);
    }

    for(uint8_t i=0 ; i<OFFSET_MAX ; ++i)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, destination, (uint8_t)(p_data->n_offset_data[i])) == HAL_OK)
        {
            /* Validate the written value */
            if (*(uint8_t*)destination != (uint8_t)(p_data->n_offset_data[i]))
            {
                HAL_FLASH_Lock();
                return (FLASHIF_WRITINGCTRL_ERROR);
            }
            /* Increase BYTE length */
            destination += 1;
        }
        else
        {
            HAL_FLASH_Lock();
            return (FLASHIF_WRITING_ERROR);
        }
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, destination, (uint8_t)(p_data->n_adc_tolerance)) == HAL_OK)
    {
        /* Validate the written value */
        if (*(uint8_t*)destination != (uint8_t)(p_data->n_adc_tolerance))
        {
            HAL_FLASH_Lock();
            return (FLASHIF_WRITINGCTRL_ERROR);
        }
        /* Increase BYTE length */
        destination += 1;
    }
    else
    {
        HAL_FLASH_Lock();
        return (FLASHIF_WRITING_ERROR);
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, destination, (uint8_t)(p_data->n_msg_type)) == HAL_OK)
    {
        /* Validate the written value */
        if (*(uint8_t*)destination != (uint8_t)(p_data->n_msg_type))
        {
            HAL_FLASH_Lock();
            return (FLASHIF_WRITINGCTRL_ERROR);
        }
        /* Increase BYTE length */
        destination += 1;
    }
    else
    {
        HAL_FLASH_Lock();
        return (FLASHIF_WRITING_ERROR);
    }

    HAL_FLASH_Lock();
    return (FLASHIF_OK);
}

void offset_find_position(void)
{
    int16_t offset = 0;
    adc_offset_t* p_offset_data;

    for(offset = 0 ; offset < OFFSET_DATA_BLOCK ; ++offset)
    {
        p_offset_data = (adc_offset_t*)OFFSET_DATA_START + offset;
        if(p_offset_data->n_offset_id == 0xFFFFFFFF)
        {
            gn_offset_cur_pos = (offset-1);
            gn_offset_next_pos = offset;
            break;
        }
    }

    if(gn_offset_cur_pos != -1)
    {
        p_offset_data = (adc_offset_t*)OFFSET_DATA_START + gn_offset_cur_pos;

        gt_offset_data.n_offset_id = p_offset_data->n_offset_id;
        for(offset = 0 ; offset < OFFSET_MAX ; ++offset)
        {
            gt_offset_data.n_offset_data[offset] = p_offset_data->n_offset_data[offset];
        }

        gt_offset_data.n_adc_tolerance = p_offset_data->n_adc_tolerance;
        gt_offset_data.n_msg_type = p_offset_data->n_msg_type;
    }
}


/*** end of file ***/

