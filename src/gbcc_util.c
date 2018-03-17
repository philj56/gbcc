#include "gbcc_util.h"

uint16_t high_byte(uint16_t x)
{
	return (x & 0xFF00u) >> 8;
}

uint16_t low_byte(uint16_t x)
{
	return x & 0x00FFu;
}
