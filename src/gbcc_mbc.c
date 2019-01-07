#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_mbc.h"
#include "gbcc_time.h"
#include <stdio.h>
#include <time.h>

static void update_mbc1_banks(struct gbc *gbc);

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
	gbcc_log_error("Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_none_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr >= SRAM_START && addr < SRAM_END && addr - SRAM_START < gbc->cart.ram_size) {
		gbc->memory.sram[addr - SRAM_START] = val;
	} else {
		gbcc_log_error("Writing memory address 0x%04X out of bounds.\n", addr);
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
			gbcc_log_debug("Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (gbc->cart.mbc.sram_enable) {
			return gbc->memory.sram[addr - SRAM_START];
		}
		gbcc_log_debug("SRAM not enabled!\n");
		return 0xFFu;
	}
	gbcc_log_error("Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_mbc1_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log_debug("Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log_debug("SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.ramg = val;
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romb0 = val;
	} else if (addr < 0x6000u) {
		gbc->cart.mbc.romb1 = val;
	} else if (addr < 0x8000u) {
		gbc->cart.mbc.ramb = val;
	} else {
		gbcc_log_error("Writing memory address %04X out of bounds.\n", addr);
		return;
	}
	update_mbc1_banks(gbc);
}

void update_mbc1_banks(struct gbc *gbc)
{
	struct gbcc_mbc *mbc = &gbc->cart.mbc;
	
	/* RAMG switches on SRAM when it has 0x0A in the lower 4 bits. */
	mbc->sram_enable = ((mbc->ramg & 0x0Fu) == 0x0Au);

	/* 
	 * ROMB0 selects lower 5 bits of ROMX bank number.
	 * 0 maps to 1, so banks 0x00, 0x20, 0x40 & 0x60 can
	 * never be mapped to ROMX.
	 */
	mbc->romx_bank = mbc->romb0 & 0x1Fu;
	mbc->romx_bank += !mbc->romx_bank;

	/* 
	 * ROMB1 selects either upper 2 bits of ROMX bank number
	 * or RAM bank number, depending on bit 0 of RAMB.
	 */
	if (!check_bit(mbc->ramb, 0)) {
		/* Mode 0 (ROM banking mode) */
		mbc->romx_bank |= (mbc->romb1 & 0x03u) << 5u;
		mbc->sram_bank = 0;
		mbc->rom0_bank = 0;
	} else {
		/* Mode 1 (RAM banking mode) */
		mbc->sram_bank = mbc->romb1 & 0x03u;
		/*
		 * Some weirdness here; in RAM banking mode, as well as
		 * controlling the RAM bank, ROMB1 moves ROM0 around!
		 */
		mbc->rom0_bank = (mbc->romb1 & 0x03u) << 5u;
		mbc->romx_bank |= (mbc->romb1 & 0x03u) << 5u;
	}

	/* 
	 * Perform some sanity checks on selected values,
	 * discarding bits that don't make sense.
	 */
	if (mbc->romx_bank > gbc->cart.rom_banks - 1) {
		gbcc_log_debug("Invalid rom bank %u.\n", mbc->romx_bank);
		mbc->rom0_bank &= (gbc->cart.rom_banks - 1);
		mbc->romx_bank &= (gbc->cart.rom_banks - 1);
	}
	if (mbc->sram_bank > gbc->cart.ram_banks - 1) {
		gbcc_log_debug("Invalid ram bank %u.\n", mbc->sram_bank);
		mbc->sram_bank &= (gbc->cart.ram_banks - 1);
	}

	/* And finally update the actual banks */
	gbc->memory.rom0 = gbc->cart.rom + mbc->rom0_bank * ROM0_SIZE;
	gbc->memory.romx = gbc->cart.rom + mbc->romx_bank * ROMX_SIZE;
	gbc->memory.sram = gbc->cart.ram + mbc->sram_bank * SRAM_SIZE;
}

uint8_t gbcc_mbc_mbc2_read(struct gbc *gbc, uint16_t addr)
{
	gbcc_log_debug("Stubbed function gbcc_mbc_mbc2_read() called");
	return 0xFFu;
}

