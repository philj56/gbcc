#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_video.h"
#include <stdio.h>

#define BACKGROUND_MAP_BANK_1 0x9800u
#define BACKGROUND_MAP_BANK_2 0x9C00u
#define NUM_SPRITES 40

static void gbcc_draw_line(struct gbc *gbc);
static void gbcc_draw_background_line(struct gbc *gbc);
static void gbcc_draw_sprite_line(struct gbc *gbc);
static void gbcc_draw_all_sprites(struct gbc *gbc);
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
	if ((gbc->clock % 456 == 4) && gbc->memory.ioreg[LY - IOREG_START] == 144) {
		gbc->memory.ioreg[IF - IOREG_START] |= 1u;
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_VBLANK);
	} else if ((gbc->clock % 456 == 8) && gbc->memory.ioreg[LY - IOREG_START] == 144) {
		gbc->memory.ioreg[IF - IOREG_START] &= 0xFEu;
	} else if (gbc->memory.ioreg[LY - IOREG_START] == 154 ) {
		gbc->memory.ioreg[LY - IOREG_START] = 0;
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
	}
}

void gbcc_draw_line(struct gbc *gbc)
{
	gbcc_draw_background_line(gbc);
	gbcc_draw_sprite_line(gbc);
	//gbcc_draw_all_sprites(gbc);
}

void gbcc_draw_background_line(struct gbc *gbc)
{
	uint8_t scy = gbc->memory.ioreg[SCY - IOREG_START];
	uint8_t scx = gbc->memory.ioreg[SCX - IOREG_START];
	
	uint8_t ly = gbc->memory.ioreg[LY - IOREG_START];
	uint8_t ty = ((scy + ly) / 8u) % 32u;
	uint8_t line_offset = 2 * ((scy + ly) % 8);
	uint8_t palette = gbc->memory.ioreg[BGP - IOREG_START];
	uint8_t lcdc = gbc->memory.ioreg[LCDC - IOREG_START];
	uint16_t map = (lcdc & 0x08u) ? BACKGROUND_MAP_BANK_2 : BACKGROUND_MAP_BANK_1;
	printf("BG: %04X\n", map);

	if (lcdc & 0x01u) {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			uint8_t tx = ((scx + x) / 8u) % 32u;
			uint8_t xoff = (scx + x) % 8u;
			uint8_t tile = gbc->memory.vram[map + 32 * ty + tx - VRAM_START];
			uint8_t lo = gbc->memory.vram[16 * tile + line_offset]; 
			uint8_t hi = gbc->memory.vram[16 * tile + line_offset + 1]; 
			uint8_t colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
			gbc->memory.screen[ly][x] = get_palette_colour(palette, colour);
		}
	} else {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			gbc->memory.screen[ly][x] = 0xc4cfa1u;
		}
	}

}

void gbcc_draw_sprite_line(struct gbc *gbc)
{
	uint8_t ly = gbc->memory.ioreg[LY - IOREG_START];
	uint8_t lcdc = gbc->memory.ioreg[LCDC - IOREG_START];
	//uint8_t palette = gbc->memory.ioreg[BGP - IOREG_START]; // TODO: REMOVE
	uint8_t palette;
	if (lcdc & (1u << 1u)) {
		uint8_t size = 1 + ((lcdc & 0x04u) >> 2u); /* 1 or 2 8x8 tiles */
		if (size > 1) {
			gbcc_log(GBCC_LOG_ERROR, "Size > 1 not implemented\n");
			exit(1);
		}
		for (size_t s = 0; s < NUM_SPRITES; s += size) {
			uint8_t sy = gbc->memory.oam[4 * s] - 8u;
			if (sy < ly || sy > (ly + size * 7u)) {
				continue;
			}
			uint8_t sx = gbc->memory.oam[4 * s + 1] - 8;
			gbcc_log(GBCC_LOG_DEBUG, "Drawing sprite %lu at (%u, %u)\n", s, sx, sy);
			uint8_t tile = gbc->memory.oam[4 * s + 2];
			palette = (gbc->memory.oam[4 * s + 3] & 0x10u) ? gbc->memory.ioreg[OBP1 - IOREG_START] : gbc->memory.ioreg[OBP0 - IOREG_START];
			uint8_t lo = gbc->memory.vram[16 * tile + 2 * (7 - (sy - ly))];
			uint8_t hi = gbc->memory.vram[16 * tile + 2 * (7 - (sy - ly)) + 1];
			for (uint8_t x = 0; x < 8 && (x + sx) < GBC_SCREEN_WIDTH; x++) {
				uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
				gbc->memory.screen[ly][x + sx] = get_palette_colour(palette, colour);
			}
		}
	}
}

void gbcc_draw_all_sprites(struct gbc *gbc)
{
	uint8_t ly = gbc->memory.ioreg[LY - IOREG_START];
	uint8_t lcdc = gbc->memory.ioreg[LCDC - IOREG_START];
	uint8_t palette = gbc->memory.ioreg[BGP - IOREG_START]; // TODO: REMOVE
	if (lcdc & (1u << 1u)) {
		uint8_t size = 1 + ((lcdc & 0x04u) >> 2u); /* 1 or 2 8x8 tiles */
		if (size > 1) {
			gbcc_log(GBCC_LOG_ERROR, "Size > 1 not implemented\n");
			exit(1);
		}
		for (size_t s = 0; s < 192; s += size) {
			uint8_t sy = 2*8*(1 + s/(GBC_SCREEN_WIDTH / 8));//gbcc_memory_read(gbc, OAM_START + 4 * s);
			if (sy < ly || sy > (ly + size * 8u)) {
				continue;
			}
			uint8_t sx = 8*s % GBC_SCREEN_WIDTH;//gbcc_memory_read(gbc, OAM_START + 4 * s + 1);
			gbcc_log(GBCC_LOG_DEBUG, "Drawing sprite %lu at (%u, %u)\n", s, sx, sy);
			uint8_t tile = s;//gbcc_memory_read(gbc, OAM_START + 4 * s + 2);
			uint8_t lo = gbc->memory.vram[16 * tile + 2 * (7 - (sy - ly))];
			uint8_t hi = gbc->memory.vram[16 * tile + 2 * (7 - (sy - ly)) + 1];
			for (uint8_t x = 0; x < 8 && (x + sx) < GBC_SCREEN_WIDTH; x++) {
				uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
				gbc->memory.screen[ly][x + sx] = get_palette_colour(palette, colour);
			}
		}
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
			return 0x6b7353u;
		case 1:
			return 0x8b956du;
		case 0:
			return 0xc4cfa1u;
		default:
			return 0;
	}
}
