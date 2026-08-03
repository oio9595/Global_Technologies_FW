#include "framework.h"

void fw_memset(void* ptr, uint8_t val, uint32_t size)
{
    uint8_t *p = (uint8_t*)ptr;

    for(uint32_t i=0U; i<size; ++i)
    {
        p[i] = val;
    }
}

