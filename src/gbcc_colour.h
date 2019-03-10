#ifndef GBCC_COLOUR_H
#define GBCC_COLOUR_H

#include <stdint.h>

void gbcc_fill_lut(uint8_t *lut);
uint32_t gbcc_lerp_colour(uint8_t r, uint8_t g, uint8_t b);
uint32_t gbcc_colour_correct(uint32_t hex);
uint32_t gbcc_add_colours(uint32_t c1, uint32_t c2, float t);

#endif /* GBCC_COLOUR_H */
