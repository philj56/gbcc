#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_mbc.h"
#include <stdio.h>

uint8_t gbcc_mbc_none_read(struct gbc *gbc, uint16_t addr) {
	if (addr < ROMX_START) {
		return gbc->memory.rom0[addr];
	}
	if (addr >= ROMX_START && addr < ROMX_END) {
		return gbc->memory.romx[addr - ROMX_START];
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		return gbc->memory.sram[addr - SRAM_START];
	}
	gbcc_log(GBCC_LOG_ERROR, "Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_none_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (gbc->cart.ram_size == 0) {
		gbcc_log(GBCC_LOG_DEBUG, "Trying to write to SRAM when there isn't any!\n", addr);
		return;
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		gbc->memory.sram[addr - SRAM_START] = val;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address 0x%04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mbc1_read(struct gbc *gbc, uint16_t addr) {
	if (addr < ROMX_START) {
		return gbc->memory.rom0[addr];
	}
	if (addr >= ROMX_START && addr < ROMX_END) {
		return gbc->memory.romx[addr - ROMX_START];
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (gbc->cart.mbc.sram_enable) {
			return gbc->memory.sram[addr - SRAM_START];
		}
		gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
	}
	gbcc_log(GBCC_LOG_ERROR, "Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_mbc1_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Au) == 0x0Au);
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romx_bank &= ~0x1Fu;
		gbc->cart.mbc.romx_bank |= val & 0x1Fu;
		gbc->cart.mbc.romx_bank += !(val & 0x1Fu);
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		if (gbc->cart.mbc.bank_mode == ROM) {
			gbc->cart.mbc.romx_bank &= ~0x60u;
			gbc->cart.mbc.romx_bank |= val & 0x60u;
			gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			if ((size_t)(gbc->memory.romx - gbc->memory.rom0) > gbc->cart.rom_size) {
				gbc->cart.mbc.romx_bank &= ~0x60u;
				gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			}
		} else {
			gbc->cart.mbc.sram_bank = val & 0x03u;
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		}
	} else if (addr < 0x8000u) {
		if (val & 0x01u) {
			gbc->cart.mbc.bank_mode = RAM;
			//gbc->cart.mbc.sram_bank = (gbc->cart.mbc.romx_bank & 0x60u) >> 4u; /* TODO: Are these upper 2 bits remembered? */
			gbc->cart.mbc.romx_bank &= ~0x60u; /* TODO: Are these upper 2 bits remembered? */
			gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		} else {
			gbc->cart.mbc.bank_mode = ROM;
			//gbc->cart.mbc.romx_bank = (uint8_t)(gbc->cart.mbc.sram_bank << 4u); /* TODO: Are these upper 2 bits remembered? */
			gbc->cart.mbc.sram_bank = 0x00u; /* TODO: Are these upper 2 bits remembered? */
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		}
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mbc2_read(struct gbc *gbc, uint16_t addr) {
	return 0xFFu;
}

void gbcc_mbc_mbc2_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}

uint8_t gbcc_mbc_mbc3_read(struct gbc *gbc, uint16_t addr) {
	if (addr < ROMX_START) {
		return gbc->memory.rom0[addr];
	}
	if (addr >= ROMX_START && addr < ROMX_END) {
		return gbc->memory.romx[addr - ROMX_START];
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (gbc->cart.mbc.sram_enable) {
			return gbc->memory.sram[addr - SRAM_START];
		}
		gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
	}
	gbcc_log(GBCC_LOG_ERROR, "Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_mbc3_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Au) == 0x0Au);
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romx_bank = val & 0x7Fu;
		gbc->cart.mbc.romx_bank += !(val & 0x7Fu);
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		gbc->cart.mbc.sram_bank = val & 0x03u;
		gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		if (val >= 0x08u && val <= 0x0Cu) {
			gbcc_log(GBCC_LOG_DEBUG, "TODO: RTC Register Select\n");
		}
	} else if (addr < 0x8000u) {
		gbcc_log(GBCC_LOG_DEBUG, "TODO: Latch Clock Data\n");
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mbc4_read(struct gbc *gbc, uint16_t addr) {
	return 0xFFu;
}

void gbcc_mbc_mbc4_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}

uint8_t gbcc_mbc_mbc5_read(struct gbc *gbc, uint16_t addr) {
	if (addr < ROMX_START) {
		return gbc->memory.rom0[addr];
	}
	if (addr >= ROMX_START && addr < ROMX_END) {
		return gbc->memory.romx[addr - ROMX_START];
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (gbc->cart.mbc.sram_enable) {
			return gbc->memory.sram[addr - SRAM_START];
		}
		gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
	}
	gbcc_log(GBCC_LOG_ERROR, "Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_mbc5_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Au) == 0x0Au);
	} else if (addr < 0x3000u) {
		gbc->cart.mbc.romx_bank &= ~0x00FFu;
		gbc->cart.mbc.romx_bank |= val;
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 4000u) {
		gbc->cart.mbc.romx_bank &= 0x0100u;
		gbc->cart.mbc.romx_bank |= (val & 0x01u) << 8u;
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		gbc->cart.mbc.sram_bank = val & 0x0Fu;
		gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mmm01_read(struct gbc *gbc, uint16_t addr) {
	return 0xFFu;
}

void gbcc_mbc_mmm01_write(struct gbc *gbc, uint16_t addr, uint8_t val) {
}
