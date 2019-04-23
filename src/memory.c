#include "gbcc.h"
#include "apu.h"
#include "bit_utils.h"
#include "debug.h"
#include "hdma.h"
#include "mbc.h"
#include "memory.h"
#include "ppu.h"
#include <stdio.h>

static const uint8_t ioreg_read_masks[0x80] = {
/* 0xFF00 */	0x3F, 0xFF, 0x83, 0x00, 0xFF, 0xFF, 0xFF, 0x07,
/* 0xFF08 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
/* 0xFF10 */	0x7F, 0xC0, 0xFF, 0x00, 0x40, 0x00, 0xC0, 0xFF,
/* 0xFF18 */	0x00, 0x40, 0x80, 0x00, 0x60, 0x00, 0x40, 0x00,
/* 0xFF20 */	0x00, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, 0x8F, 0x00,
/* 0xFF28 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF30 */	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF38 */	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF40 */	0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF48 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x01,
/* 0xFF50 */	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x00,
/* 0xFF58 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF60 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF68 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
/* 0xFF70 */	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF78 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t ioreg_write_masks[0x80] = {
/* 0xFF00 */	0x30, 0xFF, 0x83, 0x00, 0xFF, 0xFF, 0xFF, 0x07,
/* 0xFF08 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
/* 0xFF10 */	0x7F, 0xFF, 0xFF, 0xFF, 0xC7, 0x00, 0xFF, 0xFF,
/* 0xFF18 */	0xFF, 0xC7, 0x80, 0xFF, 0x60, 0xFF, 0xC7, 0x00,
/* 0xFF20 */	0x3F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0x80, 0x00,
/* 0xFF28 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF30 */	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF38 */	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF40 */	0xFF, 0x78, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
/* 0xFF48 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0x01,
/* 0xFF50 */	0x00, 0xFF, 0xF0, 0x1F, 0xF0, 0xFF, 0xC2, 0x00,
/* 0xFF58 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF60 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF68 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
/* 0xFF70 */	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF78 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t vram_read(struct gbc *gbc, uint16_t addr);
static void vram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

static uint8_t wram_read(struct gbc *gbc, uint16_t addr);
static void wram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

static uint8_t echo_read(struct gbc *gbc, uint16_t addr);
static void echo_write(struct gbc *gbc, uint16_t addr, uint8_t val);

static uint8_t oam_read(struct gbc *gbc, uint16_t addr, bool override);
static void oam_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t unused_read(struct gbc *gbc, uint16_t addr);
static void unused_write(struct gbc *gbc, uint16_t addr, uint8_t val);

static uint8_t ioreg_read(struct gbc *gbc, uint16_t addr, bool override);
static void ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t hram_read(struct gbc *gbc, uint16_t addr);
static void hram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

void gbcc_memory_increment(struct gbc *gbc, uint16_t addr, bool override)
{
	gbcc_memory_write(gbc, addr, gbcc_memory_read(gbc, addr, override) + 1, override);
}

void gbcc_memory_copy(struct gbc *gbc, uint16_t src, uint16_t dest, bool override)
{
	gbcc_memory_write(gbc, dest, gbcc_memory_read(gbc, src, override), override);
}

void gbcc_memory_set_bit(struct gbc *gbc, uint16_t addr, uint8_t b, bool override)
{
	gbcc_memory_write(gbc, addr, gbcc_memory_read(gbc, addr, override) | bit(b), override);
}

void gbcc_memory_clear_bit(struct gbc *gbc, uint16_t addr, uint8_t b, bool override)
{
	gbcc_memory_write(gbc, addr, gbcc_memory_read(gbc, addr, override) & (uint8_t)~bit(b), override);
}

uint8_t gbcc_memory_read(struct gbc *gbc, uint16_t addr, bool override)
{
	if (addr < ROMX_END || (addr >= SRAM_START && addr < SRAM_END)) {
		switch (gbc->cart.mbc.type) {
			case NONE:
				return gbcc_mbc_none_read(gbc, addr);
			case MBC1:
				return gbcc_mbc_mbc1_read(gbc, addr);
			case MBC2:
				return gbcc_mbc_mbc2_read(gbc, addr);
			case MBC3:
				return gbcc_mbc_mbc3_read(gbc, addr);
			case MBC5:
				return gbcc_mbc_mbc5_read(gbc, addr);
			case MMM01:
				return gbcc_mbc_mmm01_read(gbc, addr);
		}
	}
	if (addr >= VRAM_START && addr < VRAM_END) {
		return vram_read(gbc, addr);
	}
	if (addr >= WRAM0_START && addr < WRAMX_END) {
		return wram_read(gbc, addr);
	}
	if (addr >= ECHO_START && addr < ECHO_END) {
		return echo_read(gbc, addr);
	}
	if (addr >= OAM_START && addr < OAM_END) {
		return oam_read(gbc, addr, override);
	}
	if (addr >= UNUSED_START && addr < UNUSED_END) {
		return unused_read(gbc, addr);
	}
	if (addr >= IOREG_START && addr < IOREG_END) {
		return ioreg_read(gbc, addr, override);
	}
	if (addr >= HRAM_START && addr < HRAM_END) {
		return hram_read(gbc, addr);
	}
	if (addr == IE) {
		return gbc->memory.iereg;
	}
	gbcc_log_error("Reading from unknown memory address %04X.\n", addr);
	return 0;
}

void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	if (addr < ROMX_END || (addr >= SRAM_START && addr < SRAM_END)) {
		switch (gbc->cart.mbc.type) {
			case NONE:
				gbcc_mbc_none_write(gbc, addr, val);
				break;
			case MBC1:
				gbcc_mbc_mbc1_write(gbc, addr, val);
				break;
			case MBC2:
				gbcc_mbc_mbc2_write(gbc, addr, val);
				break;
			case MBC3:
				gbcc_mbc_mbc3_write(gbc, addr, val);
				break;
			case MBC5:
				gbcc_mbc_mbc5_write(gbc, addr, val);
				break;
			case MMM01:
				gbcc_mbc_mmm01_write(gbc, addr, val);
				break;
		}
		return;
	}
	if (addr >= VRAM_START && addr < VRAM_END) {
		vram_write(gbc, addr, val);
	} else if (addr >= WRAM0_START && addr < WRAMX_END) {
		wram_write(gbc, addr, val);
	} else if (addr >= ECHO_START && addr < ECHO_END) {
		echo_write(gbc, addr, val);
	} else if (addr >= OAM_START && addr < OAM_END) {
		oam_write(gbc, addr, val, override);
	} else if (addr >= UNUSED_START && addr < UNUSED_END) {
		unused_write(gbc, addr, val);
	} else if (addr >= IOREG_START && addr < IOREG_END) {
		ioreg_write(gbc, addr, val, override);
	} else if (addr >= HRAM_START && addr < HRAM_END) {
		hram_write(gbc, addr, val);
	} else if (addr == IE) {
		gbc->memory.iereg = val;
	} else {
		gbcc_log_error("Writing to unknown memory address %04X.\n", addr);
	}
}

uint8_t vram_read(struct gbc *gbc, uint16_t addr)
{
	return gbc->memory.vram[addr - VRAM_START];
}

void vram_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbc->memory.vram[addr - VRAM_START] = val;
}

