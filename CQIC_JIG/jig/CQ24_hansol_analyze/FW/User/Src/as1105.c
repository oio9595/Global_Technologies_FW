/** @file as1105.c
 * 
 * @brief A LED driver[AS1105]
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

#include "main.h"
#include "as1105.h"

/* AS1105 Register Address Map */
#define REG_NO_OP           0x0000
#define REG_DIGIT_0         0x0100
#define REG_DIGIT_1         0x0200
#define REG_DIGIT_2         0x0300
#define REG_DIGIT_3         0x0400
#define REG_DECODE_MODE     0x0900
#define REG_INTENSITY       0x0A00
#define REG_SCAN_LIMIT      0x0B00
#define REG_SHUTDOWN        0x0C00
#define REG_NOT_USED        0x0D00
#define REG_RESET_N_EXTCLK  0x0E00
#define REG_DISPLAY_TEST    0x0F00

/* Shutdown Register Format (address (hex) = 0xXC) */
#define SHUTDOWN_MODE       0x0000
#define NORMAL_OPERATION    0x0001

/* Decode-mode Register Examples (address (hex) = 0xX9) */
#define DECODE_MODE_NONE            0x0000
#define DECODE_MODE_CODEB_DIGIT0    0x0001  /* Code B decode for digit 0, No decode for digits 4â€“1 */
#define DECODE_MODE_CODEB_DIGIT3_0  0x000F  /* Code B decode for digits 3â€“0 */
#define DECODE_MODE_CODEB_DIGIT4_0  0x00FF  /* Code B decode for digits 4â€“0 */

/* No-decode Mode Data Bits and Corresponding Segment Lines */
#define SEGMNET_DOT         0x80
#define SEGMNET_CHAR_0      0x7E
#define SEGMNET_CHAR_1      0x30
#define SEGMNET_CHAR_2      0x6D
#define SEGMNET_CHAR_3      0x79
#define SEGMNET_CHAR_4      0x33
#define SEGMNET_CHAR_5      0x5B
#define SEGMNET_CHAR_6      0x5F
#define SEGMNET_CHAR_7      0x70
#define SEGMNET_CHAR_8      0x7F
#define SEGMNET_CHAR_9      0x7B

#define SEGMNET_CHAR_DASH   0x01
#define SEGMNET_CHAR_E      0x4F
#define SEGMNET_CHAR_H      0x37
#define SEGMNET_CHAR_L      0x0E
#define SEGMNET_CHAR_P      0x67
#define SEGMNET_CHAR_o      0x1D
#define SEGMNET_CHAR_n      0x15
#define SEGMNET_CHAR_F      0x47
#define SEGMNET_CHAR_A      0x77
#define SEGMNET_CHAR_BLANK  0x00

/* Intensity Register Format (address (hex) = 0xXA) */
#define INTENSITY_1_32      0x0000    /* min on */
#define INTENSITY_3_32      0x0001
#define INTENSITY_5_32      0x0002
#define INTENSITY_7_32      0x0003
#define INTENSITY_9_32      0x0004
#define INTENSITY_11_32     0x0005
#define INTENSITY_13_32     0x0006
#define INTENSITY_15_32     0x0007
#define INTENSITY_17_32     0x0008
#define INTENSITY_19_32     0x0009
#define INTENSITY_21_32     0x000A
#define INTENSITY_23_32     0x000B
#define INTENSITY_25_32     0x000C
#define INTENSITY_27_32     0x000D
#define INTENSITY_29_32     0x000E
#define INTENSITY_31_32     0x000F    /* max on */

/* Scan-limit Register Format (address (hex) = 0xXB) */
#define DISPLAY_DIGIT_0         0x0000
#define DISPLAY_DIGIT_01        0x0001
#define DISPLAY_DIGIT_012       0x0002
#define DISPLAY_DIGIT_0123      0x0003

/* Reset and external Clock register (address (hex) = 0xXE) */
#define MODE_NORMAL_INTERNAL_CLK       0x0000 /* Normal Operation, internal clock */
#define MODE_NORMAL_EXTERNAL_CLK       0x0001 /* Normal Operation, external clock */
#define MODE_RESET_INTERNAL_CLK        0x0002 /* Reset state, internal clock */
#define MODE_RESET_EXTERNAL_CLK        0x0003 /* Reset state, external clock */

#define BIT_MASK        0x8000  /* The first bit is D15, the most significant bit (MSB). */
/* ********************************************* */
//#define USE_DELAY_FOR_AS1115
#ifdef USE_DELAY_FOR_AS1115
static void us_delay(uint32_t n_delay) /* not 1us delay */
{
    uint32_t i = 0;
    for( ; n_delay != 0 ; --n_delay)
    {
        for(i=0 ; i<1 ; ++i)
        {
            asm("NOP");
        }
    }
}
#endif

