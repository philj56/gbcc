#include "gbcc.h"
#include "bit_utils.h"
#include "colour.h"
#include "debug.h"
#include "memory.h"
#include "palettes.h"
#include "ppu.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

static void draw_background_pixel(struct gbc *gbc);
static void draw_window_pixel(struct gbc *gbc);
static void draw_sprite_pixel(struct gbc *gbc);
static void composite_line(struct gbc *gbc);
static uint8_t get_video_mode(uint8_t stat);
static uint8_t set_video_mode(uint8_t stat, uint8_t mode);
static uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, enum palette_flag pf);
static void load_bg_tile(struct gbc *gbc);
static void load_window_tile(struct gbc *gbc);
static void load_sprite_tile(struct gbc *gbc, int n);
static uint8_t get_tile_pixel(uint8_t hi, uint8_t lo, uint8_t x, bool flip);

void gbcc_disable_lcd(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	if (gbc->mode == GBC) {
		memset(ppu->screen.sdl, 0xFFu, sizeof(ppu->screen.buffer_0));
	} else {
		uint32_t colour = get_palette_colour(gbc, 0, 0, BACKGROUND);
		for (int y = 0; y < GBC_SCREEN_HEIGHT; y++) {
			for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
				ppu->screen.sdl[y * GBC_SCREEN_WIDTH + x] = colour;
			}
		}
		for (int y = 0; y < GBC_SCREEN_HEIGHT; y++) {
			for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
				ppu->screen.gbc[y * GBC_SCREEN_WIDTH + x] = colour;
			}
		}
	}
	ppu->lcd_disable = true;
	ppu->ly = 0;
	gbcc_memory_clear_bit(gbc, IF, 1, true);
	gbcc_memory_write(gbc, LY, 0, true);
	
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	stat = set_video_mode(stat, GBC_LCD_MODE_HBLANK);
	gbcc_memory_write(gbc, STAT, stat, true);
}

void gbcc_enable_lcd(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	if (!ppu->lcd_disable) {
		return;
	}
	ppu->lcd_disable = false;
	ppu->clock = 0;
	/* 
	 * TODO: Mooneye's test roms seem to imply that when enabling the
	 * screen, it starts in HBLANK? Is this true? 
	 * (Currently, it skips the first line of the first frame)
	 */
	ppu->clock = 248;
}