uint8_t wram_read(struct gbc *gbc, uint16_t addr)
{
	if (addr < WRAMX_START) {
		return gbc->memory.wram0[addr - WRAM0_START];
	}
	return gbc->memory.wramx[addr - WRAMX_START];
}

void wram_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr < WRAMX_START) {
		gbc->memory.wram0[addr - WRAM0_START] = val;
	} else {
		gbc->memory.wramx[addr - WRAMX_START] = val;
	}
}

uint8_t echo_read(struct gbc *gbc, uint16_t addr)
{
	return wram_read(gbc, addr - WRAMX_SIZE - WRAM0_SIZE);
}

void echo_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	wram_write(gbc, addr - WRAMX_SIZE - WRAM0_SIZE, val);
}

uint8_t oam_read(struct gbc *gbc, uint16_t addr, bool override)
{
	if (gbc->cpu.dma.running && !override) {
		return 0xFFu;
	}
	return gbc->memory.oam[addr - OAM_START];
}

void oam_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	if (gbc->cpu.dma.running && !override) {
		return;
	}
	gbc->memory.oam[addr - OAM_START] = val;
}

uint8_t unused_read(struct gbc *gbc, uint16_t addr)
{
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	return gbc->memory.unused[addr - UNUSED_START - offset];
}

void unused_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	gbc->memory.unused[addr - UNUSED_START - offset] = val;
}

