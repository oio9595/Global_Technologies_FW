/** @file user_flash.h
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif

#define OFFSET_DATA_START   0x08004000
#define OFFSET_DATA_END     0x08007FFFF
#define OFFSET_DATA_SIZE    (16U<<10)   /* 16kB */
#define OFFSET_DATA_BLOCK   (OFFSET_DATA_SIZE / sizeof(adc_offset_t) - 1)

/* Error code */
enum 
{
	FLASHIF_OK = 0,
	FLASHIF_ERASEKO,
	FLASHIF_WRITINGCTRL_ERROR,
	FLASHIF_WRITING_ERROR,
	FLASHIF_PROTECTION_ERRROR
};

typedef enum
{
    OFFSET_LEAKAGE_O1 = 0,
    OFFSET_LEAKAGE_O2,
    OFFSET_LEAKAGE_O3,
    OFFSET_LEAKAGE_O4,
    OFFSET_IOUT_O1,
    OFFSET_IOUT_O2,
    OFFSET_IOUT_O3,
    OFFSET_IOUT_O4,
    OFFSET_MAX,
}offset_data_id_t;

typedef struct _OFFSET_DATA_
{
    uint32_t n_offset_id;
    int8_t n_offset_data[OFFSET_MAX];
    uint8_t n_adc_tolerance;
    uint8_t n_msg_type;
    uint8_t n_dummy[2];
}adc_offset_t;

extern adc_offset_t gt_offset_data;
extern int16_t gn_offset_cur_pos;
extern int16_t gn_offset_next_pos;

void offset_find_position(void);
uint32_t FLASH_If_Erase(void);
uint32_t FLASH_If_Write(int16_t n_offset_save_pos, adc_offset_t *p_data);

#ifdef __cplusplus
}
#endif

#endif /* ~__USER_FLASH_H__ */

/*** end of file ***/

