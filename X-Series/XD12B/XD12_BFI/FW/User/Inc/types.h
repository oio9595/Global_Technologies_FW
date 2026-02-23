/** @file types.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) GT. All rights reserved.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define TRUE    1
#define FALSE   0

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

typedef enum
{
    KEY_NONE = 0,
    KEY_POWER_UP,
    KEY_TRIMMING_UP,
    KEY_B1_UP,
    KEY_MAX,
}key_event_t;

typedef enum
{
    PWR_OFF = 0,
    PWR_ON,
}power_state_t;

typedef enum
{
    PWR_ON_5V0 = 0,
    PWR_ON_5V7,
    PWR_OFF_ALL,
}power_volt_t;

typedef enum
{
    OFF = 0,
    ON,
}onoff_state_t;

#define PRINT_BUFF_SIZE 256

#endif /* ~__TYPES_H__ */

/*** end of file ***/
