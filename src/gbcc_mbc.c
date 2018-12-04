#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_mbc.h"
#include "gbcc_time.h"
#include <stdio.h>
#include <time.h>

uint8_t gbcc_mbc_none_read(struct gbc *gbc, uint16_t addr)
{
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

uint8_t gbcc_mbc_mbc1_read(struct gbc *gbc, uint16_t addr)
{
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

void gbcc_mbc_mbc1_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
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
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log(GBCC_LOG_DEBUG, "Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		if (gbc->cart.mbc.bank_mode == ROM) {
			gbc->cart.mbc.romx_bank &= ~0x60u;
			gbc->cart.mbc.romx_bank |= val & 0x60u;
			if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
				gbcc_log(GBCC_LOG_DEBUG, "Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
				gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
			}
			gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			if ((size_t)(gbc->memory.romx - gbc->memory.rom0) > gbc->cart.rom_size) {
				gbc->cart.mbc.romx_bank &= ~0x60u;
				gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			}
		} else {
			gbc->cart.mbc.sram_bank = val & 0x03u;
			if (gbc->cart.mbc.sram_bank > gbc->cart.ram_banks) {
				gbcc_log(GBCC_LOG_DEBUG, "Invalid ram bank %u.\n", gbc->cart.mbc.sram_bank);
				gbc->cart.mbc.sram_bank &= (gbc->cart.ram_banks - 1);
			}
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		}
	} else if (addr < 0x8000u) {
		if (val & 0x01u) {
			gbc->cart.mbc.bank_mode = RAM;
			gbc->cart.mbc.sram_bank = (gbc->cart.mbc.romx_bank & 0x60u) >> 4u; /* TODO: Are these upper 2 bits remembered? */
			gbc->cart.mbc.romx_bank &= ~0x60u; /* TODO: Are these upper 2 bits remembered? */
			gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		} else {
			gbc->cart.mbc.bank_mode = ROM;
			gbc->cart.mbc.romx_bank = (uint8_t)(gbc->cart.mbc.sram_bank << 4u); /* TODO: Are these upper 2 bits remembered? */
			gbc->cart.mbc.sram_bank = 0x00u; /* TODO: Are these upper 2 bits remembered? */
			gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		}
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mbc2_read(struct gbc *gbc, uint16_t addr)
{
	gbcc_log(GBCC_LOG_DEBUG, "Stubbed function gbcc_mbc_mbc2_read() called");
	return 0xFFu;
}

void gbcc_mbc_mbc2_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbcc_log(GBCC_LOG_DEBUG, "Stubbed function gbcc_mbc_mbc2_write() called");
}

uint8_t gbcc_mbc_mbc3_read(struct gbc *gbc, uint16_t addr)
{
	struct gbcc_rtc *rtc = &gbc->cart.mbc.rtc;
	if (addr < ROMX_START) {
		return gbc->memory.rom0[addr];
	}
	if (addr >= ROMX_START && addr < ROMX_END) {
		return gbc->memory.romx[addr - ROMX_START];
	}
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (rtc->mapped) {
			switch (rtc->cur_reg) {
				case 0:
					return rtc->seconds;
				case 1:
					return rtc->minutes;
				case 2:
					return rtc->hours;
				case 3:
					return rtc->day_low;
				case 4:
					return rtc->day_high;
				default:
					gbcc_log(GBCC_LOG_ERROR, "Invalid rtc reg %u\n", rtc->cur_reg);
					return 0;
			}
		}
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

void gbcc_mbc_mbc3_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	struct gbcc_rtc *rtc = &gbc->cart.mbc.rtc;
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (rtc->mapped) {
			switch (rtc->cur_reg) {
				case 0:
					rtc->seconds = val;
				case 1:
					rtc->minutes = val;
				case 2:
					rtc->hours = val;
				case 3:
					rtc->day_low = val;
				case 4:
					rtc->day_high = val;
				default:
					gbcc_log(GBCC_LOG_ERROR, "Invalid rtc reg %u\n", rtc->cur_reg);
			}
		} else if (gbc->cart.ram_size == 0) {
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
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log(GBCC_LOG_DEBUG, "Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		if (val < 0x04u) {
			gbc->cart.mbc.sram_bank = val & 0x03u;
			if (gbc->cart.mbc.sram_bank > gbc->cart.ram_banks) {
				gbcc_log(GBCC_LOG_DEBUG, "Invalid ram bank %u.\n", gbc->cart.mbc.sram_bank);
				gbc->cart.mbc.sram_bank &= (gbc->cart.ram_banks - 1);
			}
			gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
		} else if (val >= 0x08u && val <= 0x0Cu) {
			rtc->cur_reg = val - 0x08u;
		}
	} else if (addr < 0x8000u) {
		if (rtc->latch != 0  || val != 0x01u) {
			rtc->latch = val;
			return;
		}
		rtc->latch = val;
		uint64_t diff;
		struct timespec cur_time;
		rtc->base_time.tv_sec -= rtc->seconds;
		rtc->base_time.tv_sec -= rtc->minutes * (MINUTE / SECOND);
		rtc->base_time.tv_sec -= rtc->hours * (HOUR / SECOND);
		rtc->base_time.tv_sec -= rtc->day_low * (DAY / SECOND);
		rtc->base_time.tv_sec -= ((rtc->day_high & 0x01u) << 8u) * (DAY / SECOND);
		clock_gettime(CLOCK_REALTIME, &cur_time);
		diff = gbcc_time_diff(&cur_time, &gbc->cart.mbc.rtc.base_time);
		rtc->seconds = (diff / SECOND) % (MINUTE / SECOND);
		rtc->minutes = (diff / MINUTE) % (HOUR / MINUTE);
		rtc->hours = (diff / HOUR) % (DAY / HOUR);
		rtc->day_low = (diff / DAY) & 0xFFu;
		rtc->day_high &= (uint8_t)(~bit(0));
		rtc->day_high |= ((diff / DAY) >> 8u) & bit(0);
		if (diff / DAY > 0x100u) {
			rtc->day_high |= bit(7);
		}
		rtc->base_time = cur_time;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mbc5_read(struct gbc *gbc, uint16_t addr)
{
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

void gbcc_mbc_mbc5_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log(GBCC_LOG_DEBUG, "Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log(GBCC_LOG_DEBUG, "SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Fu) == 0x0Au);
	} else if (addr < 0x3000u) {
		gbc->cart.mbc.romx_bank &= ~0x00FFu;
		gbc->cart.mbc.romx_bank |= val;
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log(GBCC_LOG_DEBUG, "Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romx_bank &= ~0x0100u;
		gbc->cart.mbc.romx_bank |= (val & 0x01u) << 8u;
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log(GBCC_LOG_DEBUG, "Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		gbc->cart.mbc.sram_bank = val & 0x0Fu;
		if (gbc->cart.mbc.sram_bank > gbc->cart.ram_banks) {
			gbcc_log(GBCC_LOG_DEBUG, "Invalid ram bank %u.\n", gbc->cart.mbc.sram_bank);
			gbc->cart.mbc.sram_bank &= (gbc->cart.ram_banks - 1);
		}
		gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
	} else {
		gbcc_log(GBCC_LOG_ERROR, "Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mmm01_read(struct gbc *gbc, uint16_t addr)
{
	gbcc_log(GBCC_LOG_DEBUG, "Stubbed function gbcc_mbc_mmm01_read() called");
	return 0xFFu;
}

void gbcc_mbc_mmm01_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbcc_log(GBCC_LOG_DEBUG, "Stubbed function gbcc_mbc_mmm01_write() called");
}
