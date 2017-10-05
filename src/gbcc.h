#ifndef GBCC_H
#define GBCC_H

#include <stdint.h>
#include <stdbool.h>
#include "gbcc_constants.h"

struct gbc {
	/* Registers */
	struct {
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				union {
					struct {
						uint8_t zf : 1;
						uint8_t nf : 1;
						uint8_t hf : 1;
						uint8_t cf : 1;
						uint8_t    : 4;
					};
					uint8_t f;
				};
				uint8_t a;
				#else
				uint8_t a;
				union {
					struct {
						uint8_t zf : 1;
						uint8_t nf : 1;
						uint8_t hf : 1;
						uint8_t cf : 1;
						uint8_t    : 4;
					};
					uint8_t f;
				};
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
		uint8_t *emu_vram;	/* Actual location of WRAM */
	} memory;

	/* Non-Register state data */
	enum CART_MODE mode;
	uint8_t opcode;

	/* Cartridge data & flags */
	struct {
		uint8_t *rom;
		size_t rom_size;
		uint8_t *ram;
		size_t ram_size;
		enum MBC mbc;
		bool battery;
		bool timer;
		bool rumble;
	} cart;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);

#endif /* GBCC_H */
