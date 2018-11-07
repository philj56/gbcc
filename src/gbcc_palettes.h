#ifndef GBCC_PALETTE_H
#define GBCC_PALETTE_H

#include <stdint.h>

struct palette {
       uint32_t background[4];
       uint32_t sprite1[4];
       uint32_t sprite2[4];
};

struct palettes {
	struct palette brown; //up
	struct palette red; //up a
	struct palette darkbrown; //up b
	struct palette pastel; //down
	struct palette orange; //down a
	struct palette yellow; //down b
	struct palette blue; //left
	struct palette darkblue; //left a
	struct palette grey; //left b
	struct palette green; //right
	struct palette darkgreen; //right a
	struct palette invert; //right b
	struct palette zelda;
};

static const struct palettes palettes = {
	.brown = {
		.background = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	.red = {
		.background = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite1    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u},
		.sprite2    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u}
	},
	.darkbrown = {
		.background = {0xffe7c5u, 0xce9c85u, 0x846b29u, 0x5b3109u},
		.sprite1    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	.pastel = {
		.background = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite1    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u},
		.sprite2    = {0xffffa5u, 0xfe9494u, 0x9394feu, 0x000000u}
	},
	.orange = {
		.background = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite1    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u},
		.sprite2    = {0xffffffu, 0xffff00u, 0xfe0000u, 0x000000u}
	},
	.yellow = {
		.background = {0xffffffu, 0xffff00u, 0x7d4900u, 0x000000u},
		.sprite1    = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
	.blue = {
		.background = {0xffffffu, 0x65a49bu, 0x0000feu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x833100u, 0x000000u},
		.sprite2    = {0xffffffu, 0x7bff30u, 0x008300u, 0x000000u}
	},
	.darkblue = {
		.background = {0xffffffu, 0x8b8cdeu, 0x53528cu, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xffad63u, 0x833100u, 0x000000u}
	},
	.grey = {
		.background = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite1    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u},
		.sprite2    = {0xffffffu, 0xa5a5a5u, 0x525252u, 0x000000u}
	},
	.green = {
		.background = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite1    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u},
		.sprite2    = {0xffffffu, 0x51ff00u, 0xff4200u, 0x000000u}
	},
	.darkgreen = {
		.background = {0xffffffu, 0x7bff30u, 0x0163c6u, 0x000000u},
		.sprite1    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u},
		.sprite2    = {0xffffffu, 0xff8584u, 0x943a3au, 0x000000u}
	},
	.invert = {
		.background = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite1    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu},
		.sprite2    = {0x000000u, 0x008486u, 0xffde00u, 0xffffffu}
	},
	.zelda = {
		.background = {0xf8f8f8u, 0xff8096u, 0x7f3848u, 0x000000u},
		.sprite1    = {0xf8f8f8u, 0x1fba1fu, 0x376019u, 0x093609u},
		.sprite2    = {0xf8f8f8u, 0x71b6d0u, 0x0f3eaau, 0x000000u}
	}
};

#endif
