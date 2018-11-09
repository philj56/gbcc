#include "gbcc_colour.h"
#include "gbcc_palettes.h"

const char* gbcc_palette_names[13] = { 
    "classic",
    "brown",
    "red",
    "darkbrown",
    "pastel",
    "orange",
    "yellow",
    "blue",
    "darkblue",
    "grey",
    "green",
    "darkgreen",
    "invert"
};

const struct palette gbcc_palettes[13] = {
    { /* classic */
		.background = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u},
		.sprite1    = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u},
		.sprite2    = {0xc4cfa1u, 0x8b956du, 0x6b7353u, 0x000000u}
	},
    { /* brown: up */
		.background = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
    { /* red: up a */
		.background = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite1    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u},
		.sprite2    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u}
	},
    { /* darkbrown: up b */
		.background = {0xffe7c5u, 0xce9c85u, 0x846b29u, 0x5b3109u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
    { /* pastel: down */
		.background = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite1    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite2    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u}
	},
    { /* orange: down a */
		.background = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u}
	},
    { /* yellow: down b */
		.background = {0xffffffu, 0xffff00u, 0x7d4900u, 0x000000u},
		.sprite1    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
    { /* blue: left */
		.background = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
    { /* darkblue: left a */
		.background = {0xffffffu, 0x8b8cdeu, 0x53528cu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
    { /* grey: left b */
		.background = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite1    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite2    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u}
	},
    { /* green: right */
		.background = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite1    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite2    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u}
	},
    { /* darkgreen: right a */
		.background = {0xffffffu, 0x7bff30u, 0x0163c6u, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u}
	},
    { /* invert: right b */
		.background = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite1    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite2    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu}
	}
};

struct palette gbcc_palette_correct(const struct palette *pal)
{
	struct palette newpal = {
		.background = {
			gbcc_colour_correct(pal->background[0]),
			gbcc_colour_correct(pal->background[1]),
			gbcc_colour_correct(pal->background[2]),
			gbcc_colour_correct(pal->background[3])
		},
		.sprite1 = {
			gbcc_colour_correct(pal->sprite1[0]),
			gbcc_colour_correct(pal->sprite1[1]),
			gbcc_colour_correct(pal->sprite1[2]),
			gbcc_colour_correct(pal->sprite1[3])
		},
		.sprite2 = {
			gbcc_colour_correct(pal->sprite2[0]),
			gbcc_colour_correct(pal->sprite2[1]),
			gbcc_colour_correct(pal->sprite2[2]),
			gbcc_colour_correct(pal->sprite2[3])
		}
	};
	return newpal;
}
