#include "gbcc.h"
#include "gbcc_constants.h"
#include "gbcc_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static void gbcc_load_rom(struct gbc *gbc, const char *filename);
static void gbcc_parse_header(struct gbc *gbc);
static void gbcc_get_cartridge_hardware(struct gbc *gbc);
static void gbcc_init_ram(struct gbc *gbc);
static void gbcc_init_mode(struct gbc *gbc);
static void gbcc_init_mmap(struct gbc *gbc);
static void gbcc_init_registers(struct gbc *gbc);

void gbcc_initialise(struct gbc *gbc, const char *filename)
{
	gbc->cart.rom = NULL;
	gbc->cart.ram = NULL;
	gbc->cart.mbc = NONE;
	gbc->cart.battery = false;
	gbc->cart.timer = false;
	gbc->cart.rumble = false;
	gbc->cart.ram_size = 0;
	gbc->mode = DMG;
	gbc->ime = true;
	gbc->halt.set = false;
	gbc->halt.no_interrupt = false;
	gbc->halt.skip = 0;
	gbc->stop = false;
	gbc->instruction_timer = 0;
	gbc->clock = 0;
	gbcc_load_rom(gbc, filename);
	gbcc_parse_header(gbc);
	gbcc_init_mode(gbc);
	gbcc_init_mmap(gbc);
	gbcc_init_registers(gbc);
}

void gbcc_free(struct gbc *gbc)
{
	free(gbc->cart.rom);
	if (gbc->cart.ram_size > 0) {
		free(gbc->cart.ram);
	}
	free(gbc->memory.emu_wram);
	free(gbc->memory.emu_vram);
}

