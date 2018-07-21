#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_ppu.h"
#include <math.h>
#include <stdio.h>

#define BACKGROUND_MAP_BANK_1 0x9800u
#define BACKGROUND_MAP_BANK_2 0x9C00u
#define NUM_SPRITES 40

static void gbcc_draw_line(struct gbc *gbc);
static void gbcc_draw_background_line(struct gbc *gbc);
static void gbcc_draw_window_line(struct gbc *gbc);
static void gbcc_draw_sprite_line(struct gbc *gbc);
static uint8_t gbcc_get_video_mode(struct gbc *gbc);
static void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode);
static uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, bool sprite);

void gbcc_ppu_clock(struct gbc *gbc)
{
	uint8_t mode = gbcc_get_video_mode(gbc);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	uint16_t clock = gbc->clock % 456;

	if (clock == 0) {
		gbcc_memory_increment(gbc, LY, true);
	}
	/* Line-drawing timings */
	if (mode != GBC_LCD_MODE_VBLANK) {
		if (clock == 0) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
		} else if (clock == GBC_LCD_MODE_PERIOD) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_VRAM_READ);
		} else if (clock == (3 * GBC_LCD_MODE_PERIOD)) {
			gbcc_draw_line(gbc);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_HBLANK);
		}
	}
	/* LCD STAT Interrupt */
	if (ly != 0 && ly == gbcc_memory_read(gbc, LYC, true)) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, STAT, 2, true);
		} else if (clock == 8) {
			gbcc_memory_clear_bit(gbc, STAT, 2, true);
		}
	}
	if ((check_bit(stat, 2) && check_bit(stat, 6))
			|| (mode == GBC_LCD_MODE_HBLANK && check_bit(stat, 3))
			|| (mode == GBC_LCD_MODE_OAM_READ && check_bit(stat, 5))
			|| (mode == GBC_LCD_MODE_VBLANK && check_bit(stat, 4))) {
		gbcc_memory_set_bit(gbc, IF, 1, true);
	}
	/* VBLANK interrupt flag */
	if (ly == 144) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, IF, 0, true);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_VBLANK);
			uint32_t *tmp = gbc->memory.gbc_screen;
			gbc->memory.gbc_screen = gbc->memory.sdl_screen;
			gbc->memory.sdl_screen = tmp;
		} else if (clock == 8) {
			gbcc_memory_clear_bit(gbc, IF, 0, true);
		}
	} else if (ly == 154) {
		gbcc_memory_write(gbc, LY, 0, true);
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
	}
}

void gbcc_draw_line(struct gbc *gbc)
{
	if (gbcc_memory_read(gbc, LY, true) > GBC_SCREEN_HEIGHT) {
		return;
	}
	gbcc_draw_background_line(gbc);
	gbcc_draw_window_line(gbc);
	gbcc_draw_sprite_line(gbc);
}

/* TODO: GBC BG-to-OAM Priority */
void gbcc_draw_background_line(struct gbc *gbc)
{
	uint8_t scy = gbcc_memory_read(gbc, SCY, true);
	uint8_t scx = gbcc_memory_read(gbc, SCX, true);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t palette = gbcc_memory_read(gbc, BGP, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	uint8_t ty = ((scy + ly) / 8u) % 32u;
	uint16_t line_offset = 2 * ((scy + ly) % 8u); /* 2 bytes per line */
	uint16_t map;
	if (check_bit(lcdc, 3)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	if (!check_bit(lcdc, 0)) {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = 0xc4cfa1u;
		}
		return;
	}

	if (gbc->mode == DMG) {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			uint8_t tx = ((scx + x) / 8u) % 32u;
			uint8_t xoff = (scx + x) % 8u;
			uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
			uint16_t tile_addr;
			/* TODO: Put this somewhere better */
			if (check_bit(lcdc, 4)) {
				tile_addr = VRAM_START + 16 * tile;
			} else {
				tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
			}
			uint8_t lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
			uint8_t hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
			uint8_t colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = get_palette_colour(gbc, palette, colour, false);
		}
	} else {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			uint8_t tx = ((scx + x) / 8u) % 32u;
			uint8_t xoff = (scx + x) % 8u;
			uint8_t tile = gbc->memory.vram_bank[0][map + 32 * ty + tx - VRAM_START];
			uint8_t attr = gbc->memory.vram_bank[1][map + 32 * ty + tx - VRAM_START];
			uint8_t *vbk;
			uint16_t tile_addr;
			if (check_bit(attr, 3)) {
				vbk = gbc->memory.vram_bank[1];
			} else {
				vbk = gbc->memory.vram_bank[0];
			}
			/* TODO: Put this somewhere better */
			if (check_bit(lcdc, 4)) {
				tile_addr = 16 * tile;
			} else {
				tile_addr = (uint16_t)(0x1000 + 16 * (int8_t)tile);
			}
			uint8_t lo;
			uint8_t hi;
			/* Check for Y-flip */
			if (check_bit(attr, 6)) {
				lo = vbk[tile_addr + (14 - line_offset)];
				hi = vbk[tile_addr + (14 - line_offset) + 1];
			} else {
				lo = vbk[tile_addr + line_offset];
				hi = vbk[tile_addr + line_offset + 1];
			}
			uint8_t colour;
			/* Check for X-flip */
			if (check_bit(attr, 5)) {
				colour = (uint8_t)(check_bit(hi, xoff) << 1u) | check_bit(lo, xoff);
			} else {
				colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
			}
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = get_palette_colour(gbc, attr & 0x07u, colour, false);
		}
	}
}

