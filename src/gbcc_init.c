#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_bit_utils.h"
#include "gbcc_constants.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_palettes.h"
#include "gbcc_save.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const uint8_t nintendo_logo[CART_LOGO_SIZE] = {
	0xCEu, 0xEDu, 0x66u, 0x66u, 0xCCu, 0x0Du, 0x00u, 0x0Bu,
	0x03u, 0x73u, 0x00u, 0x83u, 0x00u, 0x0Cu, 0x00u, 0x0Du,
	0x00u, 0x08u, 0x11u, 0x1Fu, 0x88u, 0x89u, 0x00u, 0x0Eu,
	0xDCu, 0xCCu, 0x6Eu, 0xE6u, 0xDDu, 0xDDu, 0xD9u, 0x99u,
	0xBBu, 0xBBu, 0x67u, 0x63u, 0x6Eu, 0x0Eu, 0xECu, 0xCCu,
	0xDDu, 0xDCu, 0x99u, 0x9Fu, 0xBBu, 0xB9u, 0x33u, 0x3Eu
};

static void load_rom(struct gbc *gbc, const char *filename);
static void parse_header(struct gbc *gbc);
static void verify_cartridge(struct gbc *gbc);
static void load_title(struct gbc *gbc);
static void init_mode(struct gbc *gbc);
static void print_licensee_code(struct gbc *gbc);
static void get_cartridge_hardware(struct gbc *gbc);
static void init_ram(struct gbc *gbc);
static void print_destination_code(struct gbc *gbc);
static void init_registers(struct gbc *gbc);
static void init_mmap(struct gbc *gbc);
static void init_ioreg(struct gbc *gbc);

void gbcc_initialise(struct gbc *gbc, const char *filename)
{
	*gbc = (const struct gbc){{{{0}}}};
	gbc->cart.filename = filename;
	gbc->cart.mbc.type = NONE;
	gbc->cart.mbc.romx_bank = 0x01u;
	gbc->cart.mbc.sram_bank = 0x00u;
	gbc->cart.mbc.bank_mode = ROM;
	gbc->ime = true;
	gbc->clock = GBC_LCD_MODE_PERIOD;
	gbc->ppu_clock = gbc->clock;
	gbc->palette = gbcc_get_palette("default");
	gbc->memory.gbc_screen = gbc->memory.screen_buffer_0;
	gbc->memory.sdl_screen = gbc->memory.screen_buffer_1;
	timespec_get(&gbc->real_time.current, TIME_UTC);
	load_rom(gbc, filename);
	parse_header(gbc);
	init_mmap(gbc);
	init_ioreg(gbc);
	gbcc_apu_init(gbc);
}

void gbcc_free(struct gbc *gbc)
{
	free(gbc->cart.rom);
	if (gbc->cart.ram_size > 0) {
		free(gbc->cart.ram);
	}
}