static void gbcc_load_rom(struct gbc *gbc, const char *filename)
{
	size_t read;
	uint8_t rom_size_flag;

	FILE *rom = fopen(filename, "rbe");
	if (rom == NULL)
	{
		fprintf(stderr, "Error opening file: %s.\n", filename);
		exit(1);
	}

	fseek(rom, CART_ROM_SIZE, SEEK_SET);
	if (ferror(rom)) {
		fprintf(stderr, "Error seeking in file: %s.\n", filename);
		fclose(rom);
		exit(1);
	}

	read = fread(&rom_size_flag, 1, 1, rom);
	if (read == 0) {
		fprintf(stderr, "Error reading ROM size from: %s.\n", filename);
		fclose(rom);
		exit(1);
	}

	if (rom_size_flag < 0x08u) {
		gbc->cart.rom_size = 0x8000u << rom_size_flag; /* 32KB shl N */
	} else if (rom_size_flag == 0x52u) {
		gbc->cart.rom_size = 0x120000u;
	} else if (rom_size_flag == 0x53u) {
		gbc->cart.rom_size = 0x140000u;
	} else if (rom_size_flag == 0x54u) {
		gbc->cart.rom_size = 0x180000u;
	} else {
		fprintf(stderr, "Unknown ROM size flag: %u.\n", rom_size_flag);
		fclose(rom);
		exit(1);
	}

	gbc->cart.rom = (uint8_t *) calloc(gbc->cart.rom_size, 1);
	if (gbc->cart.rom == NULL) {
		fprintf(stderr, "Error allocating ROM.\n");
		fclose(rom);
		exit(1);
	}


	if (fseek(rom, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Error seeking in file: %s.\n", filename);
		fclose(rom);
		exit(1);
	}

	read = fread(gbc->cart.rom, 1, gbc->cart.rom_size, rom);
	if (read == 0) {
		fprintf(stderr, "Error reading from file: %s.\n", filename);
		fclose(rom);
		exit(1);
	}

	fclose(rom);
}

static void gbcc_parse_header(struct gbc *gbc)
{
	gbcc_get_cartridge_hardware(gbc);
	gbcc_init_ram(gbc);
}

static void gbcc_get_cartridge_hardware(struct gbc *gbc)
{
	switch (gbc->cart.rom[CART_TYPE]) {
		case 0x00u:	/* ROM ONLY */
			gbc->cart.mbc = NONE;
			break;
		case 0x01u:	/* MBC1 */
		case 0x02u:	/* MBC1 + RAM */
			gbc->cart.mbc = MBC1;
			break;
		case 0x03u:	/* MBC1 + RAM + BATTERY */
			gbc->cart.mbc = MBC1;
			gbc->cart.battery = true;
			break;
		case 0x05u:	/* MBC2 */
			gbc->cart.mbc = MBC2;
			break;
		case 0x06u:	/* MBC2 + BATTERY */
			gbc->cart.mbc = MBC2;
			gbc->cart.battery = true;
			break;
		case 0x08u:	/* ROM + RAM */
			break;
		case 0x09u:	/* ROM + RAM + BATTERY */
			gbc->cart.battery = true;
			break;
		case 0x0Bu:	/* MMM01 */
		case 0x0Cu:	/* MMM01 + RAM */
			gbc->cart.mbc = MMM01;
			break;
		case 0x0Du:	/* MMM01 + RAM + BATTERY */
			gbc->cart.mbc = MMM01;
			gbc->cart.battery = true;
			break;
		case 0x0Fu:	/* MBC3 + TIMER + BATTERY */
		case 0x10u:	/* MBC3 + TIMER + RAM + BATTERY */
			gbc->cart.timer = true;
			gbc->cart.battery = true;
			/* Fall through */
		case 0x11u:	/* MBC3 */
		case 0x12u:	/* MBC3 + RAM */
			gbc->cart.mbc = MBC3;
			break;
		case 0x13u:	/* MBC3 + RAM + BATTERY */
			gbc->cart.mbc = MBC3;
			gbc->cart.battery = true;
			break;
		case 0x15u:	/* MBC4 */
		case 0x16u:	/* MBC4 + RAM */
			gbc->cart.mbc = MBC4;
			break;
		case 0x17u:	/* MBC4 + RAM + BATTERY */
			gbc->cart.mbc = MBC4;
			gbc->cart.battery = true;
			break;
		case 0x19u:	/* MBC5 */
		case 0x1Au:	/* MBC5 + RAM */
			gbc->cart.mbc = MBC5;
			break;
		case 0x1Bu:	/* MBC5 + RAM + BATTERY */
			gbc->cart.mbc = MBC5;
			gbc->cart.battery = true;
			break;
		case 0x1Cu:	/* MBC5 + RUMBLE */
		case 0x1Du:	/* MBC5 + RUMBLE + RAM */
			gbc->cart.mbc = MBC5;
			gbc->cart.rumble = true;
			break;
		case 0x1Eu:	/* MBC5 + RUMBLE + RAM + BATTERY */
			gbc->cart.mbc = MBC5;
			gbc->cart.rumble = true;
			gbc->cart.battery = true;
			break;
	}
	printf("MBC%d\n", gbc->cart.mbc);
}

static void gbcc_init_ram(struct gbc *gbc)
{
	uint8_t ram_size_flag;

	ram_size_flag = gbc->cart.rom[CART_RAM_SIZE];

	switch (ram_size_flag) {
		case 0x00u:
			if (gbc->cart.mbc == MBC2) {
				gbc->cart.ram_size = 0x200u;
			}
			break;
		case 0x01u:
			gbc->cart.ram_size = 0x0800u;
			break;
		case 0x02u:
			gbc->cart.ram_size = 0x2000u;
			break;
		case 0x03u:
			gbc->cart.ram_size = 0x8000u;
			break;
		default:
			fprintf(stderr, "Unknown ram size flag: %u.\n", ram_size_flag);
			exit(1);
	}

	if (gbc->cart.ram_size > 0) {
		gbc->cart.ram = (uint8_t *) calloc(CART_RAM_SIZE, 1);
		if (gbc->cart.ram == NULL) {
			fprintf(stderr, "Error allocating RAM.\n");
			exit(1);
		}
	}
}

static void gbcc_init_mode(struct gbc *gbc)
{
	uint8_t mode_flag;

	mode_flag = gbc->cart.rom[CART_GBC_FLAG];
	if (mode_flag == 0x80u || mode_flag == 0xC0u) {
		gbc->mode = GBC;
	}
	if ((mode_flag & (1u << 6u)) && (mode_flag & (3u << 2u))) {
		/* TODO: Handle this */
	}

	if (gbc->mode == DMG) {
		gbc->reg.af = 0x1180u;
		gbc->reg.bc = 0x0000u;
		gbc->reg.de = 0x0008u;
		gbc->reg.hl = 0x007Cu;
		gbc->reg.sp = 0xFFFEu;
		gbc->reg.pc = 0x0100u;
	} else if (gbc->mode == GBC) {
		gbc->reg.af = 0x1180u;
		gbc->reg.bc = 0x0000u;
		gbc->reg.de = 0xFF56u;
		gbc->reg.hl = 0x000Du;
		gbc->reg.sp = 0xFFFEu;
		gbc->reg.pc = 0x0100u;
	}
}

static void gbcc_init_mmap(struct gbc *gbc)
{
	unsigned int wram_mult;
	unsigned int vram_mult;
	if (gbc->mode == DMG) {
		wram_mult = 2;
		vram_mult = 1;
	} else if (gbc->mode == GBC) {
		wram_mult = 8;
		vram_mult = 2;
	} else {
		fprintf(stderr, "Unrecognised GBC mode.\n");
		exit(1);
	}

	gbc->memory.emu_wram = (uint8_t *) calloc(WRAM0_SIZE * wram_mult, 1);
	if (gbc->memory.emu_wram == NULL) {
		fprintf(stderr, "Error allocating WRAM.\n");
		exit(1);
	}

	gbc->memory.emu_vram = (uint8_t *) calloc(VRAM_SIZE * vram_mult, 1);
	if (gbc->memory.emu_vram == NULL) {
		fprintf(stderr, "Error allocating VRAM.\n");
		exit(1);
	}

	gbc->memory.rom0 = gbc->cart.rom;
	gbc->memory.romx = gbc->cart.rom + ROMX_START;
	gbc->memory.vram = gbc->memory.emu_vram;
	gbc->memory.sram = gbc->cart.ram;
	gbc->memory.wram0 = gbc->memory.emu_wram;
	gbc->memory.wramx = gbc->memory.emu_wram + WRAM0_SIZE;
	gbc->memory.echo = gbc->memory.wram0;
}

static void gbcc_init_registers(struct gbc *gbc)
{
	for (uint16_t i = IOREG_START; i < IOREG_START + IOREG_SIZE; i++) {
		gbcc_memory_write(gbc, i, 0);
	}
}