uint8_t ioreg_read(struct gbc *gbc, uint16_t addr, bool override)
{
	/* Ignore GBC-specific registers when in DMG mode */
	if (gbc->mode == DMG) {
		switch (addr) {
			case KEY1:
			case VBK:
			case HDMA1:
			case HDMA2:
			case HDMA3:
			case HDMA4:
			case HDMA5:
			case RP:
			case BGPI:
			case BGPD:
			case OBPI:
			case OBPD:
			case SVBK:
				return 0xFFu;
			default:
				break;
		}
	}

	if (override) {
		return gbc->memory.ioreg[addr - IOREG_START];
	}

	uint8_t mask = ioreg_read_masks[addr - IOREG_START];
	uint8_t ret = gbc->memory.ioreg[addr - IOREG_START];

	if (addr >= WAVE_START && addr < WAVE_END) {
		//printf("Reading wave from %04X\n", gbc->apu.wave.addr);
		if (gbc->apu.ch3.enabled) {
			return gbc->memory.ioreg[gbc->apu.wave.addr - IOREG_START];
		}
	}

	switch (addr) {
		case LY:
			if (gbc->ppu.lcd_disable) {
				return 0;
			}
			break;
		case JOYP:
			/* Only update the keys when we actually want to read from them */
			if (check_bit(gbc->memory.ioreg[addr - IOREG_START], 5)) {
				ret &= (uint8_t)~(uint8_t)(gbc->keys.start << 3u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.select << 2u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.b << 1u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.a << 0u);
			}
			if (check_bit(gbc->memory.ioreg[addr - IOREG_START], 4)) {
				ret &= (uint8_t)~(uint8_t)(gbc->keys.dpad.down << 3u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.dpad.up << 2u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.dpad.left << 1u);
				ret &= (uint8_t)~(uint8_t)(gbc->keys.dpad.right << 0u);
			}
			break;
		case DIV:
			return high_byte(gbc->cpu.div_timer);
		case NR52:
			ret &= 0xF0u;
			ret |= (uint8_t)(gbc->apu.ch1.enabled << 0u);
			ret |= (uint8_t)(gbc->apu.ch2.enabled << 1u);
			ret |= (uint8_t)(gbc->apu.ch3.enabled << 2u);
			ret |= (uint8_t)(gbc->apu.ch4.enabled << 3u);
			break;
		case BGPD:
			ret = gbc->ppu.bgp[gbc->memory.ioreg[BGPI - IOREG_START] & 0x3Fu];
			break;
		case OBPD:
			ret = gbc->ppu.obp[gbc->memory.ioreg[OBPI - IOREG_START] & 0x3Fu];
			break;
		default:
			ret = gbc->memory.ioreg[addr - IOREG_START];
			break;
	}
	return ret | (uint8_t)~mask;
}

void ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	/*
	if (addr == LCDC) {
		printf("LCDC = 0b");
		for (int i = 7; i >= 0; i--) {
			printf("%u", check_bit(val, i));
		}
		printf("\n");
	}
	if (addr == STAT) {
		printf("STAT = 0b");
		val |= ~ioreg_read_masks[addr - IOREG_START];
		for (int i = 7; i >= 0; i--) {
			printf("%u", check_bit(val, i));
		}
		printf(" (0x%02X)\n", val);
	}
	*/
	/* Ignore GBC-specific registers when in DMG mode */
	if (gbc->mode == DMG) {
		switch (addr) {
			case KEY1:
			case VBK:
			case HDMA1:
			case HDMA2:
			case HDMA3:
			case HDMA4:
			case HDMA5:
			case RP:
			case BGPI:
			case BGPD:
			case OBPI:
			case OBPD:
			case SVBK:
				return;
			default:
				break;
		}
	}

	uint8_t *dest = &gbc->memory.ioreg[addr - IOREG_START];

	if (override) {
		*dest = val;
		return;
	}

	uint8_t mask = ioreg_write_masks[addr - IOREG_START];
	uint8_t tmp = *dest & (uint8_t)~mask;
	
	if (addr >= NR10 && addr <= NR52) {
		if (addr != NR52 && gbc->apu.disabled) {
			return;
		}
		*dest = tmp | (uint8_t)(val & mask);
		gbcc_apu_memory_write(gbc, addr, val);
		return;
	}

	switch (addr) {
		case LY:
			*dest = 0;
			break;
		case JOYP:
			if (check_bit(val, 5)) {
				*dest = set_bit(*dest, 4);
				*dest = clear_bit(*dest, 5);
			} else if (check_bit(val, 4)) {
				*dest = set_bit(*dest, 5);
				*dest = clear_bit(*dest, 4);
			}
			break;
		case SC:
			if (val & 0x81u) {
				fprintf(stderr, "%c", gbc->memory.ioreg[SB - IOREG_START]);
				/* 
				 * For now, just immediately complete. If
				 * link_cable_loop is true, don't overwrite SB.
				 * This means the gameboy acts like it's
				 * talking to an exact clone of itself.
				 */
				gbcc_memory_set_bit(gbc, IF, 3, true);
				if (!gbc->link_cable_loop) {
					gbc->memory.ioreg[SB - IOREG_START] = 0xFFu;
				}
				gbc->memory.ioreg[SC - IOREG_START] = 0x01u;
			}
			break;
		case DIV:
			//printf("DIV reset from %04X\n", gbc->div_timer);
			gbc->cpu.div_timer = 0;
			break;
		case LCDC:
			if (check_bit(val, 7)) {
				gbcc_enable_lcd(gbc);
			} else {
				gbcc_disable_lcd(gbc);
			}
			*dest = tmp | (uint8_t)(val & mask);
			break;
		case DMA:
			gbc->cpu.dma.new_source = (uint16_t)val << 8u;
			if (gbc->cpu.dma.new_source > WRAMX_END) {
				/* Can't DMA from ECHO or IOREG areas */
				gbc->cpu.dma.new_source -= 0x2000u;
			}
			gbc->cpu.dma.requested = true;
			*dest = tmp | (uint8_t)(val & mask);
			break;
		case VBK:
			*dest = tmp | (uint8_t)(val & mask);
			gbc->memory.vram = gbc->memory.vram_bank[*dest];
			break;
		case HDMA1:
			if (val < 0x80u || (val >= 0xA0u && val < 0xE0u)) {
				*dest = val;
			}
			break;
		case HDMA5:
			{
				if (!check_bit(val, 7)) {
					if (gbc->hdma.length > 0) {
						gbc->hdma.length = 0;
						gbc->hdma.to_copy = 0;
						*dest |= 0x80u;
						return;
					}
				}
				uint8_t src_hi = gbc->memory.ioreg[HDMA1 - IOREG_START];
				uint8_t src_lo = gbc->memory.ioreg[HDMA2 - IOREG_START];
				uint8_t dst_hi = gbc->memory.ioreg[HDMA3 - IOREG_START];
				uint8_t dst_lo = gbc->memory.ioreg[HDMA4 - IOREG_START];
				/* Lower 4 bits of source are ignored */
				src_lo &= 0xF0u;
				
				/*
				 * Upper 3 & lower 4 bits of destination are
				 * ignored (destination is always VRAM) 
				 */
				dst_hi &= 0x1Fu;
				dst_hi |= 0x80u;
				dst_lo &= 0xF0u;
				gbc->hdma.source = cat_bytes(src_lo, src_hi);
				gbc->hdma.dest = cat_bytes(dst_lo, dst_hi);
				gbc->hdma.length = ((val & 0x7Fu) + 1u) * 0x10u;
				*dest = val;
				if (check_bit(val, 7)) {
					/* H-Blank DMA */
					gbc->hdma.hblank = true;
				} else {
					/* General DMA */
					gbc->hdma.to_copy = gbc->hdma.length;
					gbc->hdma.hblank = false;
				}
			}
			break;
		case RP:
			/* TODO: Handle */
			break;
		case BGPD:
			{
				uint8_t index = gbc->memory.ioreg[BGPI - IOREG_START];
				gbc->ppu.bgp[index & 0x3Fu] = val;
				if (check_bit(index, 7)) {
					index++;
					if ((index & 0x7Fu) == 0x40u) {
						index = bit(7);
					}
					gbc->memory.ioreg[BGPI - IOREG_START] = index;
				}
			}
			break;
		case OBPD:
			{
				uint8_t index = gbc->memory.ioreg[OBPI - IOREG_START];
				gbc->ppu.obp[index & 0x3Fu] = val;
				if (check_bit(index, 7)) {
					index++;
					if ((index & 0x7Fu) == 0x40u) {
						index = bit(7);
					}
					gbc->memory.ioreg[OBPI - IOREG_START] = index;
				}
			}
			break;
		case SVBK:
			{
				uint8_t bank = val & 0x07u;
				bank += !bank;
				*dest = bank;
				gbc->memory.wramx = gbc->memory.wram_bank[bank];
			}
			break;
		default:
			*dest = tmp | (uint8_t)(val & mask);
			break;
	}
}

uint8_t hram_read(struct gbc *gbc, uint16_t addr)
{
	return gbc->memory.hram[addr - HRAM_START];
}

void hram_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbc->memory.hram[addr - HRAM_START] = val;
}
