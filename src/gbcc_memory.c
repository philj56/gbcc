#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_mbc.h"
#include "gbcc_memory.h"
#include <stdio.h>

static const uint8_t ioreg_read_masks[0x80] = {
/* 0xFF00 */	0x3F, 0xFF, 0x83, 0x00, 0xFF, 0xFF, 0xFF, 0x07,
/* 0xFF08 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
/* 0xFF10 */	0x7F, 0xFF, 0xFF, 0x00, 0x40, 0x00, 0xFF, 0xFF,
/* 0xFF18 */	0x00, 0x40, 0x80, 0xFF, 0x60, 0x00, 0x40, 0x00,
/* 0xFF20 */	0x3F, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, 0x8F, 0x00,
/* 0xFF28 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF30 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF38 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF40 */	0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
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
/* 0xFF48 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF,
/* 0xFF50 */	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC2, 0x00,
/* 0xFF58 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF60 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF68 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
/* 0xFF70 */	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF78 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t gbcc_vram_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_vram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_wram_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_wram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_echo_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_echo_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_oam_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_oam_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_unused_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_unused_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_ioreg_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

static uint8_t gbcc_hram_read(struct gbc *gbc, uint16_t addr, bool override);
static void gbcc_hram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override);

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
	/*if (!override && gbc->dma.timer > 0) {
		if (!(addr >= HRAM_START && addr < HRAM_END)) {
			return 0xFFu;
		}
	}*/
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
			case MBC4:
				return gbcc_mbc_mbc4_read(gbc, addr);
			case MBC5:
				return gbcc_mbc_mbc5_read(gbc, addr);
			case MMM01:
				return gbcc_mbc_mmm01_read(gbc, addr);
		}
	}
	if (addr >= VRAM_START && addr < VRAM_END) {
		return gbcc_vram_read(gbc, addr, override);
	}
	if (addr >= WRAM0_START && addr < WRAMX_END) {
		return gbcc_wram_read(gbc, addr, override);
	}
	if (addr >= ECHO_START && addr < ECHO_END) {
		return gbcc_echo_read(gbc, addr, override);
	}
	if (addr >= OAM_START && addr < OAM_END) {
		return gbcc_oam_read(gbc, addr, override);
	}
	if (addr >= UNUSED_START && addr < UNUSED_END) {
		return gbcc_unused_read(gbc, addr, override);
	}
	if (addr >= IOREG_START && addr < IOREG_END) {
		return gbcc_ioreg_read(gbc, addr, override);
	}
	if (addr >= HRAM_START && addr < HRAM_END) {
		return gbcc_hram_read(gbc, addr, override);
	}
	if (addr == IE) {
		return gbc->memory.iereg;
	}
	gbcc_log(GBCC_LOG_ERROR, "Reading from unknown memory address %04X.\n", addr);
	return 0;
}

void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	/*if (!override && gbc->dma.timer > 0) {
		if (!(addr >= HRAM_START && addr < HRAM_END)) {
			return;
		}
	}*/
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
			case MBC4:
				gbcc_mbc_mbc4_write(gbc, addr, val);
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
		gbcc_vram_write(gbc, addr, val, override);
	} else if (addr >= WRAM0_START && addr < WRAMX_END) {
		gbcc_wram_write(gbc, addr, val, override);
	} else if (addr >= ECHO_START && addr < ECHO_END) {
		gbcc_echo_write(gbc, addr, val, override);
	} else if (addr >= OAM_START && addr < OAM_END) {
		gbcc_oam_write(gbc, addr, val, override);
	} else if (addr >= UNUSED_START && addr < UNUSED_END) {
		gbcc_unused_write(gbc, addr, val, override);
	} else if (addr >= IOREG_START && addr < IOREG_END) {
		gbcc_ioreg_write(gbc, addr, val, override);
	} else if (addr >= HRAM_START && addr < HRAM_END) {
		gbcc_hram_write(gbc, addr, val, override);
	} else if (addr == IE) {
		gbc->memory.iereg = val;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing to unknown memory address %04X.\n", addr);
	}
}

uint8_t gbcc_vram_read(struct gbc *gbc, uint16_t addr, bool override)
{
	return gbc->memory.vram[addr - VRAM_START];
}

