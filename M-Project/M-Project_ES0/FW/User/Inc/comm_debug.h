#ifndef __COMM_DEBUG_H__
#define __COMM_DEBUG_H__

#include "config.h"

#define RX_BUFF_SIZE        8
#define TX_BUFF_SIZE        64

#define RX_PACKET_SIZE      32
#define TX_PACKET_SIZE      240

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
extern char gn_tx_buff[TX_BUFF_SIZE];

typedef struct
{
    uint16_t length;
    char buffer[RX_PACKET_SIZE];
}
rx_packet_t;

typedef struct
{
    uint16_t RxInCnt;
    uint16_t RxOutCnt;
    rx_packet_t Rxbuff[RX_BUFF_SIZE];
}RX_UART_t;

void print(const char *fmt, ...);
void comm_debugging_process(void);
static uint8_t comm_get_rx_packet(rx_packet_t** pData);
static void comm_rx_handler(uint8_t rx_data);
static void uart_echo(char data);







#endif /* end of __COMM_DEBUG_H__ */
