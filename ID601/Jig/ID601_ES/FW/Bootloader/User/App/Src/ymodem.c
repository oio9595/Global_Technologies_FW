/*!
 * @file        ymodem.c
 *
 * @brief       Include Ymodem operation to HyperTerminal
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
#include "ymodem.h"
#include <string.h>

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
 * @brief       Receive byte(s) from sender
 *
 * @param       character: input character
 *
 * @param       revTimeout: receive timeout
 *
 * @retval      SUCCESS : Byte be received
 *              ERROR: receive timeout
 * @note
 */
static int32_t Receive_Byte(uint8_t *character, uint32_t revTimeout)
{
    while(revTimeout-- > 0)
    {
        if(GetKeyPressed(character) == SUCCESS)
        {
            return SUCCESS;
        }
    }

    return ERROR;
}

/*!
 * @brief       Send a byte to receiver
 *
 * @param       character: character to be sent
 *
 * @retval      reture value
 *              @arg 0: Byte sent
 *
 * @note
 */
static uint32_t Send_Byte(uint8_t character)
{
    Send_Char(character);

    return 0;
}

/*!
 * @brief       Update CRC16 for input byte
 *
 * @param       value: CRC input value
 *
 * @param       b: input byte
 *
 * @retval      returnTemp: Updated CRC value
 *
 * @note
 */
uint16_t CRC16_Update(uint16_t value, uint8_t b)
{
    uint32_t value_crc = value;
    uint32_t value_int = b | 0x100;
    uint16_t returnTemp;

    do
    {
        value_crc <<= 1;
        value_int <<= 1;

        if(value_int & 0x100)
        {
            ++ value_crc;
        }

        if(value_crc & 0x10000)
        {
            value_crc ^= 0x1021;
        }
    }
    while(!(value_int & 0x10000));
    
    returnTemp = (uint16_t)(value_crc & 0xffff);

    return returnTemp;
}

/*!
 * @brief       Cal CRC16
 *
 * @param       data
 *
 * @param       len
 *
 * @retval      CRC16 sum
 *
 * @note
 */
uint16_t CRC16_Sum(const uint8_t *data, uint32_t len)
{
    uint32_t crc_sum = 0;
    const uint8_t *data_len = data + len;

    do
    {
        crc_sum = CRC16_Update(crc_sum,*data++);
    }
    while(data < data_len);

    crc_sum = CRC16_Update(CRC16_Update(crc_sum,0),0);

    crc_sum &= (uint16_t)0xffff;

    return crc_sum;
}

#if 0
/*!
 * @brief       Cal Check sum for YModem Packet
 *
 * @param       data
 *
 * @param       len
 *
 * @retval      checksum
 *
 * @note
 */
uint8_t Check_sum(const uint8_t *data, uint32_t len)
{
    uint32_t checksum = 0;
    const uint8_t *data_len = data+len;

    do
    {
        checksum += *data++;
    }
    while (data < data_len);

    checksum &= (uint16_t)0xff;
    
    return checksum;
}
#endif

/*!
 * @brief       Receive a Ymodem packet from sender
 *
 * @param       data
 *
 * @param       len
 *
 * @param       timeout
 *              @arg 0 : end
 *              @arg -1: abort
 *              @arg >0: pack length
 *
 * @retval      receive status
 *              @arg 0 : normally return
 *              @arg -1: timeout or pack error
 *              @arg 1 : abort
 *
 * @note
 */
