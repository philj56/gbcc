#ifndef GBCC_BIT_UTILS_H
#define GBCC_BIT_UTILS_H

#include <stdbool.h>
#include <stdint.h>

uint8_t high_byte(uint16_t x);
uint8_t low_byte(uint16_t x);

uint8_t set_bit(uint8_t byte, uint8_t bit);
uint8_t clear_bit(uint8_t byte, uint8_t bit);
uint8_t toggle_bit(uint8_t byte, uint8_t bit);
bool check_bit(uint8_t byte, uint8_t bit);

#endif /* GBCC_BIT_UTILS_H */
