/*!
 * @file        menu.h
 *
 * @brief       Header for menu.c module 
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
#ifndef __MENU_H
#define __MENU_H

/* Includes */
#include "main.h"

/** @addtogroup Examples
  @{
  */

/** @addtogroup IAP_BootLoader
  @{
  */

/** @defgroup IAP_BootLoader_Macros Macros
  @{
  */

#define MENU_WAIT_TIMEOUT  0xFFFFFF

/**@} end of group IAP_BootLoader_Macros*/

/** @defgroup IAP_BootLoader_Variables Variables
  @{
  */

/** extern variables*/

/**@} end of group IAP_BootLoader_Variables*/

/** @defgroup IAP_BootLoader_Functions Functions
  @{
  */

/** function declaration*/
void Select_Menu(void);

#endif

/**@} end of group IAP_BootLoader_Functions */
/**@} end of group IAP_BootLoader */
/**@} end of group Examples */
