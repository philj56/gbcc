#include "gbcc.h"
#include "gbcc_debug.h"
#include "gbcc_save.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAX_NAME_LEN 4096

static void strip_ext(char *fname);

void gbcc_save(struct gbc *gbc)
{
	if (gbc->cart.ram_size == 0) {
		return;
	}
	if (!gbc->initialised) {
		gbcc_log(GBCC_LOG_INFO, "GBC not initialised, not saving.\n");
		return;
	}
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
	gbcc_log(GBCC_LOG_INFO, "Saving %s...\n", fname);
	sav = fopen(fname, "wbe");
	if (sav == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Can't open save file %s\n", fname);
		exit(EXIT_FAILURE);
	}
	fwrite(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	fclose(sav);
	gbcc_log(GBCC_LOG_INFO, "Saved.\n");
}

void gbcc_load(struct gbc *gbc)
{
	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.sav", tmp) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (sav == NULL) {
		return;
	}
	gbcc_log(GBCC_LOG_INFO, "Loading %s...\n", fname);
	fread(gbc->cart.ram, 1, gbc->cart.ram_size, sav);
	fclose(sav);
}

void gbcc_save_state(struct gbc *gbc)
{
	unsigned int wram_mult;
	switch (gbc->mode) {
		case DMG:
			wram_mult = 2;
			break;
		case GBC:
			wram_mult = 8;
			break;
	}

	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.s%d", tmp, gbc->save_state) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
	sav = fopen(fname, "wbe");
	if (!sav) {
		gbcc_log(GBCC_LOG_ERROR, "Error opening %s: %s\n", fname, strerror(errno));
		return;
	}
	gbc->save_state = 0;
	gbc->load_state = 0;
	fwrite(gbc, sizeof(struct gbc), 1, sav);
	fwrite(gbc->memory.emu_wram, WRAM0_SIZE * wram_mult, 1, sav);
	fclose(sav);
}

void gbcc_load_state(struct gbc *gbc)
{
	unsigned int wram_mult;
	uint8_t *emu_wram = gbc->memory.emu_wram;
	uint8_t *rom = gbc->cart.rom;
	uint8_t *ram = gbc->cart.ram;
	const char *name = gbc->cart.filename;
	switch (gbc->mode) {
		case DMG:
			wram_mult = 2;
			break;
		case GBC:
			wram_mult = 8;
			break;
	}

	char fname[MAX_NAME_LEN];
	char tmp[MAX_NAME_LEN];
	FILE *sav;
	if (snprintf(tmp, MAX_NAME_LEN, "%s", gbc->cart.filename) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", tmp);
	}
	strip_ext(tmp);
	if (snprintf(fname, MAX_NAME_LEN, "%s.s%d", tmp, gbc->load_state) >= MAX_NAME_LEN) {
		gbcc_log(GBCC_LOG_ERROR, "Filename %s too long\n", fname);
	}
	sav = fopen(fname, "rbe");
	if (!sav) {
		gbcc_log(GBCC_LOG_ERROR, "Error opening %s: %s\n", fname, strerror(errno));
		gbc->save_state = 0;
		gbc->load_state = 0;
		return;
	}
	fread(gbc, sizeof(struct gbc), 1, sav);
	gbc->memory.emu_wram = emu_wram;
	/* FIXME: Thread-unsafe, screen could try to read from here while the
	 * pointer is still invalid */
	gbc->memory.gbc_screen = gbc->memory.screen_buffer_0;
	gbc->memory.sdl_screen = gbc->memory.screen_buffer_1;
	fread(gbc->memory.emu_wram, WRAM0_SIZE * wram_mult, 1, sav);
	fclose(sav);

	gbc->cart.rom = rom;
	gbc->cart.ram = ram;
	gbc->cart.filename = name;

	gbc->memory.romx = gbc->cart.rom + (gbc->memory.romx - gbc->memory.rom0);
	gbc->memory.rom0 = gbc->cart.rom;
	gbc->memory.vram = gbc->memory.vram_bank0;
	gbc->memory.sram = gbc->cart.ram;
	gbc->memory.wramx = gbc->memory.emu_wram + (gbc->memory.wramx - gbc->memory.wram0);
	gbc->memory.wram0 = gbc->memory.emu_wram;
	gbc->memory.echo = gbc->memory.wram0;
	
	gbc->keys.a = false;
	gbc->keys.b = false;
	gbc->keys.start = false;
	gbc->keys.select = false;
	gbc->keys.dpad.up = false;
	gbc->keys.dpad.down = false;
	gbc->keys.dpad.left = false;
	gbc->keys.dpad.right = false;
	gbc->keys.turbo = false;
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