void gbcc_ppu_clock(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	if (ppu->lcd_disable) {
		return;
	}
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);

	/* Start of a new scanline */
	if (ppu->clock == 0 && get_video_mode(stat) != GBC_LCD_MODE_VBLANK) {
		/* Clear our buffers */
		memset(ppu->bg_line.attr, 0, sizeof(ppu->bg_line.attr));
		memset(ppu->window_line.attr, 0, sizeof(ppu->window_line.attr));
		memset(ppu->sprite_line.attr, 0, sizeof(ppu->sprite_line.attr));

		/* 
		 * The ppu ignores writes to some variables during a scanline,
		 * so we make copies of them here, and again just before
		 * rendering starts.
		 */
		ppu->scy = gbcc_memory_read(gbc, SCY, true);
		ppu->scx = gbcc_memory_read(gbc, SCX, true);
		ppu->ly = gbcc_memory_read(gbc, LY, true);
		ppu->lyc = gbcc_memory_read(gbc, LYC, true);
		ppu->wy = gbcc_memory_read(gbc, WY, true);
		ppu->wx = gbcc_memory_read(gbc, WX, true);

		/*
		 * First off, we enter OAM scanning mode for 80 cycles. As the
		 * OAM is inaccessable by the CPU during this time, we can just
		 * do all the calculation at once and then wait for 79 cycles.
		 */
		stat = set_video_mode(stat, GBC_LCD_MODE_OAM_READ);

		ppu->n_sprites = 0;
		uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);
		bool double_size = check_bit(lcdc, 2); /* 8x8 or 8x16 tiles */
		for (uint16_t addr = OAM_START; addr < OAM_END; addr += 4) {
			uint8_t y = gbcc_memory_read(gbc, addr, true);
			uint8_t x = gbcc_memory_read(gbc, addr + 1, true);
			if (ppu->ly >= y || ppu->ly + 16 < y || (!double_size && ppu->ly + 8 >= y)) {
				/* Sprite isn't on this line */
				continue;
			}
			ppu->sprites[ppu->n_sprites].y = y;
			ppu->sprites[ppu->n_sprites].x = x;
			ppu->sprites[ppu->n_sprites].address = addr;
			ppu->sprites[ppu->n_sprites].loaded = false;
			ppu->n_sprites++;
			/* GameBoy can only draw 10 sprites per line */
			if (ppu->n_sprites == 10) {
				break;
			}
		}
	}
	if (ppu->clock == 80 && get_video_mode(stat) != GBC_LCD_MODE_VBLANK) {
		/* Final value of these variables for the rest of this line */
		ppu->scy = gbcc_memory_read(gbc, SCY, true);
		ppu->scx = gbcc_memory_read(gbc, SCX, true);
		ppu->ly = gbcc_memory_read(gbc, LY, true);
		ppu->lyc = gbcc_memory_read(gbc, LYC, true);
		ppu->wy = gbcc_memory_read(gbc, WY, true);
		ppu->wx = gbcc_memory_read(gbc, WX, true);

		/* Start the actual rendering of this line */
		stat = set_video_mode(stat, GBC_LCD_MODE_OAM_VRAM_READ);
		ppu->x = 0;
		load_bg_tile(gbc);
		/*
		 * Rendering at the beginning of a scanline pauses if
		 * SCX % 8 != 0, while the ppu discards offscreen pixels
		 */
		ppu->next_dot = 88 + ppu->scx % 8;
		ppu->bg_tile.x = ppu->scx % 8;
		ppu->window_tile.x = 0;
	}
	if (get_video_mode(stat) == GBC_LCD_MODE_OAM_VRAM_READ) {
		if (ppu->clock == ppu->next_dot) {
			draw_background_pixel(gbc);
			draw_window_pixel(gbc);
			draw_sprite_pixel(gbc);
			ppu->x++;
			ppu->next_dot++;
		}
		if (ppu->x == 160) {
			stat = set_video_mode(stat, GBC_LCD_MODE_HBLANK);
			composite_line(gbc);
			if (gbc->hdma.hblank && gbc->hdma.length > 0) {
				gbc->hdma.to_copy = 0x10u;
			}
		}
	}
	
	/* LCDSTAT interrupt */
	//printf("LYC: %u\n", gbcc_memory_read(gbc, LYC, true));
	//printf("Clock = %u\tMode = %u\n", ppu->clock, get_video_mode(stat));
	ppu->clock++;
	ppu->clock %= 456;
	if (ppu->clock == 0) {
		ppu->ly++;
	}
	if (ppu->ly == 153 && ppu->clock == 10) {
		/*
		 * Some strangeness here - after a few clocks on line 153, the
		 * gameboy goes back to ly=0, and spends the rest of the
		 * scanline there in VBLANK mode, before resetting.
		 */
		ppu->ly = 0;
	}
	if (ppu->ly == 1 && get_video_mode(stat) == GBC_LCD_MODE_VBLANK) {
		ppu->frame++;
		ppu->ly = 0;
		stat = set_video_mode(stat, GBC_LCD_MODE_OAM_READ);
	}

	if (ppu->ly == gbcc_memory_read(gbc, LYC, true)) {
		stat = set_bit(stat, 2);
	} else {
		stat = clear_bit(stat, 2);
	}
	
	/* VBLANK interrupt flag */
	if (ppu->ly == 144 && ppu->clock == 0) {
		gbcc_memory_set_bit(gbc, IF, 0, true);
		stat = set_video_mode(stat, GBC_LCD_MODE_VBLANK);

		uint32_t *tmp = ppu->screen.gbc;
		ppu->screen.gbc = ppu->screen.sdl;
		ppu->screen.sdl = tmp;
	} 
	/* Check STAT interrupt */
	/* 
	 * The last_stat flag is used as the STAT interrupt is only triggered
	 * when this condition goes from 0->1, so switching between two
	 * different conditions being satisfied in a single cycle will *NOT*
	 * trigger the interrupt.
	 */
	if ((check_bit(stat, 2) && check_bit(stat, 6)) /* LY = LYC */
			|| (check_bit(stat, 3) && get_video_mode(stat) == GBC_LCD_MODE_HBLANK)
			|| (check_bit(stat, 4) && get_video_mode(stat) == GBC_LCD_MODE_VBLANK)
			|| (check_bit(stat, 5) && get_video_mode(stat) == GBC_LCD_MODE_OAM_READ)) {
		if (!ppu->last_stat) {
			ppu->last_stat = true;
			gbcc_memory_set_bit(gbc, IF, 1, true);
		}
	} else {
		ppu->last_stat = false;
	}
	gbcc_memory_write(gbc, LY, ppu->ly, true);
	gbcc_memory_write(gbc, STAT, stat, true);
}

