#ifndef __COMM_DEBUG_H__
#define __COMM_DEBUG_H__

#include "main.h"

#define UART_PACKET_SIZE    256
#define UART_BUFF_SIZE      32

#define CLI_KEY_BACK        0x08
#define CLI_KEY_DEL         0x7F
#define CLI_KEY_ENTER       0x0D
#define CLI_KEY_ESC         0x1B
#define CLI_KEY_LEFT        0x44
#define CLI_KEY_RIGHT       0x43
#define CLI_KEY_UP          0x41
#define CLI_KEY_DOWN        0x42
#define CLI_KEY_HOME        0x31
#define CLI_KEY_END         0x34

extern uint8_t gn_rx_temp;

typedef struct
{
    uint16_t length;
    char buffer[UART_PACKET_SIZE];
}_packet_t;

typedef struct
{
    uint16_t InCnt;
    uint16_t OutCnt;
    _packet_t buff[UART_BUFF_SIZE];
}_uart_ring_buffer_t_;

void print(const char *fmt, ...);
void comm_debugging_process(void);
void comm_dma_rx_handler(void);

void comm_init(void);

static void comm_get_tx_packet(void);
static uint8_t comm_get_rx_packet(_packet_t** pData);
static void comm_rx_handler(uint8_t rx_data);
static void uart_echo(char data);


#endif /* end of __COMM_DEBUG_H__ */
