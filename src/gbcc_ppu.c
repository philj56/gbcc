#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_colour.h"
#include "gbcc_debug.h"
#include "gbcc_hdma.h"
#include "gbcc_memory.h"
#include "gbcc_palettes.h"
#include "gbcc_ppu.h"
#include "gbcc_screenshot.h"
#include <math.h>
#include <stdio.h>

#define BACKGROUND_MAP_BANK_1 0x9800u
#define BACKGROUND_MAP_BANK_2 0x9C00u
#define NUM_SPRITES 40

/* 
 * Guide to line buffer attribute bits:
 * 0 - is this pixel being drawn?
 * 1 - is this colour 0?
 * 2 - priority bit
 */

#define ATTR_DRAWN (bit(0))
#define ATTR_COLOUR0 (bit(1))
#define ATTR_PRIORITY (bit(2))

enum palette_flag { BACKGROUND, SPRITE_1, SPRITE_2 };

static void draw_line(struct gbc *gbc);
static void draw_background_line(struct gbc *gbc);
static void draw_window_line(struct gbc *gbc);
static void draw_sprite_line(struct gbc *gbc);
static void composite_line(struct gbc *gbc);
static uint8_t gbcc_get_video_mode(struct gbc *gbc);
static void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode);
static uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, enum palette_flag pf);

void gbcc_ppu_clock(struct gbc *gbc)
{
	gbc->ppu_clock += 4;
	uint8_t mode = gbcc_get_video_mode(gbc);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	uint16_t clock = gbc->ppu_clock % 456;

	/* Line-drawing timings */
	if (mode != GBC_LCD_MODE_VBLANK) {
		if (clock == 0) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
			if (check_bit(stat, 5)) {
				gbcc_memory_set_bit(gbc, IF, 1, true);
			}
		} else if (clock == GBC_LCD_MODE_PERIOD) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_VRAM_READ);
		} else if (clock == (3 * GBC_LCD_MODE_PERIOD)) {
			draw_line(gbc);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_HBLANK);
			gbcc_hdma_copy_block(gbc);
			if (check_bit(stat, 3)) {
				gbcc_memory_set_bit(gbc, IF, 1, true);
			}
		}
	}
	/* LCD STAT Interrupt */
	if (ly != 0 && ly == gbcc_memory_read(gbc, LYC, true)) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, STAT, 2, true);
			if (check_bit(stat, 6)) {
				gbcc_memory_set_bit(gbc, IF, 1, true);
			}
		} else if (clock == 8) {
			if (check_bit(stat, 6)) {
				gbcc_memory_clear_bit(gbc, IF, 1, true);
			}
		}
	} else {
		gbcc_memory_clear_bit(gbc, STAT, 2, true);
	}
	if (clock == 0) {
		ly++;
	}
	/* VBLANK interrupt flag */
	if (ly == 144) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, IF, 0, true);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_VBLANK);
			if (check_bit(stat, 4)) {
				gbcc_memory_set_bit(gbc, IF, 1, true);
			}
			
			/* TODO: All this should really be somewhere else */
			if (gbc->screenshot) {
				gbc->screenshot = false;
				gbcc_screenshot(gbc);
			}
			uint32_t *tmp = gbc->memory.gbc_screen;
			gbc->memory.gbc_screen = gbc->memory.sdl_screen;
			gbc->memory.sdl_screen = tmp;
		} else if (clock == 8) {
			gbcc_memory_clear_bit(gbc, IF, 0, true);
		}
	} else if (ly == 154) {
		gbc->memory.frame++;
		ly = 0;
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
	}
	gbcc_memory_write(gbc, LY, ly, true);
}

void draw_line(struct gbc *gbc)
{
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	if (ly > GBC_SCREEN_HEIGHT) {
		return;
	}
	for (uint8_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
		gbc->memory.background_buffer.attr[x] = 0;
		gbc->memory.window_buffer.attr[x] = 0;
		gbc->memory.sprite_buffer.attr[x] = 0;
	}
	if (!gbc->interlace || ((gbc->memory.frame) % 2 == ly % 2)) {
		draw_background_line(gbc);
		draw_window_line(gbc);
		draw_sprite_line(gbc);
		composite_line(gbc);
	} else {
		for (uint8_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			uint32_t p = gbc->memory.sdl_screen[ly * GBC_SCREEN_WIDTH + x]; 
			uint8_t r = (p & 0xFF0000u) >> 17u;
			uint8_t g = (p & 0x00FF00u) >> 9u;
			uint8_t b = (p & 0x0000FFu) >> 1u;
			p = (uint32_t)(r << 16u) | (uint32_t)(g << 8u) | b;
			gbc->memory.gbc_screen[ly * GBC_SCREEN_WIDTH + x] = p;
		}
	}
}

/* TODO: GBC BG-to-OAM Priority */
void draw_background_line(struct gbc *gbc)
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
			gbc->memory.background_buffer.colour[x] = get_palette_colour(gbc, palette, colour, BACKGROUND);
			gbc->memory.background_buffer.attr[x] |= ATTR_DRAWN;
			if (colour == 0) {
				gbc->memory.background_buffer.attr[x] |= ATTR_COLOUR0;
			}
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
			gbc->memory.background_buffer.colour[x] = get_palette_colour(gbc, attr & 0x07u, colour, BACKGROUND);
			gbc->memory.background_buffer.attr[x] |= ATTR_DRAWN;
			if (colour == 0) {
				gbc->memory.background_buffer.attr[x] |= ATTR_COLOUR0;
			}
			if (check_bit(attr, 7)) {
				gbc->memory.background_buffer.attr[x] |= ATTR_PRIORITY;
			}
		}
	}
}

