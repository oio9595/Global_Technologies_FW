#include <stdint.h>
#include <stdio.h>

#define CRC8_POLY   0x1D
#define CRC8_INIT   0xFF
#define CRC8_XOROUT 0xFF




uint8_t crc8_sae_j1850_22bit(uint32_t data)
{
    uint8_t crc = CRC8_INIT;

    // 22-bit data
    data &= 0x3FFFFF;

    // MSB-first
    // I_DATA[21] -> I_DATA[0]
    for (int i = 21; i >= 0; i--)
    {
        uint8_t data_bit = (data >> i) & 0x01;
        uint8_t feedback = ((crc >> 7) & 0x01) ^ data_bit;

        crc <<= 1;

        if (feedback)
            crc ^= CRC8_POLY;
    }

    return crc ^ CRC8_XOROUT;
}

uint8_t crc8_sae_j1850_10bit(uint16_t data)
{
    uint8_t crc = 0xFF;

    // 10-bit data, MSB-first
    for (int i = 9; i >= 0; i--)
    {
        uint8_t data_bit = (data >> i) & 0x01;
        uint8_t crc_msb = (crc >> 7) & 0x01;

        crc <<= 1;

        if (crc_msb ^ data_bit)
        {
            crc ^= 0x1D;
        }
    }

    // Final XOR
    crc ^= 0xFF;

    return crc;
}



int main(void)
{
    uint16_t data = 0x101;
    uint8_t crc_10 = crc8_sae_j1850_10bit(data);

    printf("DATA(10 bits) = 0x%03X\n", data);
    printf("CRC  = 0x%02X\n\n", crc_10);

    uint32_t payload = 0xFF;
    uint8_t crc_22 = crc8_sae_j1850_22bit(payload);
    printf("Payload(22 bits) : 0x%06X\n", payload);
    printf("CRC     : 0x%02X\n", crc_22);

    return 0;
}