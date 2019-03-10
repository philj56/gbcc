#ifndef GBCC_H
#define GBCC_H

#include "apu.h"
#include "constants.h"
#include "ppu.h"
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
		uint8_t timer;
		bool target_state;
	} ime_timer;
	struct {
		bool set;
		bool no_interrupt;
		bool skip;
	} halt;
	struct {
		uint16_t source;
		uint16_t new_source;
		uint16_t timer;
		bool requested;
		bool running;
	} dma;
	struct {
		uint16_t source;
		uint16_t dest;
		uint16_t length;
		bool hblank;
	} hdma;
	struct {
		uint16_t addr;
		bool request;
	} interrupt;
	struct {
		uint16_t addr;
		uint8_t op1;
		uint8_t op2;
		uint8_t step;
		bool running;
		bool prefix_cb;
	} instruction;
	bool stop;
	uint16_t div_timer;
	bool tac_bit;
	uint8_t tima_reload;
	uint8_t clock;
	uint64_t debug_clock;
	uint8_t cpu_clock;
	struct {
		struct timespec current;
		struct timespec old;
	} real_time;
	bool quit;
	bool pause;
	bool interlace;
	int8_t save_state;
	int8_t load_state;
	bool double_speed;
	float turbo_speed;

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

	/* PPU */
	struct ppu ppu;

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
		struct gbcc_mbc {
			enum MBC type;
			bool sram_enable;
			uint8_t rom0_bank;
			uint16_t romx_bank;
			uint8_t sram_bank;
			uint8_t ramg;
			uint8_t romb0;
			uint8_t romb1;
			uint8_t ramb;
			bool mode;
			struct gbcc_rtc {
				uint8_t seconds;
				uint8_t minutes;
				uint8_t hours;
				uint8_t day_low;
				uint8_t day_high;
				uint8_t latch;
				uint8_t cur_reg;
				struct timespec base_time;
				bool mapped;
			} rtc;
		} mbc;
		char title[CART_TITLE_SIZE + 1];
	} cart;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);
void gbcc_free(struct gbc *gbc);

#endif /* GBCC_H */