void draw_window_line(struct gbc *gbc)
{
	uint8_t wy = gbcc_memory_read(gbc, WY, true);
	uint8_t wx = gbcc_memory_read(gbc, WX, true);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t palette = gbcc_memory_read(gbc, BGP, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	if (ly < wy || !check_bit(lcdc, 5) || (gbc->mode == DMG && !check_bit(lcdc, 0))) {
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
			gbc->memory.window_buffer.colour[x] = get_palette_colour(gbc, palette, colour, BACKGROUND);
			gbc->memory.window_buffer.attr[x] |= ATTR_DRAWN;
			if (colour == 0) {
				gbc->memory.window_buffer.attr[x] |= ATTR_COLOUR0;
			}
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
			gbc->memory.window_buffer.colour[x] = get_palette_colour(gbc, attr & 0x07u, colour, BACKGROUND);
			gbc->memory.window_buffer.attr[x] |= ATTR_DRAWN;
			if (colour == 0) {
			    gbc->memory.window_buffer.attr[x] |= ATTR_COLOUR0;
			}
			if (check_bit(attr, 7)) {
				gbc->memory.window_buffer.attr[x] |= ATTR_PRIORITY;
			}
		}
	}
}

void draw_sprite_line(struct gbc *gbc)
{
	/* 
	 * FIXME: Possible off-by-one error in y - check Link's awakening,
	 * when at top of screen.
	 */
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);
	enum palette_flag pf;
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
			continue;
		}
		if (size > 1) {
			/* Check for Y-flip, and swap top & bottom tiles correspondingly */
			if (ly < sy - 8) {
				/* We are drawing the top tile */
				if (check_bit(attr, 6)) {
					tile |= 0x01u;
				} else {
					tile &= 0xFEu;
				}
				sy -= 8;
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
			if (ly < sy - 8) {
				sy -= 8;
			} else {
				continue;
			}
		}
		uint16_t tile_offset = 16 * tile;
		uint8_t palette;
		if (gbc->mode == DMG) {
			if (check_bit(attr, 4)) {
				palette = gbcc_memory_read(gbc, OBP1, true);
				pf = SPRITE_1;
			} else {
				palette = gbcc_memory_read(gbc, OBP0, true);
				pf = SPRITE_2;
			}
		} else {
			palette = attr & 0x07u;
			pf = SPRITE_1;
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
				gbc->memory.sprite_buffer.colour[screen_x] = get_palette_colour(gbc, palette, colour, pf);
				gbc->memory.sprite_buffer.attr[screen_x] |= ATTR_DRAWN;
				if (check_bit(attr, 7)) {
					gbc->memory.sprite_buffer.attr[screen_x] |= ATTR_PRIORITY;
				}
			}
		}
	}
}

void composite_line(struct gbc *gbc)
{
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t bg_attr;
	uint8_t win_attr;
	uint8_t ob_attr;
	uint16_t pixel;
	for (uint8_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
		pixel = ly * GBC_SCREEN_WIDTH + x;

		bg_attr = gbc->memory.background_buffer.attr[x];
		win_attr = gbc->memory.window_buffer.attr[x];
		ob_attr = gbc->memory.sprite_buffer.attr[x];
		if (bg_attr & ATTR_DRAWN) {
			gbc->memory.gbc_screen[pixel] = gbc->memory.background_buffer.colour[x];
		}

		if (win_attr & ATTR_DRAWN) {
			gbc->memory.gbc_screen[pixel] = gbc->memory.window_buffer.colour[x];
		}

		if (ob_attr & ATTR_DRAWN) {
			if ((win_attr & ATTR_DRAWN)) {
				if (win_attr & ATTR_PRIORITY) {
					continue;
				}
				if (ob_attr & ATTR_PRIORITY) {
					continue;
				}
			}
			if ((bg_attr & ATTR_DRAWN)) {
				if (bg_attr & ATTR_PRIORITY && !(bg_attr & ATTR_COLOUR0)) {
					continue;
				}
				if (!(bg_attr & ATTR_COLOUR0) && (ob_attr & ATTR_PRIORITY)) {
					continue;
				}
			}
			gbc->memory.gbc_screen[pixel] = gbc->memory.sprite_buffer.colour[x];
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

uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, enum palette_flag pf)
{
	if (gbc->mode == DMG) {
		uint8_t colours[4] = {
			(palette & 0x03u) >> 0u,
			(palette & 0x0Cu) >> 2u,
			(palette & 0x30u) >> 4u,
			(palette & 0xC0u) >> 6u
		};
		switch (pf) {
			case BACKGROUND:
				return gbc->palette.background[colours[n]];
			case SPRITE_1:
				return gbc->palette.sprite1[colours[n]];
			case SPRITE_2:
				return gbc->palette.sprite2[colours[n]];
		}
	}
	uint8_t index = palette * 8;
	uint8_t lo;
	uint8_t hi;
	if (pf != BACKGROUND) {
		lo = gbc->memory.obp[index + 2 * n];
		hi = gbc->memory.obp[index + 2 * n + 1];
	} else {
		lo = gbc->memory.bgp[index + 2 * n];
		hi = gbc->memory.bgp[index + 2 * n + 1];
	} 
	uint8_t red = lo & 0x1Fu;
	uint8_t green = ((lo & 0xE0u) >> 5u) | (uint8_t)((hi & 0x03u) << 3u);
	uint8_t blue = (hi & 0x7Cu) >> 2u;
	//return red << 19u | green << 11u | blue << 3u;
	return gbcc_lerp_colour(red, green, blue);
}

