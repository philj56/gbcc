#include "gbcc.h"
#include "debug.h"
#include "memory.h"
#include "save.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MAX_NAME_LEN 4096

static void strip_ext(char *fname);

void gbcc_save(struct gbc *gbc)
{
	if (gbc->cart.ram_size == 0 && gbc->cart.mbc.type != MBC7) {
		return;
	}
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	gbcc_log_info("Saving %s...\n", fname);
	sav = fopen(fname, "wbe");
	if (sav == NULL) {
		gbcc_log_error("Can't open save file %s\n", fname);
		return;
	}
	fwrite(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	if (gbc->cart.mbc.type == MBC7) {
		fwrite(gbc->cart.mbc.eeprom.data, 2, 128, sav);
	}
	if (gbc->cart.mbc.type == MBC3) {
		fprintf(sav, "\n%u:%u:%u:%u:%u:%u:%u:%lu:%lu\n",
				gbc->cart.mbc.rtc.seconds,
				gbc->cart.mbc.rtc.minutes,
				gbc->cart.mbc.rtc.hours,
				gbc->cart.mbc.rtc.day_low,
				gbc->cart.mbc.rtc.day_high,
				gbc->cart.mbc.rtc.latch,
				gbc->cart.mbc.rtc.cur_reg,
				gbc->cart.mbc.rtc.base_time.tv_sec,
				gbc->cart.mbc.rtc.base_time.tv_nsec
				);
	}
	fclose(sav);
	gbcc_log_info("Saved.\n");
}

void gbcc_load(struct gbc *gbc)
{
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (sav == NULL) {
		if (gbc->cart.mbc.type == MBC3) {
			clock_gettime(CLOCK_REALTIME, &gbc->cart.mbc.rtc.base_time);
		}
		return;
	}
	gbcc_log_info("Loading %s...\n", fname);
	fread(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	if (gbc->cart.mbc.type == MBC7) {
		fread(gbc->cart.mbc.eeprom.data, 2, 128, sav);
	}
	if (gbc->cart.mbc.type == MBC3) {
		int matched;
		matched = fscanf(sav, "\n%" SCNu8 ":%" SCNu8 ":%" SCNu8 ":%"
				SCNu8 ":%" SCNu8 ":%" SCNu8 ":%" SCNu8 ":%lu:%lu",
				&gbc->cart.mbc.rtc.seconds,
				&gbc->cart.mbc.rtc.minutes,
				&gbc->cart.mbc.rtc.hours,
				&gbc->cart.mbc.rtc.day_low,
				&gbc->cart.mbc.rtc.day_high,
				&gbc->cart.mbc.rtc.latch,
				&gbc->cart.mbc.rtc.cur_reg,
				&gbc->cart.mbc.rtc.base_time.tv_sec,
				&gbc->cart.mbc.rtc.base_time.tv_nsec
				);
		if (matched < 9) {
			gbcc_log_error("Couldn't read rtc data, "
					"resetting base time to now.\n");
			clock_gettime(CLOCK_REALTIME, &gbc->cart.mbc.rtc.base_time);
		}
	}
	fclose(sav);
}

void gbcc_save_state(struct gbc *gbc)
{
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.s%d", tmp, gbc->save_state) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	sav = fopen(fname, "wbe");
	if (!sav) {
		gbcc_log_error("Error opening %s: %s\n", fname, strerror(errno));
		return;
	}
	gbc->save_state = 0;
	gbc->load_state = 0;
	fwrite(gbc, sizeof(struct gbc), 1, sav);
	if (gbc->cart.ram_size > 0) {
		fwrite(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	}
	fclose(sav);
	gbcc_log_info("Saved state %s\n", fname);
}

void gbcc_load_state(struct gbc *gbc)
{
	uint8_t *rom = gbc->cart.rom;
	uint8_t *ram = gbc->cart.ram;
	uint8_t wram_bank;
	uint8_t vram_bank;
	const char *name = gbc->cart.filename;

	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.s%d", tmp, gbc->load_state) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (!sav) {
		gbcc_log_error("Error opening %s: %s\n", fname, strerror(errno));
		gbc->save_state = 0;
		gbc->load_state = 0;
		return;
	}
	fread(gbc, sizeof(struct gbc), 1, sav);
	/* FIXME: Thread-unsafe, screen could try to read from here while the
	 * pointer is still invalid */
	gbc->ppu.screen.gbc = gbc->ppu.screen.buffer_0;
	gbc->ppu.screen.sdl = gbc->ppu.screen.buffer_1;

	gbc->cart.rom = rom;
	gbc->cart.ram = ram;
	gbc->cart.filename = name;
	
	if (gbc->cart.ram_size > 0) {
		fread(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	}

	switch (gbc->mode) {
		case DMG:
			wram_bank = 1;
			vram_bank = 0;
			break;
		case GBC:
			wram_bank = gbcc_memory_read(gbc, SVBK, false) & 0x07u;
			vram_bank = gbcc_memory_read(gbc, VBK, false) & 0x01u;
			break;
	}

	gbc->memory.rom0 = gbc->cart.rom;
	gbc->memory.romx = gbc->cart.rom + gbc->cart.mbc.romx_bank * ROMX_SIZE;
	gbc->memory.vram = gbc->memory.vram_bank[vram_bank];
	gbc->memory.sram = gbc->cart.ram + gbc->cart.mbc.sram_bank * SRAM_SIZE;
	gbc->memory.wram0 = gbc->memory.wram_bank[0];
	gbc->memory.wramx = gbc->memory.wram_bank[wram_bank];
	gbc->memory.echo = gbc->memory.wram0;
	
	memset(&gbc->keys, 0, sizeof(gbc->keys));
	fclose(sav);
	gbcc_log_info("Loaded state %s\n", fname);
}

void strip_ext(char *fname)
{
	char *end = fname + strlen(fname);
	while (end > fname && *end != '.') {
		--end;
	}

	if (end > fname) {
		*end = '\0';
	}
}
