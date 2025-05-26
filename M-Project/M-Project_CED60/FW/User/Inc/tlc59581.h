#ifndef __TLC59581_H__
#define __TLC59581_H__

typedef enum _tag_tlc59581_pattern_t_
{
    PATTERN_NONE = 0,
    PATTERN_RED,
    PATTERN_GREEN,
    PATTERN_BLUE,
    PATTERN_HORIZONTAL,
    PATTERN_VERTICAL,
    PATTERN_CHECKERBOARD_1,
    PATTERN_CHECKERBOARD_2,
    PATTERN_MAX
}_tlc59581_pattern_t_;

static const char* gs_tlc59581_pattern[PATTERN_MAX] =
{
    "PATTERN_NONE",
    "PATTERN_RED",
    "PATTERN_GREEN",
    "PATTERN_BLUE"
};

extern volatile bool gb_vsync_flag;
extern volatile bool gb_spi_tx_flag;

extern uint16_t tlc59581_get_gray_scale(void);
extern void tlc59581_set_gray_scale(uint32_t gs_value);

extern _tlc59581_pattern_t_ tlc59581_get_pattern(void);
extern void tlc59581_set_pattern(_tlc59581_pattern_t_ pattern);

extern void tlc59581_transmit_vsync(void);
extern void tlc59581_init(void);
extern void tlc59581_process(void);

#endif /* end of __TLC59581__ */
