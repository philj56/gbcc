#ifndef GBCC_H
#define GBCC_H

#include "gbcc_constants.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* TODO: Find a better way to do this */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

struct gbc {
	/* Registers */
	struct {
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t f;
				uint8_t a;
				#else
				uint8_t a;
				uint8_t f;
				#endif
			};
			uint16_t af;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t c;
				uint8_t b;
				#else
				uint8_t b;
				uint8_t c;
				#endif
			};
			uint16_t bc;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t e;
				uint8_t d;
				#else
				uint8_t d;
				uint8_t e;
				#endif
			};
			uint16_t de;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t l;
				uint8_t h;
				#else
				uint8_t h;
				uint8_t l;
				#endif
			};
			uint16_t hl;
		};
		uint16_t sp;
		uint16_t pc;
	} reg;

	/* Non-Register state data */
	enum CART_MODE mode;
	uint8_t opcode;
	bool ime;
	struct {
		bool set;
		bool no_interrupt;
		uint8_t skip;
	} halt;
	struct {
		uint16_t source;
		uint16_t timer;
	} dma;
	bool stop;
	uint8_t instruction_timer;
	uint8_t div_timer;
	uint64_t clock;
	struct {
		struct timespec current;
		struct timespec old;
	} real_time;
	bool quit;

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
		uint8_t *emu_wram;	/* Actual location of WRAM */
		uint8_t *emu_vram;	/* Actual location of VRAM */
		uint32_t screen[GBC_SCREEN_HEIGHT][GBC_SCREEN_WIDTH];
	} memory;

	/* Current key states */
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

	/* Cartridge data & flags */
	struct {
		const char *filename;
		uint8_t *rom;
		size_t rom_size;
		uint8_t *ram;
		size_t ram_size;
		bool battery;
		bool timer;
		bool rumble;
		bool padding; /* TODO: remove */
		struct {
			enum MBC type;
			enum BANK_MODE bank_mode;
			bool sram_enable;
			uint8_t romx_bank;
			uint8_t sram_bank;
			bool padding;
		} mbc;
		char title[CART_TITLE_SIZE];
	} cart;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);
void gbcc_free(struct gbc *gbc);

#endif /* GBCC_H */