/* TODO: Implement GBC tile attrs as for background */
void gbcc_draw_window_line(struct gbc *gbc)
{
	uint8_t wy = gbcc_memory_read(gbc, WY, true);
	uint8_t wx = gbcc_memory_read(gbc, WX, true);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t palette = gbcc_memory_read(gbc, BGP, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	if (ly < wy || !check_bit(lcdc, 5)) {
		return;
	}

	uint8_t ty = ((ly - wy) / 8u) % 32u;
	uint16_t line_offset = 2 * ((ly - wy) % 8);
	uint16_t map;
	if (check_bit(lcdc, 6)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	if (gbc->mode == DMG) {
		for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
			if (x < wx - 7) {
				continue;
			}
			uint8_t tx = ((x - wx + 7) / 8) % 32;
			uint8_t xoff = (x - wx + 7) % 8;
			uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
			uint16_t tile_addr;
			/* TODO: Put this somewhere better */
			if (check_bit(lcdc, 4)) {
				tile_addr = VRAM_START + 16 * tile;
			} else {
				tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
			}
			uint8_t lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
			uint8_t hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
			uint8_t colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = get_palette_colour(gbc, palette, colour, false);
		}
	} else {
		for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
			if (x < wx - 7) {
				continue;
			}
			uint8_t tx = ((x - wx + 7) / 8) % 32;
			uint8_t xoff = (x - wx + 7) % 8;
			uint8_t tile = gbc->memory.vram_bank[0][map + 32 * ty + tx - VRAM_START];
			uint8_t attr = gbc->memory.vram_bank[1][map + 32 * ty + tx - VRAM_START];
			uint8_t *vbk;
			uint16_t tile_addr;
			if (check_bit(attr, 3)) {
				vbk = gbc->memory.vram_bank[1];
			} else {
				vbk = gbc->memory.vram_bank[0];
			}
			/* TODO: Put this somewhere better */
			if (check_bit(lcdc, 4)) {
				tile_addr = 16 * tile;
			} else {
				tile_addr = (uint16_t)(0x1000 + 16 * (int8_t)tile);
			}
			uint8_t lo;
			uint8_t hi;
			/* Check for Y-flip */
			if (check_bit(attr, 6)) {
				lo = vbk[tile_addr + (14 - line_offset)];
				hi = vbk[tile_addr + (14 - line_offset) + 1];
			} else {
				lo = vbk[tile_addr + line_offset];
				hi = vbk[tile_addr + line_offset + 1];
			}
			uint8_t colour;
			/* Check for X-flip */
			if (check_bit(attr, 5)) {
				colour = (uint8_t)(check_bit(hi, xoff) << 1u) | check_bit(lo, xoff);
			} else {
				colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
			}
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = get_palette_colour(gbc, attr & 0x07u, colour, false);
		}
	}
}

