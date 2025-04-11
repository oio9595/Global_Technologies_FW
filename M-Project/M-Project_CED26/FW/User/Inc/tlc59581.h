#ifndef __TLC59581_H__
#define __TLC59581_H__
/* App Note, Page 13 */
typedef union
{
    uint64_t value;
    struct
    {
        uint64_t LODVTH             : 2;
        uint64_t SEL_TD0            : 2;
        uint64_t LOD_REMOVAL_EN     : 1;
        uint64_t                    : 3;
        uint64_t PREC_EN            : 1;
        uint64_t PREC_MODE3         : 1;
        uint64_t                    : 3;
        uint64_t C_U_E              : 1;
        uint64_t CCB                : 9;    /*  color brightness control, OUT_B0 ~ OUT_B15 */
        uint64_t CCG                : 9;    /*  color brightness control, OUT_G0 ~ OUT_G15 */
        uint64_t CCR                : 9;    /*  color brightness control, OUT_R0 ~ OUT_R15 */
        uint64_t BC                 : 3;    /* global brightness control */
        uint64_t CMD_FC1            : 4;    /* 0b1001 */
    }u;
}_tlc_fc1_t_;


/* App Note, Page 16 */
typedef union
{
    uint64_t value;
    struct
    {
        uint64_t MAX_LINE           : 5;
        uint64_t PSAVE_ENA          : 1;
        uint64_t SEL_GCLK_EDGE      : 1;
        uint64_t SEL_PCHG           : 1;
        uint64_t                    : 3;
        uint64_t EMI_REDUCE_B       : 1;
        uint64_t EMI_REDUCE_G       : 1;
        uint64_t EMI_REDUCE_R       : 1;
        uint64_t SEL_PWM            : 1;
        uint64_t _1st_LINE_ENH      : 2;
        uint64_t                    : 2;
        uint64_t RSV_HIGH           : 1;
        uint64_t LGSE1_B            : 4;
        uint64_t LGSE1_G            : 4;
        uint64_t LGSE1_R            : 4;
        uint64_t INTERFERENCE_B     : 2;
        uint64_t REVERSE_V_B        : 2;
        uint64_t INTERFERENCE_G     : 2;
        uint64_t REVERSE_V_G        : 2;
        uint64_t INTERFERENCE_R     : 2;
        uint64_t REVERSE_V_R        : 2;
        uint64_t CMD_FC2            : 4;    /* 0b0110 */        
    }u;
}_tlc_fc2_t_;

typedef union
{
    uint64_t value;
    struct
    {
        uint64_t OUT_Rn             :16;
        uint64_t OUT_Bn             :16;
        uint64_t OUT_Gn             :16;
    }u;    
}_tlc_gs_memory_t_;

typedef enum
{
    CMD_WRTGS = 0,
    CMD_VSYNC,
    CMD_WRTFC,
    CMD_READSID,
    CMD_READFC1,
    CMD_READFC2,
    CMD_FCWRTEN,
    CMD_MAX,
}cmd_type_t;

typedef enum
{
    LAT_HI_POINT_WRTGS    = 48,
    LAT_HI_POINT_VSYNC    = 1,
    LAT_HI_POINT_WRTFC    = 44,
    LAT_HI_POINT_READSID  = 1,
    LAT_HI_POINT_READFC1  = 1,
    LAT_HI_POINT_READFC2  = 1,
    LAT_HI_POINT_FCWRTEN  = 1,
}LAT_HI_POINT_T;

typedef enum
{
    LAT_LO_POINT_WRTGS    = 48,
    LAT_LO_POINT_VSYNC    = 3,
    LAT_LO_POINT_WRTFC    = 48,
    LAT_LO_POINT_READSID  = 7,
    LAT_LO_POINT_READFC1  = 10,
    LAT_LO_POINT_READFC2  = 11,
    LAT_LO_POINT_FCWRTEN  = 15,
}LAT_LO_POINT_T;
    
/* ---------------------------------------------- */

/* ||              GS DATA PACKET              || */

/* || BIT 47 - 32 || BIT 31 - 16 || BIT 15 - 0 || */

/* ||   B 15 -  0 ||   G 15 -  0 ||   R 15 - 0 || */

/* ---------------------------------------------- */

/* GCLK 257 + dummy 5 * DUMMY_LENGTH for both side set LOW */
#define DUMMY_LENGTH        3
#define VALID_DATA_LENGTH   256
#define GCLK_TABLE_LENGTH   (VALID_DATA_LENGTH + 2 * DUMMY_LENGTH)

#define LED_BUFF_LINE_NUM   26  // Line #0 ~ Line #25, 26 ea
#define LED_BUFF_CH_NUM     32  //  dev1 : OUT_X0 ~  OUT_X15, 16 ea
                                //  dev2 : OUT_X0 ~  OUT_X15, valid : 10 ea, dummy : 6 ea

#define DIMMING_LINE_NUM    26  // Line #0 ~ Line #25, 26 ea
#define DIMMING_CH_NUM      16  //  OUT_X0 ~  OUT_X15, 16 ea

#define TLC_DATA_LENGTH     48
#define TLC_DAISY_SIZE      2

extern uint32_t gn_gclk_pwm_table[GCLK_TABLE_LENGTH];

typedef enum
{
    ADDRESS_BANK_A = 0, 
    ADDRESS_BANK_B,
    ADDRESS_BANK_MAX,
}_tlc_gs_bank_address_t_;

void tlc59581_init();
void tlc_fc1_reg_init();
void tlc_fc2_reg_init();

void tlc_send_daisy_fc_write_enable_command();
void tlc_send_daisy_reg_write_command(cmd_type_t cmd_type, uint64_t dev1_value, uint64_t dev2_value);

void tlc_set_gs_buff();
void tlc_parsing_gs_buff();

_tlc_gs_bank_address_t_ tlc_get_gs_bank_address();
void tlc_set_gs_bank_address(_tlc_gs_bank_address_t_ buff_address);

uint16_t tlc_get_blk_num();
void tlc_set_blk_num(uint16_t n_blk_num);

void tlc_change_pattern(void);

__STATIC_INLINE void delay_100ns(volatile uint32_t delay)
{
    while(delay)
    {
        //__NOP();
        --delay;
    }
}

_tlc_gs_memory_t_ (*tlc_get_parsing_bank_address(void))[LED_BUFF_CH_NUM];
#endif /* end of __TLC59581_H__ */