void gbcc_vram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	gbc->memory.vram[addr - VRAM_START] = val;
}

uint8_t gbcc_wram_read(struct gbc *gbc, uint16_t addr, bool override)
{
	if (addr < WRAMX_START) {
		return gbc->memory.wram0[addr - WRAM0_START];
	}
	return gbc->memory.wramx[addr - WRAMX_START];
}

void gbcc_wram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	if (addr < WRAMX_START) {
		gbc->memory.wram0[addr - WRAM0_START] = val;
	} else {
		gbc->memory.wramx[addr - WRAMX_START] = val;
	}
}

uint8_t gbcc_echo_read(struct gbc *gbc, uint16_t addr, bool override)
{
	return gbcc_wram_read(gbc, addr - WRAMX_SIZE - WRAM0_SIZE, override);
}

void gbcc_echo_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	gbcc_wram_write(gbc, addr - WRAMX_SIZE - WRAM0_SIZE, val, override);
}

uint8_t gbcc_oam_read(struct gbc *gbc, uint16_t addr, bool override)
{
	return gbc->memory.oam[addr - OAM_START];
}

void gbcc_oam_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	gbc->memory.oam[addr - OAM_START] = val;
}

uint8_t gbcc_unused_read(struct gbc *gbc, uint16_t addr, bool override)
{
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	return gbc->memory.unused[addr - UNUSED_START - offset];
}

void gbcc_unused_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	gbc->memory.unused[addr - UNUSED_START - offset] = val;
}

uint8_t gbcc_ioreg_read(struct gbc *gbc, uint16_t addr, bool override)
{
	if (override) {
		return gbc->memory.ioreg[addr - IOREG_START];
	}
	uint8_t mask = ioreg_read_masks[addr - IOREG_START];
	/* Only update the keys when we actually want to read from them */
	if (addr == JOYP) {
		uint8_t joyp = gbc->memory.ioreg[addr - IOREG_START];
		if (check_bit(gbc->memory.ioreg[addr - IOREG_START], 5)) {
			joyp &= ~(uint8_t)(gbc->keys.start << 3u);
			joyp &= ~(uint8_t)(gbc->keys.select << 2u);
			joyp &= ~(uint8_t)(gbc->keys.b << 1u);
			joyp &= ~(uint8_t)(gbc->keys.a << 0u);
		}
		if (gbc->memory.ioreg[addr - IOREG_START] & (1u << 4u)) {
			joyp &= ~(uint8_t)(gbc->keys.dpad.down << 3u);
			joyp &= ~(uint8_t)(gbc->keys.dpad.up << 2u);
			joyp &= ~(uint8_t)(gbc->keys.dpad.left << 1u);
			joyp &= ~(uint8_t)(gbc->keys.dpad.right << 0u);
		}
		return joyp | (uint8_t)~mask;
	} 
	if (gbc->mode == GBC && addr == BGPD) {
		return gbc->memory.bgp[gbc->memory.ioreg[BGPI - IOREG_START]];
	}
	if (gbc->mode == GBC && addr == OBPD) {
		return gbc->memory.obp[gbc->memory.ioreg[OBPI - IOREG_START]];
	}
	/* GBC-specific registers should return 0xFF when in DMG mode */
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
		}
	}
	return gbc->memory.ioreg[addr - IOREG_START] | (uint8_t)~mask;
}

