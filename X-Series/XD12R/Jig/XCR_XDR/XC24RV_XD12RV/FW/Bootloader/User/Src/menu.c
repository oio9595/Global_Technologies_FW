/*!
 * @file        menu.c
 *
 * @brief       Display a usart menu to user and include IAP download and upload and area jump operation
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
#include <string.h>

#include "common.h"

/** @addtogroup Examples
  @{
  */

/** @addtogroup IAP_BootLoader
  @{
  */

/** @defgroup IAP_BootLoader_Variables Variables
  @{
  */
static uint8_t FileN[FILE_NAME_MAX];

/**@} end of group IAP_BootLoader_Variables */

/** @defgroup IAP_BootLoader_Functions Functions
  @{
  */

/*!
 * @brief       Download a file via serial port
 *
 * @param       Application
 *              @arg APP1 : Application1
 *              @arg APP2 : Application2
 *
 * @retval      None
 *
 * @note
 */
static void Download(void)
{
    uint8_t Number[16] = {0, }, i = 0;
    uint32_t timeout = 0xFFFF;
    int32_t Size = 0;

    SendString(">> Waiting for file to be sent ... (press 'a' to abort)\r\n");
    Size = recceiveFile(FileN);
    while (timeout-- > 0);

    SendString("\n\r");

    switch (Size)
    {
    case -7:
        SendString(">> error count!.\r\n");
        break;

    case -4:
        SendString(">> erase error!.\r\n");
        break;

    case -3:
        SendString(">> Operation aborted!.\r\n");
        break;

    case -2:
        SendString(">> Verification failed!\r\n");
        break;

    case -1:
        SendString(">> file is too large!\r\n");
        break;

    default:
        SendString(">> Completed Successfully!\r\n");

        SendString("***********");

        for (i = 0; (i < 255U) && (FileN[i] != 0x00); i++)
        {
            SendString("*");
        }

        SendString("\n\r   Name: ");
        SendString(FileN);
        IntConStr(Number, Size);
        SendString("\n\r   Size: ");
        SendString(Number);
        SendString(" Bytes\r\n");

        SendString("***********");

        for (i = 0; (i < 255U) && (FileN[i] != 0x00); i++)
        {
            SendString("*");
        }
        break;
    }
}

/*!
 * @brief       Display the Main Menu on HyperTerminal
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void Select_Menu(void)
{
    uint8_t key = 0;
    uint8_t flag = 0;
    uint8_t enter_menu = 0;
    
    SendString("\n\r Build : ");
    SendString(__DATE__);

    SendString("\n\r>> Bootloader : ");
    __enable_irq();
    gn_msec_tick_count = 500;  /* ms */
    /** Waiting for user input */
    while(gn_msec_tick_count)
    {
        if(GetKeyPressed((uint8_t *)&key) == SUCCESS)
        {
            Send_Char(key); /* echo back */
            if(key == 'G' || key == 'g')
            {
                if(flag == 0)
                {
                    flag |= 1;
                    gn_msec_tick_count = 1000;
                }
                else
                {
                    break;
                }
            }
            else if(key == 'T' || key == 't')
            {
                if(flag == 1)
                {
                    flag |= 2;
                    gn_msec_tick_count = 1000;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if(flag == 3)
        {
            enter_menu = 1;
            break;
        }
    }
    __disable_irq();
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

    if(enter_menu == 1)
    {
        uint8_t escape_loop = 0;

        SendString("\r\n*****************************************");
        SendString("\r\n*           STM32F446RE IAP             *");
        SendString("\r\n*****************************************");
        SendString("\r\n");

        while(!escape_loop)
        {
            SendString("\r\n** Please select an operation item \r\n");
            SendString("*  1.Download application --------> 1  *\r\n");
            SendString("*  2.Jump to application ---------> 2  *\r\n");
            SendString("****************************************\r\n");

            /** Receive key */
            key = ReadKey_TimeOut(MENU_WAIT_TIMEOUT);

            switch(key)
            {
            case 0x31:/** Download user application in the Flash */
                Download();
                break;

            case 0x32:/** execute the new program */
            case 0xFF:/** execute the new program */
                SendString(">> Jump to user application \r\n");
                escape_loop = 1;
                break;

            default:
                SendString(">> Invalid Number ! ==> The number should be 1 or 2.\r\n");
                break;
            }
        }
    }
}

/**@} end of group IAP_BootLoader_Functions */
/**@} end of group IAP_BootLoader */
/**@} end of group Examples */
