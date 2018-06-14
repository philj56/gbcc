#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_mbc.h"
#include "gbcc_memory.h"
#include <stdio.h>

static const uint8_t ioreg_read_masks[0x80] = {
/* 0xFF00 */	0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF08 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
/* 0xFF10 */	0xFF, 0xFF, 0xFF, 0x00, 0x40, 0x00, 0xFF, 0xFF,
/* 0xFF18 */	0x00, 0x40, 0xFF, 0xFF, 0xFF, 0x00, 0x40, 0x00,
/* 0xFF20 */	0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, 0x8F, 0x00,
/* 0xFF28 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF30 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF38 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF40 */	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
/* 0xFF48 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0xFF,
/* 0xFF50 */	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x00,
/* 0xFF58 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF60 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF68 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
/* 0xFF70 */	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF78 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t ioreg_write_masks[0x80] = {
/* 0xFF00 */	0x30, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFF08 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
/* 0xFF10 */	0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x00, 0xFF, 0xFF,
/* 0xFF18 */	0xFF, 0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x00,
/* 0xFF20 */	0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0x80, 0x00,
/* 0xFF28 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF30 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF38 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF40 */	0xFF, 0xF8, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
/* 0xFF48 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF,
/* 0xFF50 */	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC2, 0x00,
/* 0xFF58 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF60 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF68 */	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
/* 0xFF70 */	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xFF78 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t gbcc_vram_read(struct gbc *gbc, uint16_t addr);
void gbcc_vram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_wram_read(struct gbc *gbc, uint16_t addr);
void gbcc_wram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_echo_read(struct gbc *gbc, uint16_t addr);
void gbcc_echo_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_oam_read(struct gbc *gbc, uint16_t addr);
void gbcc_oam_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_unused_read(struct gbc *gbc, uint16_t addr);
void gbcc_unused_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_ioreg_read(struct gbc *gbc, uint16_t addr);
void gbcc_ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val);

uint8_t gbcc_hram_read(struct gbc *gbc, uint16_t addr);
void gbcc_hram_write(struct gbc *gbc, uint16_t addr, uint8_t val);

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
		//printf("Reading from address %04X (VRAM).\n", addr);
		return gbcc_vram_read(gbc, addr);
	}
	if (addr >= WRAM0_START && addr < WRAMX_END) {
		//printf("Reading from address %04X (WRAM).\n", addr);
		return gbcc_wram_read(gbc, addr);
	}
	if (addr >= ECHO_START && addr < ECHO_END) {
		//printf("Reading from address %04X (ECHO).\n", addr);
		return gbcc_echo_read(gbc, addr);
	}
	if (addr >= OAM_START && addr < OAM_END) {
		//printf("Reading from address %04X (OAM).\n", addr);
		return gbcc_oam_read(gbc, addr);
	}
	if (addr >= UNUSED_START && addr < UNUSED_END) {
		//printf("Reading from address %04X (UNUSED).\n", addr);
		return gbcc_unused_read(gbc, addr);
	}
	if (addr >= IOREG_START && addr < IOREG_END) {
		//printf("Reading from address %04X (IOREG).\n", addr);
		return gbcc_ioreg_read(gbc, addr);
	}
	if (addr >= HRAM_START && addr < HRAM_END) {
		//printf("Reading from address %04X (HRAM).\n", addr);
		return gbcc_hram_read(gbc, addr);
	}
	if (addr == IE) {
		//printf("Reading from address %04X (IE).\n", addr);
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
	//printf("Writing %02X to address %04X\n", val, addr);
	if (addr < ROMX_END || (addr > SRAM_START && addr < SRAM_END)) {
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
		gbcc_vram_write(gbc, addr, val);
		//printf("(VRAM).");
	} else if (addr >= WRAM0_START && addr < WRAMX_END) {
		gbcc_wram_write(gbc, addr, val);
		//printf("(WRAM).");
	} else if (addr >= ECHO_START && addr < ECHO_END) {
		gbcc_echo_write(gbc, addr, val);
		//printf("(ECHO).");
	} else if (addr >= OAM_START && addr < OAM_END) {
		gbcc_oam_write(gbc, addr, val);
		//printf("(OAM).");
	} else if (addr >= UNUSED_START && addr < UNUSED_END) {
		gbcc_unused_write(gbc, addr, val);
		//printf("(UNUSED).");
	} else if (addr >= IOREG_START && addr < IOREG_END) {
		gbcc_ioreg_write(gbc, addr, val);
		//printf("(IOREG).");
	} else if (addr >= HRAM_START && addr < HRAM_END) {
		gbcc_hram_write(gbc, addr, val);
		//printf("(HRAM).");
	} else if (addr == IE) {
		gbc->memory.iereg = val;
		//printf("(IE).");
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing to unknown memory address %04X.\n", addr);
	}
	//printf("\n");
}

