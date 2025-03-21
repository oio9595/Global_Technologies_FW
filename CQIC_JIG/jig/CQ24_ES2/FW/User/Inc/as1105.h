/** @file as1105.h
 * 
 * @brief A LED driver[AS1105]
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#ifndef __AS1105_H__
#define __AS1105_H__

#ifdef __cplusplus
 extern "C" {
#endif

#define CHAR_DASH   10
#define CHAR_E      11
#define CHAR_H      12
#define CHAR_L      13
#define CHAR_P      14
#define CHAR_BLANK  15
#define CHAR_o      16
#define CHAR_n      17
#define CHAR_F      18
#define CHAR_A      19

void as1105_init(void);

void as1105_set_decode(uint16_t n_decode);
void as1105_set_intensity(uint16_t n_intensity);
void as1105_set_font(uint8_t n_font_type);
void as1105_write_digit(uint8_t n_digit_num, uint8_t n_value, uint8_t b_dot);
void as1105_write_digit_multi(uint8_t n_digit_num, uint8_t* pn_data, uint8_t n_num_byte);
void as1105_set_test_mode(uint8_t n_value);

#ifdef __cplusplus
}
#endif

#endif /* ~__AS1105_H__ */

/*** end of file ***/