/* TODO: GBC BG-to-OAM Priority */
void draw_background_pixel(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	struct tile *t = &ppu->bg_tile;
	if (t->x == 0) {
		load_bg_tile(gbc);
	}

	uint8_t colour = get_tile_pixel(t->hi, t->lo, t->x, check_bit(t->attr, 5));
	uint8_t palette;
	if (gbc->mode == DMG) {
		palette = gbcc_memory_read(gbc, BGP, true);
	} else {
		palette = t->attr & 0x07u;
	}
	ppu->bg_line.colour[ppu->x] = get_palette_colour(gbc, palette, colour, BACKGROUND);
	ppu->bg_line.attr[ppu->x] |= ATTR_DRAWN;
	if (colour == 0) {
		ppu->bg_line.attr[ppu->x] |= ATTR_COLOUR0;
	}
	if (check_bit(t->attr, 7)) {
		ppu->bg_line.attr[ppu->x] |= ATTR_PRIORITY;
	}
	t->x++;
	t->x %= 8;
}

void draw_window_pixel(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	struct tile *t = &ppu->window_tile;
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	if (ppu->ly < ppu->wy || !check_bit(lcdc, 5) || (gbc->mode == DMG && !check_bit(lcdc, 0))) {
		return;
	}
	if (ppu->x + 7 < ppu->wx) {
		return;
	}
	if (t->x == 0) {
		load_window_tile(gbc);
	}

	uint8_t colour = get_tile_pixel(t->hi, t->lo, t->x, check_bit(t->attr, 5));
	uint8_t palette;
	if (gbc->mode == DMG) {
		palette = gbcc_memory_read(gbc, BGP, true);
	} else {
		palette = t->attr & 0x07u;
	}
	ppu->window_line.colour[ppu->x] = get_palette_colour(gbc, palette, colour, BACKGROUND);
	ppu->window_line.attr[ppu->x] |= ATTR_DRAWN;
	if (colour == 0) {
		ppu->window_line.attr[ppu->x] |= ATTR_COLOUR0;
	}
	if (check_bit(t->attr, 7)) {
		ppu->window_line.attr[ppu->x] |= ATTR_PRIORITY;
	}
	t->x++;
	t->x %= 8;
}

void draw_sprite_pixel(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	if (!check_bit(gbcc_memory_read(gbc, LCDC, true), 1)) {
		/* Sprites are disabled */
		return;
	}
	if (gbc->cpu.dma.running) {
		/* Sprites are disabled while DMA is running*/
		return;
	}
	for (int i = 0; i < ppu->n_sprites; i++) {
		struct sprite *s = &ppu->sprites[i];
		if (ppu->x + 8 < s->x || ppu->x >= s->x) {
			continue;
		}
		if (ppu->sprite_line.attr[ppu->x] & ATTR_DRAWN) {
			/* On the GBC, sprites earlier in OAM have priority */
			/* 
			 * TODO: On DMG, the sprite with the lowest x should
			 * have priority
			 */
			continue;
		}
		if (!s->loaded) {
			load_sprite_tile(gbc, i);
			/*
			 * Each new sprite causes a delay in rendering
			 * depending on its position over the background.
			 * TODO: This should take a different amount of time
			 * when drawing over the window - ppu->scx should be
			 * replaced with (255 - ppu->wx)
			 */
			ppu->next_dot += 11 - MIN(5, (ppu->x + ppu->scx) % 8);
		}
		uint8_t x = ppu->x + 8 - s->x;
		if (check_bit(s->tile.attr, 5)) {
			/* x-flip */
			x = 7 - x;
		}
		uint8_t colour = (uint8_t)(check_bit(s->tile.hi, 7 - x) << 1u) | check_bit(s->tile.lo, 7 - x);
		/* Colour 0 is transparent */
		if (!colour) {
			continue;
		}
		uint8_t palette;
		enum palette_flag pf;
		if (gbc->mode == DMG) {
			if (check_bit(s->tile.attr, 4)) {
				palette = gbcc_memory_read(gbc, OBP1, true);
				pf = SPRITE_1;
			} else {
				palette = gbcc_memory_read(gbc, OBP0, true);
				pf = SPRITE_2;
			}
		} else {
			palette = s->tile.attr & 0x07u;
			pf = SPRITE_1;
		}
		ppu->sprite_line.colour[ppu->x] = get_palette_colour(gbc, palette, colour, pf);
		ppu->sprite_line.attr[ppu->x] |= ATTR_DRAWN;
		if (check_bit(s->tile.attr, 7)) {
			ppu->sprite_line.attr[ppu->x] |= ATTR_PRIORITY;
		}
	}
}