void gbcc_mbc_mbc2_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbcc_log_debug("Stubbed function gbcc_mbc_mbc2_write() called");
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
					gbcc_log_error("Invalid rtc reg %u\n", rtc->cur_reg);
					return 0;
			}
		}
		if (gbc->cart.ram_size == 0) {
			gbcc_log_debug("Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (!gbc->cart.mbc.sram_enable) {
			gbcc_log_debug("SRAM not enabled!\n");
		}
		return gbc->memory.sram[addr - SRAM_START];
	}
	gbcc_log_error("Reading memory address 0x%04X out of bounds.\n", addr);
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
					gbcc_log_error("Invalid rtc reg %u\n", rtc->cur_reg);
			}
		} else if (gbc->cart.ram_size == 0) {
			gbcc_log_debug("Trying to write to SRAM when there isn't any!\n", addr);
		} else {
			if (!gbc->cart.mbc.sram_enable) {
				gbcc_log_debug("SRAM not enabled!\n");
			}
			gbc->memory.sram[addr - SRAM_START] = val;
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Au) == 0x0Au);
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romx_bank = val & 0x7Fu;
		gbc->cart.mbc.romx_bank += !(val & 0x7Fu);
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log_debug("Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		if (val < 0x04u) {
			gbc->cart.mbc.sram_bank = val & 0x03u;
			if (gbc->cart.mbc.sram_bank > gbc->cart.ram_banks) {
				gbcc_log_debug("Invalid ram bank %u.\n", gbc->cart.mbc.sram_bank);
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
		gbcc_log_error("Writing memory address %04X out of bounds.\n", addr);
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
			gbcc_log_debug("Trying to read SRAM when there isn't any!\n", addr);
			return 0xFFu;
		}
		if (gbc->cart.mbc.sram_enable) {
			return gbc->memory.sram[addr - SRAM_START];
		}
		gbcc_log_debug("SRAM not enabled!\n");
	}
	gbcc_log_error("Reading memory address 0x%04X out of bounds.\n", addr);
	return 0xFFu;
}

void gbcc_mbc_mbc5_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	if (addr >= SRAM_START && addr < SRAM_END) {
		if (gbc->cart.ram_size == 0) {
			gbcc_log_debug("Trying to write to SRAM when there isn't any!\n", addr);
		} else if (gbc->cart.mbc.sram_enable) {
			gbc->memory.sram[addr - SRAM_START] = val;
		} else {
			gbcc_log_debug("SRAM not enabled!\n");
		}
	} else if (addr < 0x2000u) {
		gbc->cart.mbc.sram_enable = ((val & 0x0Fu) == 0x0Au);
	} else if (addr < 0x3000u) {
		gbc->cart.mbc.romx_bank &= ~0x00FFu;
		gbc->cart.mbc.romx_bank |= val;
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log_debug("Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x4000u) {
		gbc->cart.mbc.romx_bank &= ~0x0100u;
		gbc->cart.mbc.romx_bank |= (val & 0x01u) << 8u;
		if (gbc->cart.mbc.romx_bank > gbc->cart.rom_banks) {
			gbcc_log_debug("Invalid rom bank %u.\n", gbc->cart.mbc.romx_bank);
			gbc->cart.mbc.romx_bank &= (gbc->cart.rom_banks - 1);
		}
		gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	} else if (addr < 0x6000u) {
		gbc->cart.mbc.sram_bank = val & 0x0Fu;
		if (gbc->cart.mbc.sram_bank > gbc->cart.ram_banks) {
			gbcc_log_debug("Invalid ram bank %u.\n", gbc->cart.mbc.sram_bank);
			gbc->cart.mbc.sram_bank &= (gbc->cart.ram_banks - 1);
		}
		gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
	} else {
		gbcc_log_error("Writing memory address %04X out of bounds.\n", addr);
	}
}

uint8_t gbcc_mbc_mmm01_read(struct gbc *gbc, uint16_t addr)
{
	gbcc_log_debug("Stubbed function gbcc_mbc_mmm01_read() called");
	return 0xFFu;
}

void gbcc_mbc_mmm01_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	gbcc_log_debug("Stubbed function gbcc_mbc_mmm01_write() called");
}