void gbcc_ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	if (override) {
		gbc->memory.ioreg[addr - IOREG_START] = val;
		return;
	}
	uint8_t tmp = gbc->memory.ioreg[addr - IOREG_START];
	uint8_t mask = ioreg_write_masks[addr - IOREG_START];
	if (addr == SC) {
		if (val & 0x80u) {
			//fprintf(stderr, "%c", gbc->memory.ioreg[SB - IOREG_START]);
		}
	} else if (addr == DIV) {
		gbc->memory.ioreg[addr - IOREG_START] = 0;
		gbc->div_timer = 0;
	} else if (addr == DMA) {
		gbc->dma.new_source = (uint16_t)(((uint16_t)val) << 8u);
		gbc->dma.requested = true;
	} else if (addr == JOYP) {
		if (val & (1u << 5u)) {
			gbc->memory.ioreg[addr - IOREG_START] |= 0x10u;
			gbc->memory.ioreg[addr - IOREG_START] &= ~0x20u;
		} else if (val & (1u << 4u)) {
			gbc->memory.ioreg[addr - IOREG_START] |= 0x20u;
			gbc->memory.ioreg[addr - IOREG_START] &= ~0x10u;
		}
	} else if (gbc->mode == GBC && addr == BGPD) {
		uint8_t index = gbc->memory.ioreg[BGPI - IOREG_START];
		gbc->memory.bgp[index & 0x3Fu] = val;
		if (index & 0x80u) {
			index++;
			if (index == 0x40u) {
				index = 0;
			}
			gbc->memory.ioreg[BGPI - IOREG_START] = index;
		}
	} else if (gbc->mode == GBC && addr == OBPD) {
		uint8_t index = gbc->memory.ioreg[OBPI - IOREG_START];
		gbc->memory.obp[index & 0x3Fu] = val;
		if (index & 0x80u) {
			index++;
			if (index == 0x40u) {
				index = 0;
			}
			gbc->memory.ioreg[OBPI - IOREG_START] = index;
		}
	} else if (gbc->mode == GBC && addr == VBK) {
		//printf("VBK = %u\n", val);
		gbc->memory.ioreg[addr - IOREG_START] = val & 0x01u;
		if (val & 0x01u) {
			gbc->memory.vram = gbc->memory.vram_bank[1];
		} else {
			gbc->memory.vram = gbc->memory.vram_bank[0];
		}
	} else if (gbc->mode == GBC && addr == SVBK) {
		//printf("SVBK = %u\n", val & 0x07u);
		uint8_t bank = val & 0x07u;
		bank += !bank;
		gbc->memory.ioreg[addr - IOREG_START] = bank;
		gbc->memory.wramx = gbc->memory.wram_bank[bank];
	} else if (gbc->mode == GBC && addr == HDMA1) {
		if (val < 0x80u || (val >= 0xA0u && val < 0xE0u)) {
			gbc->memory.ioreg[addr - IOREG_START] = val;
		}
	} else if (gbc->mode == GBC && addr == HDMA2) {
		gbc->memory.ioreg[addr - IOREG_START] = val & 0xF0u;
	} else if (gbc->mode == GBC && addr == HDMA3) {
		gbc->memory.ioreg[addr - IOREG_START] = (val & 0x1Fu) | 0x80u;
	} else if (gbc->mode == GBC && addr == HDMA4) {
		gbc->memory.ioreg[addr - IOREG_START] = val & 0xF0u;
	} else if (gbc->mode == GBC && addr == HDMA5) {
		uint8_t src_hi = gbc->memory.ioreg[HDMA1 - IOREG_START];
		uint8_t src_lo = gbc->memory.ioreg[HDMA2 - IOREG_START];
		uint8_t dst_hi = gbc->memory.ioreg[HDMA3 - IOREG_START];
		uint8_t dst_lo = gbc->memory.ioreg[HDMA4 - IOREG_START];
		gbc->hdma.source = cat_bytes(src_lo, src_hi);
		gbc->hdma.dest = cat_bytes(dst_lo, dst_hi);
		gbc->hdma.length = ((val & 0x7Fu) + 1u) * 0x10u;
		if (check_bit(val, 7)) {
			gbcc_log(GBCC_LOG_DEBUG, "TODO: H-Blank DMA\n");
		}
	} else if (gbc->mode == GBC && addr == KEY1) {
		gbc->memory.ioreg[addr - IOREG_START] = val & 0x01u;
	} else {
		gbc->memory.ioreg[addr - IOREG_START] = (uint8_t)(tmp & (uint8_t)(~mask)) | (uint8_t)(val & mask);
	}
}

uint8_t gbcc_hram_read(struct gbc *gbc, uint16_t addr, bool override)
{
	return gbc->memory.hram[addr - HRAM_START];
}

void gbcc_hram_write(struct gbc *gbc, uint16_t addr, uint8_t val, bool override)
{
	/*
	if (addr == 0xFFC8u) {
		printf("Writing %u to 0x%04X\n", val, addr);
	}
	*/
	gbc->memory.hram[addr - HRAM_START] = val;
}
