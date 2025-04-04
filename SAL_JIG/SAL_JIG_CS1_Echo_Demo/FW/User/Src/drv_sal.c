/** @file drv_sal.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */

#include "drv_sal.h"

#define BROADCAST       (0x00)
#define PREAMBLE        (0x0A)
#define DUMMY_BIT       (0x0A)

#define CRC_BYTE        (1)

#define COLOR_RED       ((64 << 16) | ( 0 << 0) | (  0 << 0))
#define COLOR_GREEN     (( 0 << 16) | (64 << 8) | (  0 << 0))
#define COLOR_BLUE      (( 0 << 16) | ( 0 << 8) | (255 << 0))
#define COLOR_WHITE     ((64 << 16) | (64 << 8) | ( 64 << 0))
#define COLOR_CYAN      (( 0 << 16) | (64 << 8) | ( 64 << 0))
#define COLOR_MAGENTA   ((64 << 16) | ( 0 << 8) | ( 64 << 0))
#define COLOR_YELLOW    ((64 << 16) | (64 << 8) | (  0 << 0))

typedef enum _SAL_CMD_LIST_
{
    LIST_RESET = 0,
    LIST_INITBIDIR,
    LIST_CLRERROR,
    LIST_GOSLEEP,
    LIST_GOACTIVE,
    LIST_GODEEPSLEEP,
    LIST_SET_SETUP1,
    LIST_SET_SETUP2,
    LIST_SET_MCAST,
    LIST_SET_RGB,
    LIST_SET_TEMPTH,
    LIST_SET_TEMPHYS,
    LIST_SET_CURR_MAX_LVL,
    LIST_SET_TEMP_LUT_TC1,
    LIST_SET_TEMP_LUT_TC2,
    LIST_SET_TEMP_LUT_TC3,
    LIST_SET_TEMP_LUT_TC4,
    LIST_SET_TEMP_LUT_TC5,
    LIST_SET_TEMP_LUT_TC6,
    LIST_SET_TEMP_LUT_TC7,
    LIST_SET_TEMP_LUT_TC8,
    LIST_SET_TEMP_LUT_TC9,
    LIST_SET_TEMP_LUT_TC10,
    LIST_SET_TIMEOUT,
    LIST_READ_STATUS1,
    LIST_READ_STATUS2,
    LIST_READ_TEMP,
    LIST_READ_TEMPST,
    LIST_READ_VEXT_TM,
    LIST_READ_SETUP1,
    LIST_READ_SETUP2,
    LIST_READ_MCAST,
    LIST_READ_RGB,
    LIST_READ_TEMPTH,
    LIST_READ_TEMPHYS,
    LIST_READ_PWM_RED_VAL,
    LIST_READ_PWM_GREEN_VAL,
    LIST_READ_PWM_BLUE_VAL,
    LIST_READ_CURR_MAX_LVL,
    LIST_READ_TEMP_LUT_TC1,
    LIST_READ_TEMP_LUT_TC2,
    LIST_READ_TEMP_LUT_TC3,
    LIST_READ_TEMP_LUT_TC4,
    LIST_READ_TEMP_LUT_TC5,
    LIST_READ_TEMP_LUT_TC6,
    LIST_READ_TEMP_LUT_TC7,
    LIST_READ_TEMP_LUT_TC8,
    LIST_READ_TEMP_LUT_TC9,
    LIST_READ_TEMP_LUT_TC10,
    LIST_READ_TIMEOUT,

    LIST_CMD_MAX,
}SAL_CMD_LIST_T;

typedef struct _SAL_CMD_T_
{
    uint8_t cmd;
    struct {
        uint8_t broadcast       : 1;    /* Broadcast possible */
        uint8_t                 : 1;
        uint8_t multicast       : 1;    /* Multicast possible */
        uint8_t                 : 1;
        uint8_t status_request  : 1;    /* Status request allowed(SR) */
        uint8_t                 : 3;
    };
    uint8_t request_bits;   /* Payload size (bits) */
    uint8_t response_bits;  /* Response payload size (bits) + SR */
    const char* str;
}SAL_CMD_T;