void load_rom(struct gbc *gbc, const char *filename)
{
	gbcc_log_info("Loading %s...\n", filename);
	size_t read;
	uint8_t rom_size_flag;

	FILE *rom = fopen(filename, "rbe");
	if (rom == NULL)
	{
		gbcc_log_error("Error opening file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fseek(rom, CART_ROM_SIZE_FLAG, SEEK_SET);
	if (ferror(rom)) {
		gbcc_log_error("Error seeking in file: %s\n", filename);
		fclose(rom);
		exit(EXIT_FAILURE);
	}

	read = fread(&rom_size_flag, 1, 1, rom);
	if (read == 0) {
		gbcc_log_error("Error reading ROM size from: %s\n", filename);
		fclose(rom);
		exit(EXIT_FAILURE);
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
		gbcc_log_error("Unknown ROM size flag: %u\n", rom_size_flag);
		fclose(rom);
		exit(EXIT_FAILURE);
	}
	gbc->cart.rom_banks = gbc->cart.rom_size / ROM0_SIZE;
	gbcc_log_info("\tCartridge size: 0x%X bytes (%u banks)\n", gbc->cart.rom_size, gbc->cart.rom_banks);

	gbc->cart.rom = (uint8_t *) calloc(gbc->cart.rom_size, 1);
	if (gbc->cart.rom == NULL) {
		gbcc_log_error("Error allocating ROM.\n");
		fclose(rom);
		exit(EXIT_FAILURE);
	}


	if (fseek(rom, 0, SEEK_SET) != 0) {
		gbcc_log_error("Error seeking in file: %s\n", filename);
		fclose(rom);
		exit(EXIT_FAILURE);
	}

	read = fread(gbc->cart.rom, 1, gbc->cart.rom_size, rom);
	if (read == 0) {
		gbcc_log_error("Error reading from file: %s\n", filename);
		fclose(rom);
		exit(EXIT_FAILURE);
	}

	fclose(rom);
	gbcc_log_info("\tROM loaded.\n");
}

void parse_header(struct gbc *gbc)
{
	gbcc_log_info("Parsing header...\n");
	verify_cartridge(gbc);
	load_title(gbc);
	print_licensee_code(gbc);
	init_mode(gbc);
	get_cartridge_hardware(gbc);
	init_ram(gbc);
	print_destination_code(gbc);
	init_registers(gbc);
	gbcc_log_info("\tHeader parsed.\n");
}

void verify_cartridge(struct gbc *gbc)
{
	for (size_t i = CART_LOGO_START; i < CART_LOGO_GBC_CHECK_END; i++) {
		if (gbc->cart.rom[i] != nintendo_logo[i - CART_LOGO_START]) {
			gbcc_log_error("Cartridge logo check failed on byte %04X\n", i);
			exit(EXIT_FAILURE);
		}
	}
	gbcc_log_info("\tCartridge logo check passed.\n");
	uint16_t sum = 0;
	for (size_t i = CART_HEADER_CHECKSUM_START; i < CART_HEADER_CHECKSUM_START + CART_HEADER_CHECKSUM_SIZE + 1; i++) {
		sum += gbc->cart.rom[i];
	}
	sum = low_byte(sum + 25u);
	if (sum) {
		gbcc_log_error("Cartridge checksum failed with value %04X.\n", sum);
		exit(EXIT_FAILURE);
	}	
	gbcc_log_info("\tCartridge checksum passed.\n");
}

void load_title(struct gbc *gbc)
{
	for (size_t i = CART_TITLE_START; i < CART_TITLE_END; i++) {
		gbc->cart.title[i - CART_TITLE_START] = (char)gbc->cart.rom[i];
	}
	gbcc_log_info("\tTitle: %s\n", gbc->cart.title);
}

void print_licensee_code(struct gbc *gbc)
{
	uint8_t old_code = gbc->cart.rom[CART_OLD_LICENSEE_CODE];
	char buffer[CART_NEW_LICENSEE_CODE_SIZE + 21];
	size_t len = sizeof(buffer) / sizeof(buffer[0]);
	if (old_code != 0x33u) {
		snprintf(buffer, len, "\tLicensee code: 0x%02X\n", old_code);
	} else {
		unsigned char new_code[CART_NEW_LICENSEE_CODE_SIZE + 1];
		for (size_t i = CART_NEW_LICENSEE_CODE_START; i < CART_NEW_LICENSEE_CODE_END; i++) {
			new_code[i - CART_NEW_LICENSEE_CODE_START] = gbc->cart.rom[i];
		}
		new_code[CART_NEW_LICENSEE_CODE_SIZE] = '\0';
		snprintf(buffer, len, "\tLicensee code: %s\n", new_code);
	}
	gbcc_log_info(buffer);
}

void init_mode(struct gbc *gbc)
{
	uint8_t mode_flag;

	mode_flag = gbc->cart.rom[CART_GBC_FLAG];
	if (mode_flag == 0x80u || mode_flag == 0xC0u) {
		gbc->mode = GBC;
	} else {
		gbc->mode = DMG;
	}
	if ((mode_flag & (1u << 6u)) && (mode_flag & (3u << 2u))) {
		/* TODO: Handle this */
	}
	switch (gbc->mode) {
		case DMG:
			gbcc_log_info("\tMode: DMG\n");
			break;
		case GBC:
			gbcc_log_info("\tMode: GBC\n");
			break;
	}
}

void get_cartridge_hardware(struct gbc *gbc)
{
	switch (gbc->cart.rom[CART_TYPE]) {
		case 0x00u:	/* ROM ONLY */
			gbc->cart.mbc.type = NONE;
			gbcc_log_info("\tHardware: ROM only\n");
			break;
		case 0x01u:	/* MBC1 */
			gbc->cart.mbc.type = MBC1;
			gbcc_log_info("\tHardware: MBC1\n");
			break;
		case 0x02u:	/* MBC1 + RAM */
			gbc->cart.mbc.type = MBC1;
			gbcc_log_info("\tHardware: MBC1 + RAM\n");
			break;
		case 0x03u:	/* MBC1 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC1;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC1 + RAM + Battery\n");
			break;
		case 0x05u:	/* MBC2 */
			gbc->cart.mbc.type = MBC2;
			gbcc_log_info("\tHardware: MBC2\n");
			break;
		case 0x06u:	/* MBC2 + BATTERY */
			gbc->cart.mbc.type = MBC2;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC2 + Battery\n");
			break;
		case 0x08u:	/* ROM + RAM */
			gbcc_log_info("\tHardware: ROM + RAM\n");
			break;
		case 0x09u:	/* ROM + RAM + BATTERY */
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: ROM + RAM + Battery\n");
			break;
		case 0x0Bu:	/* MMM01 */
			gbc->cart.mbc.type = MMM01;
			gbcc_log_info("\tHardware: ROM + RAM + Battery\n");
			break;
		case 0x0Cu:	/* MMM01 + RAM */
			gbc->cart.mbc.type = MMM01;
			gbcc_log_info("\tHardware: MMM01\n");
			break;
		case 0x0Du:	/* MMM01 + RAM + BATTERY */
			gbc->cart.mbc.type = MMM01;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MMM01 + Battery\n");
			break;
		case 0x0Fu:	/* MBC3 + TIMER + BATTERY */
			gbc->cart.mbc.type = MBC3;
			gbc->cart.timer = true;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC3 + Timer + Battery\n");
			break;
		case 0x10u:	/* MBC3 + TIMER + RAM + BATTERY */
			gbc->cart.timer = true;
			gbc->cart.battery = true;
			gbc->cart.mbc.type = MBC3;
			gbcc_log_info("\tHardware: MBC3 + Timer + RAM + Battery\n");
			break;
		case 0x11u:	/* MBC3 */
			gbc->cart.mbc.type = MBC3;
			gbcc_log_info("\tHardware: MBC3\n");
			break;
		case 0x12u:	/* MBC3 + RAM */
			gbc->cart.mbc.type = MBC3;
			gbcc_log_info("\tHardware: MBC3 + RAM\n");
			break;
		case 0x13u:	/* MBC3 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC3;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC3 + RAM + Battery\n");
			break;
		case 0x19u:	/* MBC5 */
			gbc->cart.mbc.type = MBC5;
			gbcc_log_info("\tHardware: MBC5\n");
			break;
		case 0x1Au:	/* MBC5 + RAM */
			gbc->cart.mbc.type = MBC5;
			gbcc_log_info("\tHardware: MBC5 + RAM\n");
			break;
		case 0x1Bu:	/* MBC5 + RAM + BATTERY */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC5 + RAM + Battery\n");
			break;
		case 0x1Cu:	/* MBC5 + RUMBLE */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbcc_log_info("\tHardware: MBC5 + Rumble\n");
			break;
		case 0x1Du:	/* MBC5 + RUMBLE + RAM */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbcc_log_info("\tHardware: MBC5 + Rumble + RAM\n");
			break;
		case 0x1Eu:	/* MBC5 + RUMBLE + RAM + BATTERY */
			gbc->cart.mbc.type = MBC5;
			gbc->cart.rumble = true;
			gbc->cart.battery = true;
			gbcc_log_info("\tHardware: MBC5 + Rumble + RAM + Battery\n");
			break;
		default:
			gbcc_log_error("Unrecognised hardware %02X\n", gbc->cart.rom[CART_TYPE]);
			/* TODO: Handle Pocket Camera, TAMA5, HuC3 & HuC1 */
			break;
	}
}

void init_ram(struct gbc *gbc)
{
	uint8_t ram_size_flag;

	ram_size_flag = gbc->cart.rom[CART_RAM_SIZE_FLAG];

	switch (ram_size_flag) {
		case 0x00u:
			if (gbc->cart.mbc.type == MBC2) {
				gbc->cart.ram_size = 0x0200u;
			}
			break;
		case 0x01u:
			if (gbc->cart.mbc.type == MBC5) {
				gbc->cart.ram_size = 0x2000u;
			} else {
				gbc->cart.ram_size = 0x0800u;
			}
			break;
		case 0x02u:
			if (gbc->cart.mbc.type == MBC5) {
				gbc->cart.ram_size = 0x8000u;
			} else {
				gbc->cart.ram_size = 0x2000u;
			}
			break;
		case 0x03u:
			if (gbc->cart.mbc.type == MBC5) {
				gbc->cart.ram_size = 0x20000u;
			} else {
				gbc->cart.ram_size = 0x8000u;
			}
			break;
		default:
			gbcc_log_error("Unknown ram size flag: %u\n", ram_size_flag);
			exit(EXIT_FAILURE);
	}

	/*if (gbc->cart.mbc.type == NONE) {
		gbc->cart.ram_size = 0x0200u;
	}*/

	if (gbc->cart.ram_size > 0) {
		gbc->cart.ram_banks = gbc->cart.ram_size / SRAM_SIZE;
		gbcc_log_info("\tCartridge RAM: 0x%0X bytes (%u banks)\n", gbc->cart.ram_size, gbc->cart.ram_banks);
		gbc->cart.ram = (uint8_t *) calloc(gbc->cart.ram_size, 1);
		if (gbc->cart.ram == NULL) {
			gbcc_log_error("Error allocating RAM.\n");
			exit(EXIT_FAILURE);
		}
	}
}

void print_destination_code(struct gbc *gbc)
{
	uint8_t dest_code = gbc->cart.rom[CART_DESTINATION_CODE];
	switch (dest_code) {
		case 0x00u:
			gbcc_log_info("\tRegion: Japan\n");
			break;
		case 0x01u:
			gbcc_log_info("\tRegion: Non-Japanese\n");
			break;
		default:
			gbcc_log_error("Unrecognised region code: 0x%02X\n", dest_code);
			exit(EXIT_FAILURE);
	}
}

void init_registers(struct gbc *gbc) {
	switch (gbc->mode) {
		case DMG:
			gbc->reg.af = 0x01B0u;
			gbc->reg.bc = 0x0013u;
			gbc->reg.de = 0x00D8u;
			gbc->reg.hl = 0x014Du;
			gbc->reg.sp = 0xFFFEu;
			gbc->reg.pc = 0x0100u;
			break;
		case GBC:
			gbc->reg.af = 0x1180u;
			gbc->reg.bc = 0x0000u;
			gbc->reg.de = 0xFF56u;
			gbc->reg.hl = 0x000Du;
			gbc->reg.sp = 0xFFFEu;
			gbc->reg.pc = 0x0100u;
			break;
	}
}

void init_mmap(struct gbc *gbc)
{
	gbc->memory.rom0 = gbc->cart.rom;
	gbc->memory.romx = gbc->cart.rom + ROM0_SIZE;
	gbc->memory.vram = gbc->memory.vram_bank[0];
	gbc->memory.sram = gbc->cart.ram;
	gbc->memory.wram0 = gbc->memory.wram_bank[0];
	gbc->memory.wramx = gbc->memory.wram_bank[1];
	gbc->memory.echo = gbc->memory.wram0;
}

void init_ioreg(struct gbc *gbc)
{
	/* Values taken from the pandocs and mooneye tests*/
	gbc->memory.ioreg[JOYP - IOREG_START] = 0xCFu;
	gbc->memory.ioreg[SC - IOREG_START] = 0x7Eu;
	gbc->memory.ioreg[DIV - IOREG_START] = 0xACu;
	gbc->memory.ioreg[TIMA - IOREG_START] = 0x00u;
	gbc->memory.ioreg[TMA - IOREG_START] = 0x00u;
	gbc->memory.ioreg[TAC - IOREG_START] = 0x00u;
	gbc->memory.ioreg[IF - IOREG_START] = 0x01u;
	gbc->memory.ioreg[NR10 - IOREG_START] = 0x80u;
	gbc->memory.ioreg[NR11 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR12 - IOREG_START] = 0xF3u;
	gbc->memory.ioreg[NR14 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR21 - IOREG_START] = 0x3Fu;
	gbc->memory.ioreg[NR22 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR24 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR30 - IOREG_START] = 0x7Fu;
	gbc->memory.ioreg[NR31 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[NR32 - IOREG_START] = 0x9Fu;
	gbc->memory.ioreg[NR33 - IOREG_START] = 0xBFu;
	gbc->memory.ioreg[NR41 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[NR42 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR43 - IOREG_START] = 0x00u;
	gbc->memory.ioreg[NR50 - IOREG_START] = 0x77u;
	gbc->memory.ioreg[NR51 - IOREG_START] = 0xF3u;
	gbc->memory.ioreg[NR52 - IOREG_START] = 0xF1u;
	gbc->memory.ioreg[LCDC - IOREG_START] = 0x91u;
	gbc->memory.ioreg[SCY - IOREG_START] = 0x00u;
	gbc->memory.ioreg[SCX - IOREG_START] = 0x00u;
	gbc->memory.ioreg[LY - IOREG_START] = 0x00u;
	gbc->memory.ioreg[LYC - IOREG_START] = 0x00u;
	gbc->memory.ioreg[BGP - IOREG_START] = 0xFCu;
	gbc->memory.ioreg[OBP0 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[OBP1 - IOREG_START] = 0xFFu;
	gbc->memory.ioreg[WY - IOREG_START] = 0x00u;
	gbc->memory.ioreg[WX - IOREG_START] = 0x00u;
	gbc->memory.ioreg[HDMA5 - IOREG_START] = 0x80u;
	gbc->memory.ioreg[SVBK - IOREG_START] = 0x01u;
	gbc->memory.iereg = 0x00u;
	for (size_t i = 0; i < sizeof(gbc->memory.bgp)/sizeof(gbc->memory.bgp[0]); i++) {
		gbc->memory.bgp[i] = 0xFFu;
	}
}