static void as1105_strobe_input(uint16_t n_data_out)
{
    return;
    
#if 0    
    /* This shifts 16 bits out MSB first, on the rising edge of the clock, clock idles low */
    GPIO_PinState pinState;

    //clear everything out just in case to
    //prepare shift register for bit shifting
    HAL_GPIO_WritePin(AS1105_LOAD_GPIO_Port, AS1105_LOAD_Pin, GPIO_PIN_RESET);

    //for each bit in the byte myDataOut?
    //NOTICE THAT WE ARE COUNTING DOWN in our for loop
    //This means that %00000001 or "1" will go through such
    //that it will be pin Q0 that lights. 
    for(uint8_t i=0; i<16; ++i) 
    {
        HAL_GPIO_WritePin(AS1105_CLK_GPIO_Port, AS1105_CLK_Pin, GPIO_PIN_RESET);
        //if the value passed to myDataOut and a bitmask result 
        // true then... so if we are at i=6 and our value is
        // %11010100 it would the code compares it to %01000000 
        // and proceeds to set pinState to 1.
        if( n_data_out & (BIT_MASK>>i) )
        {
            pinState = GPIO_PIN_SET;
        }
        else
        {
            pinState = GPIO_PIN_RESET;
        }

#ifdef USE_DELAY_FOR_AS1115
        us_delay(1);
#endif
        //Sets the pin to HIGH or LOW depending on pinState
        HAL_GPIO_WritePin(AS1105_DIN_GPIO_Port, AS1105_DIN_Pin, pinState);
#ifdef USE_DELAY_FOR_AS1115
        us_delay(1);
#endif
        /* The data is shifted into the internal 16-Bit register with the rising edge of the CLK signal. */
        HAL_GPIO_WritePin(AS1105_CLK_GPIO_Port, AS1105_CLK_Pin, GPIO_PIN_SET);
#ifdef USE_DELAY_FOR_AS1115
        us_delay(1);
#endif
        //zero the data pin after shift to prevent bleed through
        HAL_GPIO_WritePin(AS1105_DIN_GPIO_Port, AS1105_DIN_Pin, GPIO_PIN_RESET);
    }

    //us_delay(1);

    /* With the rising edge of the LOAD signal, the data is latched into a digital or control register depending on the address. */
    /* The LOAD signal must go to high after the 16th rising clock edge. */
    HAL_GPIO_WritePin(AS1105_LOAD_GPIO_Port, AS1105_LOAD_Pin, GPIO_PIN_SET);

    /* clock idles low */
    HAL_GPIO_WritePin(AS1105_CLK_GPIO_Port, AS1105_CLK_Pin, GPIO_PIN_RESET);
//#ifdef USE_DELAY_FOR_AS1115
    //us_delay(2);
    //HAL_GPIO_WritePin(AS1105_LOAD_GPIO_Port, AS1105_LOAD_Pin, GPIO_PIN_RESET);
//#endif
    
#endif    
}


/*!
 * @brief : as1105
 *
 * @param[in] :
 *
 * @return :
 */
void as1105_init(void)
{
    uint16_t n_reg_data = 0;

    n_reg_data = (REG_RESET_N_EXTCLK | MODE_RESET_INTERNAL_CLK);
    as1105_strobe_input(n_reg_data);

    n_reg_data = (REG_SHUTDOWN | NORMAL_OPERATION);
    as1105_strobe_input(n_reg_data);

    n_reg_data = (REG_SCAN_LIMIT | DISPLAY_DIGIT_0123);
    as1105_strobe_input(n_reg_data);

    n_reg_data = (REG_INTENSITY | INTENSITY_15_32);
    as1105_strobe_input(n_reg_data);

    n_reg_data = (REG_DECODE_MODE | DECODE_MODE_NONE);
    as1105_strobe_input(n_reg_data);

    n_reg_data = (REG_DIGIT_0 | SEGMNET_CHAR_DASH);
    as1105_strobe_input(n_reg_data);
    n_reg_data = (REG_DIGIT_1 | SEGMNET_CHAR_DASH);
    as1105_strobe_input(n_reg_data);
    n_reg_data = (REG_DIGIT_2 | SEGMNET_CHAR_DASH);
    as1105_strobe_input(n_reg_data);
    n_reg_data = (REG_DIGIT_3 | SEGMNET_CHAR_DASH);
    as1105_strobe_input(n_reg_data);
}

