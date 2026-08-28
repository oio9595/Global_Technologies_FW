from datetime import datetime
from pathlib import Path


def get_description(file_name: str) -> str:
    """
    Generate source file description based on file name.
    """

    description_map = {
        "spi": "SPI driver implementation",
        "uart": "UART driver implementation",
        "i2c": "I2C driver implementation",
        "adc": "ADC driver implementation",
        "dac": "DAC driver implementation",
        "timer": "Timer driver implementation",
        "tim": "Timer driver implementation",
        "gpio": "GPIO driver implementation",
        "dma": "DMA driver implementation",
        "can": "CAN driver implementation",
        "iwdg": "Independent watchdog driver implementation",
        "wdg": "Watchdog driver implementation",
        "flash": "Flash driver implementation",
        "eeprom": "EEPROM driver implementation",
        "rtc": "RTC driver implementation",
    }

    # drv_spi → spi
    if file_name.startswith("drv_"):
        device_name = file_name[4:].lower()

        if device_name in description_map:
            return description_map[device_name]

        return f"{device_name.upper()} driver implementation"

    return f"{file_name} implementation"


def create_source_files(file_name: str) -> None:
    # 확장자가 입력되어 있어도 제거
    base_name = Path(file_name).stem

    # 현재 날짜
    now = datetime.now()

    year = now.strftime("%Y")
    date = now.strftime("%Y. %m. %d.")

    # Header Guard
    header_guard = f"{base_name.upper()}_H"

    # Description
    description = get_description(base_name)

    # 파일 생성 경로
    output_dir = Path(__file__).resolve().parent

    c_path = output_dir / f"{base_name}.c"
    h_path = output_dir / f"{base_name}.h"

    # -------------------------------------------------------------------------
    # .c file
    # -------------------------------------------------------------------------

    c_content = f"""/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : {base_name}.c
  * @brief          : {description}
  ******************************************************************************
  * @attention
  *
  * Copyright (c) {year} Global Technologies.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. Direct pairing header (Corresponding header for this source file) */

/* 2. C standard library headers (Alphabetical order) */

/* 3. Project internal / System-related headers */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
"""

    # -------------------------------------------------------------------------
    # .h file
    # -------------------------------------------------------------------------

    h_content = f"""/* USER CODE BEGIN Header */
/*
 * File:   {base_name}.h
 * Author: GT
 *
 * Created on {date}
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __{header_guard}__
#define __{header_guard}__

#ifdef __cplusplus
extern "C" {{
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 1. C standard library headers (Alphabetical order) */

/* 2. Project internal / System-related headers */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}}
#endif

#endif /* __{header_guard}__ */
"""

    # 파일 생성
    c_path.write_text(c_content, encoding="utf-8")
    h_path.write_text(h_content, encoding="utf-8")

    # 절대 경로 출력
    print()
    print(f"Created: {c_path}")
    print(f"Created: {h_path}")


if __name__ == "__main__":
    file_name = input("File name: ").strip()

    if not file_name:
        print("Error: File name is empty.")
    else:
        create_source_files(file_name)