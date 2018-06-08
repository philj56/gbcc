#include "gbcc_bit_utils.h"

uint8_t high_byte(uint16_t x)
{
	return (x & 0xFF00u) >> 8u;
}

uint8_t low_byte(uint16_t x)
{
	return x & 0x00FFu;
}

uint8_t set_bit(uint8_t byte, uint8_t bit)
{
	return byte | (uint8_t)(1u << bit);
}

uint8_t clear_bit(uint8_t byte, uint8_t bit)
{
	return byte & ~(1u << bit);
}

uint8_t toggle_bit(uint8_t byte, uint8_t bit)
{
	return byte ^ (uint8_t)(1u << bit);
}

bool check_bit(uint8_t byte, uint8_t bit)
{
	return byte & (1u << bit);
}
