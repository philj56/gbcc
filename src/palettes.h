/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_PALETTE_H
#define GBCC_PALETTE_H

#include <stdbool.h>
#include <stdint.h>

#define GBCC_NUM_PALETTES 14

struct palette {
	const char *name;
	uint32_t background[4];
	uint32_t sprite1[4];
	uint32_t sprite2[4];
};

extern const struct palette gbcc_palettes[GBCC_NUM_PALETTES];

struct palette gbcc_get_palette(const char *name);

#endif /* GBCC_PALETTE_H */
