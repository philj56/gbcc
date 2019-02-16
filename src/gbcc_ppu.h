#ifndef GBCC_PPU_H
#define GBCC_PPU_H

#include "gbcc_constants.h"
#include "gbcc_palettes.h"
#include <stdbool.h>
#include <stdint.h>

struct gbc;

struct line_buffer {
	uint32_t colour[GBC_SCREEN_WIDTH];
	uint8_t attr[GBC_SCREEN_WIDTH];
};

struct ppu {
	uint64_t frame;
	uint16_t clock;
	bool lcd_disable;
	uint8_t bgp[64]; 	/* 8 x 8-byte sprites */
	uint8_t obp[64]; 	/* 8 x 8-byte sprites */
	struct palette palette;
	struct line_buffer bg_line;
	struct line_buffer window_line;
	struct line_buffer sprite_line;
	struct {
		uint32_t buffer_0[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		uint32_t buffer_1[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		uint32_t *gbc;
		uint32_t *sdl;
	} screen;
};

void gbcc_ppu_clock(struct gbc *gbc);
void gbcc_disable_lcd(struct gbc *gbc);
void gbcc_enable_lcd(struct gbc *gbc);

#endif /* GBCC_PPU_H */