void composite_line(struct gbc *gbc)
{
	/* 
	 * Composite the line according to various attributes. In order of
	 * increasing priority, these are:
	 * ob_attr & ATTR_PRIORITY: if set, draw sprites below background
	 * 			    colours 1-3
	 * (win|bg)_attr & ATTR_PRIORITY: same as above
	 * bit(lcdc, 0): TODO: different for DMG & GBC
	 */
	struct ppu *ppu = &gbc->ppu;
	uint8_t ly = ppu->ly;
	uint32_t *line = &ppu->screen.gbc[ly * GBC_SCREEN_WIDTH];

	if (!gbc->hide_background) {
		memcpy(line, ppu->bg_line.colour, GBC_SCREEN_WIDTH * sizeof(line[0]));
	} else {
		memset(line, 0xFFu, GBC_SCREEN_WIDTH * sizeof(line[0]));
	}
	for (uint8_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
		uint8_t bg_attr = ppu->bg_line.attr[x];
		uint8_t win_attr = ppu->window_line.attr[x];
		uint8_t ob_attr = ppu->sprite_line.attr[x];

		if (win_attr & ATTR_DRAWN && !gbc->hide_window) {
			line[x] = ppu->window_line.colour[x];
			if (win_attr & ATTR_PRIORITY && !(win_attr & ATTR_COLOUR0)) {
				continue;
			}
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
			if (gbc->hide_sprites) {
				continue;
			}
			line[x] = ppu->sprite_line.colour[x];
		}
	}
}

uint8_t get_video_mode(uint8_t stat)
{
	return stat & 0x03u;
}

uint8_t set_video_mode(uint8_t stat, uint8_t mode)
{
	stat &= 0xFCu;
	stat |= mode;
	return stat;
}

uint32_t get_palette_colour(struct gbc *gbc, uint8_t palette, uint8_t n, enum palette_flag pf)
{
	struct ppu *ppu = &gbc->ppu;
	if (gbc->mode == DMG) {
		uint8_t colours[4] = {
			(palette & 0x03u) >> 0u,
			(palette & 0x0Cu) >> 2u,
			(palette & 0x30u) >> 4u,
			(palette & 0xC0u) >> 6u
		};
		switch (pf) {
			case BACKGROUND:
				return ppu->palette.background[colours[n]];
			case SPRITE_1:
				return ppu->palette.sprite1[colours[n]];
			case SPRITE_2:
				return ppu->palette.sprite2[colours[n]];
		}
	}
	uint8_t index = palette * 8;
	uint8_t lo;
	uint8_t hi;
	if (pf == BACKGROUND) {
		lo = ppu->bgp[index + 2 * n];
		hi = ppu->bgp[index + 2 * n + 1];
	} else {
		lo = ppu->obp[index + 2 * n];
		hi = ppu->obp[index + 2 * n + 1];
	} 
	uint8_t r = lo & 0x1Fu;
	uint8_t g = ((lo & 0xE0u) >> 5u) | (uint8_t)((hi & 0x03u) << 3u);
	uint8_t b = (hi & 0x7Cu) >> 2u;
	uint32_t res = 0;
	res |= (uint32_t)(r << 27u);
	res |= (uint32_t)(g << 19u);
	res |= (uint32_t)(b << 11u);
	res |= 0xFFu;

	return res;
}

void load_bg_tile(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	ppu->bg_tile.x = 0;
	uint8_t tx = ((ppu->scx + ppu->x) / 8u) % 32u;
	uint8_t ty = ((ppu->scy + ppu->ly) / 8u) % 32u;
	uint16_t line_offset = 2 * ((ppu->scy + ppu->ly) % 8u); /* 2 bytes per line */
	uint16_t map;
	if (check_bit(lcdc, 3)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	if (gbc->mode == DMG) {
		uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
		uint16_t tile_addr;
		if (check_bit(lcdc, 4)) {
			tile_addr = VRAM_START + 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
		}
		ppu->bg_tile.lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
		ppu->bg_tile.hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
		ppu->bg_tile.attr = 0;
	} else {
		uint8_t tile = gbc->memory.vram_bank[0][map + 32 * ty + tx - VRAM_START];
		ppu->bg_tile.attr = gbc->memory.vram_bank[1][map + 32 * ty + tx - VRAM_START];
		uint8_t *vbk;
		uint16_t tile_addr;
		if (check_bit(ppu->bg_tile.attr, 3)) {
			vbk = gbc->memory.vram_bank[1];
		} else {
			vbk = gbc->memory.vram_bank[0];
		}
		if (check_bit(lcdc, 4)) {
			tile_addr = 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x1000 + 16 * (int8_t)tile);
		}
		/* Check for Y-flip */
		if (check_bit(ppu->bg_tile.attr, 6)) {
			ppu->bg_tile.lo = vbk[tile_addr + (14 - line_offset)];
			ppu->bg_tile.hi = vbk[tile_addr + (14 - line_offset) + 1];
		} else {
			ppu->bg_tile.lo = vbk[tile_addr + line_offset];
			ppu->bg_tile.hi = vbk[tile_addr + line_offset + 1];
		}
	}
}

