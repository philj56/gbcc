#include "gbcc_bit_utils.h"

uint8_t high_byte(uint16_t x)
{
	return (x & 0xFF00u) >> 8u;
}

uint8_t low_byte(uint16_t x)
{
	return x & 0x00FFu;
}

uint16_t cat_bytes(uint8_t low, uint8_t high)
{
	return (uint16_t)low | (uint16_t)(high << 8u);
}

uint8_t set_bit(uint8_t byte, uint8_t b)
{
	return byte | bit(b);
}

uint8_t clear_bit(uint8_t byte, uint8_t b)
{
	return byte & (uint8_t)~bit(b);
}

uint8_t toggle_bit(uint8_t byte, uint8_t b)
{
	return byte ^ bit(b);
}

bool check_bit(uint8_t byte, uint8_t b)
{
	return byte & bit(b);
}

uint8_t bit(uint8_t b)
{
	return (1u << b);
}
