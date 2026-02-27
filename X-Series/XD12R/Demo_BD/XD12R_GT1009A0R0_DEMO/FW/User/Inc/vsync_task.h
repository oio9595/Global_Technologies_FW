/** @file vsync_task.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __VSYNC_TASK_H__
#define __VSYNC_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN */
extern void Svsync_Update_Handler(void);
extern void Vsync_Timer_Start(void);
extern void Vsync_Timer_Stop(void);
extern void Vsync_Update_Handler(void);

extern void XDIC_Vsync_Task(void);

extern void XDIC_Set_Write_Target_Reg(uint8_t addr, uint16_t data);
extern void XDIC_Set_Read_Target_Reg(uint8_t addr);
/* USER CODE END */

#ifdef __cplusplus
}
#endif

#endif /* ~__VSYNC_TASK_H__ */
/*** end of file ***/