void load_window_tile(struct gbc *gbc)
{
	struct ppu *ppu = &gbc->ppu;
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	ppu->window_tile.x = 0;
	uint8_t tx = ((ppu->x - ppu->wx + 7) / 8u) % 32u;
	uint8_t ty = ((ppu->ly - ppu->wy) / 8u) % 32u;
	uint16_t line_offset = 2 * ((ppu->ly - ppu->wy) % 8u); /* 2 bytes per line */
	uint16_t map;
	if (check_bit(lcdc, 6)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	if (gbc->mode == DMG) {
		uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
		uint16_t tile_addr;
		if (check_bit(lcdc, 4)) {
			tile_addr = VRAM_START + 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
		}
		ppu->window_tile.lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
		ppu->window_tile.hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
		ppu->window_tile.attr = 0;
	} else {
		uint8_t tile = gbc->memory.vram_bank[0][map + 32 * ty + tx - VRAM_START];
		ppu->window_tile.attr = gbc->memory.vram_bank[1][map + 32 * ty + tx - VRAM_START];
		uint8_t *vbk;
		uint16_t tile_addr;
		if (check_bit(ppu->window_tile.attr, 3)) {
			vbk = gbc->memory.vram_bank[1];
		} else {
			vbk = gbc->memory.vram_bank[0];
		}
		if (check_bit(lcdc, 4)) {
			tile_addr = 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x1000 + 16 * (int8_t)tile);
		}
		/* Check for Y-flip */
		if (check_bit(ppu->window_tile.attr, 6)) {
			ppu->window_tile.lo = vbk[tile_addr + (14 - line_offset)];
			ppu->window_tile.hi = vbk[tile_addr + (14 - line_offset) + 1];
		} else {
			ppu->window_tile.lo = vbk[tile_addr + line_offset];
			ppu->window_tile.hi = vbk[tile_addr + line_offset + 1];
		}
	}
}

void load_sprite_tile(struct gbc *gbc, int n)
{
	struct ppu *ppu = &gbc->ppu;
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);
	bool double_size = check_bit(lcdc, 2); /* 1 or 2 8x8 tiles */
	struct tile *t = &ppu->sprites[n].tile;
	/* 
	 * Together SY & SX define the bottom right corner of the
	 * sprite. X=1, Y=1 is the top left corner, so each of these
	 * are offset by 1 for array values.
	 * TODO: Is this off-by-one true?
	 */
	uint8_t sy = ppu->sprites[n].y;
	uint8_t tile = gbcc_memory_read(gbc, ppu->sprites[n].address + 2, true);
	t->attr = gbcc_memory_read(gbc, ppu->sprites[n].address + 3, true);
	bool yflip = check_bit(t->attr, 6);
	uint8_t sprite_line = sy - ly;
	uint8_t *vram_bank;
	if (gbc->mode == DMG) {
		vram_bank = gbc->memory.vram_bank[0];
	} else {
		vram_bank = gbc->memory.vram_bank[check_bit(t->attr, 3)];
	}
	if (double_size) {
		/* 
		 * In 8x16 mode, the lower bit of the tile number is
		 * ignored, and the sprite is stored as upper tile then
		 * lower tile.
		 */
		if (ly + 8 < sy) {
			/* We are drawing the top tile */
			tile = cond_bit(tile, 0, yflip);
		} else {
			/* We are drawing the bottom tile */
			tile = cond_bit(tile, 0, !yflip);
			sprite_line += 8;
		}
	}
	if (yflip) {
		sprite_line -= 9;
	} else {
		sprite_line = 16 - sprite_line;
	}
	t->lo = vram_bank[16 * tile + 2 * sprite_line];
	t->hi = vram_bank[16 * tile + 2 * sprite_line + 1];
	t->x = 0;
	ppu->sprites[n].loaded = true;
}

uint8_t get_tile_pixel(uint8_t hi, uint8_t lo, uint8_t x, bool flip)
{
	if (flip) {
		return (uint8_t)(check_bit(hi, x) << 1u) | check_bit(lo, x);
	}
	return (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
}
