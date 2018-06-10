#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_memory.h"
#include "gbcc_video.h"
#include <stdio.h>

#define BACKGROUND_MAP_START 0x9800u

static void gbcc_draw_line(struct gbc *gbc);
static uint8_t gbcc_get_video_mode(struct gbc *gbc);
static void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode);
static uint32_t get_palette_colour(uint8_t palette, uint8_t n);

void gbcc_video_update(struct gbc *gbc)
{
	if (gbc->clock % 456 == 0) {
		gbc->memory.ioreg[LY - IOREG_START] += 1;
	}
	/* Update mode while not in V-Blank */
	if ((gbcc_get_video_mode(gbc) & 3u) != 1u) {
		if (gbc->clock % 456 == 0) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
		} else if (gbc->clock % 456 == GBC_LCD_MODE_PERIOD) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_VRAM_READ);
		} else if ((gbc->clock % 456) == (3 * GBC_LCD_MODE_PERIOD)) {
			gbcc_draw_line(gbc);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_HBLANK);
		}
	}
	if ((gbc->clock % 456 == 4) && gbcc_memory_read(gbc, LY) == 144) {
		gbcc_memory_write(gbc, IF, gbcc_memory_read(gbc, IF) | 1u);
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_VBLANK);
	} else if ((gbc->clock % 456 == 8) && gbcc_memory_read(gbc, LY) == 144) {
		gbcc_memory_write(gbc, IF, gbcc_memory_read(gbc, IF) & 0xFEu);
	} else if (gbcc_memory_read(gbc, LY) == 154 ) {
		gbc->memory.ioreg[LY - IOREG_START] = 0;
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
	}
}

void gbcc_draw_line(struct gbc *gbc)
{
	uint8_t scy = gbcc_memory_read(gbc, SCY);
	uint8_t scx = gbcc_memory_read(gbc, SCX);
	
	uint8_t ly = gbcc_memory_read(gbc, LY);
	uint8_t ty = ((scy + ly) / 8u) % 32u;
	uint8_t line_offset = 2 * ((scy + ly) % 8);
	uint8_t palette = gbcc_memory_read(gbc, BGP);

	for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
		uint8_t tx = ((scx + x) / 8u) % 32u;
		uint8_t xoff = (scx + x) % 8u;
		uint8_t tile = gbcc_memory_read(gbc, BACKGROUND_MAP_START + 32 * ty + tx);
		uint8_t lo = gbcc_memory_read(gbc, VRAM_START + 16 * tile + line_offset); 
		uint8_t hi = gbcc_memory_read(gbc, VRAM_START + 16 * tile + line_offset + 1); 
		uint8_t color = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
		gbc->memory.screen[ly][x] = get_palette_colour(palette, color);
	}
}

uint8_t gbcc_get_video_mode(struct gbc *gbc)
{
	return gbc->memory.ioreg[STAT - IOREG_START] & 0x03u;
}

void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode)
{
	gbc->memory.ioreg[STAT - IOREG_START] &= 0xFCu;
	gbc->memory.ioreg[STAT - IOREG_START] |= mode;
}

uint32_t get_palette_colour(uint8_t palette, uint8_t n)
{
	uint8_t colors[4] = {
		(palette & 0x03u) >> 0u,
		(palette & 0x0Cu) >> 2u,
		(palette & 0x30u) >> 4u,
		(palette & 0xC0u) >> 6u
	};
	switch (colors[n]) {
		case 3:
			return 0x000000u;
		case 2:
			return 0x555555u;
		case 1:
			return 0xAAAAAAu;
		case 0:
			return 0xFFFFFFu;
		default:
			return 0;
	}
}
