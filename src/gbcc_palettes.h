#ifndef GBCC_PALETTE_H
#define GBCC_PALETTE_H

#include <stdint.h>
#include <stdlib.h>

struct palette {
       uint32_t background[4];
       uint32_t sprite1[4];
       uint32_t sprite2[4];
};

extern const char* gbcc_palette_names[13]; 
extern const struct palette gbcc_palettes[13];

struct palette gbcc_palette_correct(const struct palette *pal);

#endif /* GBCC_PALETTE_H */
