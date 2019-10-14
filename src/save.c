#include "core.h"
#include "debug.h"
#include "memory.h"
#include "save.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 4096

static void strip_ext(char *fname);

void gbcc_save(struct gbcc *gbc)
{
	struct gbcc_core *core = &gbc->core;
	if (core->cart.ram_size == 0 && core->cart.mbc.type != MBC7) {
		return;
	}
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", core->cart.filename) >= MAX_NAME_LEN) {
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
	fwrite(core->cart.ram, 1, core->cart.ram_size, sav);
	if (core->cart.mbc.type == MBC7) {
		fwrite(core->cart.mbc.eeprom.data, 2, 128, sav);
	}
	if (core->cart.mbc.type == MBC3) {
		fprintf(sav, "\n%u:%u:%u:%u:%u:%u:%u:%ld:%ld\n",
				core->cart.mbc.rtc.seconds,
				core->cart.mbc.rtc.minutes,
				core->cart.mbc.rtc.hours,
				core->cart.mbc.rtc.day_low,
				core->cart.mbc.rtc.day_high,
				core->cart.mbc.rtc.latch,
				core->cart.mbc.rtc.cur_reg,
				core->cart.mbc.rtc.base_time.tv_sec,
				core->cart.mbc.rtc.base_time.tv_nsec
				);
	}
	fclose(sav);
	gbcc_log_info("Saved.\n");
}

void gbcc_load(struct gbcc *gbc)
{
	struct gbcc_core *core = &gbc->core;
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", core->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (sav == NULL) {
		for (size_t i = 0; i < core->cart.ram_size; i++) {
			core->cart.ram[i] = rand();
		}
		if (core->cart.mbc.type == MBC3) {
			clock_gettime(CLOCK_REALTIME, &core->cart.mbc.rtc.base_time);
		}
		return;
	}
	gbcc_log_info("Loading %s...\n", fname);
	fread(core->cart.ram, 1, core->cart.ram_size, sav);
	if (core->cart.mbc.type == MBC7) {
		fread(core->cart.mbc.eeprom.data, 2, 128, sav);
	}
	if (core->cart.mbc.type == MBC3) {
		int matched;
		matched = fscanf(sav, "\n%" SCNu8 ":%" SCNu8 ":%" SCNu8 ":%"
				SCNu8 ":%" SCNu8 ":%" SCNu8 ":%" SCNu8 ":%ld:%ld",
				&core->cart.mbc.rtc.seconds,
				&core->cart.mbc.rtc.minutes,
				&core->cart.mbc.rtc.hours,
				&core->cart.mbc.rtc.day_low,
				&core->cart.mbc.rtc.day_high,
				&core->cart.mbc.rtc.latch,
				&core->cart.mbc.rtc.cur_reg,
				&core->cart.mbc.rtc.base_time.tv_sec,
				&core->cart.mbc.rtc.base_time.tv_nsec
				);
		if (matched < 9) {
			gbcc_log_error("Couldn't read rtc data, "
					"resetting base time to now.\n");
			clock_gettime(CLOCK_REALTIME, &core->cart.mbc.rtc.base_time);
		}
	}
	fclose(sav);
}

void gbcc_save_state(struct gbcc *gbc)
{
	struct gbcc_core *core = &gbc->core;
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", core->cart.filename) >= MAX_NAME_LEN) {
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
	fwrite(gbc, sizeof(struct gbcc_core), 1, sav);
	if (core->cart.ram_size > 0) {
		fwrite(core->cart.ram, 1, core->cart.ram_size, sav);
	}
	fclose(sav);
	gbcc_log_info("Saved state %s\n", fname);
	gbc->save_state = 0;
	gbc->load_state = 0;
}

void gbcc_load_state(struct gbcc *gbc)
{
	struct gbcc_core *core = &gbc->core;
	uint8_t *rom = core->cart.rom;
	uint8_t *ram = core->cart.ram;
	uint8_t wram_bank;
	uint8_t vram_bank;
	const char *name = core->cart.filename;
	uint32_t *buf0 = core->ppu.screen.buffer_0;
	uint32_t *buf1 = core->ppu.screen.buffer_1;

	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", core->cart.filename) >= MAX_NAME_LEN) {
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
	fread(gbc, sizeof(struct gbcc_core), 1, sav);
	/* FIXME: Thread-unsafe, screen could try to read from here while the
	 * pointer is still invalid */
	core->ppu.screen.buffer_0 = buf0;
	core->ppu.screen.buffer_1 = buf1;
	core->ppu.screen.gbc = buf0;
	core->ppu.screen.sdl = buf1;

	core->cart.rom = rom;
	core->cart.ram = ram;
	core->cart.filename = name;
	
	if (core->cart.ram_size > 0) {
		fread(core->cart.ram, 1, core->cart.ram_size, sav);
	}
	fclose(sav);

	switch (core->mode) {
		case DMG:
			wram_bank = 1;
			vram_bank = 0;
			break;
		case GBC:
			wram_bank = gbcc_memory_read(core, SVBK, false) & 0x07u;
			vram_bank = gbcc_memory_read(core, VBK, false) & 0x01u;
			break;
	}

	core->memory.rom0 = core->cart.rom;
	core->memory.romx = core->cart.rom + core->cart.mbc.romx_bank * ROMX_SIZE;
	core->memory.vram = core->memory.vram_bank[vram_bank];
	core->memory.sram = core->cart.ram + core->cart.mbc.sram_bank * SRAM_SIZE;
	core->memory.wram0 = core->memory.wram_bank[0];
	core->memory.wramx = core->memory.wram_bank[wram_bank];
	core->memory.echo = core->memory.wram0;
	gbc->save_state = 0;
	gbc->load_state = 0;
	
	memset(&core->keys, 0, sizeof(core->keys));
	gbcc_log_info("Loaded state %s\n", fname);
}

bool gbcc_check_savestate(struct gbcc *gbc, int state)
{
	struct gbcc_core *core = &gbc->core;

	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", core->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.s%d", tmp, state) >= MAX_NAME_LEN) {
		gbcc_log_error("Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (!sav) {
		return false;
	}
	fclose(sav);
	return true;
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