/*!
 * @brief : as1105 DECODE_MODE ¼³Á¤.
 *
 * @param[in] : n_decode.
 *
 * @return : ¾øÀ½
 */
void as1105_set_decode(uint16_t n_decode)
{
    uint16_t n_reg_data = (REG_DECODE_MODE | n_decode);
    as1105_strobe_input(n_reg_data);
}

/*!
 * @brief : as1105 Intensity ¼³Á¤.
 *
 * @param[in] : n_reg.
 * @param[in] : n_intensity.
 *
 * @return : ¾øÀ½
 */
void as1105_set_intensity(uint16_t n_intensity)
{
    uint16_t n_reg_data = (REG_INTENSITY | n_intensity);
    as1105_strobe_input(n_reg_data);
}

/*!
 * @brief : as1105 ÆùÆ® ¼³Á¤.
 *
 * @param[in] : n_font_type.
 *
 * @return : ¾øÀ½
 */
void as1105_set_font(uint8_t n_font_type)
{
}

/*!
 * @brief : as1105 digit ¼³Á¤.
 *
 * @param[in] : n_digit_num.
 * @param[in] : n_data.
 * @param[in] : b_dot.
 *
 * @return : ¾øÀ½
 */
void as1105_write_digit(uint8_t n_digit_pos, uint8_t n_data, uint8_t b_dot)
{
    uint16_t n_reg_addr = 0;
    uint16_t n_reg_data = 0;

    switch(n_digit_pos)
    {
        case 0:  n_reg_addr = REG_DIGIT_0;   break;
        case 1:  n_reg_addr = REG_DIGIT_1;   break;
        case 2:  n_reg_addr = REG_DIGIT_2;   break;
        case 3:  n_reg_addr = REG_DIGIT_3;   break;
        default: n_reg_addr = REG_NO_OP;     break;
    }

    switch(n_data)
    {
        case 0: n_reg_data = SEGMNET_CHAR_0;        break;
        case 1: n_reg_data = SEGMNET_CHAR_1;        break;
        case 2: n_reg_data = SEGMNET_CHAR_2;        break;
        case 3: n_reg_data = SEGMNET_CHAR_3;        break;
        case 4: n_reg_data = SEGMNET_CHAR_4;        break;
        case 5: n_reg_data = SEGMNET_CHAR_5;        break;
        case 6: n_reg_data = SEGMNET_CHAR_6;        break;
        case 7: n_reg_data = SEGMNET_CHAR_7;        break;
        case 8: n_reg_data = SEGMNET_CHAR_8;        break;
        case 9: n_reg_data = SEGMNET_CHAR_9;        break;
        
        case CHAR_DASH:     n_reg_data = SEGMNET_CHAR_DASH;    break;
        case CHAR_E:        n_reg_data = SEGMNET_CHAR_E;       break;
        case CHAR_H:        n_reg_data = SEGMNET_CHAR_H;       break;
        case CHAR_L:        n_reg_data = SEGMNET_CHAR_L;       break;
        case CHAR_P:        n_reg_data = SEGMNET_CHAR_P;       break;
        case CHAR_BLANK:    n_reg_data = SEGMNET_CHAR_BLANK;   break;
        case CHAR_o:        n_reg_data = SEGMNET_CHAR_o;       break;
        case CHAR_n:        n_reg_data = SEGMNET_CHAR_n;       break;
        case CHAR_F:        n_reg_data = SEGMNET_CHAR_F;       break;
        case CHAR_A:        n_reg_data = SEGMNET_CHAR_A;       break;
    }

    if(b_dot)
    {
        n_reg_data |= SEGMNET_DOT;
    }

    n_reg_data = (n_reg_addr | n_reg_data);
    as1105_strobe_input(n_reg_data);
}

/*!
 * @brief : as1105 digit ¼³Á¤.
 *
 * @param[in] : n_digit_num.
 * @param[in] : n_data.
 * @param[in] : n_num_byte.
 *
 * @return : ¾øÀ½
 */
void as1105_write_digit_multi(uint8_t n_dot_pos, uint8_t* pn_data, uint8_t n_num_byte)
{
    uint8_t n_dot_num = 0;
    for(uint8_t i=0 ; i<n_num_byte ; ++i)
    {
        n_dot_num = 0;
        if(n_dot_pos == i)
        {
            n_dot_num = 1;
        }

        as1105_write_digit(i, pn_data[i], n_dot_num);
    }
}

/*!
 * @brief : as1105 test mode ¼³Á¤.
 *
 * @param[in] : n_value.
 *
 * @return : ¾øÀ½
 */
void as1105_set_test_mode(uint8_t n_value)
{
}

/*** end of file ***/
