/** @file types.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char       BOOL;
typedef unsigned char       U8;
typedef char                S8;
typedef unsigned short      U16;
typedef short               S16;
typedef unsigned long       U32;
typedef long                S32;
typedef unsigned long long  U64;
typedef long long           S64;
typedef void*               HANDLE;
typedef float               FLOAT;
typedef double              DOUBLE;

typedef struct _bits
{
    unsigned b0	: 1;
    unsigned b1	: 1;
    unsigned b2	: 1;
    unsigned b3	: 1;
    unsigned b4	: 1;
    unsigned b5	: 1;
    unsigned b6	: 1;
    unsigned b7	: 1;
}BITS;

typedef union
{
    BITS bits_t;
    U8 value;
}BYTE;

#ifndef NULL
#define NULL	((void*)0)
#endif

typedef enum tag_POWER_STATE_T
{
    PWR_OFF = 0,
    PWR_ON,
}power_state_t;

typedef enum tag_POWER_VOLT_T
{
    PWR_ON_5V0 = 0,
    PWR_ON_5V5,
    PWR_OFF_ALL,
}power_volt_t;

typedef enum tag_ONOFF_STATE_T
{
    OFF = 0,
    ON,
}onoff_state_t;

#define PRINT_BUFF_SIZE 400

#ifdef __cplusplus
}
#endif

#endif /* ~__TYPES_H__ */

/*** end of file ***/
