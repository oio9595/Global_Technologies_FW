/** @file MCP4251.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */#ifndef MCP4251_H_
#define MCP4251_H_

#include "main.h"

#define MCP_RES     (256)       // MAX 0x100
#define MCP_R_W     (75.0f)     // ohm
#define MCP_R_AB    (10000.0f)  // ohm

#define MCP_R_REF   (20000.0f)  // ohm
#define MCP_V_REF   (5.0f)      // volt

typedef enum
{
    MCP_CMD_WRITE = 0,
    MCP_CMD_INCR,
    MCP_CMD_DECR,
    MCP_CMD_READ,
}MCP_CMD_CODE;

typedef enum
{
    MCP_WIPER_PORT_0 = 0,
    MCP_WIPER_PORT_1,
    MCP_WIPER_PORT_MAX,
}MCP_WIPER_PORT_T;

typedef enum
{
    MCP_ADDR_WIPER_0 = 0,
    MCP_ADDR_WIPER_1,
    MCP_ADDR_V_WIPER_0,
    MCP_ADDR_V_WIPER_1,
    MCP_ADDR_V_TCON_REG,
    MCP_ADDR_STATUS_REG,
}MCP_REG_ADDR;

typedef union
{
    uint8_t val;
    struct
    {
        uint8_t data : 2;
        uint8_t cmd  : 2;
        uint8_t addr : 4;
    };
}mcp_8_bit_cmd_t;

typedef union
{
    uint16_t val_16;
    uint8_t val[2];
    struct
    {
        uint8_t data_msb : 2;
        uint8_t cmd      : 2;
        uint8_t addr     : 4;
        uint8_t data_lsb : 8;
    };
}mcp_16_bit_cmd_t;

EXTERN void mcp_wiper_level_incr(MCP_WIPER_PORT_T n_MCP_WIPER_PORT);
EXTERN void mcp_wiper_level_decr(MCP_WIPER_PORT_T n_MCP_WIPER_PORT);

EXTERN void mcp_wiper_init(void);
EXTERN float mcp_set_wiper_resist_value(MCP_WIPER_PORT_T n_MCP_WIPER_PORT, uint16_t n_data);

EXTERN void mcp_debug_show_vled(void);


#endif /* MCP4251_H_ */
