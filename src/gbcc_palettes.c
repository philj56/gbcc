#include "gbcc_colour.h"
#include "gbcc_debug.h"
#include "gbcc_palettes.h"
#include <strings.h>

#define NUM_PALETTES 13

static struct palette palette_colour_correct(const struct palette *pal);

static const struct palette palettes[NUM_PALETTES] = {
	{ /* Default */
		.name = "Default",
		.background = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u},
		.sprite1    = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u},
		.sprite2    = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u}
	},
	{ /* brown: up */
		.name = "Brown",
		.background = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	{ /* red: up a */
		.name = "Red",
		.background = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite1    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u},
		.sprite2    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u}
	},
	{ /* darkbrown: up b */
		.name = "DarkBrown",
		.background = {0xffe7c5u, 0xce9c85u, 0x846b29u, 0x5b3109u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	{ /* pastel: down */
		.name = "Pastel",
		.background = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite1    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite2    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u}
	},
	{ /* orange: down a */
		.name = "Orange",
		.background = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u}
	},
	{ /* yellow: down b */
		.name = "Yellow",
		.background = {0xffffffu, 0xffff00u, 0x7d4900u, 0x000000u},
		.sprite1    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
	{ /* blue: left */
		.name = "Blue",
		.background = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
	{ /* darkblue: left a */
		.name = "DarkBlue",
		.background = {0xffffffu, 0x8b8cdeu, 0x53528cu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	{ /* grey: left b */
		.name = "Grey",
		.background = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite1    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite2    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u}
	},
	{ /* green: right */
		.name = "Green",
		.background = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite1    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite2    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u}
	},
	{ /* darkgreen: right a */
		.name = "DarkGreen",
		.background = {0xffffffu, 0x7bff30u, 0x0163c6u, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u}
	},
	{ /* invert: right b */
		.name = "Invert",
		.background = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite1    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite2    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu}
	}
};

struct palette gbcc_get_palette(const char *name)
{
	unsigned int p;
	for (p = 0; p < NUM_PALETTES; p++) {
		if (strcasecmp(name, palettes[p].name) == 0) {
			break;
		}
	}
	if (p >= NUM_PALETTES) {
		gbcc_log_error("Invalid palette \"%s\"\n", name);
		exit(EXIT_FAILURE);
	}
	if (p != 0) {
		return palette_colour_correct(&palettes[p]);
	}
	return palettes[p];
}

struct palette palette_colour_correct(const struct palette *palette)
{
	struct palette newpal = {
		.name = palette->name,
		.background = {
			gbcc_colour_correct(palette->background[0]),
			gbcc_colour_correct(palette->background[1]),
			gbcc_colour_correct(palette->background[2]),
			gbcc_colour_correct(palette->background[3])
		},
		.sprite1 = {
			gbcc_colour_correct(palette->sprite1[0]),
			gbcc_colour_correct(palette->sprite1[1]),
			gbcc_colour_correct(palette->sprite1[2]),
			gbcc_colour_correct(palette->sprite1[3])
		},
		.sprite2 = {
			gbcc_colour_correct(palette->sprite2[0]),
			gbcc_colour_correct(palette->sprite2[1]),
			gbcc_colour_correct(palette->sprite2[2]),
			gbcc_colour_correct(palette->sprite2[3])
		}
	};
	return newpal;
}