static SAL_CMD_T gt_sal_cmd_item[LIST_CMD_MAX] =
{
    { CMD_SAL_RESET,                { 1, 1, 0 },  0,  0, "RESET",               },
    { CMD_SAL_INITBIDIR,            { 0, 0, 0 },  0,  0, "INITBIDIR",           },
    { CMD_SAL_CLRERROR,             { 1, 1, 1 },  0, 12, "CLRERROR",            },
    { CMD_SAL_GOSLEEP,              { 1, 1, 1 },  0, 12, "GOSLEEP",             },
    { CMD_SAL_GOACTIVE,             { 1, 1, 1 },  0, 12, "GOACTIVE",            },
    { CMD_SAL_GODEEPSLEEP,          { 1, 1, 1 },  0, 12, "GODEEPSLEEP",         },
    { CMD_SAL_SET_SETUP1,           { 1, 1, 1 }, 12, 12, "SET_SETUP1",          },
    { CMD_SAL_SET_SETUP2,           { 1, 1, 1 }, 12, 12, "SET_SETUP2",          },
    { CMD_SAL_SET_MCAST,            { 0, 0, 1 }, 24, 12, "SET_MCAST",           },
    { CMD_SAL_SET_RGB,              { 1, 1, 1 }, 24, 12, "SET_RGB",             },
    { CMD_SAL_SET_TEMPTH,           { 1, 1, 1 }, 12, 12, "SET_TEMPTH",          },
    { CMD_SAL_SET_TEMPHYS,          { 1, 1, 1 }, 12, 12, "SET_TEMPHYS",         },
    { CMD_SAL_SET_CURR_MAX_LVL,     { 1, 1, 1 }, 12, 12, "SET_CURR_MAX_LVL",    },
    { CMD_SAL_SET_TEMP_LUT_TC1,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC1",    },
    { CMD_SAL_SET_TEMP_LUT_TC2,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC2",    },
    { CMD_SAL_SET_TEMP_LUT_TC3,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC3",    },
    { CMD_SAL_SET_TEMP_LUT_TC4,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC4",    },
    { CMD_SAL_SET_TEMP_LUT_TC5,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC5",    },
    { CMD_SAL_SET_TEMP_LUT_TC6,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC6",    },
    { CMD_SAL_SET_TEMP_LUT_TC7,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC7",    },
    { CMD_SAL_SET_TEMP_LUT_TC8,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC8",    },
    { CMD_SAL_SET_TEMP_LUT_TC9,     { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC9",    },
    { CMD_SAL_SET_TEMP_LUT_TC10,    { 1, 1, 1 }, 12, 12, "SET_TEMP_LUT_TC10",   },
    { CMD_SAL_SET_TIMEOUT,          { 1, 1, 1 }, 12, 12, "SET_TIMEOUT",         },

    { CMD_SAL_READ_STATUS1,         { 1, 0, 0 },  0, 12, "READ_STATUS1",        },
    { CMD_SAL_READ_STATUS2,         { 1, 0, 0 },  0, 12, "READ_STATUS2",        },
    { CMD_SAL_READ_TEMP,            { 1, 0, 0 },  0, 12, "READ_TEMP",           },
    { CMD_SAL_READ_TEMPST,          { 1, 0, 0 },  0, 24, "READ_TEMPST",         },
    { CMD_SAL_READ_VEXT_TM,         { 1, 0, 0 },  0, 12, "READ_VEXT_TM",        },
    { CMD_SAL_READ_SETUP1,          { 1, 0, 0 },  0, 12, "READ_SETUP1",         },
    { CMD_SAL_READ_SETUP2,          { 1, 0, 0 },  0, 12, "READ_SETUP2",         },
    { CMD_SAL_READ_MCAST,           { 1, 0, 0 },  0, 24, "READ_MCAST",          },
    { CMD_SAL_READ_RGB,             { 1, 0, 0 },  0, 24, "READ_RGB",            },
    { CMD_SAL_READ_TEMPTH,          { 1, 0, 0 },  0, 12, "READ_TEMPTH",         },
    { CMD_SAL_READ_TEMPHYS,         { 1, 0, 0 },  0, 12, "READ_TEMPHYS",        },
    { CMD_SAL_READ_PWM_RED_VAL,     { 1, 0, 0 },  0, 12, "READ_PWM_RED_VAL",    },
    { CMD_SAL_READ_PWM_GREEN_VAL,   { 1, 0, 0 },  0, 12, "READ_PWM_GREEN_VAL",  },
    { CMD_SAL_READ_PWM_BLUE_VAL,    { 1, 0, 0 },  0, 12, "READ_PWM_BLUE_VAL",   },
    { CMD_SAL_READ_CURR_MAX_LVL,    { 1, 0, 0 },  0, 12, "READ_CURR_MAX_LVL",   },
    { CMD_SAL_READ_TEMP_LUT_TC1,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC1",   },
    { CMD_SAL_READ_TEMP_LUT_TC2,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC2",   },
    { CMD_SAL_READ_TEMP_LUT_TC3,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC3",   },
    { CMD_SAL_READ_TEMP_LUT_TC4,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC4",   },
    { CMD_SAL_READ_TEMP_LUT_TC5,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC5",   },
    { CMD_SAL_READ_TEMP_LUT_TC6,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC6",   },
    { CMD_SAL_READ_TEMP_LUT_TC7,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC7",   },
    { CMD_SAL_READ_TEMP_LUT_TC8,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC8",   },
    { CMD_SAL_READ_TEMP_LUT_TC9,    { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC9",   },
    { CMD_SAL_READ_TEMP_LUT_TC10,   { 1, 0, 0 },  0, 12, "READ_TEMP_LUT_TC10",  },
    { CMD_SAL_READ_TIMEOUT,         { 1, 0, 0 },  0, 12, "READ_TIMEOUT",        },
};

typedef union _sal_cmd_data
{
    uint16_t ALL[LIST_CMD_MAX];
    struct
    {
        _sal_dummy_t            _d00;
        _sal_dummy_t            _d01;
        _sal_dummy_t            _d02;
        _sal_dummy_t            _d03;
        _sal_dummy_t            _d04;
        _sal_dummy_t            _d05;
        _sal_setup1_t           setup1;
        _sal_setup2_t           setup2;
        _sal_dummy_t            mcast;
        _sal_dummy_t            rgb;
        _sal_tempth_t           tempth;
        _sal_temphys_t          temphys;
        _sal_curr_max_lvl_t     curr_max_lvl;
        _sal_temp_lut_tc1_t     temp_lut_tc1;
        _sal_temp_lut_tc2_t     temp_lut_tc2;
        _sal_temp_lut_tc3_t     temp_lut_tc3;
        _sal_temp_lut_tc4_t     temp_lut_tc4;
        _sal_temp_lut_tc5_t     temp_lut_tc5;
        _sal_temp_lut_tc6_t     temp_lut_tc6;
        _sal_temp_lut_tc7_t     temp_lut_tc7;
        _sal_temp_lut_tc8_t     temp_lut_tc8;
        _sal_temp_lut_tc9_t     temp_lut_tc9;
        _sal_temp_lut_tc10_t    temp_lut_tc10;
    };
}_sal_cmd_data_t;

//------------------- crc lookup table for polynom 0x2F --------------------//
static const uint8_t gn_crc8_LUT[256] =
{
    0x00, 0x2F, 0x5E, 0x71, 0xBC, 0x93, 0xE2, 0xCD, 0x57, 0x78, 0x09, 0x26, 0xEB, 0xC4, 0xB5, 0x9A,
    0xAE, 0x81, 0xF0, 0xDF, 0x12, 0x3D, 0x4C, 0x63, 0xF9, 0xD6, 0xA7, 0x88, 0x45, 0x6A, 0x1B, 0x34,
    0x73, 0x5C, 0x2D, 0x02, 0xCF, 0xE0, 0x91, 0xBE, 0x24, 0x0B, 0x7A, 0x55, 0x98, 0xB7, 0xC6, 0xE9,
    0xDD, 0xF2, 0x83, 0xAC, 0x61, 0x4E, 0x3F, 0x10, 0x8A, 0xA5, 0xD4, 0xFB, 0x36, 0x19, 0x68, 0x47,
    0xE6, 0xC9, 0xB8, 0x97, 0x5A, 0x75, 0x04, 0x2B, 0xB1, 0x9E, 0xEF, 0xC0, 0x0D, 0x22, 0x53, 0x7C,
    0x48, 0x67, 0x16, 0x39, 0xF4, 0xDB, 0xAA, 0x85, 0x1F, 0x30, 0x41, 0x6E, 0xA3, 0x8C, 0xFD, 0xD2,
    0x95, 0xBA, 0xCB, 0xE4, 0x29, 0x06, 0x77, 0x58, 0xC2, 0xED, 0x9C, 0xB3, 0x7E, 0x51, 0x20, 0x0F,
    0x3B, 0x14, 0x65, 0x4A, 0x87, 0xA8, 0xD9, 0xF6, 0x6C, 0x43, 0x32, 0x1D, 0xD0, 0xFF, 0x8E, 0xA1,
    0xE3, 0xCC, 0xBD, 0x92, 0x5F, 0x70, 0x01, 0x2E, 0xB4, 0x9B, 0xEA, 0xC5, 0x08, 0x27, 0x56, 0x79,
    0x4D, 0x62, 0x13, 0x3C, 0xF1, 0xDE, 0xAF, 0x80, 0x1A, 0x35, 0x44, 0x6B, 0xA6, 0x89, 0xF8, 0xD7,
    0x90, 0xBF, 0xCE, 0xE1, 0x2C, 0x03, 0x72, 0x5D, 0xC7, 0xE8, 0x99, 0xB6, 0x7B, 0x54, 0x25, 0x0A,
    0x3E, 0x11, 0x60, 0x4F, 0x82, 0xAD, 0xDC, 0xF3, 0x69, 0x46, 0x37, 0x18, 0xD5, 0xFA, 0x8B, 0xA4,
    0x05, 0x2A, 0x5B, 0x74, 0xB9, 0x96, 0xE7, 0xC8, 0x52, 0x7D, 0x0C, 0x23, 0xEE, 0xC1, 0xB0, 0x9F,
    0xAB, 0x84, 0xF5, 0xDA, 0x17, 0x38, 0x49, 0x66, 0xFC, 0xD3, 0xA2, 0x8D, 0x40, 0x6F, 0x1E, 0x31,
    0x76, 0x59, 0x28, 0x07, 0xCA, 0xE5, 0x94, 0xBB, 0x21, 0x0E, 0x7F, 0x50, 0x9D, 0xB2, 0xC3, 0xEC,
    0xD8, 0xF7, 0x86, 0xA9, 0x64, 0x4B, 0x3A, 0x15, 0x8F, 0xA0, 0xD1, 0xFE, 0x33, 0x1C, 0x6D, 0x42
};

//------------------- manchester encoding lookup table --------------------//
static const uint16_t gn_manchester_LUT[256] =
{
    0xaaaa, 0xaaa9, 0xaaa6, 0xaaa5, 0xaa9a, 0xaa99, 0xaa96, 0xaa95,
    0xaa6a, 0xaa69, 0xaa66, 0xaa65, 0xaa5a, 0xaa59, 0xaa56, 0xaa55,
    0xa9aa, 0xa9a9, 0xa9a6, 0xa9a5, 0xa99a, 0xa999, 0xa996, 0xa995,
    0xa96a, 0xa969, 0xa966, 0xa965, 0xa95a, 0xa959, 0xa956, 0xa955,
    0xa6aa, 0xa6a9, 0xa6a6, 0xa6a5, 0xa69a, 0xa699, 0xa696, 0xa695,
    0xa66a, 0xa669, 0xa666, 0xa665, 0xa65a, 0xa659, 0xa656, 0xa655,
    0xa5aa, 0xa5a9, 0xa5a6, 0xa5a5, 0xa59a, 0xa599, 0xa596, 0xa595,
    0xa56a, 0xa569, 0xa566, 0xa565, 0xa55a, 0xa559, 0xa556, 0xa555,
    0x9aaa, 0x9aa9, 0x9aa6, 0x9aa5, 0x9a9a, 0x9a99, 0x9a96, 0x9a95,
    0x9a6a, 0x9a69, 0x9a66, 0x9a65, 0x9a5a, 0x9a59, 0x9a56, 0x9a55,
    0x99aa, 0x99a9, 0x99a6, 0x99a5, 0x999a, 0x9999, 0x9996, 0x9995,
    0x996a, 0x9969, 0x9966, 0x9965, 0x995a, 0x9959, 0x9956, 0x9955,
    0x96aa, 0x96a9, 0x96a6, 0x96a5, 0x969a, 0x9699, 0x9696, 0x9695,
    0x966a, 0x9669, 0x9666, 0x9665, 0x965a, 0x9659, 0x9656, 0x9655,
    0x95aa, 0x95a9, 0x95a6, 0x95a5, 0x959a, 0x9599, 0x9596, 0x9595,
    0x956a, 0x9569, 0x9566, 0x9565, 0x955a, 0x9559, 0x9556, 0x9555,
    0x6aaa, 0x6aa9, 0x6aa6, 0x6aa5, 0x6a9a, 0x6a99, 0x6a96, 0x6a95,
    0x6a6a, 0x6a69, 0x6a66, 0x6a65, 0x6a5a, 0x6a59, 0x6a56, 0x6a55,
    0x69aa, 0x69a9, 0x69a6, 0x69a5, 0x699a, 0x6999, 0x6996, 0x6995,
    0x696a, 0x6969, 0x6966, 0x6965, 0x695a, 0x6959, 0x6956, 0x6955,
    0x66aa, 0x66a9, 0x66a6, 0x66a5, 0x669a, 0x6699, 0x6696, 0x6695,
    0x666a, 0x6669, 0x6666, 0x6665, 0x665a, 0x6659, 0x6656, 0x6655,
    0x65aa, 0x65a9, 0x65a6, 0x65a5, 0x659a, 0x6599, 0x6596, 0x6595,
    0x656a, 0x6569, 0x6566, 0x6565, 0x655a, 0x6559, 0x6556, 0x6555,
    0x5aaa, 0x5aa9, 0x5aa6, 0x5aa5, 0x5a9a, 0x5a99, 0x5a96, 0x5a95,
    0x5a6a, 0x5a69, 0x5a66, 0x5a65, 0x5a5a, 0x5a59, 0x5a56, 0x5a55,
    0x59aa, 0x59a9, 0x59a6, 0x59a5, 0x599a, 0x5999, 0x5996, 0x5995,
    0x596a, 0x5969, 0x5966, 0x5965, 0x595a, 0x5959, 0x5956, 0x5955,
    0x56aa, 0x56a9, 0x56a6, 0x56a5, 0x569a, 0x5699, 0x5696, 0x5695,
    0x566a, 0x5669, 0x5666, 0x5665, 0x565a, 0x5659, 0x5656, 0x5655,
    0x55aa, 0x55a9, 0x55a6, 0x55a5, 0x559a, 0x5599, 0x5596, 0x5595,
    0x556a, 0x5569, 0x5566, 0x5565, 0x555a, 0x5559, 0x5556, 0x5555
};

static _sal_cmd_data_t gt_sal_cmd_data;

bool gb_sal_crc_en;
bool gb_sal_init_flag;

volatile uint8_t gn_sal_rx_timeout;
static bool gb_sal_rx_timeout;

demo_pat_t gt_demo_pat = DEMO_PAT_RED;

static uint32_t gn_sal_rgb_buffer[100];
static uint32_t gn_sal_rgb_saved_buffer[100];

/*
// expert color, 51ea
#define COLOR_TABLE_SIZE 51
static const uint16_t gn_sal_color_table[COLOR_TABLE_SIZE][3] =
{
    {225,238,253},{245,224,200},{254,130,  3},{243, 36, 23},{124, 12,239},{ 57,  7,224},{ 22, 64,244},{ 34,199,192},{ 86,244,  0},{129,184,233},
    { 42, 88,195},{ 92,140, 82},{ 64, 49,142},{155, 67,104},{155, 58, 30},{210,150, 40},{ 36,145,160},{248,243,242},{190, 17,  6},{150,175,250},
    {177,146,240},{248, 92, 33},{230, 45,  6},{220, 23,  9},{235, 21,  7},{ 90,  8,240},{ 36,  3,247},{ 26, 21,247},{ 36,240,129},{103,230,  4},
    { 13, 21,240},{ 32,254,  0},{244,180,  0},{ 32,254,  0},{250, 22,  7},{246, 49,  8},{194, 33, 23},{ 31,  2,182},{ 12,  0,246},{ 12, 21,235},
    { 33,248, 53},{ 32,254,  0},{248,243,242},{ 40,220,184},{232, 41,  7},{232, 41,  7},{ 26,118,234},{ 36,  3,148},{124,184,234},{145,255,  0},
    {34,  3,140},
};
*/

// custom color, 64ea
#define COLOR_TABLE_SIZE 64
static const uint16_t gn_sal_color_table[COLOR_TABLE_SIZE][3] =
{
    { 20,  1,170},{ 26,  4,154},{ 36,  4,154},{ 47,  5,142},{ 62,  6,139},{ 79,  6,127},{103, 10,119},{117, 11, 93},{163, 14, 76},{190, 17, 50},
    {208, 19, 28},{228, 20, 18},{237, 22, 11},{232, 18,  9},{245, 20,  9},{248, 23,  7},{237, 24,  6},{225, 22,  8},{214, 24,  7},{230, 28,  8},
    {222, 30,  6},{230, 37,  7},{230, 42,  7},{230, 48,  7},{230, 54,  7},{218, 61,  5},{200, 67,  5},{186, 75,  4},{168, 88,  3},{163,100,  2},
    {156,112,  2},{142,138,  0},{135,165,  0},{127,200,  0},{110,218,  0},{ 94,230,  0},{ 84,234,  0},{ 85,247,  0},{ 80,247,  4},{ 75,247, 15},
    { 60,212, 27},{ 58,220, 45},{ 53,211, 66},{ 48,207, 95},{ 36,180,114},{ 34,180,152},{ 31,180,187},{ 29,161,208},{ 29,147,238},{ 28,125,249},
    { 23, 95,248},{ 19, 74,250},{ 19, 53,253},{ 16, 38,255},{ 13, 26,255},{ 13, 14,255},{ 13,  7,255},{ 11,  4,237},{ 13,  3,222},{ 13,  2,203},
    { 14,  2,188},{ 15,  2,176},{ 16,  1,163},{ 17,  1,152},
};

static const uint16_t gn_sal_echo_demo_table[5][3] =
{
    {255,  0,  0},
    {  0,255,  0},
    {  0,  0,255},
    {255,255,255},
#if 0
    { 51, 10,215},
#else
    { 14,  2,188}, //예전 HMC61번 튜닝한 값
#endif
};

const static uint8_t gn_sal_id_map[100] =
{
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,  20,  21,
    41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24,  23,  22,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,  60,  61,
    81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64,  63,  62,
    82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101,
};

static uint32_t gn_sal_read_target_dev_id;
static uint32_t gn_sal_read_target_cmd;
static bool gb_sal_read_flag;
static bool gb_sal_status2_read_flag;

static uint32_t gn_sal_write_target_dev_id;
static uint32_t gn_sal_write_target_value;
static uint32_t gn_sal_write_target_cmd;
static bool gb_sal_write_flag;

static uint32_t gn_sal_write_SR_target_dev_id;
static uint32_t gn_sal_write_SR_target_value;
static uint32_t gn_sal_write_SR_target_cmd;
static bool gb_sal_write_SR_flag;

static uint8_t gn_sal_preamble = PREAMBLE;

bool gb_sal_sync_flag;
static bool gb_sal_rw_flag;

static uint8_t gn_sal_daisy_length;
static bool gb_sal_parsing_flag;
static uint8_t gn_sal_parsing_buffer_idx;

static uint8_t sal_calculate_crc(uint8_t* data, uint8_t size);
static void sal_manchester_encoding(uint64_t data, uint8_t* p, uint8_t size);

static void sal_reset(void);
static void sal_initiates(void);
static void sal_clear_error(void);

static void sal_spi_rx_timeout(void);

static void sal_set_dimming_buffer();

static void sal_reset(void)
{
    _sal_single_ended_info_t _sal_info_ = {0, };
    _sal_info_.dev_id = BROADCAST;
    _sal_info_.command = gt_sal_cmd_item[LIST_RESET].cmd;
    _sal_info_.data_size = gt_sal_cmd_item[LIST_RESET].request_bits;
    _sal_info_.data = gt_sal_cmd_data.ALL[LIST_RESET];
    print(LOG_LV_INFO, "%u : %s - 0x%03X\r\n", LIST_RESET, gt_sal_cmd_item[LIST_RESET].str, gt_sal_cmd_data.ALL[LIST_RESET]);

    sal_write_reg_single_ended(&_sal_info_);

    gb_sal_init_flag = false;

    sal_set_chain_length(0);
}

static void sal_initiates(void)
{
    _sal_single_ended_info_t _sal_info_ = {0, };
    _sal_info_.dev_id = 1;
    _sal_info_.command = gt_sal_cmd_item[LIST_INITBIDIR].cmd;
    _sal_info_.data_size = gt_sal_cmd_item[LIST_INITBIDIR].request_bits;
    _sal_info_.data = gt_sal_cmd_data.ALL[LIST_INITBIDIR];
    print(LOG_LV_INFO, "%u : %s - 0x%03X, size : %u\r\n", LIST_INITBIDIR, gt_sal_cmd_item[LIST_INITBIDIR].str, gt_sal_cmd_data.ALL[LIST_INITBIDIR], gt_sal_cmd_item[LIST_INITBIDIR].request_bits);

    sal_read_reg_single_ended(&_sal_info_);

    gb_sal_init_flag = true;
}

static void sal_clear_error(void)
{
    _sal_single_ended_info_t _sal_info_ = {0, };
    _sal_info_.dev_id = BROADCAST;
    _sal_info_.command = gt_sal_cmd_item[LIST_CLRERROR].cmd;
    _sal_info_.data_size = gt_sal_cmd_item[LIST_CLRERROR].request_bits;
    _sal_info_.data = gt_sal_cmd_data.ALL[LIST_CLRERROR];
    print(LOG_LV_INFO, "%u : %s - 0x%03X, size : %u\r\n", LIST_CLRERROR, gt_sal_cmd_item[LIST_CLRERROR].str, gt_sal_cmd_data.ALL[LIST_CLRERROR], gt_sal_cmd_item[LIST_CLRERROR].request_bits);

    sal_write_reg_single_ended(&_sal_info_);
}

static void sal_change_power_mode(uint8_t mode)
{
    SAL_CMD_LIST_T item = LIST_GOSLEEP;
    _sal_single_ended_info_t _sal_info_ = {0, };

    if(mode == 0)
    {
        item = LIST_GOSLEEP;
    }
    else
    {
        item = LIST_GOACTIVE;
    }

    _sal_info_.dev_id = BROADCAST;
    _sal_info_.command = (gt_sal_cmd_item[item].cmd);
    _sal_info_.data_size = gt_sal_cmd_item[item].request_bits;
    _sal_info_.data = gt_sal_cmd_data.ALL[item];
    print(LOG_LV_INFO, "%u : %s - 0x%03X\r\n", item, gt_sal_cmd_item[item].str, gt_sal_cmd_data.ALL[item]);

    sal_write_reg_single_ended(&_sal_info_);
}
#if 0
void sal_make_crc_enable(uint8_t crc_en)
{
    _sal_single_ended_info_t _sal_info_ = {0, };

    gt_sal_cmd_data.setup1.crc_e = crc_en;

    _sal_info_.dev_id = BROADCAST;
    _sal_info_.command = gt_sal_cmd_item[LIST_SET_SETUP1].cmd;
    _sal_info_.data_size = gt_sal_cmd_item[LIST_SET_SETUP1].request_bits;
    _sal_info_.data = gt_sal_cmd_data.ALL[LIST_SET_SETUP1];
    print(LOG_LV_INFO, "%u : %s - 0x%03X\r\n", LIST_SET_SETUP1, gt_sal_cmd_item[LIST_SET_SETUP1].str, gt_sal_cmd_data.ALL[LIST_SET_SETUP1]);

    sal_write_reg_single_ended(&_sal_info_);
}
#endif
void sal_make_mcu_crc(bool crc_en)
{
    if (crc_en == true)
    {
        print(LOG_LV_INFO, "MCU CRC Enable!!!\r\n");
        gb_sal_crc_en = true;
    }
    else
    {
        print(LOG_LV_INFO, "MCU CRC Disable!!!\r\n");
        gb_sal_crc_en = false;
    }
}

static void sal_regs_init(void)
{
    for (SAL_CMD_LIST_T list = LIST_SET_SETUP1 ; list <= LIST_SET_TEMP_LUT_TC10 ; ++list)
    {
        switch (list)
        {
#if 0
        case LIST_SET_SETUP1:
            gt_sal_cmd_data.setup1.com_flt_e = 1;
            break;
        case LIST_SET_SETUP2:
            gt_sal_cmd_data.setup2.val = 0;
            gt_sal_cmd_data.setup2.sh_lvl = 2;
            gt_sal_cmd_data.setup2.uv_lvl = 3;
            break;
        case LIST_SET_TEMPTH:
            break;
        case LIST_SET_TEMPHYS:
            break;
#endif
        case LIST_SET_CURR_MAX_LVL:
            gt_sal_cmd_data.curr_max_lvl.b_curr_max_lvl = SAL_R_MAX_CURR;
            gt_sal_cmd_data.curr_max_lvl.g_curr_max_lvl = SAL_G_MAX_CURR;
            gt_sal_cmd_data.curr_max_lvl.r_curr_max_lvl = SAL_B_MAX_CURR;
            break;
#if 0
        case LIST_SET_TEMP_LUT_TC1:
            gt_sal_cmd_data.temp_lut_tc1.temp_lut_tc1 = 0x200;
            break;
        case LIST_SET_TEMP_LUT_TC2:
            gt_sal_cmd_data.temp_lut_tc2.temp_lut_tc2 = 0x200;
            break;
        case LIST_SET_TEMP_LUT_TC3:
            gt_sal_cmd_data.temp_lut_tc3.temp_lut_tc3 = 0x21A;
            break;
        case LIST_SET_TEMP_LUT_TC4:
            gt_sal_cmd_data.temp_lut_tc4.temp_lut_tc4 = 0x231;
            break;
        case LIST_SET_TEMP_LUT_TC5:
            gt_sal_cmd_data.temp_lut_tc5.temp_lut_tc5 = 0x248;
            break;
        case LIST_SET_TEMP_LUT_TC6:
            gt_sal_cmd_data.temp_lut_tc6.temp_lut_tc6 = 0x25F;
            break;
        case LIST_SET_TEMP_LUT_TC7:
            gt_sal_cmd_data.temp_lut_tc7.temp_lut_tc7 = 0x276;
            break;
        case LIST_SET_TEMP_LUT_TC8:
            gt_sal_cmd_data.temp_lut_tc8.temp_lut_tc8 = 0x28D;
            break;
        case LIST_SET_TEMP_LUT_TC9:
            gt_sal_cmd_data.temp_lut_tc9.temp_lut_tc9 = 0x2A4;
            break;
        case LIST_SET_TEMP_LUT_TC10:
            gt_sal_cmd_data.temp_lut_tc10.temp_lut_tc10 = 0x2BB;
            break;
#endif
         default:
            continue;
        }

        if(gt_sal_cmd_item[list].broadcast == 1)
        {
            _sal_single_ended_info_t _sal_info_ = {0, };
            _sal_info_.dev_id = BROADCAST;
            _sal_info_.command = (gt_sal_cmd_item[list].cmd);
            _sal_info_.data_size = gt_sal_cmd_item[list].request_bits;
            _sal_info_.data = gt_sal_cmd_data.ALL[list];
            print(LOG_LV_INFO, "%u : %s - 0x%03X\r\n", list, gt_sal_cmd_item[list].str, gt_sal_cmd_data.ALL[list]);

            sal_write_reg_single_ended(&_sal_info_);
        }
    }
}

static bool gb_sal_manual_crc;
static uint8_t gn_sal_manual_crc;

void sal_set_crc_manual(uint8_t manual_crc_en, uint8_t manual_crc)
{
    if (manual_crc_en == 0)
    {
        gb_sal_manual_crc = false;
        gn_sal_manual_crc = 0;
    }
    else
    {
        gb_sal_manual_crc = true;
        gn_sal_manual_crc = manual_crc;
    }
}

void sal_set_preamble(uint8_t n_preamble)
{
    gn_sal_preamble = n_preamble;
}

uint8_t sal_get_preamble(void)
{
    return gn_sal_preamble;
}

static uint8_t sal_calculate_crc(uint8_t* data, uint8_t size)
{
    uint8_t crc = 0;
    uint8_t* p = data + size;

    while(size) // Skip calculating the last byte since the last byte is crc.
    {
        crc ^= *p;                  //xor current crc with value of pointer
        crc = gn_crc8_LUT[crc];     //take new crc from lookup table
        --p;                        //increase address of pointer
        --size;
    }
    //print(LOG_LV_DEBUG, "crc : 0x%02X \r\n", crc);

    if (gb_sal_manual_crc == true)
    {
        crc = gn_sal_manual_crc;
    }

    return crc;
}

static void sal_manchester_encoding(uint64_t data, uint8_t* p, uint8_t size)
{
    uint8_t encoder_temp[64] = {0, };
    uint8_t* q = encoder_temp;

    uint8_t spi_tx_len = size * 2; // After Manchester encoding, the data length doubled.
    uint16_t lut = 0;

    //print(LOG_LV_DEBUG, "data(include CRC) = 0x%016llX, bit_size = %u, byte_size = %u, spi_tx_len = %u \r\n", data, size, size, spi_tx_len);

    p += (size - 1);
    for(uint8_t counter = 0; counter < size; counter++)
    {
        lut = gn_manchester_LUT[*p];
        *q = (lut >> 8);
        ++q;
        *q = (lut >> 0);
        ++q;
        --p;
    }

#if 0
    print(LOG_LV_DEBUG, "man_encoded_data : ");
    for (uint8_t i = 0 ; i < spi_tx_len ; ++i)
    {
        print(LOG_LV_DEBUG, "0x%02X  ", encoder_temp[i]);
    }
    print(LOG_LV_DEBUG, "\r\n");
#endif

    encoder_temp[spi_tx_len] = 0xFF;
    spi_write(encoder_temp, spi_tx_len + 1);
}

void sal_init(void)
{
    uint8_t mosi_temp = 0xFF;

    //decode_mode_set((decode_mode_t)DECODE_TEST_V_TRIM);
    decode_mode_set((decode_mode_t)DECODE_NONE);

    sal_communication_mode_selection(SAL_COM_MCU, SAL_COM_EOL);

    sal_vcc_level_set(SAL_VCC_5V0);
    sal_vcc_en(PWR_ON);
    sal_vled_en(PWR_ON);
    LL_mDelay(10-1);

    sal_make_mcu_mode();

    LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
    spi_write(&mosi_temp, 1);

    sal_reset();
    us_delay(300);

    sal_clear_error();
    us_delay(300);

    sal_initiates();
    us_delay(300);

    sal_change_power_mode(1);
    us_delay(300);

    sal_regs_init();

    uint32_t* p_buffer = sal_get_rgb_buffer();
    for (uint8_t idx = 0 ; idx < 100 ; ++idx)
    {
        *(p_buffer + idx) = ((10 << 16) | (10 << 8) | (10 << 0));
    }

    gn_sal_daisy_length = sal_get_chain_length();

    gb_demo_start_flag = true;
}

static void sal_spi_rx_timeout(void)
{
    print(LOG_LV_ERROR, "\033[0;31m === spi rx timeout === \033[0m\r\n");

    LL_DMA_DisableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_SPI_Disable(SPI1);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, 0);

    LL_DMA_ClearFlag_TC0(DMA2);

    gb_spi_rx_started[0] = false;

    gb_sal_rx_timeout = true;
}

void sal_write_reg_single_ended(_sal_single_ended_info_t *p_info)
{
    _data_frame_format_t _bit_format_ = {0, };
    uint8_t spi_tx_byte_size = 0;

    if(LL_SPI_GetMode(SPI1) != LL_SPI_MODE_MASTER)
    {
        LL_SPI_Disable(SPI1);
        LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
        LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
        LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
        LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
        LL_SPI_Enable(SPI1);
    }

    switch (p_info->data_size)
    {
    case 0:
        spi_tx_byte_size = 3;
        _bit_format_.bit0.preamble = gn_sal_preamble;
        _bit_format_.bit0.dev_addr = p_info->dev_id;
        _bit_format_.bit0.command = p_info->command;
        _bit_format_.bit0.crc = sal_calculate_crc(_bit_format_.buffer, spi_tx_byte_size);
        break;
    case 12:
        spi_tx_byte_size = 5;
        _bit_format_.bit12.preamble = gn_sal_preamble;
        _bit_format_.bit12.dev_addr = p_info->dev_id;
        _bit_format_.bit12.command = p_info->command;
        _bit_format_.bit12.data = p_info->data;
        _bit_format_.bit12.dummy = DUMMY_BIT;
        _bit_format_.bit12.crc = sal_calculate_crc(_bit_format_.buffer, spi_tx_byte_size);
        break;
    case 24:
        spi_tx_byte_size = 6;
        _bit_format_.bit24.preamble = gn_sal_preamble;
        _bit_format_.bit24.dev_addr = p_info->dev_id;
        _bit_format_.bit24.command = p_info->command;
        _bit_format_.bit24.data = p_info->data;
        _bit_format_.bit24.crc = sal_calculate_crc(_bit_format_.buffer, spi_tx_byte_size);
        break;
    }

    // make manchester and spi tx start!!
    if (gb_sal_crc_en)
    {
        //print(LOG_LV_INFO, "tx : preamble - 0x%02X, addr - %3u, cmd - 0x%02X, data - 0x%06X, crc - 0x%02X\r\n", gn_sal_preamble, p_info->dev_id, p_info->command, p_info->data, _bit_format_.buffer[0]);
        ++spi_tx_byte_size;
        sal_manchester_encoding(_bit_format_.val, _bit_format_.buffer, spi_tx_byte_size);
    }
    else
    {
        //print(LOG_LV_INFO, "tx : preamble - 0x%02X, addr - %3u, cmd - 0x%02X, data - 0x%06X\r\n", gn_sal_preamble, p_info->dev_id, p_info->command, p_info->data);
        _bit_format_.val = _bit_format_.val >> 8;
        sal_manchester_encoding(_bit_format_.val, _bit_format_.buffer, spi_tx_byte_size);
    }
}

uint32_t sal_read_reg_single_ended(_sal_single_ended_info_t *p_info)
{
    uint32_t ret = 0;
    uint8_t spi_rx_len = 0;
    uint8_t spi_parsing_length = 1;

    _sal_single_ended_info_t write_p_info = {0, };

    write_p_info.dev_id = p_info->dev_id;
    write_p_info.command = p_info->command;

    LED_HI();
    sal_write_reg_single_ended(&write_p_info);

    while(gb_spi_tx_started[0]) {}
    LED_LO();

    if(LL_SPI_GetMode(SPI1) != LL_SPI_MODE_SLAVE)
    {
        LL_SPI_Disable(SPI1);
        LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
        LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
        LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
        LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
        LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
        LL_SPI_Enable(SPI1);
    }

    while(LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        volatile uint8_t dummy = LL_SPI_ReceiveData8(SPI1);
    }

    switch (p_info->data_size)
    {
    case 0:
        spi_rx_len = 3;
        break;
    case 12:
        spi_rx_len = 5;
        break;
    case 24:
        spi_rx_len = 6;
        break;
    }

    if (gb_sal_crc_en)
    {
        spi_rx_len += CRC_BYTE;
    }

    if (gb_sal_init_flag == true)
    {
        if ((p_info->dev_id == BROADCAST) && (p_info->command != CMD_SAL_INITBIDIR))
        {
            spi_parsing_length = gn_sal_daisy_length;
        }
    }

    gb_sal_rx_timeout = false;

    for (uint8_t len = 0 ; len < spi_parsing_length ; ++len)
    {
        gn_sal_rx_timeout = 100;
        LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)(gn_spi_rx_buff[0] + spi_rx_len * len));

        for (volatile uint16_t i = 0 ; i < 75 ; ++i)
        {
            __NOP();
        }

        LL_SPI_Enable(SPI1);
        spi_read(spi_rx_len);
        LED_HI();

        while(gb_spi_rx_started[0])
        {
            if (!gn_sal_rx_timeout)
            {
                sal_spi_rx_timeout();
                break;
            }
        }
        LL_SPI_Disable(SPI1);

        if (gb_sal_rx_timeout == true)
        {
            break;
        }
        LED_LO();
    }

    if (gb_sal_rx_timeout == false)
    {
        for (uint8_t len = 0 ; len < spi_parsing_length ; ++len)
        {
            ret = sal_rx_parser((gn_spi_rx_buff[0] + (spi_rx_len * len)), spi_rx_len, 0);
        }
    }

    return ret;
}

uint8_t sal_write_SR_reg_single_ended(_sal_single_ended_info_t *p_info)
{
    uint8_t ret = 0;
    uint8_t spi_rx_len = 0;
    uint8_t spi_parsing_length = 1;

    LED_HI();
    sal_write_reg_single_ended(p_info);

    while(gb_spi_tx_started[0]) {}
    LED_LO();

    if(LL_SPI_GetMode(SPI1) != LL_SPI_MODE_SLAVE)
    {
        LL_SPI_Disable(SPI1);
        LL_SPI_SetMode(SPI1, LL_SPI_MODE_SLAVE);
        LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
        LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);
        LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
        LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
        LL_SPI_Enable(SPI1);
    }

    while(LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
        volatile uint8_t dummy = LL_SPI_ReceiveData8(SPI1);
    }

    if (gb_sal_crc_en)
    {
        spi_rx_len = 6 + CRC_BYTE;
    }
    else
    {
        spi_rx_len = 6;
    }

    if (gb_sal_init_flag == true)
    {
        if (p_info->dev_id == BROADCAST)
        {
            spi_parsing_length = gn_sal_daisy_length;
        }
    }

    gb_sal_rx_timeout = false;

    for (uint8_t len = 0 ; len < spi_parsing_length ; ++len)
    {
        gn_sal_rx_timeout = 100;
        LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)(gn_spi_rx_buff[0] + spi_rx_len * len));

        for (volatile uint16_t i = 0 ; i < 75 ; ++i)
        {
            __NOP();
        }
        LL_SPI_Enable(SPI1);
        spi_read(spi_rx_len);
        LED_HI();

        while(gb_spi_rx_started[0])
        {
            if (!gn_sal_rx_timeout)
            {
                sal_spi_rx_timeout();
                break;
            }
        }
        LL_SPI_Disable(SPI1);

        if (gb_sal_rx_timeout == true)
        {
            break;
        }
    }
    LED_LO();

    if (gb_sal_rx_timeout == false)
    {
        for (uint8_t len = 0 ; len < spi_parsing_length ; ++len)
        {
            sal_rx_parser((gn_spi_rx_buff[0] + (spi_rx_len * len)), spi_rx_len, 0);
        }
    }
    return ret;
}

void sal_set_write_target(uint32_t dev_id, uint32_t cmd, uint32_t value)
{
    gn_sal_write_target_dev_id = dev_id;
    gn_sal_write_target_value = value;
    gn_sal_write_target_cmd = cmd;
    gb_sal_write_flag = true;
}

static void sal_write_register(void)
{
    if (gb_sal_write_flag)
    {
        _sal_single_ended_info_t _info_ = {0, };
        _info_.dev_id = gn_sal_write_target_dev_id;
        _info_.command = gn_sal_write_target_cmd;
        _info_.data = gn_sal_write_target_value;

        if (gn_sal_write_target_cmd == CMD_SAL_RESET || gn_sal_write_target_cmd == CMD_SAL_INITBIDIR || gn_sal_write_target_cmd == CMD_SAL_CLRERROR || gn_sal_write_target_cmd == CMD_SAL_GOSLEEP || gn_sal_write_target_cmd == CMD_SAL_GOACTIVE || gn_sal_write_target_cmd == CMD_SAL_GODEEPSLEEP)
        {
            _info_.data_size = 0;
        }
        else if (gn_sal_write_target_cmd == CMD_SAL_SET_MCAST || gn_sal_write_target_cmd == CMD_SAL_SET_RGB)
        {
            _info_.data_size = 24;
        }
        else
        {
            _info_.data_size = 12;
        }

        sal_write_reg_single_ended(&_info_);
        gb_sal_write_flag = false;
    }
}

void sal_set_read_target(uint32_t dev_id, uint32_t cmd)
{
    gn_sal_read_target_dev_id = dev_id;
    gn_sal_read_target_cmd = cmd;
    gb_sal_read_flag = true;
}

static void sal_read_register(void)
{
    if (gb_sal_read_flag)
    {
        _sal_single_ended_info_t _info_ = {0, };
        _info_.dev_id = gn_sal_read_target_dev_id;
        _info_.command = gn_sal_read_target_cmd;
        _info_.data = 0;

        if (gn_sal_read_target_cmd == CMD_SAL_READ_TEMPST || gn_sal_read_target_cmd == CMD_SAL_READ_MCAST || gn_sal_read_target_cmd == CMD_SAL_READ_RGB)
        {
            _info_.data_size = 24;
        }
        else
        {
            _info_.data_size = 12;
        }

        sal_read_reg_single_ended(&_info_);
        gb_sal_read_flag = false;
    }
}

void sal_set_status2_read_enable(void)
{
    gb_sal_status2_read_flag = true;
}

static void sal_read_status2_register(void)
{
    static uint16_t prev_status2 = 0;
    uint16_t now_status2 = 0;

    if (gb_sal_status2_read_flag)
    {
        _sal_single_ended_info_t _info_ = {0, };
        _info_.dev_id = 1;
        _info_.command = 0x42;
        _info_.data = 0;
        _info_.data_size = 12;

        now_status2 = sal_read_reg_single_ended(&_info_);

        if (prev_status2 != now_status2)
        {
            _sal_status2_t sal_status2 = {0, };
            sal_status2.val = now_status2;
            print(LOG_LV_INFO, "OTP_CRC [%u] T_OUT [%u] \r\nOPEN_R [%u] OPEN_G [%u] OPEN_B [%u] \r\nSHORT_R [%u] SHORT_G [%u] SHORT_B [%u] \r\nOT_FLT [%u] UV_FLT [%u] CRC_FLT [%u] COM_FLT [%u]\r\n", \
            sal_status2.otpcrc, sal_status2.t_out, sal_status2.open_r, sal_status2.open_g, sal_status2.open_b, sal_status2.short_r, sal_status2.short_g, sal_status2.short_b, \
            sal_status2.ot_flt, sal_status2.uv_flt, sal_status2.crc_flt, sal_status2.com_flt);
            prev_status2 = now_status2;
        }
        gb_sal_status2_read_flag = false;
    }
}

void sal_set_write_SR_target(uint32_t dev_id, uint32_t cmd, uint32_t value)
{
    gn_sal_write_SR_target_dev_id = dev_id;
    gn_sal_write_SR_target_value = value;
    gn_sal_write_SR_target_cmd = cmd;
    gb_sal_write_SR_flag = true;
}

static void sal_write_SR_register(void)
{
    if (gb_sal_write_SR_flag)
    {
        _sal_single_ended_info_t _info_ = {0, };
        _info_.dev_id = gn_sal_write_SR_target_dev_id;
        _info_.command = gn_sal_write_SR_target_cmd;
        _info_.data = gn_sal_write_SR_target_value;

        if (gn_sal_write_SR_target_cmd == 0xF3 || gn_sal_write_SR_target_cmd == 0xF4 || gn_sal_write_SR_target_cmd == 0xF8 || gn_sal_write_SR_target_cmd == 0xFC)
        {
            _info_.data_size = 0;
        }
        else if (gn_sal_write_SR_target_cmd == 0xC8 || gn_sal_write_SR_target_cmd == 0xE0)
        {
            _info_.data_size = 24;
        }
        else
        {
            _info_.data_size = 12;
        }

        sal_write_SR_reg_single_ended(&_info_);
        gb_sal_write_SR_flag = false;
    }
}

uint32_t* sal_get_rgb_buffer(void)
{
    return gn_sal_rgb_buffer;
}

static void sal_parsing_rgb_data(void)
{
    _sal_single_ended_info_t _sal_info_ = {0, };

    if (gn_sal_rgb_saved_buffer[gn_sal_parsing_buffer_idx] != gn_sal_rgb_buffer[gn_sal_parsing_buffer_idx])
    {
        _sal_info_.dev_id = gn_sal_id_map[gn_sal_parsing_buffer_idx];
        _sal_info_.command = gt_sal_cmd_item[LIST_SET_RGB].cmd;
        _sal_info_.data_size = gt_sal_cmd_item[LIST_SET_RGB].request_bits;
        _sal_info_.data = *(gn_sal_rgb_buffer + gn_sal_parsing_buffer_idx);
        sal_write_reg_single_ended(&_sal_info_);
        gn_sal_rgb_saved_buffer[gn_sal_parsing_buffer_idx] = gn_sal_rgb_buffer[gn_sal_parsing_buffer_idx];
    }

    ++gn_sal_parsing_buffer_idx;
    if (gn_sal_parsing_buffer_idx == (gn_sal_daisy_length - 1)) //except SAL with device ID 1 (in control B'D)
    {
        gn_sal_parsing_buffer_idx = 0;
        gb_sal_parsing_flag = false;
        gb_sal_rw_flag = true;
        LED_LO();
    }
}

void sal_set_pattern(demo_pat_t n_demo_pat)
{
    gt_demo_pat = n_demo_pat;
}

static void sal_set_dimming_buffer()
{
    uint32_t* p_buffer = sal_get_rgb_buffer();
    switch (gt_demo_pat)
    {
    case DEMO_PAT_NONE:
        break;
    case DEMO_PAT_RED :
        for (uint8_t idx = 0 ; idx < 100 ; ++idx)
        {
            *(p_buffer + idx) = ((gn_sal_echo_demo_table[0][0] << 16) | (gn_sal_echo_demo_table[0][1] << 8) | (gn_sal_echo_demo_table[0][2] << 0));
        }
        break;
    case DEMO_PAT_GREEN :
        for (uint8_t idx = 0 ; idx < 100 ; ++idx)
        {
            *(p_buffer + idx) = ((gn_sal_echo_demo_table[1][0] << 16) | (gn_sal_echo_demo_table[1][1] << 8) | (gn_sal_echo_demo_table[1][2] << 0));
        }
        break;
    case DEMO_PAT_BLUE :
        for (uint8_t idx = 0 ; idx < 100 ; ++idx)
        {
            *(p_buffer + idx) = ((gn_sal_echo_demo_table[2][0] << 16) | (gn_sal_echo_demo_table[2][1] << 8) | (gn_sal_echo_demo_table[2][2] << 0));
        }
        break;
    case DEMO_PAT_WHITE :
        for (uint8_t idx = 0 ; idx < 100 ; ++idx)
        {
            *(p_buffer + idx) = ((gn_sal_echo_demo_table[3][0] << 16) | (gn_sal_echo_demo_table[3][1] << 8) | (gn_sal_echo_demo_table[3][2] << 0));
        }
        break;
    case DEMO_PAT_HMC61 :
        for (uint8_t idx = 0 ; idx < 100 ; ++idx)
        {
            *(p_buffer + idx) = ((gn_sal_echo_demo_table[4][0] << 16) | (gn_sal_echo_demo_table[4][1] << 8) | (gn_sal_echo_demo_table[4][2] << 0));
        }
        break;
    }
}

void sal_demo_process(void)
{
    if (gb_sal_sync_flag == true)
    {
        LED_HI();
        sal_set_dimming_buffer();
        gb_sal_sync_flag = false;
        gb_sal_parsing_flag = true;
    }

    if (gb_sal_parsing_flag == true)
    {
        sal_parsing_rgb_data();
    }

    if (gb_sal_rw_flag == true)
    {
        sal_write_register();
        sal_write_SR_register();
        sal_read_register();
        sal_read_status2_register();

        gb_sal_rw_flag = false;
    }
}
/*
static uint16_t gn_sal_crc_test_fail_count = 0;
void sal_crc_repeat_test(void)
{
    for (uint16_t i = 0 ; i < 1000 ; ++i)
    {
        sal_reset();
        LL_mDelay(10 - 1);
        sal_make_crc_enable(1);
        LL_mDelay(10 - 1);
        sal_initiates();
        LL_mDelay(10 - 1);

        if (gb_sal_rx_timeout)
        {
            ++gn_sal_crc_test_fail_count;
            //print(LOG_LV_ERROR, "CRC test failed!! \r\n");
        }
        else
        {
            //print(LOG_LV_ERROR, "CRC test success!! \r\n");
        }
    }
    print(LOG_LV_INFO, "CRC test failed count : %u \r\n", gn_sal_crc_test_fail_count);
}
*/
/* END - INTERFACE FUNCTIONS ************************************************************************/

/*** end of file ***/