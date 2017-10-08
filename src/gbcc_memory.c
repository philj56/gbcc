#include "gbcc_memory.h"
#include "gbcc_mbc.h"

const uint8_t ioreg_read_masks[0x80] = {
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

const uint8_t ioreg_write_masks[0x80] = {
/* 0xFF00 */	0xF0, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
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

uint8_t gbcc_memory_read(struct gbc *gbc, uint16_t addr)
{
	if (addr < ROMX_START + ROMX_SIZE 
			|| (addr >= SRAM_START && addr < SRAM_START + SRAM_SIZE)) {
		switch (gbc->cart.mbc) {
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
	if (addr >= VRAM_START && addr < SRAM_START) {
		return gbcc_vram_read(gbc, addr);
	} else if (addr >= WRAM0_START && addr < ECHO_START) {
		return gbcc_wram_read(gbc, addr);
	} else if (addr >= ECHO_START && addr < OAM_START) {
		return gbcc_echo_read(gbc, addr);
	} else if (addr >= OAM_START && addr < UNUSED_START) {
		return gbcc_oam_read(gbc, addr);
	} else if (addr >= UNUSED_START && addr < IOREG_START) {
		return gbcc_unused_read(gbc, addr);
	} else if (addr >= IOREG_START && addr < HRAM_START) {
		return gbcc_ioreg_read(gbc, addr);
	} else if (addr >= HRAM_START && addr < IE) {
		return gbcc_hram_read(gbc, addr);
	} else if (addr == IE) {
		return gbc->memory.iereg;
	}
	return 0;
}

void gbcc_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr < ROMX_START + ROMX_SIZE 
			|| (addr > SRAM_START && addr < SRAM_START + SRAM_SIZE)) {
		switch (gbc->cart.mbc) {
			case NONE:
				return;
			case MBC1:
				gbcc_mbc_mbc1_write(gbc, addr, val);
			case MBC2:
				gbcc_mbc_mbc2_write(gbc, addr, val);
			case MBC3:
				gbcc_mbc_mbc3_write(gbc, addr, val);
			case MBC4:
				gbcc_mbc_mbc4_write(gbc, addr, val);
			case MBC5:
				gbcc_mbc_mbc5_write(gbc, addr, val);
			case MMM01:
				gbcc_mbc_mmm01_write(gbc, addr, val);
		}
	}
	if (addr >= VRAM_START && addr < SRAM_START) {
		gbcc_vram_write(gbc, addr, val);
	} else if (addr >= WRAM0_START && addr < ECHO_START) {
		gbcc_wram_write(gbc, addr, val);
	} else if (addr >= ECHO_START && addr < OAM_START) {
		gbcc_echo_write(gbc, addr, val);
	} else if (addr >= OAM_START && addr < UNUSED_START) {
		gbcc_oam_write(gbc, addr, val);
	} else if (addr >= UNUSED_START && addr < IOREG_START) {
		gbcc_unused_write(gbc, addr, val);
	} else if (addr >= IOREG_START && addr < HRAM_START) {
		gbcc_ioreg_write(gbc, addr, val);
	} else if (addr >= HRAM_START && addr < IE) {
		gbcc_hram_write(gbc, addr, val);
	} else if (addr == IE) {
		gbc->memory.iereg = val;
	}
}

uint8_t gbcc_vram_read(struct gbc *gbc, uint16_t addr) {
	uint16_t offset = 0;
	if ((gbc->mode == GBC) && (gbcc_ioreg_read(gbc, VBK) & 1)) {
		offset = VRAM_SIZE;
	}
	return gbc->memory.emu_vram[addr - VRAM_START + offset];
}

void gbcc_vram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	uint16_t offset = 0;
	if ((gbc->mode == GBC) && (gbcc_ioreg_read(gbc, VBK) & 1)) {
		offset = VRAM_SIZE;
	}
	gbc->memory.emu_vram[addr - VRAM_START + offset] = val;
}

uint8_t gbcc_wram_read(struct gbc *gbc, uint16_t addr) {
	if (addr < WRAMX_START) {
		return gbc->memory.emu_wram[addr - WRAM0_START];
	}
	uint16_t offset = gbcc_ioreg_read(gbc, SVBK) & 0x07;
	offset += !offset;
	offset *= WRAMX_SIZE;
	return gbc->memory.emu_wram[addr - WRAM0_START + offset];
}

void gbcc_wram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	if (addr < WRAMX_START) {
		gbc->memory.emu_wram[addr - WRAM0_START] = val;
	}
	uint16_t offset = gbcc_ioreg_read(gbc, SVBK) & 0x07;
	offset += !offset;
	offset *= WRAMX_SIZE;
	gbc->memory.emu_wram[addr - WRAM0_START + offset] = val;
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
	return gbc->memory.ioreg[addr - IOREG_START] | ~mask;
}

void gbcc_ioreg_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	uint8_t tmp = gbc->memory.ioreg[addr - IOREG_START];
	uint8_t mask = ioreg_write_masks[addr - IOREG_START];
	gbc->memory.ioreg[addr - IOREG_START] = (tmp & ~mask) | (val & mask);
}

uint8_t gbcc_hram_read(struct gbc *gbc, uint16_t addr) {
	return gbc->memory.hram[addr - HRAM_START];
}

void gbcc_hram_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	gbc->memory.hram[addr - HRAM_START] = val;
}
