#ifndef GBCC_H
#define GBCC_H

#include "apu.h"
#include "constants.h"
#include "cpu.h"
#include "mbc.h"
#include "ppu.h"
#include "printer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

struct gbc {
	/* Core emulator areas */
	struct cpu cpu;
	struct apu apu;
	struct ppu ppu;

	enum CART_MODE mode;
	struct {
		uint16_t source;
		uint16_t dest;
		uint16_t length;
		uint16_t to_copy;
		bool hblank;
	} hdma;

	/* Memory map */
	struct {
		/* GBC areas */
		uint8_t *rom0;	/* Non-switchable ROM */
		uint8_t *romx;	/* Switchable ROM */
		uint8_t *vram;	/* VRAM (switchable in GBC mode) */
		uint8_t *sram;	/* Cartridge RAM */
		uint8_t *wram0;	/* Non-switchable Work RAM */
		uint8_t *wramx;	/* Work RAM (switchable in GBC mode) */
		uint8_t *echo;	/* Mirror of WRAM */
		uint8_t oam[OAM_SIZE];	/* Object Attribute Table */
		uint8_t unused[UNUSED_SIZE];	/* Complicated unused memory */
		uint8_t ioreg[IOREG_SIZE];	/* I/O Registers */
		uint8_t hram[HRAM_SIZE];	/* Internal CPU RAM */
		uint8_t iereg;	/* Interrupt enable flags */
		/* Emulator areas */
		uint8_t wram_bank[8][WRAM0_SIZE];	/* Actual location of WRAM */
		uint8_t vram_bank[2][VRAM_SIZE]; 	/* Actual location of VRAM */
	} memory;

	/* Cartridge data & flags */
	struct {
		const char *filename;
		uint8_t *rom;
		size_t rom_size;
		size_t rom_banks;
		uint8_t *ram;
		size_t ram_size;
		size_t ram_banks;
		bool battery;
		bool timer;
		bool rumble;
		char title[CART_TITLE_SIZE + 1];
		struct gbcc_mbc mbc;
	} cart;

	/* IO & peripherals */
	struct {
		bool a;
		bool b;
		bool start;
		bool select;
		struct {
			bool up;
			bool down;
			bool left;
			bool right;
		} dpad;
		bool turbo;
	} keys;

	struct printer printer;

	struct {
		uint8_t received;
		uint8_t current_bit;
		uint16_t divider;
		uint16_t clock;
	} link_cable;

	/* Settings */
	bool quit;
	bool pause;
	bool interlace;
	int8_t save_state;
	int8_t load_state;
	bool double_speed;
	float turbo_speed;
	bool hide_background;
	bool hide_window;
	bool hide_sprites;
	bool link_cable_loop;
	bool background_play;
	bool has_focus;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);
void gbcc_free(struct gbc *gbc);

#endif /* GBCC_H */
