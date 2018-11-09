#ifndef GBCC_COLOUR_H
#define GBCC_COLOUR_H

#include <stdint.h>

uint32_t gbcc_lerp_colour(uint8_t r, uint8_t g, uint8_t b);
uint32_t gbcc_colour_correct(uint32_t hex);

#endif /* GBCC_COLOUR_H */