uint8_t gbcc_vram_read(struct gbc *gbc, uint16_t addr) {
	return gbc->memory.vram[addr - VRAM_START];
}

void gbcc_vram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	gbc->memory.vram[addr - VRAM_START] = val;
}

uint8_t gbcc_wram_read(struct gbc *gbc, uint16_t addr) {
	if (addr < WRAMX_START) {
		return gbc->memory.wram0[addr - WRAM0_START];
	}
	return gbc->memory.wramx[addr - WRAMX_START];
}

void gbcc_wram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	if (addr < WRAMX_START) {
		gbc->memory.wram0[addr - WRAM0_START] = val;
	} else {
		gbc->memory.wramx[addr - WRAMX_START] = val;
	}
}

uint8_t gbcc_echo_read(struct gbc *gbc, uint16_t addr) {
	return gbcc_wram_read(gbc, addr - WRAMX_SIZE - WRAM0_SIZE);
}

void gbcc_echo_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	gbcc_wram_write(gbc, addr - WRAMX_SIZE - WRAM0_SIZE, val);
}

uint8_t gbcc_oam_read(struct gbc *gbc, uint16_t addr) {
	return gbc->memory.oam[addr - OAM_START];
}

void gbcc_oam_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	gbc->memory.oam[addr - OAM_START] = val;
}

uint8_t gbcc_unused_read(struct gbc *gbc, uint16_t addr) {
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	return gbc->memory.unused[addr - UNUSED_START - offset];
}

void gbcc_unused_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	uint16_t offset;
	if (addr < 0xFED0u) {
		offset = 0;
	} else {
		offset = addr - 0xFED0u;
	}
	gbc->memory.unused[addr - UNUSED_START - offset] = val;
}

uint8_t gbcc_ioreg_read(struct gbc *gbc, uint16_t addr) {
	uint8_t mask = ioreg_read_masks[addr - IOREG_START];
	/* Only update the keys when we actually want to read from them */
	if (addr == JOYP) {
		uint8_t joyp = gbc->memory.ioreg[addr - IOREG_START];
		if (gbc->memory.ioreg[addr - IOREG_START] & (1u << 5u)) {
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
		return joyp;
		gbc->memory.ioreg[addr - IOREG_START] = joyp;
		gbcc_log(GBCC_LOG_DEBUG, "Joypad returned %02X\n", joyp);
	} 
	return gbc->memory.ioreg[addr - IOREG_START] | (uint8_t)~mask;
}

void gbcc_ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	uint8_t tmp = gbc->memory.ioreg[addr - IOREG_START];
	uint8_t mask = ioreg_write_masks[addr - IOREG_START];
	if (addr == SC) {
		if (val & 0x80u) {
			fprintf(stderr, "%c", gbc->memory.ioreg[SB - IOREG_START]);
		}
	} else if (addr == DIV) {
		gbc->memory.ioreg[addr - IOREG_START] = 0;
	} else if (addr == DMA) {
		gbc->dma.source = (uint16_t)(((uint16_t)val) << 8u);
		gbc->dma.timer = DMA_TIMER;
		gbcc_log(GBCC_LOG_DEBUG, "DMA requested from 0x%04X\n", gbc->dma.source);
	} else if (addr == JOYP) {
		if (val & (1u << 5u)) {
			gbc->memory.ioreg[addr - IOREG_START] |= 0x10u;
			gbc->memory.ioreg[addr - IOREG_START] &= ~0x20u;
		} else if (val & (1u << 4u)) {
			gbc->memory.ioreg[addr - IOREG_START] |= 0x20u;
			gbc->memory.ioreg[addr - IOREG_START] &= ~0x10u;
		}
	} else {
		gbc->memory.ioreg[addr - IOREG_START] = (uint8_t)(tmp & (uint8_t)(~mask)) | (uint8_t)(val & mask);
	}
}

uint8_t gbcc_hram_read(struct gbc *gbc, uint16_t addr) {
	return gbc->memory.hram[addr - HRAM_START];
}

void gbcc_hram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	gbc->memory.hram[addr - HRAM_START] = val;
}
