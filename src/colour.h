#ifndef GBCC_COLOUR_H
#define GBCC_COLOUR_H

#include <stdint.h>

void gbcc_fill_lut(uint32_t *lut);
uint32_t gbcc_add_colours(uint32_t c1, uint32_t c2, float t);

#endif /* GBCC_COLOUR_H */