static int32_t Receive_YmodePacket(uint8_t *data, int32_t *len, uint32_t timeout)
{
    uint16_t i, pack_len, pack_crc;
    uint8_t character;
    *len = 0;

    if (Receive_Byte(&character, timeout) != SUCCESS)
    {
        return -1;
    }

#if 0
    if((character == FRAME_SOH) && (*data == FRAME_SOH))
    {
        //if(Receive_Byte(&character, timeout) != SUCCESS)
        {
            return -1;
        }
    }
#endif

    if(character == FRAME_SOH)
    {
        pack_len = PACKET_SIZE;
    }
    else if(character == FRAME_STX)
    {
        pack_len = PACKET_1K_SIZE;
    }
    else if(character == FRAME_EOT)
    {
        return 0;
    }
    else if(character == FRAME_CAN)
    {
        if((Receive_Byte(&character, timeout) == SUCCESS) && (character == FRAME_CAN))
        {
            *len = -1;
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if((character == ABORT1)|(character == ABORT2))
    {
        return 1;
    }
    else
    {
        return -1;
    }

    *data = character;

    for(i = 1; i < (pack_len + PACK_FRAME_OVERHEAD); i ++)
    {
        if (Receive_Byte(data + i, timeout) != SUCCESS)
        {
            return -1;
        }
    }

    if(data[PACK_FRAMENO_INDEX] != ((data[PACK_FRAMENO_COMP_INDEX] ^ 0xff) & 0xff))
    {
        return -1;
    }

    /* Calculate the CRC16 */
    pack_crc = CRC16_Sum(&data[PACK_FRAME_HEADER], (uint32_t)pack_len);

    if(pack_crc != (uint16_t)((data[pack_len + 3] << 8) | data[pack_len + 4]))
    {
        /* CRC16 error */
        return -1;
    }

    *len = pack_len;
    
    return 0;
}

/*!
 * @brief       Receive a file from sender using the Ymodem
 *
 * @param       buffer: file buffer
 *
 * @param       Application : APP1 or APP2
 *
 * @retval      The lenght of the file
 *
 * @note
 */
int32_t recceiveFile(uint8_t *buffer)
{
    static uint8_t pack_data[PACK_1K_FRAME_OVERHEAD_SIZE + 3U] = {0, };
    uint8_t file_len[FILE_SIZE_MAX] = {0, };
    uint8_t *file_p = NULL;
    
    int32_t i = 0, pack_len = 0; 
    int32_t session_finish = 0; 
    int32_t file_finish = 0; 
    int32_t packs_received = 0;
    int32_t err = 0;
    int32_t session_begin = 0;
    int32_t pack_ref = 0;
    int32_t len = 0;
    uint32_t code = 0;
    uint32_t flashaddress = APP_IMAGE_BEGIN;
    uint32_t addressEnd = (APP_IMAGE_BEGIN + APP_IMAGE_SIZE - 1U);

    for (session_finish = 0, err = 0, session_begin = 0; ;)
    {
        for (packs_received = 0, file_finish = 0; ;)
        {
            pack_ref = Receive_YmodePacket(pack_data, &pack_len, NAK_TIMEOUT);

            if (pack_ref == 0x01)
            {
                Send_Byte(FRAME_CAN);
                Send_Byte(FRAME_CAN);
                
                return -3;
            }
            else if (pack_ref == 0)
            {
                err = 0;

                if (pack_len == -1)
                {
                    Send_Byte(FRAME_ACK);
                    return 0;
                }
                else if (pack_len == 0)
                {
                    Send_Byte(FRAME_ACK);
                    file_finish = 1;
                }
                else
                {
                    if ((pack_data[PACK_FRAMENO_INDEX] & 0xff) != (packs_received & 0xff))
                    {
                        Send_Byte(FRAME_NACK);
                    }
                    else
                    {
                        if (packs_received == 0)
                        {
                            /* Ymodem Filename Pack */
                            if (pack_data[PACK_FRAME_HEADER] != 0)
                            {
                                /* Filename pack has valid data */
                                for (i = 0, file_p = pack_data + PACK_FRAME_HEADER; (*file_p != 0) && (i < FILE_NAME_MAX);)
                                {
                                    buffer[i++] = *file_p++;
                                }

                                buffer[i++] = '\0';

                                for (i = 0, file_p ++; (*file_p != ' ') && (i < (FILE_SIZE_MAX - 1));)
                                {
                                    file_len[i++] = *file_p++;
                                }

                                file_len[i++] = '\0';
                                StrConInt(file_len, &len);

                                /** File len is greater than Flash len */
                                if(len > APP_IMAGE_SIZE)
                                {
                                    /** End section */
                                    Send_Byte(FRAME_CAN);
                                    Send_Byte(FRAME_CAN);
                                    
                                    return -1;
                                }

                                /** erase user application area */
                                if(ERROR == FLASH_IAP_Erase(len))
                                {
                                    /** End section */
                                    Send_Byte(FRAME_CAN);
                                    Send_Byte(FRAME_CAN);
                                    
                                    return -4;
                                }
                                else
                                {
                                    Send_Byte(FRAME_ACK);
                                    Send_Byte(FRAME_CRC16);
                                }
                            }
                            /** Filename pack is empty, end section */
                            else
                            {
                                Send_Byte(FRAME_ACK);
                                file_finish = 1;
                                session_finish = 1;
                                break;
                            }
                        }
                        /* Ymodem Data Pack */
                        else
                        {
                            code = (uint32_t)(pack_data + PACK_FRAME_HEADER);
                            /** Write data in Flash */
                            if(FLASH_IAP_Write(&flashaddress, addressEnd, (uint32_t *)code, (uint16_t)pack_len / 4)  == SUCCESS)
                            {
                                Send_Byte(FRAME_ACK);
                            }
                            else
                            {
                                /** End section */
                                Send_Byte(FRAME_CAN);
                                Send_Byte(FRAME_CAN);
                                
                                return -2;
                            }
                        }
                        ++packs_received;
                        session_begin = 1;
                    }
                }
            }
            else
            {
                if (session_begin > 0)
                {
                    ++err;
                }

                if (err > ERRORS)
                {
                    Send_Byte(FRAME_CAN);
                    Send_Byte(FRAME_CAN);
                    
                    return -7;
                }
                Send_Byte(FRAME_CRC16);
                break;
            }

            if (file_finish != 0)
            {
                break;
            }
        }

        if (session_finish != 0)
        {
            break;
        }
    }

    return (int32_t)len;
}

/**@} end of group IAP_BootLoader_Functions */
/**@} end of group IAP_BootLoader */
/**@} end of group Examples */
