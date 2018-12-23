#ifndef GBCC_PALETTE_H
#define GBCC_PALETTE_H

#include <stdint.h>

struct palette {
	const char *name;
	uint32_t background[4];
	uint32_t sprite1[4];
	uint32_t sprite2[4];
};

struct palette gbcc_get_palette(const char *name);

#endif /* GBCC_PALETTE_H */