void gbcc_draw_sprite_line(struct gbc *gbc)
{
	/* 
	 * FIXME: Possible off-by-one error in y - check Link's awakening,
	 * when at top of screen.
	 */
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);
	uint32_t bg0 = get_palette_colour(gbc, gbcc_memory_read(gbc, BGP, true), 0, false);
	if (!check_bit(lcdc, 1)) {
		return;
	}
	uint8_t size = 1 + check_bit(lcdc, 2); /* 1 or 2 8x8 tiles */
	for (size_t s = NUM_SPRITES-1; s < NUM_SPRITES; s--) {
		/* 
		 * Together SY & SX define the bottom right corner of the
		 * sprite. X=1, Y=1 is the top left corner, so each of these
		 * are offset by 1 for array values.
		 * TODO: Is this off-by-one true?
		 */
		uint8_t sy = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s), true);
		uint8_t sx = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 1), true);
		uint8_t tile = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 2), true);
		uint8_t attr = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 3), true);
		uint8_t *vram_bank;
		switch (gbc->mode) {
			case DMG:
				vram_bank = gbc->memory.vram_bank[0];
				break;
			case GBC:
				vram_bank = check_bit(attr, 3) ? gbc->memory.vram_bank[1] : gbc->memory.vram_bank[0];
				break;
		}
		if (ly < sy - 16 || ly >= sy) {
			//if (!(sy < 16u && ly < sy)) {
			continue;
			//}
		}
		if (size > 1) {
			/* Check for Y-flip, and swap top & bottom tiles correspondingly */
			if (ly < sy - 8u) {
				/* We are drawing the top tile */
				if (check_bit(attr, 6)) {
					tile |= 0x01u;
				} else {
					tile &= 0xFEu;
				}
				sy -= 8u;
			} else {
				/* We are drawing the bottom tile */
				if (check_bit(attr, 6)) {
					tile &= 0xFEu;
				} else {
					tile |= 0x01u;
				}
			}
		} else {
			/* 
			 * 8x8 sprites draw as if they were the top tile of an
			 * 8x16 sprite.
			 */
			if (ly < sy - 8u) {
				sy -= 8u;
			} else {
				continue;
			}
		}
		uint16_t tile_offset = 16 * tile;
		uint8_t palette;
		if (gbc->mode == DMG) {
			if (check_bit(attr, 4)) {
				palette = gbcc_memory_read(gbc, OBP1, true);
			} else {
				palette = gbcc_memory_read(gbc, OBP0, true);
			}
		} else {
			palette = attr & 0x07u;
		}
		uint8_t lo;
		uint8_t hi;
		/* Check for Y-flip */
		if (check_bit(attr, 6)) {
			lo = vram_bank[tile_offset + 2 * (sy - ly - 1)];
			hi = vram_bank[tile_offset + 2 * (sy - ly - 1) + 1];
		} else {
			lo = vram_bank[tile_offset + 2 * (8 - (sy - ly))];
			hi = vram_bank[tile_offset + 2 * (8 - (sy - ly)) + 1];
		}
		for (uint8_t x = 0; x < 8; x++) {
			uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
			/* Colour 0 is transparent */
			if (!colour) {
				continue;
			}
			/* Check for X-flip */
			uint8_t screen_x;
			if (check_bit(attr, 5)) {
				screen_x = sx - x - 1;
			} else {
				screen_x = sx + x - 8;
			}
			if (screen_x < GBC_SCREEN_WIDTH) {
				if (check_bit(attr, 7) && gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + screen_x] != bg0) {
					continue;
				}
				gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + screen_x] = get_palette_colour(gbc, palette, colour, true);
			}
		}
	}
}

uint8_t gbcc_get_video_mode(struct gbc *gbc)
{
	return gbcc_memory_read(gbc, STAT, true) & 0x03u;
}

void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode)
{
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	stat &= 0xFCu;
	stat |= mode;
	gbcc_memory_write(gbc, STAT, stat, true);
}

uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, bool sprite)
{
	if (gbc->mode == DMG) {
		uint8_t colours[4] = {
			(palette & 0x03u) >> 0u,
			(palette & 0x0Cu) >> 2u,
			(palette & 0x30u) >> 4u,
			(palette & 0xC0u) >> 6u
		};
		switch (colours[n]) {
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
	uint8_t index = palette * 8;
	uint8_t lo;
	uint8_t hi;
	if (sprite) {
		lo = gbc->memory.obp[index + 2 * n];
		hi = gbc->memory.obp[index + 2 * n + 1];
	} else {
		lo = gbc->memory.bgp[index + 2 * n];
		hi = gbc->memory.bgp[index + 2 * n + 1];
	}
	uint8_t red = lo & 0x1Fu;
	uint8_t green = ((lo & 0xE0u) >> 5u) | ((hi & 0x03u) << 3u);
	uint8_t blue = hi & 0x7Cu >> 2u;
	red = (double)red / 0x1Fu * 0xFFu;
	green = (double)green / 0x1Fu * 0xFFu;
	blue = (double)blue / 0x1Fu * 0xFFu;
	return (uint32_t)((uint32_t)(red << 16u) | (uint32_t)(green << 8u)) | blue;
}
