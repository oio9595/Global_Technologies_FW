/*!
 * @file        flash_if.h
 *
 * @brief       Header for bsp_flash.c module
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

/* Define to prevent recursive inclusion */
#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

/* Includes */
#include "main.h"

/** @addtogroup Examples
  @{
  */

/** @addtogroup IAP_BootLoader
  @{
  */

/** @defgroup IAP_BootLoader_Enumerations Enumerations
  @{
  */

/*!
 * @brief    APP type define
 */

/*!
 * @brief    FMC operation type define
 */

/**@} end of group IAP_BootLoader_Enumerations*/

/** @addtogroup IAP_BootLoader_Macros Macros
  @{
  */


/**@} end of group IAP_BootLoader_Macros*/

/** @defgroup IAP_BootLoader_Functions Functions
  @{
  */

/** function declaration*/
uint32_t FLASH_IAP_Erase(int32_t len);
uint32_t FLASH_IAP_Write(__IO uint32_t* address, uint32_t addressEnd, uint32_t* data, uint16_t length);

#endif

/**@} end of group IAP_BootLoader_Functions */
/**@} end of group IAP_BootLoader */
/**@} end of group Examples */
