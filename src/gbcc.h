#ifndef GBCC_H
#define GBCC_H

#include "gbcc_apu.h"
#include "gbcc_constants.h"
#include "gbcc_palettes.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* TODO: Find a better way to do this */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

struct line_buffer {
	uint32_t colour[GBC_SCREEN_WIDTH];
	uint8_t attr[GBC_SCREEN_WIDTH];
};

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
	struct palette palette;
	uint8_t opcode;
	bool ime;
	struct {
		bool set;
		bool no_interrupt;
		uint8_t skip;
	} halt;
	struct {
		uint16_t source;
		uint16_t new_source;
		uint16_t timer;
		bool requested;
	} dma;
	struct {
		uint16_t source;
		uint16_t dest;
		uint16_t length;
		bool hblank;
	} hdma;
	struct {
		uint16_t addr;
		uint8_t delay;
		bool request;
	} rst;
	bool stop;
	uint8_t instruction_timer;
	uint8_t div_timer;
	uint64_t clock;
	uint64_t ppu_clock;
	struct {
		struct timespec current;
		struct timespec old;
	} real_time;
	bool quit;
	bool initialised;
	int8_t save_state;
	int8_t load_state;
	int8_t speed_mult;

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
		uint8_t bgp[64]; 	/* 8 x 8-byte sprites */
		uint8_t obp[64]; 	/* 8 x 8-byte sprites */
		/* Emulator areas */
		uint8_t wram_bank[8][WRAM0_SIZE];	/* Actual location of WRAM */
		uint8_t vram_bank[2][VRAM_SIZE]; 	/* Actual location of VRAM */
		uint32_t screen_buffer_0[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		uint32_t screen_buffer_1[GBC_SCREEN_HEIGHT * GBC_SCREEN_WIDTH];
		struct line_buffer background_buffer;
		struct line_buffer window_buffer;
		struct line_buffer sprite_buffer;
		uint32_t *gbc_screen;
		uint32_t *sdl_screen;
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
	
	/* APU */
	struct apu apu;

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
		bool padding; /* TODO: remove */
		struct {
			enum MBC type;
			enum BANK_MODE bank_mode;
			bool sram_enable;
			uint16_t romx_bank;
			uint8_t sram_bank;
			bool padding;
		} mbc;
		char title[CART_TITLE_SIZE];
	} cart;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);
void gbcc_free(struct gbc *gbc);

#endif /* GBCC_H */
